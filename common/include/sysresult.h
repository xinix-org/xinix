#pragma once

#include <stddef.h>

typedef enum : long {
    ERR_GENERIC = -1,

    ERR_IMAGE_WX_SEG = -36,
} sysresult_t;

#if __has_include(<bits/sysresult2_def.h>)
#include <bits/sysresult2_def.h>
#else
struct _sysresult_2 {
    sysresult_t _code;
    union {
        void *_value;
        unsigned char _uninit[sizeof(void *)];
    };
};

typedef struct _sysresult_2 sysresult2_t;

#define SYSRESULT2_CODE(val) ((val)._code)
#define SYSRESULT2_VALUE(val, T) ((T)((val)._value))

#define SYSRESULT2_ERROR(val)                                                  \
    ((sysresult2_t)((struct _sysresult_2){._code = val}))
#define SYSRESULT2_OK(val)                                                     \
    ((sysresult2_t)((struct _sysresult_2){._code = 0, ._value = (void *)val}))
#endif

#define SYSRESULT_TRY_SYSRESULT2(val)                                          \
    do {                                                                       \
        auto _val = (val);                                                     \
        if (_val < 0)                                                          \
            return SYSRESULT2_ERROR(_val);                                      \
    } while (0)
