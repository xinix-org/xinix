#include <gdt.h>
#include <stddef.h>
#include <stdint.h>

typedef struct GDT_Descriptor {
    _Alignas(8) uint16_t pad[3];
    uint16_t limit;
    gdt_entry_t *gdt;
} gdt_descriptor_t;

gdt_entry_t gdt_entries[32] = {
    {},
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(1, 0),
     .flags_and_limit_hi = GDT_16BIT},
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(0, 0),
     .flags_and_limit_hi = GDT_16BIT},
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(1, 0),
     .flags_and_limit_hi = 0xF | GDT_32BIT},
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(0, 0),
     .flags_and_limit_hi = 0xF | GDT_32BIT},
    {.limit_lo = 0,
     .access = USER_SEGMENT_ACCESS(1, 0),
     .flags_and_limit_hi = 0x0 | GDT_64BITC},
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(0, 0),
     .flags_and_limit_hi = 0xF | GDT_32BIT},
    {},
    {.access = SYSTEM_SEGMENT_ACCESS(TSS) & 0x7F},
    {}, // tss continued
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(1, 3),
     .flags_and_limit_hi = 0xF | GDT_64BITC},
    {.limit_lo = 0xFFFF,
     .access = USER_SEGMENT_ACCESS(0, 3),
     .flags_and_limit_hi = 0xF | GDT_32BIT},
};

static void load_gdt(void) {
    gdt_descriptor_t desc = {.limit = sizeof(gdt_entries) - 1,
                             .gdt = gdt_entries};

    __asm__ volatile("pushq $0x28\n"
                     "call 1f\n"
                     "jmp 2f\n"
                     "1: lgdt %0\n"
                     "lretq\n"
                     "2:"
                     "movl $0x30, %%eax\n"
                     "mov %%ax, %%ss\n"
                     "mov %%ax, %%ds\n"
                     "mov %%ax, %%es\n"

                     ::"m"(desc.limit)
                     : "eax", "memory");
}

// TODO: This is x86-64 specific. This should be moved.
typedef struct IDT_Entry {
    _Alignas(16) uint16_t offset_low;
    uint16_t segment;
    uint8_t ist;
    uint8_t type_and_perms;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} idt_entry_t;

typedef struct IDT {
    idt_entry_t entries[256];
} idt_t;

typedef struct [[gnu::packed]] IDT_Descriptor {
    uint16_t limit;
    idt_t *idt;
} idt_descriptor_t;

extern idt_t IDT;
extern uintptr_t isr_list[256];

static void load_idt(void) {
    for (int i = 0; i < 256; i++) {
        ptrdiff_t offset = isr_list[i];
        uintptr_t ptr = ((uintptr_t)&isr_list) + offset;
        IDT.entries[i].offset_low = ptr & 0xFFFF;
        IDT.entries[i].offset_mid = (ptr >> 16) & 0xFFFF;
        IDT.entries[i].offset_high = ptr >> 32;
    }
    idt_descriptor_t descriptor = {
        .limit = 255 * 16,
        .idt = &IDT,
    };
    __asm__ volatile("lidt %0" ::"m"(descriptor));
}


void load_arch_state(void) {
    load_gdt();
    load_idt();
}