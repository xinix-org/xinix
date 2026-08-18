#include "random.h"
#include "stdbit.h"
#include "sysresult.h"
#include <acpi.h>
#include <auxv.h>
#include <context.h>
#include <cpuid.h>
#include <dynld.h>
#include <elf.h>
#include <framebuffer.h>
#include <loader.h>
#include <memmap.h>
#include <memory.h>
#include <paging.h>
#include <stdio.h>

#include <auxfuncs.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <location.h>
#include <strslice.h>

[[gnu::section(".interp")]]
const char __interp[16] = "/xinix-kernel.so";

extern ElfNative_Dyn _DYNAMIC[];

static union auxval_t __auxent[128 - 2];

union auxval_t getauxval(unsigned long a_type) {
    if (a_type < 2 || a_type > 128)
        return (union auxval_t){};
    else
        return __auxent[a_type - 2];
}

size_t stdout_handler(void *data, size_t len, const void *bytes) {
    struct flanterm_context *ft_ctx = data;
    flanterm_write(ft_ctx, bytes, len);
    return len;
}

void print_feature_flag(void *v_want_comma, enum x86_feature_flag flag) {
    bool *want_comma = (bool *)v_want_comma;
    const char *name = x86_get_feature_name(flag);
    if (*want_comma) {
        printf(", %s", name);
    } else {
        printf("%s", name);
    }
    *want_comma = true;
}

enum gdt_flags : uint8_t {
    GDT_Granularity = 0b10000000,
    GDT_DB = 0b01000000,
    GDT_L = 0b00100000,

    GDT_16BIT = 0,
    GDT_32BIT = GDT_Granularity | GDT_DB,
    GDT_64BITC = GDT_L,
};

typedef struct GDT_Entry {
    _Alignas(8) uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_mid;
    volatile uint8_t access;
    uint8_t flags_and_limit_hi;
    uint8_t base_hi;
} gdt_entry_t;

#define USER_SEGMENT_ACCESS(x, dpl)                                            \
    (uint8_t)(0b10010011 | (((x) & 1) << 3) | ((dpl) & 3) << 5)

enum system_segment_type : uint8_t {
    LDT = 0x02,

    TSS = 0x09,
    TSS_Busy = 0x0B,
};

#define SYSTEM_SEGMENT_ACCESS(ty) (uint8_t)(0b10000000 | ((ty) & 0xF))

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

typedef struct GDT_Descriptor {
    _Alignas(8) uint16_t pad[3];
    uint16_t limit;
    gdt_entry_t *gdt;
} gdt_descriptor_t;

void load_idt() {
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

static gdt_entry_t gdt_base_entries[16] = {
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

void load_gdt() {
    gdt_descriptor_t desc = {.limit = sizeof(gdt_base_entries) - 1,
                             .gdt = gdt_base_entries};

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

#define print_reg(name, regexpr)                                               \
    printf("\t" name " = %#.16llX", (unsigned long long)(regexpr))

#define test_flag(eflags, bit) ((unsigned)(bool)((eflags) & (1 << (bit))))

void print_ucontext(const ucontext_t *context) {
    print_reg("RAX", context->gregs[0]);
    print_reg("R8 ", context->gregs[8]);
    printf("\r\n");
    print_reg("RCX", context->gregs[1]);
    print_reg("R9 ", context->gregs[9]);
    printf("\r\n");
    print_reg("RDX", context->gregs[2]);
    print_reg("R10", context->gregs[10]);
    printf("\r\n");
    print_reg("RBX", context->gregs[3]);
    print_reg("R11", context->gregs[11]);
    printf("\r\n");
    print_reg("RSP", context->gregs[4]);
    print_reg("R12", context->gregs[12]);
    printf("\r\n");
    print_reg("RBP", context->gregs[5]);
    print_reg("R13", context->gregs[13]);
    printf("\r\n");
    print_reg("RSI", context->gregs[6]);
    print_reg("R14", context->gregs[14]);
    printf("\r\n");
    print_reg("RDI", context->gregs[7]);
    print_reg("R15", context->gregs[15]);
    printf("\r\n");
    printf("\r\n");
    print_reg("RIP", context->rip);
    printf("\r\n");
    print_reg("RFLAGS", context->rflags);

    printf(
        "\r\n\t[CF=%X, PF=%X, AF=%X, ZF=%X, SF=%X, TF=%X, IF=%X, DF=%X, OF=%X, "
        "IOPL=%X, NT=%X, AC=%X, ID=%X]\r\n\r\n",
        test_flag(context->rflags, 0), test_flag(context->rflags, 2),
        test_flag(context->rflags, 4), test_flag(context->rflags, 6),
        test_flag(context->rflags, 7), test_flag(context->rflags, 8),
        test_flag(context->rflags, 9), test_flag(context->rflags, 10),
        test_flag(context->rflags, 11),
        (unsigned)((context->rflags >> 12) & 0x3),
        test_flag(context->rflags, 14), test_flag(context->rflags, 18),
        test_flag(context->rflags, 21));

    printf("\tES = %#.4X\tCS = %#.4X [CPL = %X]\r\n", context->sregs[0],
           context->sregs[1], context->sregs[1] & 0x3);
    printf("\tDS = %#.4X\tSS = %#.4X\r\n", context->sregs[2],
           context->sregs[3]);
}

enum exception_id : int {
    EXCEPT_DE = 0,
    EXCEPT_DB = 1,
    EXCEPT_NMI = 2,
    EXCEPT_BP = 3,
    EXCEPT_OF = 4,
    EXCEPT_BR = 5,
    EXCEPT_UD = 6,
    EXCEPT_NM = 7,
    EXCEPT_DF = 8,
    EXCEPT_MP = 9,
    EXCEPT_TS = 10,
    EXCEPT_NP = 11,
    EXCEPT_SS = 12,
    EXCEPT_GP = 13,
    EXCEPT_PF = 14,
    EXCEPT_MF = 16,
    EXCEPT_AC = 17,
    EXCEPT_MC = 18,
    EXCEPT_XM = 19,
    EXCEPT_VE = 20,
    EXCEPT_CP = 21,
    EXCEPT_HV = 28,
    EXCEPT_VC = 29,
    EXCEPT_SC = 30,
};

#define EXCEPTION_NAME_ID(name)                                                \
    case EXCEPT_##name:                                                        \
        return #name

const char *exception_name(enum exception_id id) {
    switch (id) {
        EXCEPTION_NAME_ID(DE);
        EXCEPTION_NAME_ID(DB);
        EXCEPTION_NAME_ID(NMI);
        EXCEPTION_NAME_ID(BP);
        EXCEPTION_NAME_ID(OF);
        EXCEPTION_NAME_ID(BR);
        EXCEPTION_NAME_ID(UD);
        EXCEPTION_NAME_ID(NM);
        EXCEPTION_NAME_ID(MP);
        EXCEPTION_NAME_ID(DF);
        EXCEPTION_NAME_ID(TS);
        EXCEPTION_NAME_ID(NP);
        EXCEPTION_NAME_ID(SS);
        EXCEPTION_NAME_ID(GP);
        EXCEPTION_NAME_ID(PF);
        EXCEPTION_NAME_ID(MF);
        EXCEPTION_NAME_ID(AC);
        EXCEPTION_NAME_ID(MC);
        EXCEPTION_NAME_ID(XM);
        EXCEPTION_NAME_ID(VE);
        EXCEPTION_NAME_ID(CP);
        EXCEPTION_NAME_ID(HV);
        EXCEPTION_NAME_ID(VC);
        EXCEPTION_NAME_ID(SC);
    default:
        return nullptr;
    }
}

[[gnu::used]]
ucontext_t *handle_int(ucontext_t *context, int irq) {
    const char *name = exception_name(irq);
    if (name)
        printf("Got Exception #%s\r\n", name);
    else
        printf("Got Interrupt %X)\r\n", irq);

    print_ucontext(context);

    if (irq == EXCEPT_BP && (context->sregs[1] & 3) == 0 &&
        (size_t)context->gregs[3] == DEBUG_MAGIC &&
        (size_t)context->gregs[15] == DEBUG_MAGIC2) {
        printf("\r\n");
        printf("Debug Trap from %s (%X)\r\n", context->gregs[6],
               context->gregs[1]);
        printf("Error Code: %r.\r\n", (ptrdiff_t)context->gregs[7]);
        printf("Function %s (%p)\r\n\r\n", context->gregs[2],
               context->gregs[0]);
    }

    if (irq == 0x20) {
        context->gregs[0] = "Hello from Beyond the Interrupt!";
    }

    return context;
}

[[gnu::used]]
ucontext_t *handle_int_with_code(ucontext_t *context, int irq, long errcode) {
    const char *name = exception_name(irq);
    if (name)
        printf("Got Exception #%s (err code %lX)\r\n", name, errcode);
    else
        printf("Got Interrupt %.2X (err code %lX)\r\n", irq, errcode);

    print_ucontext(context);

    if (irq == EXCEPT_PF) {
        void *cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

        char flags[17] = "G--------SKIRUWP";

        for (size_t n = 0; n < 32; n++)
            if (!test_flag(errcode, n))
                flags[15 - n] = '-';

        printf("Page Fault CR2=%p, ERR=[%S]\r\n", cr2, STRING(flags));
        hcf(-1, CURRENT());
    }

    return context;
}

extern void init_context(kcontext_t *ctx);

[[noreturn]]
extern void kmain(int argc, char *argv[], char *envp[], auxv_t auxv[],
                  void *base_addr) {
    for (auxv_t *auxv_ent = auxv; auxv_ent->a_type != AT_NULL; auxv_ent++) {
        if (auxv_ent->a_type != AT_IGNORE)
            __auxent[auxv_ent->a_type - 2] = auxv_ent->a_un;
    }

    framebuffer *fb = (framebuffer *)getauxval(AT_KXINIX_FRAMEBUFFER).a_ptr;

    load_idt();

    load_gdt();

    video_mode *fb_mode = &fb->modes[0];
    // Pick the highest-resolution mode we can find that flanterm will
    // understand Note: this doesn't actually work right now because we don't
    // change the framebuffer size, and that causes issues.
    // TODO: Figure out how to do resizing correctly
    // TODO 2: abstract the terminal into a driver
    for (int i = 0; i < fb->mode_count; i++) {
        video_mode *test_mode = &fb->modes[i];
        if (test_mode->pitch >= test_mode->width * 4) {
            if ((fb_mode == NULL || test_mode->width * test_mode->height >=
                                        fb_mode->width * fb_mode->height) &&
                test_mode->width <= 1920 && test_mode->height <= 1200) {
                fb_mode = test_mode;
            }
        }
    }

    init_heap();

    // clang-format off
    struct flanterm_context *ft_ctx = flanterm_fb_init(
        malloc,
        nullptr, // free is unneeded and unused if fb init succeedes
        fb->address, fb_mode->width, fb_mode->height, fb_mode->pitch,
        fb_mode->red_mask_size, fb_mode->red_mask_shift,
        fb_mode->green_mask_size, fb_mode->green_mask_shift,
        fb_mode->blue_mask_size, fb_mode->blue_mask_shift,
        nullptr, // background image (currently unused)
        nullptr, nullptr, // ANSI color config (using default)
        nullptr, nullptr, // default foreground/background colors (using default)
        nullptr, nullptr, // default bright foreground/background color (using default)
        nullptr, 0, 0, 1, // font and options (using default)
        1, 1, // font scale settings (choosing to fit more text)
        0, // margin (currently using no margin; may change)
        0, // rotation (don't rotate)
        true
    );
    // clang-format on

    FILE stdout_fd = (FILE){};

    stdout = &stdout_fd;
    stdout->data = ft_ctx;
    stdout->write = stdout_handler;

    const char msg[] =
        "Xinix Version 0.0.0\r\n(that's right, even less than 0.0.1)\r\n\r\n";
    flanterm_write(ft_ctx, msg, sizeof(msg));

    char *alloc_test = malloc(40);
    memcpy(alloc_test, "Did malloc work?\r\nOf course it did :D\r\n", 40);
    flanterm_write(ft_ctx, alloc_test, 40);

    printf("Address of printf is %#.16llX\r\n\r\n", printf);

    printf("Feature flags: ");
    bool want_comma = false;
    x86_enumerate_supported_features(print_feature_flag, &want_comma);
    printf("\r\n");

    printf("Feature array:");
    for (int i = 0; i < 38; i++) {
        if (i & 7)
            printf(" ");
        else
            printf("\r\n");
        printf("%08X", x86_feature_array[i]);
    }
    printf("\r\n\r\n");

    kcontext_t *ctx = calloc(1, sizeof(kcontext_t));
    if (!ctx) {
        printf("Error allocating initial core kcontext\r\n");
        hcf(ERR_GENERIC, CURRENT());
    }
    ucontext_t *tctx = aligned_alloc(alignof(ucontext_t), sizeof(ucontext_t));
    if (!tctx) {
        printf("Error allocating initial kernel thread context");
        hcf(ERR_GENERIC, CURRENT());
    }
    memset(tctx, 0, sizeof(ucontext_t));
    random_generator *gen =
        aligned_alloc(alignof(random_generator), sizeof(random_generator));
    *gen = RAND_GEN_STATIC_INIT;
    tctx->xsave_size = FXSAVE_SIZE; // Uncomment when we turn on cr4.fxsr
    ctx->total_context_size = sizeof(kcontext_t);
    ctx->self = ctx;
    ctx->current_thread = tctx;
    const uint8_t *at_rand = getauxval(AT_RANDOM).a_ptr;
    ctx->kgen = RAND_GEN_STATIC_INIT;
    if (at_rand)
        rand_ingest(&ctx->kgen, at_rand);
    else
        rand_init(&ctx->kgen);

    uint8_t grand[16];
    rand_poll(&ctx->kgen, grand);
    rand_ingest(gen, grand);

    tctx->urand_gen = gen;

    init_context(ctx);

    kcontext_t *cval = getcontext();
    printf("Kernel Context is: %p\r\n", cval);
    printf("Thread Context is: %p\r\n", cval->current_thread);

    // Test IDT
    char *intr_msg = nullptr;
    __asm__ volatile(
        "int $0x20"
        : "=a"(intr_msg)); // int3 is intercepted by qemu in debug mode

    printf("\r\n%s\r\n", intr_msg);

    // Print physical memory layout
    printf("HHDM offset: %#.16llX\r\n", getauxval(AT_KXINIX_HHDM_OFFSET).a_val);
    printf("Physical memory:\r\n");
    memmap *memory_map = getauxval(AT_KXINIX_MEMMAP).a_ptr;
    for (int i = 0; i < memory_map->entry_count; i++) {
        printf("%#.16llX--%#.16llX => %s\r\n", memory_map->entries[i].base,
               memory_map->entries[i].base - 1 + memory_map->entries[i].length,
               memmap_type_name(memory_map->entries[i].type));
    }
    printf("\r\n");

    clone_page_table();

    printf("\r\n");

    // Set up the dynamic loader, finally
    // TODO: should happen way earlier. Currently only this late due to some bad
    // circular dependencies.
    Elf64_Ehdr *ehdr = base_addr;

    Elf64_Phdr *phdrs = (Elf64_Phdr *)(((uintptr_t)base_addr) + ehdr->e_phoff);
    size_t phnum = ehdr->e_phnum;

    auto res =
        dynld_link(base_addr, _DYNAMIC, phdrs, phnum, "/xinix-kernel.so", true);

    if (SYSRESULT2_CODE(res) < 0)
        hcf(SYSRESULT2_CODE(res), CURRENT());

    auto self = SYSRESULT2_VALUE(res, struct DynLibraryEntry *);
    if (self->dylib_jmprel_type == DT_RELA && self->dylib_plt_rela) {
        size_t count = self->dylib_pltrelsz / sizeof(ElfNative_Rela);
        printf("Got extraneous relas\r\n");
        for (auto ptr = self->dylib_plt_rela;
             ptr != (self->dylib_plt_rela) + count; ptr++) {
            void *offset = ((char *)self->dylib_base) + ptr->r_offset;
            ElfNative_Reloc relty = ELFNATIVE_R_TYPE(ptr->r_info);
            size_t syme = ELFNATIVE_R_SYM(ptr->r_info);
            const ElfNative_Sym *sym = &self->dylib_symtab[syme];
            const char *name = self->dylib_strtab + sym->st_name;

            const char *relname = rel_describe(relty);
            if (relname) {
                printf("\tRELA %s (%s + %#zx): %p\r\n", relname, name,
                       ptr->r_addend, offset);
            } else {
                printf("\tRELA <unknown type %x> (%s + %#zx): %p\r\n",
                       (unsigned int)relty, name, ptr->r_addend, offset);
            }
        }

        printf("\r\n");
    }

    load_system_descriptor_tables();

    union {
        uint8_t buf[16];
        unsigned long long r[16 / sizeof(unsigned long long)];
    } rand_bytes = {};

    random_kglobal_gen(rand_bytes.buf);

    printf("KContext Random Numbers: %.16llX:%.16llX\r\n", rand_bytes.r[0],
           rand_bytes.r[1]);

    random_global_gen(rand_bytes.buf);

    printf("Random Numbers: %.16llX:%.16llX\r\n", rand_bytes.r[0],
           rand_bytes.r[1]);

    hcf(0, CURRENT());
}
