#include <stddef.h>
#include <stdint.h>

#include <bits/feat_test.h>

struct cpuid {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
};

static struct cpuid cpuid_count(uint32_t leaf,
                                uint32_t subleaf) _ATTRIBUTE_UNSEQ {
    struct cpuid ret = {.eax = leaf, .ecx = subleaf};
    __asm__ inline("cpuid"
                   : "+a"(ret.eax), "+c"(ret.ecx), "=d"(ret.edx),
                     "=b"(ret.ebx));

    return ret;
}

static struct cpuid cpuid(uint32_t leaf) _ATTRIBUTE_UNSEQ {
    return cpuid_count(leaf, 0);
}

uint32_t x86_feature_array[38];

constexpr uint32_t avx_feature_mask[16] = {
    (1 << 29),
    0,
    (1 << 9) | (1 << 10),
    0,
    (1 << 5),
    (1 << 0) | (1 << 2) | (1 << 4) | (1 << 23),
    0,
    (1 << 4) | (1 << 5) | (1 << 10),
};

constexpr uint32_t avx512_feature_mask[16] = {
    0,
    0,
    (1 << 1) | (1 << 6) | (1 << 11) | (1 << 12) | (1 << 14),
    (1 << 2) | (1 << 3) | (1 << 8) | (1 << 23),
    (1 << 16) | (1 << 17) | (1 << 21) | (1 << 26) | (1 << 27) | (1 << 28) |
        (1 << 30) | (1U << 31),
    (1 << 5),
    (1 << 19),
    0,
};

constexpr uint32_t amx_feature_mask[16] = {
    0, 0, 0, (1 << 22) | (1 << 24) | (1 << 25), (1 << 21), 0, (1 << 8)};

void init_cpu_feature_array(void) {
    auto base_leaf = cpuid(0);
    auto max_default_leaf = base_leaf.eax;
    auto ext_leaf = cpuid(0x8000'0000);
    auto max_ext_leaf = ext_leaf.eax;

    auto eax1 = max_default_leaf >= 1 ? cpuid(1) : (struct cpuid){};
    auto eax7_ecx0 = cpuid_count(7, 0);
    auto eax7_max_sub = eax7_ecx0.eax;
    auto eax7_ecx1 = eax7_max_sub >= 1 ? cpuid_count(7, 1) : (struct cpuid){};
    auto eax7_ecx2 = eax7_max_sub >= 2 ? cpuid_count(7, 2) : (struct cpuid){};
    auto eax8000_0001 = max_ext_leaf >= 0x8000'0001 ? cpuid(0x8000'0001) : (struct cpuid){};

    x86_feature_array[0] = eax1.ecx;
    x86_feature_array[1] = eax1.edx;
    x86_feature_array[2] = eax7_ecx0.ecx;
    x86_feature_array[3] = eax7_ecx0.edx;
    x86_feature_array[4] = eax7_ecx0.ebx;
    x86_feature_array[5] = eax7_ecx1.eax;
    x86_feature_array[6] = eax7_ecx1.ecx;
    x86_feature_array[7] = eax7_ecx1.edx;
    x86_feature_array[8] = eax7_ecx1.ebx;
    x86_feature_array[11] = eax7_ecx2.edx;
    x86_feature_array[13] = eax8000_0001.ecx;
    x86_feature_array[14] = eax8000_0001.edx & ~0x0183F3FF;

    bool has_xsave = eax1.ecx & (1 << 26);

    bool has_avx = false;
    bool has_avx512 = false;

    bool has_amx = false;
    bool has_apx = false;

    bool has_pku = false;
    bool has_cet_u = false;
    bool has_cet_s = false;
    bool has_uintr = false;
    bool has_pasid = false;
    bool has_pt = false;
    bool has_lbr = false;
    bool has_lwp = false;

    if (has_xsave) {
        auto eax0D_ecx0 = max_default_leaf >= 0x0D ? cpuid_count(0x0D, 0) : (struct cpuid){};
        auto eax0D_ecx1 = max_default_leaf >= 0x0D ?  cpuid_count(0x0D, 1)  : (struct cpuid){};
        uint64_t xcr0_supported = ((uint64_t)eax0D_ecx0.eax) | (((uint64_t)eax0D_ecx0.edx)<<32);
        uint64_t xss_supported = ((uint64_t)eax0D_ecx1.ecx) | (((uint64_t)eax0D_ecx1.edx)<<32);
        x86_feature_array[32] = eax0D_ecx0.eax;
        x86_feature_array[33] = eax0D_ecx0.edx;
        x86_feature_array[34] = eax0D_ecx1.eax;
        x86_feature_array[36] = eax0D_ecx1.ecx;
        x86_feature_array[37] = eax0D_ecx1.edx;
        has_avx = (xcr0_supported & (1 << 2));
        has_avx512 = (xcr0_supported & (0b111 << 5)) == (0b111 << 5);
        has_amx = (xcr0_supported & (0b11 << 17)) == (0b11 << 17);
        has_apx = (xcr0_supported & (1 << 19));
        has_pku = (xcr0_supported & (1 << 9));
        has_cet_u = (xss_supported & (1 << 11));
        has_cet_s = (xss_supported & (1 << 12));
        has_pt = xss_supported & (1 << 8);
        has_uintr = xss_supported & (1 << 14);
        has_lbr = xss_supported & (1 << 15);
        has_lwp = xcr0_supported & (1ull << 62);
        has_pasid = xss_supported & (1 << 10);
    }

    bool has_avx10 = has_avx512 && (eax7_ecx1.edx & (1 << 19));

    for (size_t i = 0; i < 16; i++) {
        if (!has_avx)
            x86_feature_array[i] &= ~avx_feature_mask[i];

        if (!has_avx512)
            x86_feature_array[i] &= ~avx512_feature_mask[i];

        if (!has_amx)
            x86_feature_array[i] &= ~amx_feature_mask[i];
    }

    if (!has_apx)
        x86_feature_array[7] &= ~(1 << 21);

    if (has_avx10) {
        auto eax24_ecx0 = max_default_leaf >= 0x24 ? cpuid_count(0x24, 0) : (struct cpuid){};
        auto max_eax24_sub = eax24_ecx0.eax;
        auto eax24_ecx1 = max_eax24_sub >= 1 ? cpuid_count(0x24, 1) : (struct cpuid){};
        x86_feature_array[16] = eax24_ecx0.ebx;

        bool has_version2 = (eax24_ecx0.ebx & 0xFF) >= 2;

        x86_feature_array[17] = eax24_ecx1.ecx | (has_version2 << 2);
    }

    if(!has_pku)
        x86_feature_array[2] &= ~(0b11 << 3);

    if(!has_cet_s || !has_cet_u) {
        x86_feature_array[2] &= ~(1 << 7);
        x86_feature_array[7] &= ~(1 << 18);
    }

    if(!has_pt) {
        x86_feature_array[4] &= ~(1 << 25);
    }

    if(!has_uintr) {
        x86_feature_array[3] &= ~(1 << 5);
    }

    if(!has_lbr)
        x86_feature_array[3] &= ~(1 << 19);

    if(!has_pasid)
        x86_feature_array[2] &= ~(1 << 29);

    if(!has_lwp)
        x86_feature_array[13] &= ~(1 << 15);
}
