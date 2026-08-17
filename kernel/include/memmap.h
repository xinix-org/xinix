#pragma once

#include <stdint.h>

// Lifted and modified from Limine

#define MEMMAP_USABLE 0
#define MEMMAP_RESERVED 1
#define MEMMAP_ACPI_RECLAIMABLE 2
#define MEMMAP_ACPI_NVS 3
#define MEMMAP_BAD_MEMORY 4
#define MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define MEMMAP_EXECUTABLE_AND_MODULES 6
#define MEMMAP_FRAMEBUFFER 7
#define MEMMAP_RESERVED_MAPPED 8
#define MEMMAP_PREKERNEL_RESERVED 9

static const char *memmap_type_name(uint64_t type) {
    switch (type) {
    case MEMMAP_USABLE:
        return "MEMMAP_USABLE";
    case MEMMAP_RESERVED:
        return "MEMMAP_RESERVED";
    case MEMMAP_ACPI_RECLAIMABLE:
        return "MEMMAP_ACPI_RECLAIMABLE";
    case MEMMAP_ACPI_NVS:
        return "MEMMAP_ACPI_NVS";
    case MEMMAP_BAD_MEMORY:
        return "MEMMAP_BAD_MEMORY";
    case MEMMAP_BOOTLOADER_RECLAIMABLE:
        return "MEMMAP_BOOTLOADER_RECLAIMABLE";
    case MEMMAP_EXECUTABLE_AND_MODULES:
        return "MEMMAP_EXECUTABLE_AND_MODULES";
    case MEMMAP_FRAMEBUFFER:
        return "MEMMAP_FRAMEBUFFER";
    case MEMMAP_RESERVED_MAPPED:
        return "MEMMAP_RESERVED_MAPPED";
    case MEMMAP_PREKERNEL_RESERVED:
        return "MEMMAP_PREKERNEL_RESERVED";
    default:
        return "<unknown>";
    }
}

typedef struct {
    uint64_t base;
    uint64_t length;
    uint64_t type;
} memmap_entry;

typedef struct {
    uint64_t entry_count;
    memmap_entry *entries;
} memmap;
