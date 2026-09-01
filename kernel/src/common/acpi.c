#include "paging.h"
#include <acpi.h>
#include <auxv.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

volatile lapic_t *lapic;
volatile ioapic_t *ioapics;
int num_ioapics;

static uint32_t unaligned_u32(uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

void print_sdt_header(sdt_header_t *sdt_p) {
    printf("Table %.4s\r\n", sdt_p->signature);
    printf("OEMID: %.6s -- Table ID: %.8s -- Revision: %08X\r\n", sdt_p->oemid,
           sdt_p->oem_table_id, sdt_p->oem_revision); // TODO: %d
    printf("               Creator ID: %08X -- Revision: %08X\r\n\r\n",
           sdt_p->creator_id,
           sdt_p->creator_revision); // TODO: %d
}

void load_madt(madt_header_t *madt_p) {
    printf("local APIC address: %#.8X\r\n", madt_p->local_apic_address);
    lapic = add_to_hhdm(kernel_pml4t, madt_p->local_apic_address,
                        PAGE_GRANULARITY_4KB, PROT_WRITE);
    printf("LAPIC ID: %#.8X\r\n", lapic->lapic_id);
    printf("LAPIC Version: %#.8X\r\n\r\n", lapic->lapic_version);
    uint8_t *byte_reader = (uint8_t *)madt_p;

    // First pass will count what we need to count; next pass will actually
    // populate structures
    size_t pos = sizeof(madt_header_t);
    while (pos < madt_p->header.length) {
        uint8_t entry_type = byte_reader[pos];
        uint8_t record_length = byte_reader[pos + 1];
        switch (entry_type) {
        case 1:
            num_ioapics += 1;
            break;
        default:
            break;
        }
        pos += record_length;
    }
    ioapics = calloc(num_ioapics, sizeof(*ioapics));
    num_ioapics = 0; // reusing as a counter

    pos = sizeof(madt_header_t);
    while (pos < madt_p->header.length) {
        uint8_t entry_type = byte_reader[pos];
        uint8_t record_length = byte_reader[pos + 1];
        switch (entry_type) {
        case 0:
            printf("TODO: processor local APIC\r\n");
            break;
        case 1:
            ioapics[num_ioapics].id = byte_reader[pos + 2];
            uint32_t ioapic_addr = unaligned_u32(&byte_reader[pos + 4]);
            ioapics[num_ioapics].register_base = add_to_hhdm(
                kernel_pml4t, ioapic_addr, PAGE_GRANULARITY_4KB, PROT_WRITE);
            ioapics[num_ioapics].global_system_interrupt_base =
                unaligned_u32(&byte_reader[pos + 8]);
            printf("IOAPIC number %02X -- ID: %#.8X -- address: %#.8X -- int "
                   "base: %#.8X\r\n",
                   num_ioapics, ioapics[num_ioapics].id, ioapic_addr,
                   ioapics[num_ioapics].global_system_interrupt_base);
            num_ioapics += 1;
            break;
        default:
            printf("unrecognized MADT entry type %X\r\n", entry_type);
            break;
        }
        pos += record_length;
    }
}

void handle_sdt(sdt_header_t *sdt_p) {
    print_sdt_header(sdt_p);
    if (memcmp(sdt_p->signature, "APIC", 4) == 0) {
        load_madt((madt_header_t *)sdt_p);
    }
}

void load_xsdt(void) {
    xsdt_t *xsdt_p = getauxval(AT_KXINIX_XSDT_ADDR).a_ptr;
    print_sdt_header(&xsdt_p->header);

    int num_sdts = (xsdt_p->header.length - sizeof(sdt_header_t)) / 8;
    size_t hhdm_offset = getauxval(AT_KXINIX_HHDM_OFFSET).a_val;
    for (int i = 0; i < num_sdts; i++) {
        handle_sdt((sdt_header_t *)(hhdm_offset + xsdt_p->entries[i]));
    }
}

void load_rsdt(void) {
    rsdt_t *rsdt_p = getauxval(AT_KXINIX_RSDT_ADDR).a_ptr;
    print_sdt_header(&rsdt_p->header);

    int num_sdts = (rsdt_p->header.length - sizeof(sdt_header_t)) / 4;
    size_t hhdm_offset = getauxval(AT_KXINIX_HHDM_OFFSET).a_val;
    for (int i = 0; i < num_sdts; i++) {
        handle_sdt((sdt_header_t *)(hhdm_offset + rsdt_p->entries[i]));
    }
}

void load_system_descriptor_tables(void) {
    if (getauxval(AT_KXINIX_XSDT_ADDR).a_ptr) {
        load_xsdt();
    } else {
        load_rsdt();
    }
}
