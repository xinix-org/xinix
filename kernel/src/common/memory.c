// Copied from the Limine sample:
// https://github.com/Limine-Bootloader/limine-c-template-x86-64/blob/trunk/kernel/src/memory.c
// And then malloc was added

#include <limits.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// They must be implemented as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = dest;
    const uint8_t *restrict psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// end copied code

#define BUMP_HEAP_SIZE 8192000

char bump_heap[BUMP_HEAP_SIZE] = {};
size_t bump_heap_ptr = 0;

void *aligned_alloc(size_t alignment, size_t size) {
    // align the heap first
    bump_heap_ptr = (bump_heap_ptr + alignment - 1) & ~(alignment - 1);
    if (BUMP_HEAP_SIZE - bump_heap_ptr < size) {
        return nullptr;
    } else {
        void *result = &bump_heap[bump_heap_ptr];
        bump_heap_ptr += size;
        return result;
    }
}

void *malloc(size_t size) { return aligned_alloc(alignof(max_align_t), size); }

void *calloc(size_t num, size_t size) {
    void *ptr = nullptr;
    if ((num != 0) && (size != 0) && ((SIZE_MAX / num) >= size)) {
        ptr = malloc((num * size));
        if (ptr == nullptr) {
            return ptr;
        }
        ptr = memset(ptr, 0, num * size);
        return ptr;
    }

    return ptr;
}

void free(void *_ptr) { return; }

// strlen because that's useful

size_t strlen(const char *str) {
    size_t result = 0;
    while (*str++ != 0) {
        result += 1;
    }
    return result;
}

size_t strnlen(const char *str, size_t maxlen) {
    size_t result = 0;
    while (*str++ != 0 && result < maxlen) {
        result += 1;
    }
    return result;
}
