#include "context.h"
#include "paging.h"
#include <acpi.h>
#include <auxv.h>
#include <memory.h>
#include <pointers.h>
#include <stdio.h>
#include <string.h>

volatile ioapic_t *ioapics;
int num_ioapics;

fadt_t *fadt;

volatile facs_t *facs;

dsdt_t *dsdt;

static uint32_t unaligned_u16(uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8);
}

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
    auto kctx = getcontext();

    kctx->lapic = add_to_hhdm(kernel_pml4t, madt_p->local_apic_address,
                              PAGE_GRANULARITY_4KB, PROT_WRITE);
    printf("LAPIC ID: %#.8X\r\n", kctx->lapic->lapic_id.value);
    printf("LAPIC Version: %#.8X\r\n\r\n", kctx->lapic->lapic_version.value);
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
        case 1: {
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
        }
        case 2: {
            uint8_t bus = byte_reader[pos + 2];
            uint8_t source = byte_reader[pos + 3];
            uint32_t gsint = unaligned_u32(&byte_reader[pos + 4]);
            uint16_t flags = unaligned_u16(&byte_reader[pos + 8]);
            printf(
                "TODO: remap bus=%02X, source=%02X, gsint=%08X, flags=%04X\r\n",
                bus, source, gsint, flags);
            break;
        }
        case 4: {
            uint8_t uid = byte_reader[pos + 2];
            uint16_t flags = unaligned_u16(&byte_reader[pos + 3]);
            uint8_t lint = byte_reader[pos + 5];
            printf("TODO: lapic nmi uid=%02X, flags=%04X, lint=%02X\r\n", uid,
                   flags, lint);
            break;
        }
        default:
            printf("unrecognized MADT entry type %X\r\n", entry_type);
            break;
        }
        pos += record_length;
    }
}

void load_fadt(fadt_t *fadt_p) {
    fadt = fadt_p;
    uint64_t dsdt_paddr;
    uint64_t facs_paddr;
    if (fadt_p->fadt_header.revision >= 2 &&
        fadt_p->fadt_header.length >= sizeof(fadt2_t)) {
        fadt2_t *fadt2_p = (fadt2_t *)fadt_p;

        if (fadt2_p->fadt_xdsdt)
            dsdt_paddr = fadt2_p->fadt_xdsdt;
        else
            dsdt_paddr = fadt_p->fadt_dsdt;

        if (fadt2_p->fadt_xfirmware_ctrl)
            facs_paddr = fadt2_p->fadt_xfirmware_ctrl;
        else
            facs_paddr = fadt_p->fadt_fwctl;
    } else {
        dsdt_paddr = fadt_p->fadt_dsdt;
        facs_paddr = fadt_p->fadt_fwctl;
    }

    printf("FACS ADDR %#.16w64X\tDSDT ADDR %#.16w64X\r\n", facs_paddr,
           dsdt_paddr);

    facs =
        add_to_hhdm(kernel_pml4t, facs_paddr, PAGE_GRANULARITY_4KB, PROT_WRITE);

    dsdt =
        add_to_hhdm(kernel_pml4t, dsdt_paddr, PAGE_GRANULARITY_4KB, PROT_WRITE);

    size_t dsdt_plen = ((dsdt_paddr & 0x3FF) + dsdt->dsdt_header.length) >> 12;

    while (dsdt_plen > 0) {
        dsdt_paddr += 4096;
        dsdt_plen -= 1;
        add_to_hhdm(kernel_pml4t, dsdt_paddr + 4096, PAGE_GRANULARITY_4KB,
                    PROT_WRITE);
    }

    dsdt = launder_pointer(dsdt);
}

void handle_sdt(sdt_header_t *sdt_p) {
    print_sdt_header(sdt_p);
    if (memcmp(sdt_p->signature, "APIC", 4) == 0) {
        load_madt((madt_header_t *)sdt_p);
    } else if (memcmp(sdt_p->signature, "FACP", 4) == 0) {
        load_fadt((fadt_t *)sdt_p);
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

uint32_t read_ioapic(uint8_t ioapic_id, uint32_t addr) {
    volatile uint32_t *register_base = ioapics[ioapic_id].register_base;
    *(register_base) = addr;
    uint32_t result = *(register_base + 4);
    return result;
}

void write_ioapic(uint8_t ioapic_id, uint32_t addr, uint32_t val) {
    read_ioapic(ioapic_id, addr);
    volatile uint32_t *register_base = ioapics[ioapic_id].register_base;
    *(register_base) = addr;
    *(register_base + 4) = val;
}

void write_io_redirect(uint8_t irq, uint8_t int_id,
                       ioredtbl_delivery_mode_t delivery_mode,
                       bool destination_is_logical, bool active_low,
                       bool level_triggered, uint8_t destination) {
    // TODO: this function currently assumes we only have one IOAPIC. This is
    // generally an okay assumption, but the function should be able to support
    // multiple in the future.
    uint32_t reg_base = 0x10 + (uint32_t)irq * 2;
    uint32_t low_reg_value = (level_triggered ? 0x8000 : 0) |
                             (active_low ? 0x2000 : 0) |
                             (destination_is_logical ? 0x0800 : 0) |
                             ((uint32_t)delivery_mode << 8) | (uint32_t)int_id;
    uint32_t high_reg_value = destination << 24;
    write_ioapic(0, reg_base, low_reg_value);
    write_ioapic(0, reg_base + 1, high_reg_value);
}
