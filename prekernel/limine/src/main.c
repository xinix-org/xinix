#include <alloc.h>
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
extern void call_kmain(size_t _hhdm_offset, framebuffer *fb, memmap *memmap,
                       void *rsdp);

[[noreturn]]
extern void hcf(void);

/// IMPLEMENTATION ///

static void find_and_init_alloc(void) {
    uintptr_t largest_addr = 0;
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
    init_alloc(
        (dual_address_t){
            .virtual = (void *)(largest_addr + hhdm_request.response->offset),
            .physical = largest_addr},
        largest_size);
}

[[noreturn]]
void pkmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
        hcf();
    }

    if (memmap_request.response == nullptr) {
        hcf();
    }

    find_and_init_alloc();

    framebuffer fb, *pfb = nullptr;
    video_mode fb_mode;
    if (framebuffer_request.response != nullptr &&
        framebuffer_request.response->framebuffer_count >= 1) {
        struct limine_framebuffer *l_fb =
            framebuffer_request.response->framebuffers[0];
        fb_mode = (video_mode){
            .pitch = l_fb->pitch,
            .width = l_fb->width,
            .height = l_fb->height,
            .bpp = l_fb->bpp,
            .memory_model = l_fb->memory_model,
            .red_mask_size = l_fb->red_mask_size,
            .red_mask_shift = l_fb->red_mask_shift,
            .green_mask_size = l_fb->green_mask_size,
            .green_mask_shift = l_fb->green_mask_shift,
            .blue_mask_size = l_fb->blue_mask_size,
            .blue_mask_shift = l_fb->blue_mask_shift,
        };
        fb = (framebuffer){
            .address = l_fb->address,
            .mode_count = l_fb->mode_count,
            .modes = &fb_mode,
        };
        pfb = &fb;
    }

    // *** NOTE: HERE BE DRAGONS. THIS MUST BE LAST, OR ELSE THE KERNEL WILL ***
    // *** BE TOLD SOME MEMORY IS USABLE THAT DEFINITELY IS NOT.             ***

    compiler_barrier();

    // one extra for the memory the bump alloc claimed
    memmap_entry *entries = malloc(sizeof(memmap_entry) *
                                   (memmap_request.response->entry_count + 1));

    // align the allocator position to a page boundary
    size_t alloc_pos_aligned = (size_t)(alloc_pos.physical + 0xFFF) & ~0xFFF;
    size_t alloc_size = alloc_pos_aligned - alloc_area_start.physical;

    int extra_inserted = 0;
    for (int i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        if (entry->base == alloc_area_start.physical) {
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
               rsdp_request.response->address);
}
