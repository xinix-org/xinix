#pragma once

#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *dest, int ch, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);

void init_heap(void);

void *malloc(size_t size);
void *aligned_alloc(size_t alignment, size_t size);
void *calloc(size_t num, size_t size);
void free(void *ptr);

size_t strlen(const char *str);
size_t strnlen(const char *str, size_t maxlen);
