#pragma once

#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *dest, int ch, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);

size_t strlen(const char* s);
size_t strnlen(const char* s, size_t n);

// TODO: the rest. Other functions need to be written to make this compliant.
