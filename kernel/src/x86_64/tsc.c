#include "auxfuncs.h"
#include "auxv.h"
#include "location.h"
#include "sysresult.h"
#include "time.h"
#include <cpuid.h>
#include <stdint.h>
#include <stdio.h>

static uint64_t tsc_ticks_per_second;

void init_tsc() {
    auto eax0 = cpuid(0);
    printf("Max supported Standard Leaf %#w32X\r\n", eax0.eax);
    if (eax0.eax < 0x15) {
        auto val = getauxval(AT_KXINIX_TSC_FREQ).a_val;

        if (!val) {
            printf("Cannot Determine TSC Frequency");
            hcf(ERR_UNUSABLE_TSC, CURRENT());
        }

        tsc_ticks_per_second = val;
    } else {
        auto eax15 = cpuid(0x15);
        uint64_t core_freq;

        if (eax15.ecx != 0) {
            core_freq = eax15.ecx;
        } else {
            auto eax16 = cpuid(0x16);
            core_freq = eax16.eax * 1'000'000;
        }

        if (eax15.eax) {
            printf("TSC Frequency Faulting (Denominator is 0)\r\n");
            hcf(ERR_UNUSABLE_TSC, CURRENT());
        }

        tsc_ticks_per_second = (core_freq * eax15.ebx) / eax15.eax;
    }

    printf("TSC Frequency: %w64u Hz", tsc_ticks_per_second);
}

static uint64_t read_raw_tsc() {
    uint64_t vallo;
    uint64_t valhi;
    __asm__ volatile("rdtsc" : "=a"(vallo), "=d"(valhi));

    return vallo | (valhi) << 32;
}

duration_t read_tsc() {
    uint64_t raw = read_raw_tsc();

    uint64_t seconds = raw / tsc_ticks_per_second;
    uint64_t ticks = raw % tsc_ticks_per_second;

    uint32_t nanos = (ticks * 1'000'000'000) / tsc_ticks_per_second;

    return (duration_t){.time_seconds = seconds, .time_nanos = nanos};
}
