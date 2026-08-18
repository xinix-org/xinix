#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct dual_address {
    uintptr_t physical;
    void *virtual;
} dual_address_t;

// TODO: remove these.
// 
// These exist for the limine pre-kernel to try (and fail)
// to update the memory map. Eventual plan is for common pre-kernel code to
// generate the kernel memory map using some interface with the
// bootloader-specific code, but we haven't figured out what that is yet.
// Possible solution is a callback that generates memory entries, letting the
// common pre-kernel modify the memory region corresponding to the memory
// allocator region as it's getting saved.
//
// Ray doesn't feel like implementing that yet.
extern dual_address_t alloc_area_start;
extern dual_address_t alloc_pos;

void init_alloc(dual_address_t start, size_t size);

void *malloc(size_t size);
void *aligned_alloc(size_t alignment, size_t size);

// only alloc; don't free in prekernel
