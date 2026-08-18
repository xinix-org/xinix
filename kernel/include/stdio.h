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

int printf(const char *restrict format, ...);
int fprintf(FILE *restrict stream, const char *restrict format, ...);
int sprintf(char *restrict buffer, const char *restrict format, ...);
int snprintf(char *restrict buffer, size_t bufsz, const char *restrict format,
             ...);

int vprintf(const char *restrict format, va_list vlist);
int vfprintf(FILE *restrict stream, const char *restrict format, va_list vlist);
int vsprintf(char *restrict buffer, const char *restrict format, va_list vlist);
int vsnprintf(char *restrict buffer, size_t bufsz, const char *restrict format,
              va_list vlist);

int fclose(FILE *stream);
