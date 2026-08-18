#pragma once

#include <bits/feat_test.h>
#include <limits.h>

static inline unsigned int
stdc_leading_zeros_ui(unsigned int _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_clzg)
    return __builtin_clzg(_value, 32);
#elif __has_builtin(__builtin_clz)
    return (_value) ? __builtin_clz(_value) : 32;
#else
    for (unsigned int x = 32; true; x--, _value >>= 1)
        if (!_value)
            return x;
#endif
}

static inline unsigned int
stdc_leading_zeros_uc(unsigned char _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_clzg)
    return __builtin_clzg(_value, 8);
#else
    return stdc_leading_zeros_ui(_value) - 24;
#endif
}

static inline unsigned int
stdc_leading_zeros_us(unsigned short _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_clzg)
    return __builtin_clzg(_value, 16);
#else
    return stdc_leading_zeros_ui(_value) - 16;
#endif
}

static inline unsigned int
stdc_leading_zeros_ul(unsigned long _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_clzg)
    return __builtin_clzg(_value, LONG_WIDTH);
#elif __has_builtin(__builtin_clzl)
    return (_value) ? __builtin_clzl(_value) : LONG_WIDTH;
#else
    for (unsigned int x = LONG_WIDTH; true; x--, _value >>= 1)
        if (!_value)
            return x;
#endif
}

static inline unsigned int
stdc_leading_zeros_ull(unsigned long long _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_clzg)
    return __builtin_clzg(_value, 64);
#elif __has_builtin(__builtin_clzll)
    return (_value) ? __builtin_clzll(_value) : 64;
#else
    for (unsigned int x = 64; true; x--, _value <<= 1)
        if (!_value)
            return x;
#endif
}

static inline unsigned int
stdc_trailing_zeros_ui(unsigned int _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_ctzg)
    return __builtin_ctzg(_value, 32);
#elif __has_builtin(__builtin_ctz)
    return (_value) ? __builtin_ctz(_value) : 32;
#else
    for (unsigned int x = 0; true; x++, _value >>= 1)
        if ((_value & 1))
            return x;
#endif
}

static inline unsigned int
stdc_trailing_zeros_uc(unsigned char _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_clzg)
    return __builtin_ctzg(_value, 8);
#else
    return stdc_trailing_zeros_ui(_value);
#endif
}

static inline unsigned int
stdc_trailing_zeros_us(unsigned short _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_ctzg)
    return __builtin_ctzg(_value, 16);
#else
    return stdc_trailing_zeros_ui(_value);
#endif
}

static inline unsigned int
stdc_trailing_zeros_ul(unsigned long _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_ctzg)
    return __builtin_clzg(_value, LONG_WIDTH);
#elif __has_builtin(__builtin_ctzl)
    return (_value) ? __builtin_ctzl(_value) : LONG_WIDTH;
#else
    for (unsigned int x = 0; true; x++, _value >>= 1)
        if ((_value & 1))
            return x;
#endif
}

static inline unsigned int
stdc_trailing_zeros_ull(unsigned long long _value) _ATTRIBUTE_UNSEQ {
#if __has_builtin(__builtin_ctzg)
    return __builtin_ctzg(_value, 64);
#elif __has_builtin(__builtin_ctzll)
    return (_value) ? __builtin_ctzll(_value) : 64;
#else
    for (unsigned int x = 0; true; x++, _value >>= 1)
        if ((_value & 1))
            return x;
#endif
}

#define _STDBIT_DEF_GENERIC_MAP(value, prefix, ...)                            \
    _Generic((value),                                                          \
        unsigned char: prefix##_uc((value)__VA_OPT__(, ) __VA_ARGS__),         \
        unsigned _BitInt(8): prefix##_uc((unsigned char)(value)__VA_OPT__(, )  \
                                             __VA_ARGS__),                     \
        unsigned short: prefix##_us((value)__VA_OPT__(, ) __VA_ARGS__),        \
        unsigned _BitInt(16): prefix##_us(                                     \
                 (unsigned short)(value)__VA_OPT__(, ) __VA_ARGS__),           \
        unsigned int: prefix##_ui((value)__VA_OPT__(, ) __VA_ARGS__),          \
        unsigned _BitInt(32): prefix##_ui((unsigned int)(value)__VA_OPT__(, )  \
                                              __VA_ARGS__),                    \
        unsigned long: prefix##_ul((value)__VA_OPT__(, ) __VA_ARGS__),         \
        unsigned long long: prefix##_ull((value)__VA_OPT__(, ) __VA_ARGS__),   \
        unsigned _BitInt(64): prefix##_ull(                                    \
                 (unsigned long long)(value)__VA_OPT__(, ) __VA_ARGS__))

#define stdc_leading_zeros(value)                                              \
    _STDBIT_DEF_GENERIC_MAP(value, stdc_leading_zeros)

#define stdc_trailing_zeros(value)                                             \
    _STDBIT_DEF_GENERIC_MAP(value, stdc_trailing_zeros)

#if __has_builtin(__builtin_stdc_rotate_right)
#define _STDBIT_DEF_ROTATE_RIGHT_FN(ty, suffix)                                \
    static inline ty stdc_rotate_right_##suffix(ty _value,                     \
                                                unsigned int _count) {         \
        return (ty)__builtin_stdc_rotate_right(_value, _count);                \
    }
#else
#define _STDBIT_DEF_ROTATE_RIGHT_FN(ty, suffix)                                \
    static inline ty stdc_rotate_right_##suffix(ty _value,                     \
                                                unsigned int _count) {         \
        constexpr static unsigned int _width = (sizeof(ty) * 8);                     \
        return ((_value) >> (_count & (_width - 1))) |                               \
               ((_value) << (_width - (_count & (_width - 1))));                     \
    }
#endif

#if __has_builtin(__builtin_stdc_rotate_left)
#define _STDBIT_DEF_ROTATE_LEFT_FN(ty, suffix)                                 \
    static inline ty stdc_rotate_left_##suffix(ty _value,                      \
                                               unsigned int _count) {          \
        return (ty)__builtin_stdc_rotate_left(_value, _count);                 \
    }
#else
#define _STDBIT_DEF_ROTATE_LEFT_FN(ty, suffix)                                 \
    static inline ty stdc_rotate_left_##suffix(ty _value,                      \
                                               unsigned int _count) {          \
        constexpr static unsigned int _width = sizeof(ty) * 8;                          \
        return ((_value) << (_count & (_width - 1))) |                               \
               ((_value) >> (_width - (_count & (_width - 1))));                     \
    }
#endif

_STDBIT_DEF_ROTATE_RIGHT_FN(unsigned char, uc)
_STDBIT_DEF_ROTATE_RIGHT_FN(unsigned short, us)
_STDBIT_DEF_ROTATE_RIGHT_FN(unsigned int, ui)
_STDBIT_DEF_ROTATE_RIGHT_FN(unsigned long, ul)
_STDBIT_DEF_ROTATE_RIGHT_FN(unsigned long long, ull)

#define stdc_rotate_right(value, count)                                        \
    _STDBIT_DEF_GENERIC_MAP(value, stdc_rotate_right, count)

_STDBIT_DEF_ROTATE_LEFT_FN(unsigned char, uc)
_STDBIT_DEF_ROTATE_LEFT_FN(unsigned short, us)
_STDBIT_DEF_ROTATE_LEFT_FN(unsigned int, ui)
_STDBIT_DEF_ROTATE_LEFT_FN(unsigned long, ul)
_STDBIT_DEF_ROTATE_LEFT_FN(unsigned long long, ull)

#define stdc_rotate_left(value, count)                                         \
    _STDBIT_DEF_GENERIC_MAP(value, stdc_rotate_left, count)


static inline unsigned int stdx_swap_bytes_ui(unsigned int _x) {
#if __has_builtin(__builtin_bswap32)
    return __builtin_bswap32(_x);
#else
    return (_x >> 24) | ((_x >> 16)&0_xFF00) | ((_x << 16) & 0_xFF0000) | (_x << 24);
#endif
}

static inline unsigned short stdx_swap_bytes_us(unsigned short _x) {
#if __has_builtin(__builtin_bswap16)
    return __builtin_bswap16(_x);
#else
    return ((unsigned short)(_x << 8)) | ((unsigned short)(_x >> 8));
#endif
}

static inline unsigned char stdx_swap_bytes_uc(unsigned char _x) {
    return _x;
}

static inline unsigned long long stdx_swap_bytes_ull(unsigned long long _x) {
#if __has_builtin(__builtin_bswap64)
    return __builtin_bswap64(_x);
#else
    return stdx_swap_bytes_ui(_x) << 32 | stdx_swap_bytes_ui((_x) >> 32);
#endif
}

static inline unsigned long stdx_swap_bytes_ul(unsigned long _x){
#if LONG_WIDTH == 32
    return stdx_swap_bytes_ui(_x);
#else
    return stdx_swap_bytes_ull(_x);
#endif
}


#define stdx_swap_bytes(_x) _STDBIT_DEF_GENERIC_MAP(_x, stdx_swap_bytes)

#define _STDBIT_DEF_DO_SWAP_BYTES_FN(name, ty, suffix)\
    static inline ty name##_##suffix(ty _val){\
        return stdx_swap_bytes_##suffix(_val);\
    }

#define _STDBIT_DEF_DO_IDENT_FN(name, ty, suffix)\
    static inline ty name##_##suffix(ty _val) {\
        return _val;\
    }

#define _STDBIT_DEF_SWAP_BYTES_FN_GROUP(name)\
    _STDBIT_DEF_DO_SWAP_BYTES_FN(name, unsigned char, uc)\
    _STDBIT_DEF_DO_SWAP_BYTES_FN(name, unsigned short, us)\
    _STDBIT_DEF_DO_SWAP_BYTES_FN(name, unsigned int, ui)\
    _STDBIT_DEF_DO_SWAP_BYTES_FN(name, unsigned long, ul)\
    _STDBIT_DEF_DO_SWAP_BYTES_FN(name, unsigned long long, ull)

#define _STDBIT_DEF_IDENT_FN_GROUP(name)\
    _STDBIT_DEF_DO_IDENT_FN(name, unsigned char, uc)\
    _STDBIT_DEF_DO_IDENT_FN(name, unsigned short, us)\
    _STDBIT_DEF_DO_IDENT_FN(name, unsigned int, ui)\
    _STDBIT_DEF_DO_IDENT_FN(name, unsigned long, ul)\
    _STDBIT_DEF_DO_IDENT_FN(name, unsigned long long, ull)

#ifndef __STDC_ENDIAN_LITTLE__
#define __STDC_ENDIAN_LITTLE__ __ORDER_LITTLE_ENDIAN__
#endif

#ifndef __STDC_ENDIAN_BIG__
#define __STDC_ENDIAN_BIG__ __ORDER_BIG_ENDIAN__
#endif

#ifndef __STDC_ENDIAN_NATIVE__
#define __STDC_ENDIAN_NATIVE__ __BYTE_ORDER__
#endif


#if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
_STDBIT_DEF_SWAP_BYTES_FN_GROUP(stdx_from_be)
_STDBIT_DEF_IDENT_FN_GROUP(stdx_from_le)
#elif __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_BIG__
_STDBIT_DEF_SWAP_BYTES_FN_GROUP(stdx_from_le)
_STDBIT_DEF_IDENT_FN_GROUP(stdx_from_be)
#endif


#define stdx_from_le(_val) _STDBIT_DEF_GENERIC_MAP(_val, stdx_from_le)
#define stdx_from_be(_val) _STDBIT_DEF_GENERIC_MAP(_val, stdx_from_be)