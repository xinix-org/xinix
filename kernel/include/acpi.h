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

typedef struct ioapic {
    void *register_base;
    int id;
    int global_system_interrupt_base;
} ioapic_t;

typedef struct [[gnu::packed]] fadt_gen_addr {
    enum : uint8_t {
        AS_SystemMemory = 0,
        AS_SystemIO = 1,
        AS_PCIConfiguration = 2,
        AS_EmbeddedController = 3,
        AS_SMBus = 4,
        AS_CMOS = 5,
        AS_PCI_BAR = 6,
        AS_IPMI = 7,
        AS_GPIO = 8,
        AS_GenSerial = 9,
        AS_Pcc = 10,

        AS_FunctionalFixed = 0x7F,

        AS_OEM_DEFINED_LO = 0xC0,
        AS_OEM_DEFINED_HI = 0xFF,
    } fadt_addr_space;
    uint8_t fadt_addr_bitw;
    uint8_t fadt_addr_bitoff;
    enum : uint8_t {
        ACCS_UNDEF = 0,
        ACCS_BYTE = 1,
        ACCS_WORD = 2,
        ACCS_DWORD = 3,
        ACCS_QWORD = 4,
    } fadt_addr_accsize;
    uint64_t fadt_addr;
} fadt_addr_t;

typedef struct [[gnu::packed]] fadt {
    sdt_header_t fadt_header;
    uint32_t fadt_fwctl;
    uint32_t fadt_dsdt;
    uint8_t _fadt_reserved8;
    uint8_t fadt_ppmp;
    uint16_t fadt_sci_intr;
    uint32_t fadt_smi_port;
    uint8_t fadt_acpi_enable;
    uint8_t fadt_acpi_disable;
    uint8_t fadt_s4bios_req;
    uint8_t fadt_pstate_ctrl;
    uint32_t fadt_pm1a_eblock;
    uint32_t fadt_pm1b_eblock;
    uint32_t fadt_pm1a_cblock;
    uint32_t fadt_pm1b_cblock;
    uint32_t fadt_pm2_cblock;
    uint32_t fadt_pm_timer_block;
    uint32_t fadt_gpe0_block;
    uint32_t fadt_gpe1_block;
    uint8_t fadt_pm1_event_len;
    uint8_t fadt_pm1_ctrl_len;
    uint8_t fadt_pm2_ctrl_len;
    uint8_t fadt_pmtimer_len;
    uint8_t fadt_gpe0_len;
    uint8_t fadt_gpe1_len;
    uint8_t fadt_gpe1_base;
    uint8_t fadt_cstate_ctrl;
    uint16_t fadt_c2_max_latency;
    uint16_t fadt_c3_max_latency;
    uint16_t fadt_flush_size;
    uint16_t fadt_flush_stride;
    uint8_t fadt_duty_offset;
    uint8_t fadt_duty_width;
    uint8_t fadt_day_alarm;
    uint8_t fadt_month_alarm;
    uint8_t fadt_century;
    uint8_t fadt_boot_arch_flags;
    uint16_t _fadt_reserved110;
    uint32_t fadt_flags;
    fadt_addr_t fadt_reset_reg;
} fadt_t;

typedef struct [[gnu::packed]] fadt2 {
    fadt_t fadt;
    uint8_t fadt_reset_value;
    uint16_t fadt_arm_boot_arch;
    uint8_t fadt_minor_version;
    uint64_t fadt_xfirmware_ctrl;
    uint64_t fadt_xdsdt;
    fadt_addr_t fadt_xpm1a_eblock;
    fadt_addr_t fadt_xpm1b_eblock;
    fadt_addr_t fadt_xpm1a_cblock;
    fadt_addr_t fadt_xpm1b_cblock;
    fadt_addr_t fadt_xpm2_cblock;
    fadt_addr_t fadt_xpm_timer_block;
    fadt_addr_t fadt_xgpe0_block;
    fadt_addr_t fadt_xgpe1_block;
} fadt2_t;

typedef struct [[gnu::packed]] fadt5 {
    fadt2_t fadt;
    fadt_addr_t fadt_sleep_ctrl;
    fadt_addr_t fadt_sleep_status;
    uint64_t fadt_hypervisor_vendor;
} fadt5_t;

typedef struct [[gnu::packed]] dsdt {
    sdt_header_t dsdt_header;
    uint8_t dsdt_block[];
} dsdt_t;

typedef struct [[gnu::packed]] facs {
    char signature[4];
    uint32_t length;
    uint32_t facs_hwsig;
    uint32_t facs_fw_wake_vector;
    uint32_t facs_global_lock;
    uint32_t facs_flags;
    uint64_t facs_xfw_wake_vector;
    uint8_t facs_version;
    uint8_t facs_reserved33[3];
    uint32_t facs_ospm_flags;
    uint32_t facs_reserved40[6];
} facs_t;

typedef enum ioredtbl_delivery_mode {
    IOREDTBL_DELIVERY_FIXED = 0b000,
    IOREDTBL_DELIVERY_LOWEST_PRIORITY = 0b001,
    IOREDTBL_DELIVERY_SMI = 0b010,
    IOREDTBL_DELIVERY_NMI = 0b100,
    IOREDTBL_DELIVERY_INIT = 0b101,
    IOREDTBL_DELIVERY_EXTINT = 0b111,
} ioredtbl_delivery_mode_t;

extern int num_ioapics;

extern void load_system_descriptor_tables(void);

// Low-level IOAPIC I/O; should typically only be used internally
extern void write_ioapic(uint8_t ioapic_id, uint32_t addr, uint32_t val);

extern void write_io_redirect(uint8_t irq, uint8_t int_id,
                              ioredtbl_delivery_mode_t delivery_mode,
                              bool destination_is_logical, bool active_low,
                              bool level_triggered, uint8_t destination);

extern fadt_t *fadt;

extern volatile facs_t *facs;

extern dsdt_t *dsdt;
