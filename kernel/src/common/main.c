#include "arch.h"
#include "clock.h"
#include "cpu.h"

#include "debugging.h"
#include "event.h"
#include "keyboard.h"

#include "random.h"

#include "sysresult.h"
#include "time.h"
#include "uuid.h"
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
#include <stdatomic.h>
#include <stdio.h>

#include <auxfuncs.h>
#include <exceptions.h>
#include <irq.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <location.h>
#include <strslice.h>
#include <thread.h>

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

void keyboard_irq(void) {
    kcontext_t* ctx = getcontext();
    int scancode = inb(0x60);
    kbd_process_scancode_byte(scancode);
    ctx->lapic->end_of_interrupt_register.value = 0;
}

void isr_increment_boottime(uint64_t dt_micros);

ucontext_t *timer_irq(ucontext_t *context) {
    kcontext_t* ctx = getcontext();

    duration_t dur = read_tsc();

    duration_t step = duration_sub(dur, ctx->last_timer_tsc);

    if (step.time_seconds < 0)
        {
            // Degenerate case, but don't be unsound
            step = (duration_t){};
        }

    ctx->last_timer_tsc = dur;

    push_event(TIMER(step.time_nanos & 0xFFFFFF));

    if(ctx->is_root_context) {
        uint64_t micros = (step.time_seconds * 1'000'000) + (step.time_nanos / 1'000);

        isr_increment_boottime(micros);
    }

    ctx->lapic->end_of_interrupt_register.value = 0;
    return context;
}

[[gnu::used]]
ucontext_t *handle_int(ucontext_t *context, int irq) {
    if (irq == IRQ_KB) {
        keyboard_irq();
        return context;
    } else if (irq == IRQ_TIMER) {
        printf("Got timer interrupt\r\n");
        return timer_irq(context);
    }
    const char *name = exception_name(irq);
    if (name)
        printf("Got Exception #%s\r\n", name);
    else
        printf("Got Interrupt %X)\r\n", irq);

    print_ucontext(context);

    if (irq == EXCEPT_BP && (context->sregs[1] & 3) == 0) {

        if ((size_t)context->gregs[3] == DEBUG_MAGIC &&
            (size_t)context->gregs[15] == DEBUG_MAGIC2) {
            printf("\r\n");
            printf("Debug Trap from %s (%X)\r\n", context->gregs[6],
                   context->gregs[1]);
            printf("Error Code: %r.\r\n", (ptrdiff_t)context->gregs[7]);
            printf("Function %s (%p)\r\n\r\n", context->gregs[2],
                   context->gregs[0]);
        }
        handle_kernel_debug(context, true);
    } else if (irq == EXCEPT_DB) {
        if (!((uintptr_t)context->dregs[4] & (1 << 11))) {
            context->dregs[4] =
                (void *)(((uintptr_t)context->dregs[4]) | (1 << 11));
        } else {
            handle_kernel_debug(context, false);
        }
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
            if (!(errcode & (1L << n)))
                flags[15 - n] = '-';
        printf("Page Fault CR2=%p, ERR=[%S]\r\n", cr2, STRING(flags));
        hcf(-1, CURRENT());
    }

    return context;
}

extern void init_context(kcontext_t *ctx);

extern void init_cpuid_array();

void install_keyboard_irq(void) {
    write_io_redirect(1, IRQ_KB, IOREDTBL_DELIVERY_FIXED, false, false, false,
                      0);
}


void setup_timer(void) {
    auto ctx = getcontext();

    auto lapic = ctx->lapic;

    lapic->divide_configuration_register.value = 0x3;
    lapic->initial_count_register.value = 0xFFFF;
    lapic->lvt_timer_register.value = (2 << 17) | IRQ_TIMER;
    lapic->spurious_interrupt_vector_register.value = IRQ_SPURIOUS;
}

void init_tsc(void);

[[noreturn]]
extern void kmain(int argc, char *argv[], char *envp[], auxv_t auxv[],
                  void *base_addr) {
    for (auxv_t *auxv_ent = auxv; auxv_ent->a_type != AT_NULL; auxv_ent++) {
        if (auxv_ent->a_type != AT_IGNORE)
            __auxent[auxv_ent->a_type - 2] = auxv_ent->a_un;
    }

    init_cpuid_array();

    framebuffer *fb = (framebuffer *)getauxval(AT_KXINIX_FRAMEBUFFER).a_ptr;

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
    load_arch_state();

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

    kcontext_t *ctx = calloc(1, sizeof(kcontext_t));
    if (!ctx) {
        printf("Error allocating initial core kcontext\r\n");
        // hcf(ERR_GENERIC, CURRENT());
        for(;;)
            __asm__ volatile("hlt");
    }
    ucontext_t *tctx = aligned_alloc(alignof(ucontext_t), sizeof(ucontext_t));
    if (!tctx) {
        printf("Error allocating initial kernel thread context");
        // hcf(ERR_GENERIC, CURRENT());
        for(;;)
            __asm__ volatile("hlt");
    }
    memset(tctx, 0, sizeof(ucontext_t));

    random_generator *gen =
        aligned_alloc(alignof(random_generator), sizeof(random_generator));
    *gen = RAND_GEN_STATIC_INIT;
    tctx->xsave_size = FXSAVE_SIZE; // Uncomment when we turn on cr4.fxsr
    ctx->total_context_size = sizeof(kcontext_t);
    ctx->self = ctx;
    ctx->current_thread = tctx;
    ctx->is_root_context = true; // Only true on the first thread
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

    init_tsc();

    ctx->last_timer_tsc = read_tsc();

    // Print physical memory layout
    printf("HHDM offset: %#.16lX\r\n", getauxval(AT_KXINIX_HHDM_OFFSET).a_val);
    printf("Physical memory:\r\n");
    memmap *memory_map = getauxval(AT_KXINIX_MEMMAP).a_ptr;
    for (int i = 0; i < memory_map->entry_count; i++) {
        printf("%#.16llX--%#.16llX => %s\r\n", memory_map->entries[i].base,
               memory_map->entries[i].base - 1 + memory_map->entries[i].length,
               memmap_type_name(memory_map->entries[i].type));
    }
    printf("\r\n");

    clone_page_table();

    save_full_ucontext(cval->current_thread);

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

    // struct process* kernel = aligned_alloc(alignof(struct process), sizeof(struct thread));

    // struct thread* thread = aligned_alloc(alignof(struct thread), sizeof(struct thread));

    // kernel->proc_owner = FULL_UUID;

    // thread->thrd_uctx = tctx;
    // thread->thrd_is_kernel = true;
    // thread->thrd_owner = FULL_UUID;
    // thread->thrd_cmono = (duration_t){};
    // thread->thrd_last_tsc = (duration_t){};
    // thread->thd_proc = kernel;

    // tctx->tdata = thread;

    // struct thread** threads = calloc(32, sizeof(struct thread*));
    // threads[0] = thread;
    // kernel->proc_threads = threads;
    
    load_system_descriptor_tables();

    ctx->lapic->task_priority_register.value = 0;
    ctx->lapic->destination_format_register.value = 0xFF000000;
    install_keyboard_irq();
    setup_timer();

    // Enable interrupts; should be abstracted out
    __asm__ volatile("sti");

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

    init_rtc();

    printf("prompt> ");
    while (1) {
        kbd_scan_code_t scan_code = kbd_poll_key();
        if (scan_code == 0) {
            spin_loop_hint();
            continue;
        }
        if (scan_code & KEY_SCAN_RELEASE) {
            continue; // ignore keyup
        }
        if (scan_code == KEY_SCAN_BACKSPACE) {
            printf("\x08 \x08"); // terrible backspace handling; TODO do a line
                                 // buffer
        } else if (scan_code == KEY_SCAN_ENTER) {
            printf("\r\n");
        } else {
            char ch = kbd_get_char_for_scancode(scan_code);
            if (ch) {
                printf("%c", ch);
            }
        }
    }
    // hcf(0, CURRENT());
}
