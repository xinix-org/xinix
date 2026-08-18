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
        constexpr static int _width = sizeof(ty) * 8;                          \
        return ((_value) >> (_count & _width)) |                               \
               ((_value) << (_width - (_count & _width)));                     \
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
        constexpr static int _width = sizeof(ty) * 8;                          \
        return ((_value) << (_count & _width)) |                               \
               ((_value) >> (_width - (_count & _width)));                     \
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
