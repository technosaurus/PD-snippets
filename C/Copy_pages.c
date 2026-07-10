#include <immintrin.h>  // For _enqcmd
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

// Standard DSA Memory Copy Descriptor (64 bytes)
typedef struct __attribute__((aligned(64))) {
    uint32_t pasid_priv;
    uint32_t flags;
    uint32_t reserved1;
    uint8_t  opcode;           // 0x01 = Memory Copy
    uint8_t  reserved2;
    uint64_t completion_addr;  // Pointer to individual page status if needed (or 0)
    uint64_t src_addr;
    uint64_t dst_addr;
    uint32_t transfer_size;    // 4096
    uint8_t  reserved3[20];    // Padding to 64 bytes
} dsa_memcpy_desc_t;

// Specialized DSA Batch Descriptor (64 bytes)
typedef struct __attribute__((aligned(64))) {
    uint32_t pasid_priv;
    uint32_t flags;            // Must include bit 2 (0x04) to request batch completion
    uint32_t reserved1;
    uint8_t  opcode;           // 0x02 = Batch Command
    uint8_t  reserved2;
    uint64_t completion_addr;  // Pointer to the master tracking status byte
    uint64_t desc_list_addr;   // Virtual pointer to our array of memcpy descriptors
    uint32_t desc_count;       // Number of descriptors in the batch (N pages)
    uint8_t  reserved3[20];    // Padding to 64 bytes
} dsa_batch_desc_t;

// The tracking record passed in by the user program
typedef struct __attribute__((aligned(32))) {
    volatile uint8_t status;   // 0 = Incomplete, 1 = Success, >1 = Error
    uint8_t  reserved[31];
} dsa_async_tracker_t;

/**
 * copy_pages_async: Submits a batch of N page copies to the hardware and returns instantly.
 * 
 * @param dst_desc_array An array of dsa_memcpy_desc_t allocated by the user (size >= num_pages)
 * @param dst_page       Pointer to destination buffer (4KB aligned)
 * @param src_page       Pointer to source buffer (4KB aligned)
 * @param num_pages      Number of pages to copy (Size of the batch)
 * @param tracker        Pointer to a user-allocated tracker to monitor completion later
 * @param mmio_portal    Pointer to the user-space mapped hardware work queue
 * 
 * @return int           0 on successful hardware submission; -1 if queue was full (retry needed)
 */
int copy_pages_async(dsa_memcpy_desc_t *dst_desc_array,
                     void *dst_page, const void *src_page, size_t num_pages, 
                     dsa_async_tracker_t *tracker, void *mmio_portal) 
{
    if (!dst_desc_array || !dst_page || !src_page || num_pages == 0 || !tracker || !mmio_portal) {
        return -2;
    }

    uint8_t *dst = (uint8_t *)dst_page;
    const uint8_t *src = (const uint8_t *)src_page;

    // 1. POPULATE THE DESCRIPTOR ARRAY IN MEMORY
    // The CPU prepares the work list. Since this is regular cacheable memory,
    // this step is blazing fast and doesn't talk to the hardware yet.
    for (size_t i = 0; i < num_pages; ++i) {
        dst_desc_array[i].opcode = 0x01; // Memcpy
        dst_desc_array[i].flags = 0;     // Individual pages don't need independent status
        dst_desc_array[i].transfer_size = PAGE_SIZE;
        dst_desc_array[i].src_addr = (uint64_t)(src + (i * PAGE_SIZE));
        dst_desc_array[i].dst_addr = (uint64_t)(dst + (i * PAGE_SIZE));
    }

    // 2. SET UP THE MASTER BATCH CONTROLLER DESCRIPTOR
    tracker->status = 0; // Initialize user's tracker to "Incomplete"

    dsa_batch_desc_t batch_desc = {0};
    batch_desc.opcode = 0x02;                             // Batch operation
    batch_desc.flags = 0x04;                              // Request completion record for the whole batch
    batch_desc.desc_list_addr = (uint64_t)dst_desc_array; // Where the list lives in RAM
    batch_desc.desc_count = (uint32_t)num_pages;          // How many pages to copy
    batch_desc.completion_addr = (uint64_t)&tracker->status;

    // 3. THE SINGLE SUBMISSION CALL
    // We try to hand the entire batch to the hardware in one shot.
    if (_enqcmd(mmio_portal, &batch_desc) != 0) {
        return -1; // Hardware queue was full. User must handle retry or do something else.
    }

    return 0; // Success! The hardware engine has accepted the entire page batch.
}

#if 0
dsa_memcpy_desc_t desc_storage[64]; // Space for a 64-page copy batch
dsa_async_tracker_t my_tracker;

// Launch the massive copy job across the system fabric
int result = copy_pages_async(desc_storage, mega_dst, mega_src, 64, &my_tracker, portal);

if (result == 0) {
    // === THE CPU IS NOW COMPLETELY FREE ===
    while (my_tracker.status == 0) {
        // Run database queries, process network packets, or compile shaders here!
        do_useful_application_work();
        
        // Optionally pass a throttle or break mechanism so you don't check too aggressively
    }
    
    // Once the loop breaks, the hardware has finished all 64 pages safely.
    if (my_tracker.status == 1) {
        process_copied_pages();
    }
}
#endif
