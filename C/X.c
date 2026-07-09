#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

/* ==========================================================================
 * 1. ARCHITECTURE DEFINITIONS & LOCK-FREE RING BUFFERS
 * ========================================================================== */

#define RING_WORDS 4096 // Must be a power of 2 (16KB)
#define RING_MASK  (RING_WORDS - 1)

#define MAX_IN_FLIGHT_REQUESTS 256 // Must be a power of 2
#define REQ_MASK (MAX_IN_FLIGHT_REQUESTS - 1)

#define X11_PACKET_ERROR 0
#define X11_PACKET_REPLY 1
#define X11_PACKET_EVENT 2

// Single-Producer Single-Consumer Lock-Free Ring Buffer for Words (4 Bytes)
typedef struct {
    uint32_t storage[RING_WORDS];
    _Atomic size_t head;       // Consumer tracks this
    _Atomic size_t tail;       // Producer tracks this
    uint32_t sequence_counter; // Monotonically tracks full 32-bit sequence
    int fd;
} x11_spsc_ring_t;

// Dedicated Input Queue for reading stream data
typedef struct {
    uint32_t storage[RING_WORDS];
    size_t head_words;
    size_t tail_words;
    int fd;
} x11_in_queue_t;

typedef enum {
    REQ_TYPE_GET_GEOMETRY,
    REQ_TYPE_INTERN_ATOM
} x11_req_type_t;

typedef struct {
    x11_req_type_t type;
    void *user_data;
} x11_pending_req_t;

// Global lock-free context table for mapping replies to original request structures
static x11_pending_req_t pending_requests[MAX_IN_FLIGHT_REQUESTS];

/* ==========================================================================
 * 2. PACKED WIRE PROTOCOL STRUCTS
 * ========================================================================== */

typedef struct __attribute__((packed)) {
    uint8_t  byte_order;       // 0x6c ('l') = Little Endian
    uint8_t  pad0;
    uint16_t protocol_major;   // 11
    uint16_t protocol_minor;   // 0
    uint16_t auth_proto_len;   // Auth name string length
    uint16_t auth_data_len;    // Auth binary data length
    uint16_t pad1;
} x11_conn_request_t;

typedef struct __attribute__((packed)) {
    uint8_t  response_type;    // 1 for a normal reply
    uint8_t  pad0;
    uint16_t sequence_number;  // 16-bit wire sequence
    uint32_t reply_length;     // Extra trailing payload size in 4-byte words
    uint32_t core_data;        // 24 bytes of specific reply details
} x11_reply_generic_t;

typedef struct __attribute__((packed)) {
    uint8_t  response_type;    // 0 for errors
    uint8_t  error_code;       // e.g., BadWindow
    uint16_t sequence_number;
    uint32_t bad_value;
    uint16_t minor_opcode;
    uint8_t  major_opcode;
    uint8_t  pad;
} x11_error_t;

typedef struct __attribute__((packed)) {
    uint8_t  response_type;    // Event opcode (e.g., 12 = Expose)
    uint8_t  pad0;
    uint16_t sequence_number;
    uint32_t window;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t count;
    uint8_t  pad1;
} x11_event_expose_t;

typedef struct __attribute__((packed)) {
    uint8_t  req_opcode;       // 1 = CreateWindow
    uint8_t  depth;
    uint16_t length;           // Total words
    uint32_t wid;
    uint32_t parent;
    int16_t  x;
    int16_t  y;
    uint16_t width;
    uint16_t height;
    uint16_t border_width;
    uint16_t window_class;
    uint32_t visual;
    uint32_t value_mask;
} x11_create_window_req_t;

/* ==========================================================================
 * 3. CORE FLUSH & FILL ENGINE
 * ========================================================================== */

// Flushes the writing ring to the kernel using writev for wrapping rings
void x11_ring_flush_to_kernel(x11_spsc_ring_t *ring) {
    size_t t = atomic_load_explicit(&ring->tail, memory_order_relaxed);
    size_t h = atomic_load_explicit(&ring->head, memory_order_relaxed);
    if (h == t) return;

    size_t total_words = t - h;
    size_t h_idx = h & RING_MASK;
    size_t t_idx = t & RING_MASK;

    if (h_idx < t_idx) {
        write(ring->fd, &ring->storage[h_idx], total_words * 4);
    } else {
        struct iovec iov[2];
        iov[0].iov_base = &ring->storage[h_idx];
        iov[0].iov_len  = (RING_WORDS - h_idx) * 4;
        iov[1].iov_base = &ring->storage[0];
        iov[1].iov_len  = t_idx * 4;
        writev(ring->fd, iov, 2);
    }
    atomic_store_explicit(&ring->head, t, memory_order_release);
}

// Low-level reader straight into the input word queue
void x11_in_queue_fill(x11_in_queue_t *ctx, size_t words_needed) {
    size_t bytes_needed = words_needed * 4;
    uint8_t *dest_byte_ptr = (uint8_t*)(ctx->storage + ctx->tail_words);
    
    ssize_t n = read(ctx->fd, dest_byte_ptr, bytes_needed);
    if (n > 0) {
        ctx->tail_words += (n / 4);
    }
}

// 16-bit to 32-bit Sequence Unwrap Reconstruction
static inline uint32_t x11_unwrap_sequence(uint32_t local_expected, uint16_t wire_seq) {
    uint32_t low = local_expected & 0xFFFF;
    uint32_t high = local_expected & 0xFFFF0000;
    
    if (wire_seq < low) {
        if ((low - wire_seq) > 0x8000) high += 0x10000;
    } else {
        if ((wire_seq - low) > 0x8000) high -= 0x10000;
    }
    return high | wire_seq;
}

/* ==========================================================================
 * 4. WRITING & READING OPTIMIZED FUNCTIONAL MACROS
 * ========================================================================== */

// Writes any packed struct straight to the ring buffer via register streams
#define X11_RING_WRITE_WORDS(ring_ptr, struct_ptr, word_len) do {              \
    size_t __w = (word_len);                                                   \
    size_t __t = atomic_load_explicit(&(ring_ptr)->tail, memory_order_relaxed);\
    size_t __h = atomic_load_explicit(&(ring_ptr)->head, memory_order_acquire);\
                                                                               \
    if ((RING_WORDS - (__t - __h)) < __w) {                                    \
        x11_ring_flush_to_kernel(ring_ptr);                                    \
    }                                                                          \
                                                                               \
    uint32_t *__dst = (ring_ptr)->storage;                                     \
    const uint32_t *__src = (const uint32_t*)(struct_ptr);                     \
                                                                               \
    for (size_t __i = 0; __i < __w; __i++) {                                   \
        __dst[(__t + __i) & RING_MASK] = __src[__i];                           \
    }                                                                          \
                                                                               \
    atomic_store_explicit(&(ring_ptr)->tail, __t + __w, memory_order_release); \
    (ring_ptr)->sequence_counter++;                                            \
} while(0)

// Drops stream data right into pointers overlaying the live input queue
#define X11_QUEUE_READ_PACKET(in_ctx_ptr, out_ptr_var, out_packet_type_var) do { \
    size_t __avail = (in_ctx_ptr)->tail_words - (in_ctx_ptr)->head_words;      \
    if (__avail < 8) {                                                         \
        x11_in_queue_fill(in_ctx_ptr, 8 - __avail);                            \
    }                                                                          \
                                                                               \
    x11_reply_generic_t *__gen = (x11_reply_generic_t*)(                       \
        (in_ctx_ptr)->storage + (in_ctx_ptr)->head_words                       \
    );                                                                         \
                                                                               \
    uint8_t __type_byte = __gen->response_type;                                \
                                                                               \
    if (__type_byte == 0) {                                                    \
        (out_packet_type_var) = X11_PACKET_ERROR;                              \
        (out_ptr_var) = (void*)__gen;                                          \
        (in_ctx_ptr)->head_words += 8;                                         \
    } else if (__type_byte == 1) {                                             \
        (out_packet_type_var) = X11_PACKET_REPLY;                              \
        size_t __total_words = 8 + __gen->reply_length;                        \
        if (((in_ctx_ptr)->tail_words - (in_ctx_ptr)->head_words) < __total_words) { \
            x11_in_queue_fill(in_ctx_ptr, __total_words - __avail);             \
        }                                                                      \
        (out_ptr_var) = (void*)__gen;                                          \
        (in_ctx_ptr)->head_words += __total_words;                             \
    } else {                                                                   \
        (out_packet_type_var) = X11_PACKET_EVENT;                              \
        (out_ptr_var) = (void*)__gen;                                          \
        (in_ctx_ptr)->head_words += 8;                                         \
    }                                                                          \
} while(0)

/* ==========================================================================
 * 5. WORKER IMPLEMENTATION EXAMPLES (HANDSHAKE, ASYNC REQUEST, CONSUMER LOOP)
 * ========================================================================== */

// Dummy processing stubs to fulfill compilation signatures
void handle_app_reply(uint32_t seq, void* data, void* ctx) { (void)seq; (void)data; (void)ctx; }
void log_x11_error(uint32_t seq, uint8_t error_code, uint8_t major) { (void)seq; (void)error_code; (void)major; }
void redraw_window(uint32_t win, uint16_t w, uint16_t h) { (void)win; (void)w; (void)h; }

/**
 * Example A: The Initial Handshake Setup
 * Uses raw byte-casting layers safely overlaid directly on top of your 
 * fast word-aligned ring buffer grids.
 */
void x11_send_connection_setup(x11_spsc_ring_t *out, const char *auth_name, const uint8_t *auth_data) {
    x11_conn_request_t req = {
        .byte_order     = 0x6c, // Little Endian ('l') matching x86_64
        .protocol_major = 11,
        .protocol_minor = 0,
        .auth_proto_len = 18,   // String length of "MIT-MAGIC-COOKIE-1"
        .auth_data_len  = 16    // Binary byte length of the authentication cookie
    };

    // Grab the current lock-free tail pointer position from the output ring
    size_t t = atomic_load_explicit(&out->tail, memory_order_relaxed);
    uint8_t *dest = (uint8_t*)(&out->storage[t & RING_MASK]);
    
    // Copy the core protocol structures/strings directly into memory sequentially
    __builtin_memcpy(dest, &req, sizeof(req));   
    dest += sizeof(req);
    
    // Write and pad the auth protocol name ("MIT-MAGIC-COOKIE-1" is 18 bytes, pads out to 20)
    __builtin_memcpy(dest, auth_name, 18);        
    dest[18] = 0; 
    dest[19] = 0; 
    dest += 20; 
    
    // Write out the cookie (16 bytes is naturally 4-byte word-aligned)
    __builtin_memcpy(dest, auth_data, 16);        
    dest += 16;

    // Recalculate how many words we just consumed and advance the tail ring barrier
    size_t written_words = ((uint32_t*)dest) - (&out->storage[t & RING_MASK]);
    atomic_store_explicit(&out->tail, t + written_words, memory_order_release);
    
    // Flush the initialization block straight down the wire socket descriptor
    x11_ring_flush_to_kernel(out);
}

/**
 * Example B: Asynchronous User Window Allocation Request
 * Writes directly to the hardware ring buffer, tracking contexts via the
 * monotonic sequence counter without copying structs over local stack frames.
 */
void x11_send_create_window_async(x11_spsc_ring_t *out, uint32_t wid, uint32_t parent, void *user_context) {
    x11_create_window_req_t req = {
        .req_opcode   = 1,                                  // X11 CreateWindow Opcode
        .depth        = 24,                                 // 24-bit TrueColor depth
        .length       = sizeof(x11_create_window_req_t) / 4,// Evaluates to 8 words
        .wid          = wid,
        .parent       = parent,
        .window_class = 1                                   // 1 = InputOutput Class
    };

    // Cache user context metadata using a clean power-of-two bitwise mask lookup
    uint32_t seq = out->sequence_counter;
    pending_requests[seq & REQ_MASK].type = REQ_TYPE_GET_GEOMETRY;
    pending_requests[seq & REQ_MASK].user_data = user_context;

    // Pass the literal 8-word count into the streamlined register macro pipeline
    X11_RING_WRITE_WORDS(out, &req, req.length);
}

/**
 * Example C: The Dedicated Consumer Stream Reader Thread Loop
 * Evaluates incoming byte categories in an endless non-blocking loop, routing 
 * packets purely using memory-cached reference pointers inside the cache line.
 */
void x11_reader_thread_loop(x11_in_queue_t *in_queue, x11_spsc_ring_t *out_queue_ref) {
    void *packet_ptr;
    int packet_type;

    while (1) {
        // Read directly from the incoming streaming socket via zero-copy macro casting
        X11_QUEUE_READ_PACKET(in_queue, packet_ptr, packet_type);
        uint32_t out_seq = out_queue_ref->sequence_counter;

        switch (packet_type) {
            case X11_PACKET_REPLY: {
                x11_reply_generic_t *reply = (x11_reply_generic_t*)packet_ptr;
                
                // Reconstruct the 32-bit sequence structure from the 16-bit wire representation
                uint32_t full_seq = x11_unwrap_sequence(out_seq, reply->sequence_number);
                void *ctx = pending_requests[full_seq & REQ_MASK].user_data;
                
                handle_app_reply(full_seq, packet_ptr, ctx);
                break;
            }
            case X11_PACKET_ERROR: {
                x11_error_t *err = (x11_error_t*)packet_ptr;
                
                // Track exactly which request generated the failure via sequence unwrap
                uint32_t failed_seq = x11_unwrap_sequence(out_seq, err->sequence_number);
                log_x11_error(failed_seq, err->error_code, err->major_opcode);
                break;
            }
            case X11_PACKET_EVENT: {
                // Strip the server-side SendEvent bit (0x80) from the response byte
                uint8_t event_code = ((uint8_t*)packet_ptr) & 0x7F;
                
                if (event_code == 12) { // 12 = Core X11 Expose/Damage notification
                    x11_event_expose_t *expose = (x11_event_expose_t*)packet_ptr;
                    redraw_window(expose->window, expose->width, expose->height);
                }
                break;
            }
        }
    }
}
