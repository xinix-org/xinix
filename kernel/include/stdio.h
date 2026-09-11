#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef struct FILE {
    int flags; // TODO
    void *data;
    size_t (*write)(void *data, size_t len, const void *bytes);
    size_t (*read)(void *data, size_t len, void *bytes);
    void (*close)(void *data);
    uint64_t (*seek)(void *data, uint64_t pos, int seek_whence);
} FILE;

extern FILE *stdout;

extern FILE *krand_dev;
extern FILE *rand_dev;

[[gnu::format(printf, 1, 2)]]
int printf(const char *restrict format, ...);

[[gnu::format(printf, 2, 3)]]
int fprintf(FILE *restrict stream, const char *restrict format, ...);

[[gnu::format(printf, 2, 3)]]
int sprintf(char *restrict buffer, const char *restrict format, ...);

[[gnu::format(printf, 3, 4)]]
int snprintf(char *restrict buffer, size_t bufsz, const char *restrict format,
             ...);

[[gnu::format(printf, 1, 0)]]
int vprintf(const char *restrict format, va_list vlist);

[[gnu::format(printf, 2, 0)]]
int vfprintf(FILE *restrict stream, const char *restrict format, va_list vlist);

[[gnu::format(printf, 2, 0)]]
int vsprintf(char *restrict buffer, const char *restrict format, va_list vlist);

[[gnu::format(printf, 3, 0)]]
int vsnprintf(char *restrict buffer, size_t bufsz, const char *restrict format,
              va_list vlist);

int fclose(FILE *stream);
