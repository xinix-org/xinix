#pragma once

#include <stddef.h>
#include <uchar.h>
#include <wchar.h>

#define STRING_SLICE(T) struct {\
    T const* string_data;\
    size_t string_len;\
}



typedef STRING_SLICE(char8_t) u8string_t;


typedef u8string_t string_t;

typedef STRING_SLICE(char16_t) u16string_t;

typedef STRING_SLICE(char32_t) u32string_t;

typedef u32string_t wstring_t;

static_assert(sizeof(wchar_t)==sizeof(char32_t));

#define CONST_STRING_DEF_INVALID_ASSOC(Ty, sl)                                 \
    Ty * : &sl,                                                                \
           Ty const * : &sl,                                                   \
                        Ty restrict * : &sl,                                   \
                                        Ty const restrict * : &sl




#define CONST_STRING_DEF_ASSOC_TYPE(stty, ccty, sl)\
    ((stty){.string_data = (const ccty*)(sl), \
            .string_len = _Generic((&sl),                                          \
                CONST_STRING_DEF_INVALID_ASSOC(char *, sl),                         \
                CONST_STRING_DEF_INVALID_ASSOC(const char *, sl),           \
                CONST_STRING_DEF_INVALID_ASSOC(char8_t *, sl),          \
                CONST_STRING_DEF_INVALID_ASSOC(const char8_t *, sl), \
                CONST_STRING_DEF_INVALID_ASSOC(char16_t *, sl), \
                CONST_STRING_DEF_INVALID_ASSOC(const char16_t *, sl), \
                CONST_STRING_DEF_INVALID_ASSOC(char32_t *, sl), \
                CONST_STRING_DEF_INVALID_ASSOC(const char32_t *, sl), \
                CONST_STRING_DEF_INVALID_ASSOC(wchar_t *, sl), \
                CONST_STRING_DEF_INVALID_ASSOC(const wchar_t *, sl), \
                default: sizeof(sl) - 1)\
            })

#define STRING(sl)                                                             \
    _Generic((sl), \
            char*: CONST_STRING_DEF_ASSOC_TYPE(string_t, char8_t, sl),\
            const char*: CONST_STRING_DEF_ASSOC_TYPE(string_t, char8_t, sl),\
            char8_t*: CONST_STRING_DEF_ASSOC_TYPE(string_t, char8_t, sl),\
            const char8_t*: CONST_STRING_DEF_ASSOC_TYPE(string_t, char8_t, sl),\
            char16_t*: CONST_STRING_DEF_ASSOC_TYPE(u16string_t, char16_t, sl),\
            const char16_t*: CONST_STRING_DEF_ASSOC_TYPE(u16string_t, char16_t, sl),\
            char32_t*: CONST_STRING_DEF_ASSOC_TYPE(u32string_t, char32_t, sl),\
            const char32_t*: CONST_STRING_DEF_ASSOC_TYPE(u32string_t, char32_t, sl),\
            wchar_t*: CONST_STRING_DEF_ASSOC_TYPE(u32string_t, char32_t, sl),\
            const wchar_t*: CONST_STRING_DEF_ASSOC_TYPE(u32string_t, char32_t, sl)\
    )

