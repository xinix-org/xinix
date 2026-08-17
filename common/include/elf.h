#pragma once

#include "bits/feat_test.h"
#include <stdbit.h>
#include <stddef.h>
#include <stdint.h>

constexpr char ELFMAGIC[4] = "\x7F"
                             "ELF";

enum ElfClass : uint8_t {
    ELFCLASSNONE = 0,
    ELFCLASS32 = 1,
    ELFCLASS64 = 2,
#if INTPTR_WIDTH == 32
    ELFCLASSNATIVE = ELFCLASS32
#else
    ELFCLASSNATIVE = ELFCLASS64
#endif
};

enum ElfData : uint8_t {
    ELFDATANONE = 0,
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2,
#if __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_LITTLE__
    ELFDATANATIVE = ELFDATA2LSB
#elif __STDC_ENDIAN_NATIVE__ == __STDC_ENDIAN_BIG__
    ELFDATANATIVE = ELFDATA2MSB
#endif
};

enum ElfVersion : uint8_t {
    EV_NONE = 0,
    EV_CURRENT = 1,
};

enum ElfOsAbi : uint8_t {
    ELFOSABINONE = 0,
};

typedef struct {
    char ei_magic[4];
    enum ElfClass ei_class;
    enum ElfData ei_data;
    enum ElfVersion ei_version;
    enum ElfOsAbi ei_osabi;
    uint8_t ei_abiversion;
    uint8_t ei_pad[16 - 9];
} Elf_Ident;

static inline long elf_validate_ident(const Elf_Ident *_e_ident,
                                      enum ElfClass req_class,
                                      enum ElfData req_data,
                                      enum ElfOsAbi supabi) _ATTRIBUTE_UNSEQ {
    if (_e_ident->ei_magic[0] != ELFMAGIC[0] ||
        _e_ident->ei_magic[1] != ELFMAGIC[1] ||
        _e_ident->ei_magic[2] != ELFMAGIC[2] ||
        _e_ident->ei_magic[3] != ELFMAGIC[3])
        return -32;
    else if (_e_ident->ei_class != req_class || _e_ident->ei_data != req_data)
        return -32;
    else if (_e_ident->ei_version != EV_CURRENT)
        return -32;
    else if (_e_ident->ei_osabi != ELFOSABINONE || _e_ident->ei_osabi != supabi)
        return -32;
    else
        for (size_t n = 0; n < sizeof(_e_ident->ei_pad); n++)
            if (_e_ident->ei_pad[n] != 0)
                return -32;

    return 0;
}

static inline long
elf_validate_ident_native(const Elf_Ident *_e_ident,
                          enum ElfOsAbi supabi) _ATTRIBUTE_UNSEQ {
    return elf_validate_ident(_e_ident, ELFCLASSNATIVE, ELFDATANATIVE, supabi);
}

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Size;
typedef int64_t Elf64_Off;

typedef uint8_t Elf64_Byte;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int8_t Elf64_Sbyte;
typedef int16_t Elf64_Shalf;
typedef int32_t Elf64_Sword;
typedef int64_t Elf64_Sxword;

enum Elf_Machine : uint16_t {
    EM_386 = 3,
    EM_X86_64 = 62,
#define ELF_WANT_NATIVE_MACHINE
#include <bits/elf_native.h>
#undef ELF_WANT_NATIVE_MACHINE
};

enum Elf_Type : uint16_t {
    ET_NONE = 0,
    ET_REL = 1,
    ET_EXEC = 2,
    ET_DYN = 3,
    ET_CORE = 4,
    ET_LOOS = 0xfe00,
    ET_HIOS = 0xfeff,
    ET_LOPROC = 0xff00,
    ET_HIPROC = 0xffff,
};

typedef struct {
    Elf_Ident e_ident;
    enum Elf_Type e_type;
    enum Elf_Machine e_machine;
    uint32_t e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    Elf64_Word sh_name;
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Word sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

enum Elf_SymBind : uint8_t {
    STB_LOCAL = 0,
    STB_GLOBAL = 0x1,
    STB_WEAK = 0x2,

    STB_LOOS = 0xA,
    STB_HIOS = 0xC,
    STB_LOPROC = 0xD,
    STB_HIPROC = 0xF,
};

enum Elf_SymType : uint8_t {
    STT_NOTYPE = 0,
    STT_OBJECT = 1,
    STT_FUNCTION = 2,
    STT_SECTION = 3,
    STT_FILE = 4,
    STT_COMMON = 5,
    STT_TLS = 6,
    STT_LOOS = 10,
    STT_GNU_IFUNC = 10,
    STT_HIOS = 12,
    STT_LOPROC = 13,
    STT_HIPROC = 15,
};

enum Elf_SymVisibility : uint8_t {
    STV_DEFAULT = 0,
    STV_INTERNAL = 1,
    STV_HIDDEN = 2,
    STV_PROTECTED = 3,
};

typedef struct {
    Elf64_Word st_name;
    uint8_t st_info;
#define ST_TYPE(sym) ((enum Elf_SymType)((sym).st_info & 0xF))
#define ST_BIND(sym) ((enum Elf_SymBind)((sym).st_info >> 4))
    uint8_t st_other;
#define ST_VISIBILITY(sym) ((enum Elf_SymVisibility)((sym).st_other & 0x3))
    Elf64_Half st_shndx;
    Elf64_Addr st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

typedef struct {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
} Elf64_Rel;

typedef struct {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;

#define XINIX_ELF_H_WANT_RELOCATIONS 1

#include <bits/elf_relocations.h>

#undef XINIX_ELF_H_WANT_RELOCATIONS

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffffL)
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t) & 0xffffffffL))

enum Elf_PhType : uint32_t {
    PT_NULL = 0,
    PT_LOAD = 1,
    PT_DYNAMIC = 2,
    PT_INTERP = 3,
    PT_NOTE = 4,
    PT_PHDR = 6,
    PT_TLS = 7,

    PT_LOOS = 0x60000000,
    PT_GNU_STACK = 0x6474e551,
    PT_GNU_RELRO = 0x6474e552,
    PT_HIOS = 0x6fffffff,

    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7fffffff,
};

enum Elf_PhFlags : uint32_t {
    PF_X = 0x01,
    PF_W = 0x02,
    PF_R = 0x04,
    PF_MASKOS = 0x0FF00000,
    PF_MASKPROC = 0xF0000000,
};

typedef struct {
    enum Elf_PhType p_type;
    enum Elf_PhFlags p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

enum Elf_DynTag : uint32_t {
    DT_NULL = 0,
    DT_NEEDED = 1,
    DT_PLTRELSZ = 2,
    DT_PLTGOT = 3,
    DT_HASH = 4,
    DT_STRTAB = 5,
    DT_SYMTAB = 6,
    DT_RELA = 7,
    DT_RELASZ = 8,
    DT_RELAENT = 9,
    DT_STRSZ = 10,
    DT_SYMENT = 11,
    DT_INIT = 12,
    DT_FINI = 13,
    DT_SONAME = 14,
    DT_RPATH = 15,
    DT_SYMBOLIC = 16,
    DT_REL = 17,
    DT_RELSZ = 18,
    DT_RELENT = 19,
    DT_PLTREL = 20,
    DT_DEBUG = 21,
    DT_TEXTREL = 22,
    DT_JMPREL = 23,
    DT_BIND_NOW = 24,
    DT_INIT_ARRAY = 25,
    DT_FINI_ARRAY = 26,
    DT_INIT_ARRAYSZ = 27,
    DT_FINI_ARRAYSZ = 28,
    DT_RUNPATH = 29,
    DT_FLAGS = 30,
#define DT_ENCODING 32
    DT_PREINIT_ARRAY = 32,
    DT_PREINIT_ARRAYSZ = 33,

    DT_RELRSZ = 35,
    DT_RELR = 36,
    DT_RELRENT = 37,
    DT_SYMTABSZ = 39,
#define DT_LOOS 0x6000000D
#define DT_HIOS 0x6ffff000
    DT_GNU_HASH = 0x6ffffef5,
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff
};



typedef struct {
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    };
} Elf64_Dyn;

#if INTPTR_WIDTH == 64
typedef Elf64_Dyn ElfNative_Dyn;
typedef Elf64_Ehdr ElfNative_Ehdr;
typedef Elf64_Phdr ElfNative_Phdr;
typedef Elf64_Sym ElfNative_Sym;
typedef Elf64_Rela ElfNative_Rela;
typedef Elf64_Rel ElfNative_Rel;
typedef Elf64_Addr ElfNative_Addr;
typedef Elf64_Sxword ElfNative_Offset;

#define ELFNATIVE_R_SYM(i) ELF64_R_SYM(i)
#define ELFNATIVE_R_TYPE(i) (ElfNative_Reloc)(ELF64_R_TYPE(i))
#endif
