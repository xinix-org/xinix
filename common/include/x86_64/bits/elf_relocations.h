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

static inline const char* rel_describe(enum X86_64_Reloc rel) {
    switch(rel) {

    case R_X86_64_NONE: return "R_X86_64_NONE";
    case R_X86_64_64: return "R_X86_64_64";
    case R_X86_64_PC32: return "R_X86_64_PC32";
    case R_X86_64_GOT32: return "R_X86_64_GOT32";
    case R_X86_64_PLT32: return "R_X86_64_PLT32";
    case R_X86_64_COPY: return "R_X86_64_COPY";
    case R_X86_64_GLOB_DAT: return "R_X86_64_GLOB_DAT";
    case R_X86_64_JUMP_SLOT: return "R_X86_64_JUMP_SLOT";
    case R_X86_64_RELATIVE: return "R_X86_64_RELATIVE";
    case R_X86_64_GOTPCREL: return "R_X86_64_GOTPCREL";
    case R_X86_64_32: return "R_X86_64_32";
    case R_X86_64_32S: return "R_X86_64_32S";
    case R_X86_64_16: return "R_X86_64_16";
    case R_X86_64_PC16: return "R_X86_64_PC16";
    case R_X86_64_8: return "R_X86_64_8";
    case R_X86_64_PC8: return "R_X86_64_PC8";
    case R_X86_64_DTPMOD64: return "R_X86_64_DTPMOD64";
    case R_X86_64_DTPOFF64: return "R_X86_64_DTPOFF64";
    case R_X86_64_TPOFF64: return "R_X86_64_TPOFF64";
    case R_X86_64_TLSGD: return "R_X86_64_TLSGD";
    case R_X86_64_TLSLD: return "R_X86_64_TLSLD";
    case R_X86_64_DTPOFF32: return "R_X86_64_DTPOFF32";
    case R_X86_64_GOTTPOFF: return "R_X86_64_GOTTPFOFF";
    case R_X86_64_TPOFF32: return "R_X86_64_TPOFF32";
    case R_X86_64_PC64: return "R_X86_64_PC64";
    case R_X86_64_GOTOFF64: return "R_X86_64_GOTOFF64";
    case R_X86_64_GOTPC32: return "R_X86_64_GOTPC32";
    case R_X86_64_SIZE32: return "R_X86_64_SIZE32";
    case R_X86_64_SIZ64: return "R_X86_64_SIZE64";
    case R_X86_64_GOTPC32_TLSDESC: return "R_X86_64_GOTPC32_TLSDESC";
    case R_X86_64_TLSDESC_CALL: return "R_X86_64_TLSDESC_CALL";
    case R_X86_64_TLSDESC: return "R_X86_64_TLSDESC";
    case R_X86_64_IRELATIVE: return "R_X86_64_IRELATIVE";
    default: return nullptr;
    }
}

#define DREL_JUMP_SLOT R_X86_64_JUMP_SLOT
#define DREL_GLOB_DAT R_X86_64_GLOB_DAT
#define DREL_COPY R_X86_64_COPY
#define DREL_RELATIVE R_X86_64_RELATIVE
#define DREL_IRELATIVE R_X86_64_IRELATIVE

#define DREL_TLSDESC R_X86_64_TLSDESC
#define DREL_DTPMOD R_X86_64_DTPMOD64
#define DREL_DTPOFF R_X86_64_DTPMOD64

typedef enum X86_64_Reloc ElfNative_Reloc;