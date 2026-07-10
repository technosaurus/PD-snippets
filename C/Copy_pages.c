#include <immintrin.h>  // Contains _enqcmd, _umonitor, _umwait
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

// Intel DSA 64-byte Descriptor Structure Alignment 
// Strictly laid out according to the hardware specification
typedef struct __attribute__((aligned(64))) {
    uint32_t pasid_priv;       // PASID / privilege control flags
    uint32_t flags;            // Command control flags (e.g., request completion record)
    uint32_t reserved1;
    uint8_t  opcode;           // 0x01 = Memor_copy
    uint8_t  reserved2[3];
    uint64_t completion_addr;  // Virtual address pointing to the completion record struct
    uint64_t src_addr;         // Source Virtual Address (Page Aligned)
    uint64_t dst_addr;         // Destination Virtual Address (Page Aligned)
    uint32_t transfer_size;    // 4096 bytes for our page copy
    uint8_t  reserved3[28];    // Padding to ensure exactly 64-byte width
} dsa_desc_t;

// Hardware status structure updated directly by the DMA hardware
typedef struct __attribute__((aligned(32))) {
    volatile uint8_t status;   // 0 = Incomplete, 1 = Success, >1 = Error code
    uint8_t  reserved[31];     // Hardware requires padding padding
} dsa_comp_record_t;

/**
 * copy_pages: Offloads the copy of N pages to the hardware DMA engine
 * 
 * @param dst_page    Pointer to the destination buffer (Must be 4KB aligned)
 * @param src_page    Pointer to the source buffer (Must be 4KB aligned)
 * @param num_pages   Number of pages to copy
 * @param mmio_portal Pointer to the user-space mapped hardware work queue
 */
void copy_pages(void *dst_page, const void *src_page, size_size_t num_pages, void *mmio_portal) {
    if (!dst_page || !src_page || num_pages == 0 || !mmio_portal) return;

    uint8_t *dst = (uint8_t *)dst_page;
    const uint8_t *src = (const uint8_t *)src_page;

    // Track a single array of records so we can pipeline submissions in parallel
    // For large operations, keep this constrained to prevent stack overflows
    dsa_comp_record_t comp_records[num_pages];

    // ==========================================
    // STEP 1: PARALLEL JOB SUBMISSION (PIPELINING)
    // ==========================================
    for (size_t i = 0; i < num_pages; ++i) {
        // Initialize completion record flag for this page
        comp_records[i].status = 0; 

        // Configure the 64-byte descriptor
        dsa_desc_t desc = {0};
        desc.opcode = 0x01;                              // Memcpy operation
        desc.flags = 0x04;                               // Flag bit 2: Request completion record
        desc.transfer_size = PAGE_SIZE;                  // 4096 bytes
        desc.src_addr = (uint64_t)(src + (i * PAGE_SIZE));
        desc.dst_addr = (uint64_t)(dst + (i * PAGE_SIZE));
        desc.completion_addr = (uint64_t)&comp_records[i];

        // Enqueue the command into the hardware portal.
        // Loop forces a retry if the internal hardware submission queue is filled up.
        while (_enqcmd(mmio_portal, &desc) != 0) {
            // Queue full backpressure loop. 
            // A brief pause chip hint prevents core overheating while retrying.
            _mm_pause(); 
        }
    }

    // ==========================================
    // STEP 2: ASYNCHRONOUS COMPLETION MONITORING
    // ==========================================
    // Since all jobs are now concurrently crunching in hardware, 
    // we step through the completion array and wait for each line to finish.
    for (size_t i = 0; i < num_pages; ++i) {
        while (comp_records[i].status == 0) {
            // Establish hardware address monitoring on the record status byte
            _umonitor((void *)&comp_records[i].status);
            
            // Re-verify after arming monitor to close tiny race condition gaps
            if (comp_records[i].status != 0) break;
            
            // Put CPU core into low-power monitor state until the DMA chip completes the write.
            // Timeout bounds provided by a quick timestamp register calculation.
            _umwait(1, __rdtsc() + 50000); 
        }

        // Error detection logic handling
        if (comp_records[i].status > 1) {
            // Handle error condition appropriately (e.g., fallback software copy, page-fault recovery)
        }
    }
}
