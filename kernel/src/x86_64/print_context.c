#include "usercontext.h"
#include <context.h>
#include <gdt.h>
#include <stdio.h>

#define print_reg(name, regexpr)                                               \
    printf("\t" name " = %#.16llX", (unsigned long long)(regexpr))

#define test_flag(eflags, bit) ((unsigned)(bool)((eflags) & (1ul << (bit))))

static void print_sreg(const char *name, uint16_t sreg, uint16_t ldtr) {
    auto cpl = sreg & 3;
    auto offset = sreg >> 3;
    gdt_entry_t *ent;
    if (!test_flag(sreg, 2)) {
        ent = &gdt_entries[offset];
    } else {
        uint64_t ldt_base =
            ((uint64_t)gdt_entries[ldtr].base_lo) |
            (((uint64_t)gdt_entries[ldtr].base_mid) << 16) |
            (((uint64_t)gdt_entries[ldtr].base_hi) << 24) |
            (((uint64_t)gdt_entries[ldtr + 1].sys_base_ext) << 32);
        gdt_entry_t *ldt = (gdt_entry_t *)ldt_base;
        ent = &ldt[offset];
    }

    uint32_t limit = ((uint32_t)ent->limit_lo) |
                     ((uint32_t)(ent->flags_and_limit_hi & 0xF)) << 16;

    uint64_t base = ((uint32_t)ent->base_lo) |
                    (((uint32_t)ent->base_mid) << 16) |
                    (((uint32_t)ent->base_hi) << 24);

    if ((ent->flags_and_limit_hi & GDT_Granularity))
        limit = (limit << 12) | 0xFFF;

    auto sys = test_flag(ent->access, 4);
    auto dpl = (ent->access >> 5) & 3;
    auto present = test_flag(ent->access, 7);

    if (!sys) {
        base |= (uint64_t)(ent[1].sys_base_ext) << 32;
        const char *segty;
        switch ((enum system_segment_type)ent->access & 0xF) {
        case LDT:
            segty = "LDT";
            break;
        case TSS:
            segty = "TSS (available)";
            break;
        case TSS_Busy:
            segty = "TSS (busy)";
            break;
        default:
            segty = "(unknown system segment)";
            break;
        }

        printf("\t%s = %.4X [%s, base = %.16llX, limit = %.8X, P = %X, "
               "DPL=%X]\r\n",
               name, sreg, segty, present, dpl);
    } else {
        auto a = test_flag(ent->access, 0);
        auto rw = test_flag(ent->access, 1);
        auto dc = test_flag(ent->access, 2);
        auto x = test_flag(ent->access, 3);

        const char *segty =
            test_flag(ent->flags_and_limit_hi, 5)
                ? "64-bit"
                : (test_flag(ent->flags_and_limit_hi, 6) ? "32-bit" : "16-bit");

        if (x) {
            printf("\t%s = %.4X [%s code, base = %.8lX, limit = %.8X, CPL = "
                   "%x, DPL = %x, P = %X, A = %X, R = %X, C = %X]\r\n",
                   name, sreg, segty, base, limit, cpl, dpl, present, a, rw,
                   dc);
        } else {
            printf("\t%s = %.4X [%s data, base = %.8lX, limit = %.8X, DPL = "
                   "%x, P = %X, A = %X, W = %X, D = %X]\r\n",
                   name, sreg, segty, base, limit, dpl, present, a, rw, dc);
        }
    }
}

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

    print_sreg("ES", context->sregs[0], context->sregs[7]);
    print_sreg("CS", context->sregs[1], context->sregs[7]);
    print_sreg("DS", context->sregs[2], context->sregs[7]);
    print_sreg("SS", context->sregs[3], context->sregs[7]);
    print_sreg("FS", context->sregs[4], context->sregs[7]);
    print_sreg("GS", context->sregs[5], context->sregs[7]);
    print_sreg("TSS", context->sregs[6], 0);
    print_sreg("LDT", context->sregs[7], 0);

    printf("\r\n");

    printf("\tFS.BASE = %p\tGS.BASE = %p\r\n", context->fsgsbase[0],
           context->fsgsbase[1]);

    printf("\tCR3 = %#.16llX\tCR4 = %#.16llX\r\n", context->cr3, context->cr4);
    printf("\t\tTSD = %X, DE = %X, MCE = %X, PGE = %X, PCE = %X, UIMP = %X, "
           "LA57 = %X\r\n",
           test_flag(context->cr4, 2), test_flag(context->cr4, 3),
           test_flag(context->cr4, 6), test_flag(context->cr4, 7),
           test_flag(context->cr4, 8), test_flag(context->cr4, 11),
           test_flag(context->cr4, 12));
    printf("\t\tVMXE = %X, SMXE = %X, FSGSBASE = %X, PCIDE = %X, SMEP = %X, "
           "SMAP = %X, PKE = %X, CET = %X, PKS = %X\r\n",
           test_flag(context->cr4, 13), test_flag(context->cr4, 14),
           test_flag(context->cr4, 16), test_flag(context->cr4, 17),
           test_flag(context->cr4, 20), test_flag(context->cr4, 21),
           test_flag(context->cr4, 22), test_flag(context->cr4, 23),
           test_flag(context->cr4, 24));

    printf("\t\tFXSR = %X, OSXMMEX = %X, XSAVE = %X\r\n",
           test_flag(context->cr4, 9), test_flag(context->cr4, 10),
           test_flag(context->cr4, 18));

    printf("\tDR0 = %.16p\tDR1 = %.16p\r\n", context->dregs[0],
           context->dregs[1]);
    printf("\tDR2 = %.16p\tDR3 = %.16p\r\n", context->dregs[2],
           context->dregs[3]);
    uintptr_t dr6 = (uintptr_t)context->dregs[4];
    printf("\tDR6 = %#.16lX [BP0 = %X, BP1 = %X, BP2 = %X, BP3 = %X, BLD = %X, "
           "BD = %X, BS = %X, BT = %X, RTM = %X]\r\n",
           dr6, test_flag(dr6, 0), test_flag(dr6, 1), test_flag(dr6, 2),
           test_flag(dr6, 3), test_flag(dr6, 11), test_flag(dr6, 13),
           test_flag(dr6, 14), test_flag(dr6, 15), !test_flag(dr6, 16));
    uintptr_t dr7 = (uintptr_t)context->dregs[5];
    char en[4][3] = {"--", "--", "--", "--"};
    for (size_t n = 0; n < 4; n++) {
        if (test_flag(dr7, 2 * n))
            en[n][0] = 'L';
        if (test_flag(dr7, 2 * n + 1))
            en[n][1] = 'G';
    }

    char cond[4][4] = {};
    constexpr static char width[4] = "1284";
    constexpr static char cval[4][2] = {"IX", "WO", "IO", "RW"};
    for (size_t n = 0; n < 4; n++) {
        auto c = (dr7 >> (16 + 4 * n)) & 0xF;
        cond[n][0] = cval[c & 3][0];
        cond[n][1] = cval[c & 3][1];
        cond[n][2] = width[c >> 2];
    }
    printf("\tDR7 = %#.16lX [RTM = %X]\r\n\t", dr7, test_flag(dr7, 11));
    for (size_t n = 0; n < 4; n++)
        printf("\tBP%x = %s (%s, LOG=%X)", n, cond[n], en[n],
               test_flag(dr7, 32 + n));
    printf("\r\n");

    if (context->xsave_size >= FXSAVE_SIZE) {
        uint16_t fsw = context->fxsave.fsw;
        uint16_t fcw = context->fxsave.fcw;
        uint8_t ftw = context->fxsave.ftw;
        uint32_t mxcsr = context->fxsave.mxcsr;

        constexpr static char legacy_excepts[8] = "ESPUOZDI";

        char legacy_except_status[9] = "--------";
        char legacy_except_mask[7] = "------";
        char mxcsr_except_status[7] = "------";
        char mxcsr_except_mask[7] = "------";

        constexpr static char rounding_modes[4][4] = {"RTN", "R-", "R+", "RTZ"};
        constexpr static char precision_modes[4][4] = {"F32", "", "F64", "X80"};

        for (auto i = 0; i < 8; i++) {
            if (test_flag(fsw, i))
                legacy_except_status[7 - i] = legacy_excepts[7 - i];
            if (i < 6 && test_flag(fcw, i))
                legacy_except_mask[5 - i] = legacy_excepts[7 - i];
            if (i < 6 && test_flag(mxcsr, i))
                mxcsr_except_status[5 - i] = legacy_excepts[7 - i];
            if (i < 6 && test_flag(mxcsr, i + 7))
                mxcsr_except_mask[5 - i] = legacy_excepts[7 - i];
        }

        const char *legacy_rc = rounding_modes[(fcw >> 10) & 3];
        const char *legacy_pc = precision_modes[(fcw >> 8) & 3];

        auto x87_cc = (fsw >> 8) & 43;
        auto x87_top = (fsw >> 11) & 3;

        auto x87_x = test_flag(fcw, 12);
        auto x86_b = test_flag(fsw, 15);

        const char *sse_rc = rounding_modes[(mxcsr >> 13) & 3];

        auto sse_daz = test_flag(mxcsr, 6);
        auto sse_ftz = test_flag(mxcsr, 15);

        printf("\tFSW = %.4X [CC = %.2X, EX = [%s]]\tFCW = %.4X [RC = %s, PC = "
               "%s, EM = [%s]]\tFTW = %.2X\r\n",
               fsw, x87_cc, legacy_except_status, fcw, legacy_rc, legacy_pc,
               legacy_except_mask, ftw);
        printf("\tMXCSR = %.8X [FTZ = %X, DAZ = %X, RC = %s, EM = [%s], EX = "
               "[%s]]\r\n",
               mxcsr, sse_ftz, sse_daz, sse_rc, mxcsr_except_mask,
               mxcsr_except_status);
    }
}
