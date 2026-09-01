#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct [[gnu::packed]] sdt_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} sdt_header_t;

typedef struct rsdt {
    sdt_header_t header;
    uint32_t entries[]; // flexible
} rsdt_t;

typedef struct [[gnu::packed]] xsdt {
    sdt_header_t header;
    uint64_t entries[]; // flexible
} xsdt_t;

typedef struct madt {
    sdt_header_t header;
    uint32_t local_apic_address;
    uint32_t flags;
} madt_header_t;

// newtype for alignment
typedef struct lapic_register {
    alignas(16) uint32_t value;
} lapic_register_t;

typedef struct lapic {
    const lapic_register_t reserved000[2];
    lapic_register_t lapic_id;
    const lapic_register_t lapic_version;
    const lapic_register_t reserved040[4];
    lapic_register_t task_priority_register;
    const lapic_register_t arbitration_priority_register;
    const lapic_register_t processor_priority_register;
    lapic_register_t end_of_interrupt_register; // write-only
    const lapic_register_t remote_read_register;
    lapic_register_t logical_destination_register;
    lapic_register_t destination_format_register;
    lapic_register_t spurious_interrupt_vector_register;
    const lapic_register_t in_service_register[8];
    const lapic_register_t trigger_mode_register[8];
    const lapic_register_t interrupt_request_register[8];
    const lapic_register_t error_status_register;
    const lapic_register_t reserved290[6];
    lapic_register_t lvt_correction_machine_check_interrupt_register;
    lapic_register_t interrupt_command_register[2];
    lapic_register_t lvt_timer_register;
    lapic_register_t lvt_thermal_sensor_register;
    lapic_register_t lvt_performance_monitoring_counters_register;
    lapic_register_t lvt_lint0_register;
    lapic_register_t lvt_lint1_register;
    lapic_register_t lvt_error_register;
    lapic_register_t initial_count_register;
    const lapic_register_t current_count_register;
    const lapic_register_t reserved3A0[4];
    lapic_register_t divide_configuration_register;
    const lapic_register_t reserved3F0;
} lapic_t;
static_assert(sizeof(lapic_t) == 0x400);
static_assert(alignof(lapic_t) >= 0x10);
static_assert(offsetof(lapic_t, reserved3F0) == 0x3F0);

extern volatile lapic_t *lapic; // TODO: thread-local

extern void load_system_descriptor_tables(void);
