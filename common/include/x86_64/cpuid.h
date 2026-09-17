#pragma once

#include <bits/feat_test.h>
#include <stdint.h>

#ifdef __X86_CPU_IMPL_NO_WANT_CONST_FEATURE_ARRAY
#define __X86_CPU_IMPL_CONST
#else
#define __X86_CPU_IMPL_CONST const
#endif

/// Contains the feature cache in the following layout (with feature flags
/// related to unsupported state components masked out):
/// * [0]: cpuid[eax=1].ecx
/// * [1]: cpuid[eax=1].edx
/// * [2]: cpuid[eax=7,ecx=0].ecx
/// * [3]: cpuid[eax=7,ecx=0].edx
/// * [4]: cpuid[eax=7,ecx=0].ebx
/// * [5]: cpuid[eax=7,ecx=1].eax
/// * [6]: cpuid[eax=7,ecx=1].ecx
/// * [7]: cpuid[eax=7,ecx=1].edx
/// * [8]: cpuid[eax=7,ecx=1].ebx
/// * [9]: reserved
/// * [10]: reserved
/// * [11]: cpuid[eax=7,ecx=2].edx
/// * [12]: reserved
/// * [13]: cpuid[eax=0x80000001].ecx
/// * [14]: cpuid[eax=0x80000001].edx (only non-redundant features)
/// * [15]:  reserved
/// * [16]: cpuid[eax=0x24,ecx=0].ebx
/// * [17]: cpuid[eax=0x24,ecx=1].ecx
/// * [18]..[31]: reserved
/// * [32]: cpuid[eax=0x0D,ecx=0].eax
/// * [33]: cpuid[eax=0x0D,ecx=0].edx
/// * [34]: cpuid[eax=0x0D,ecx=1].eax
/// * [35]: cpuid[eax=0x0D,ecx=0].ecx
/// * [36]: cpuid[eax=0x0D,ecx=1].ecx
/// * [37]: cpuid[eax=0x0D,ecx=1].edx
extern __X86_CPU_IMPL_CONST uint32_t x86_feature_array[];

struct cpuid {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
};

static inline struct cpuid cpuid_count(uint32_t leaf,
                                uint32_t subleaf) _ATTRIBUTE_UNSEQ {
    struct cpuid ret = {.eax = leaf, .ecx = subleaf};
    __asm__ inline("cpuid"
                   : "+a"(ret.eax), "+c"(ret.ecx), "=d"(ret.edx),
                     "=b"(ret.ebx));

    return ret;
}

static inline struct cpuid cpuid(uint32_t leaf) _ATTRIBUTE_UNSEQ {
    return cpuid_count(leaf, 0);
}

#define X86_CPUID_DEFINE_FEATURE_ENUM(feat, idx, bit)                          \
    _x86_feature_##feat = ((idx << 5) | bit),

enum x86_feature_flag {
#include <bits/cpuid_flags.h>
};

#undef X86_CPUID_DEFINE_FEATURE_ENUM

#define X86_CPUID_DEFINE_FEATURE_ENUM(feat, idx, bit)                          \
    case _x86_feature_##feat:                                                  \
        return #feat;

static inline const char *x86_get_feature_name(enum x86_feature_flag _flag) {
    switch (_flag) {
#include <bits/cpuid_flags.h>
    default:
        return nullptr;
    }
}

#undef X86_CPUID_DEFINE_FEATURE_ENUM

#define is_x86_feature_enabled(feature)                                        \
    (0) // TODO: Try to optimize this better, since it's important for optimized
        // out feature checks

static inline bool
_is_x86_feature_detected(enum x86_feature_flag _flag) _ATTRIBUTE_UNSEQ {
    return x86_feature_array[_flag >> 5] & (1 << (_flag & 31));
}

#define is_x86_feature_detected(feature)                                       \
    (is_x86_feature_enabled(feature) ||                                        \
     (_is_x86_feature_detected(_x86_feature_##feature)))

#define X86_CPUID_DEFINE_FEATURE_ENUM(feat, idx, bit)                          \
    if (is_x86_feature_detected(feat))                                         \
        (_callback)(_udata, _x86_feature_##feat);

static inline void x86_enumerate_supported_features(
    void (*_callback)(void *, enum x86_feature_flag _flag), void *_udata) {
#include <bits/cpuid_flags.h>
}

#undef X86_CPUID_DEFINE_FEATURE_ENUM
