#pragma once

#include <stddef.h>

struct VtableCommon {
    size_t size;
    size_t align;
    void (*destructor)(void *);
    void (*dealloc)(void *);
};
