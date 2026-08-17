// #pragma once intentionally omitted

#include <stdint.h>

#ifndef XINIX_ELF_H_WANT_RELOCATIONS
#error Cannot include this file directly, include <elf.h> instead
#endif

enum X86_64_Reloc : uint32_t {
    R_X86_64_NONE = 0,
    R_X86_64_64 = 1,
    R_X86_64_PC32 = 2,
    R_X86_64_GOT32 = 3,
    R_X86_64_PLT32 = 4,
    R_X86_64_COPY = 5,
    R_X86_64_GLOB_DAT = 6,
    R_X86_64_JUMP_SLOT = 7,
    R_X86_64_RELATIVE = 8,
    R_X86_64_GOTPCREL = 9,
    R_X86_64_32 = 10,
    R_X86_64_32S = 11,
    R_X86_64_16 = 12,
    R_X86_64_PC16 = 13,
    R_X86_64_8 = 14,
    R_X86_64_PC8 = 15,
    R_X86_64_DTPMOD64 = 16,
    R_X86_64_DTPOFF64 = 17,
    R_X86_64_TPOFF64 = 18,
    R_X86_64_TLSGD = 19,
    R_X86_64_TLSLD = 20,
    R_X86_64_DTPOFF32 = 21,
    R_X86_64_GOTTPOFF = 22,
    R_X86_64_TPOFF32 = 23,
    R_X86_64_PC64 = 24,
    R_X86_64_GOTOFF64 = 25,
    R_X86_64_GOTPC32 = 26,
    R_X86_64_SIZE32 = 32,
    R_X86_64_SIZ64 = 33,
    R_X86_64_GOTPC32_TLSDESC = 34,
    R_X86_64_TLSDESC_CALL = 35,
    R_X86_64_TLSDESC = 36,
    R_X86_64_IRELATIVE = 37,
};

#define DREL_JUMP_SLOT R_X86_64_JUMP_SLOT
#define DREL_GLOB_DAT R_X86_64_GLOB_DAT
#define DREL_COPY R_X86_64_COPY
#define DREL_RELATIVE R_X86_64_RELATIVE
#define DREL_IRELATIVE R_X86_64_IRELATIVE

#define DREL_TLSDESC R_X86_64_TLSDESC
#define DREL_DTPMOD R_X86_64_DTPMOD64
#define DREL_DTPOFF R_X86_64_DTPMOD64

typedef enum X86_64_Reloc ElfNative_Reloc;