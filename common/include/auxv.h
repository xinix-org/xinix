#pragma once

#include <bits/feat_test.h>
#include <stddef.h>

typedef struct {
    unsigned long a_type;
    union auxval_t {
        long a_val;
        void *a_ptr;
        void (*a_fnc)();
    } a_un;
} auxv_t;

// a_type values
#define AT_NULL 0   // end of list
#define AT_IGNORE 1 // no-op entry

#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_PLATFORM 8

#define AT_RANDOM 26 // Points to 16 random bytes

// Xinix specific region begins at 64

#define AT_XINIX_CPU_FEATURES_ARRAY 80 // pointer to an array of cpu features
#define AT_XINIX_CPU_FEATURES_LEN 81   // length of AT_XINIX_CPU_FEATURES_ARRAY

#define AT_XINIX_MAX_USER 95 // Max val of a_type entry for a userspace program

// Xinix Kernel region begins at 96
#define AT_KXINIX_FRAMEBUFFER 96 // framebuffer structure

#define AT_KXINIX_BOOTP_GUID 98   // Boot parition GUID (GPT labeled disk)
#define AT_KXINIX_BOOTP_MBR 99    // Boot partion MBR label
#define AT_KXINIX_MEMMAP 100      // Boot memory map
#define AT_KXINIX_HHDM_OFFSET 101 // HHDM offset
#define AT_KXINIX_RSDT_ADDR 102   // RSDT pointer (virtual address)
#define AT_KXINIX_XSDT_ADDR 103   // XSDT pointer (virtual address)

#define AT_XINIX_MAX_KERNEL 127 // Max value of a_type entry passed to kernel

union auxval_t getauxval(unsigned long a_type) _ATTRIBUTE_UNSEQ;
