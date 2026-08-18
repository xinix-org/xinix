#include <alloc.h>
#include <stddef.h>

dual_address_t alloc_area_start;
dual_address_t alloc_pos;

void init_alloc(dual_address_t start, size_t _size) {
    // TODO: use size to set an upper bound
    alloc_area_start = alloc_pos = start;
}

void *aligned_alloc(size_t alignment, size_t size) {
    void *new_start_addr =
        (void *)(((uintptr_t)alloc_pos.virtual + alignment - 1) &
                 ~(alignment - 1));
    void *new_end_addr = new_start_addr + size;
    alloc_pos.physical +=
        (uintptr_t)new_end_addr - (uintptr_t)alloc_pos.virtual;
    alloc_pos.virtual = new_end_addr;
    return new_start_addr;
}

void *malloc(size_t size) { return aligned_alloc(alignof(max_align_t), size); }
