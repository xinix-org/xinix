#pragma once

#include <stdint.h>
#include <cpuid.h>
#include <string.h>
#include <stdbit.h>

uint32_t __rand_get_event_time(void) {
    uint32_t val = 0;
    if(is_x86_feature_detected(tsc)) {
        __asm__ volatile("cpuid\n\trdtsc" : "=a"(val)::"edx", "ecx", "ebx");
    }

    return val;
}

int __rand_slow_get_entropy_backing(uint8_t _output[static restrict 16]) {
    uint64_t a[2];
    if (is_x86_feature_detected(rdseed)) {
        __asm__ volatile("2: rdseed %0\n\tjnc 2b\n\t2: rdseed %1\n\tjnc 2b"
                         : "=r"(a[0]), "=r"(a[1]));
    } else if (is_x86_feature_detected(rdrand)) {
        __asm__ volatile("2: rdrand %0\n\tjnc 2b\n\t2: rdrand %1\n\tjnc 2b"
                         : "=r"(a[0]), "=r"(a[1]));
    } else if (is_x86_feature_detected(tsc)) {
        for (size_t i = 0; i < 64; i += 2) {
            uint32_t r;
            __asm__ volatile("cpuid\n\trdtsc" : "=a"(r)::"edx", "ecx", "ebx");
            a[0] = stdc_rotate_right(a[0], 2) ^ (r & 7);
        }

        a[1] =
            ((((uint64_t)__rand_slow_get_entropy_backing) << 20) & 0xFFFF'FFFF'0000'0000);
        for (size_t i = 0; i < 32; i += 2) {
            uint32_t r;
            __asm__ volatile("cpuid\n\trdtsc" : "=a"(r)::"edx", "ecx", "ebx");
            a[1] = stdc_rotate_right(a[1], 2) | (r & 3);
        }
    } else {
        memset(_output, 0, 16);
        return -1;
    }

    memcpy(_output, a, 16);

    return 0;
}