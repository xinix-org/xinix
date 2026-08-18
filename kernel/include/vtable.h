#pragma once

#include <memory.h>
#include <stddef.h>

struct VtableCommon {
    size_t size;
    size_t align;
    void (*destructor)(void *);
    void (*dealloc)(void *, size_t, size_t);
};

#define VTABLE_TY_NO_DROP(Ty)                                                  \
    ((struct VtableCommon){.size = sizeof(Ty),                                 \
                           .align = alignof(Ty),                               \
                           .destructor = nullptr,                              \
                           .dealloc = free_aligned_size})
#define VTABLE_TY_DROP(Ty, dtor)                                               \
    ((struct VtableCommon){.size = sizeof(Ty),                                 \
                           .align = alignof(Ty),                               \
                           .descrutor = dtor,                                  \
                           .dealloc = free_aligned_size})
