#include <stddef.h>
#include <stdint.h>

#include <auxv.h>
#include <framebuffer.h>
#include <memmap.h>

#include <limine.h>

#include <membarrier.h>

/// LIMINE REQUESTS ///

[[gnu::used, gnu::section(".limine_requests")]]
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 1,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 5,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests_start")]]
static volatile uint64_t limine_requests_start_marker[] =
    LIMINE_REQUESTS_START_MARKER;

[[gnu::used, gnu::section(".limine_requests_end")]]
static volatile uint64_t limine_requests_end_marker[] =
    LIMINE_REQUESTS_END_MARKER;

/// FUNCTION PROTOTYPES ///

[[noreturn]]
extern void
call_kmain(size_t _hhdm_offset, framebuffer *fb, memmap *memmap, void *rsdp,
           void *(*aligned_alloc)(size_t align, size_t size), void **alloc_ptr);

[[noreturn]]
extern void hcf(void);

/// IMPLEMENTATION ///

static char *start_of_claim = nullptr;
static char *alloc_pos = nullptr;

static void init_alloc(void) {
    size_t largest_addr = 0;
    size_t largest_size = 0;
    for (int i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length > largest_size) {
                largest_addr = entry->base;
                largest_size = entry->length;
            }
        }
    }
    if (largest_size == 0) {
        hcf();
    }
    start_of_claim = alloc_pos = (char *)largest_addr;
}

static void *aligned_bump_alloc(size_t alignment, size_t size) {
    alloc_pos =
        (char *)(((size_t)(alloc_pos + alignment - 1)) & ~(alignment - 1));
    void *result = alloc_pos + hhdm_request.response->offset;
    alloc_pos += size;
    return result;
}

static void *bump_alloc(size_t size) {
    return aligned_bump_alloc(alignof(max_align_t), size);
}

[[noreturn]]
void pkmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
        hcf();
    }

    if (memmap_request.response == nullptr) {
        hcf();
    }

    init_alloc();

    framebuffer fb, *pfb = nullptr;
    if (framebuffer_request.response != nullptr &&
        framebuffer_request.response->framebuffer_count >= 1) {
        struct limine_framebuffer *l_fb =
            framebuffer_request.response->framebuffers[0];
        fb = (framebuffer){
            .address = l_fb->address,
            .mode_count = l_fb->mode_count,
            .modes = (video_mode **)l_fb->modes,
        };
        pfb = &fb;
    }

    // *** NOTE: HERE BE DRAGONS. THIS MUST BE LAST, OR ELSE THE KERNEL WILL ***
    // *** BE TOLD SOME MEMORY IS USABLE THAT DEFINITELY IS NOT.             ***

    compiler_barrier();

    // one extra for the memory the bump alloc claimed
    memmap_entry *entries = bump_alloc(
        sizeof(memmap_entry) * (memmap_request.response->entry_count + 1));

    // align the allocator position to a page boundary
    size_t alloc_pos_aligned = (size_t)(alloc_pos + 0xFFF) & ~0xFFF;
    size_t alloc_size = alloc_pos_aligned - (size_t)start_of_claim;

    int extra_inserted = 0;
    for (int i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        if (entry->base == (size_t)start_of_claim) {
            entries[i] = (memmap_entry){.base = entry->base,
                                        .length = alloc_size,
                                        .type = MEMMAP_PREKERNEL_RESERVED};
            entries[i + 1] =
                (memmap_entry){.base = alloc_pos_aligned,
                               .length = entry->length - alloc_size,
                               .type = MEMMAP_USABLE};
            extra_inserted = 1;
        } else {
            // types are (currently) compatible
            entries[i + extra_inserted] = *(memmap_entry *)(entry);
        }
    }
    memmap map = {.entry_count = memmap_request.response->entry_count + 1,
                  .entries = entries};

    call_kmain(hhdm_request.response->offset, pfb, &map,
               rsdp_request.response->address, aligned_bump_alloc, (void**)&alloc_pos);
}
