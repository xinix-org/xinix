#pragma once

#include <stddef.h>
#include <uchar.h>

typedef struct {
    const char8_t *string_data;
    size_t string_len;
} string_t;

#define CONST_STRING_DEF_INVALID_ASSOC(Ty, sl)                                 \
    Ty * : &sl,                                                                \
           Ty const * : &sl,                                                   \
                        Ty restrict * : &sl,                                   \
                                        Ty const restrict * : &sl,

#define STRING(sl)                                                             \
    ((string_t){                                                               \
        .string_data = _Generic((sl),                                          \
            char *: (const char8_t *)(sl),                                     \
            const char *: (const char8_t *)(sl),                               \
            char8_t *: (const char8_t *)(sl),                                  \
            const char8_t *: (sl)),                                            \
        .string_len = _Generic((&sl),                                          \
            CONST_STRING_DEF_INVALID_ASSOC(char *, sl)                         \
                    CONST_STRING_DEF_INVALID_ASSOC(const char *, sl)           \
                        CONST_STRING_DEF_INVALID_ASSOC(char8_t *, sl)          \
                            CONST_STRING_DEF_INVALID_ASSOC(                    \
                                const char8_t *, sl) default: sizeof(sl) -     \
                1)})
