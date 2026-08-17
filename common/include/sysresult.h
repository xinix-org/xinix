#pragma once

#include <stddef.h>

#define SYSRESULT_DEF_ERROR_CONSTANT(name, const) ERR_##name = const,
typedef enum : long {
#include <bits/errordef.h>
} sysresult_t;
#undef SYSRESULT_DEF_ERROR_CONSTANT

#define SYSRESULT_DEF_ERROR_CONSTANT(name, const)                              \
    case ERR_##name:                                                           \
        return #name;

inline static const char *sysresult_name(sysresult_t res) {
    switch (res) {
#include <bits/errordef.h>
    default:
        return nullptr;
    }
}

#undef SYSRESULT_DEF_ERROR_CONSTANT

inline static const char *sysresult_describe(sysresult_t res) {
    switch (res) {
    case 0:
        return "Ok";
    case ERR_GENERIC:
        return "Unknown Error";
    case ERR_IMAGE_VALIDATION_ERROR:
        return "Elf Image Not Valid For Target";
    case ERR_IMAGE_WX_SEG:
        return "Elf Image Contains Writeable Text Segment";
    case ERR_IMAGE_INVALID_RELOC:
        return "Unexpected Dynamic Relocation";
    default:
        return nullptr;
    }
}

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
    ((sysresult2_t)((struct _sysresult_2){._code = (val)}))
#define SYSRESULT2_OK(val)                                                     \
    ((sysresult2_t)((struct _sysresult_2){._code = 0, ._value = (void *)(val)}))

#define SYSRESULT2_OK_WITH_VAL(val, code)                                      \
    ((sysresult2_t)((struct _sysresult_2){._code = (code),                     \
                                          ._value = (void *)(val)}))
#endif

#define SYSRESULT_TRY_SYSRESULT2(val)                                          \
    do {                                                                       \
        auto _val = (val);                                                     \
        if (_val < 0)                                                          \
            return SYSRESULT2_ERROR(_val);                                     \
    } while (0)
