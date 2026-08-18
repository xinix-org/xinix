#pragma once
#include <stdint.h>


enum gdt_flags : uint8_t {
    GDT_Granularity = 0b10000000,
    GDT_DB = 0b01000000,
    GDT_L = 0b00100000,

    GDT_16BIT = 0,
    GDT_32BIT = GDT_Granularity | GDT_DB,
    GDT_64BITC = GDT_L,
};

typedef union {
    struct {
        _Alignas(8) uint16_t limit_lo;
        uint16_t base_lo;
        uint8_t base_mid;
        uint8_t access;
        uint8_t flags_and_limit_hi;
        uint8_t base_hi;
    };
    struct {
        _Alignas(8) uint32_t sys_base_ext;
        uint32_t sys_reserved;
    };
}  gdt_entry_t;

#define USER_SEGMENT_ACCESS(x, dpl)                                            \
    (uint8_t)(0b10010011 | (((x) & 1) << 3) | ((dpl) & 3) << 5)

enum system_segment_type : uint8_t {
    LDT = 0x02,

    TSS = 0x09,
    TSS_Busy = 0x0B,
};

#define SYSTEM_SEGMENT_ACCESS(ty) (uint8_t)(0b10000000 | ((ty) & 0xF))


extern gdt_entry_t gdt_entries[];
