#pragma once

#include "sysresult.h"
#include <auxv.h>
#include <elf.h>
#include <got.h>

typedef void elf_init_t(char **, char **, auxv_t *);
typedef void elf_fini_t(void);

struct DynLibraryEntry {
    void* dylib_base;
    ElfNative_Dyn *dylib_dynamic_section;
    ElfNative_Phdr *dylib_phdrs;
    uint32_t dylib_dynamic_size;
    uint32_t dylib_phdrs_size;
    ElfNative_Sym *dylib_symtab;
    union {
        struct dt_gnu_hash *dylib_gnu_hash;
        struct dt_hash *dylib_hash;
        size_t dylib_symcount;
    };

    union GotEntry *dylib_pltgot;
    union {
        ElfNative_Rel *dylib_plt_rel;
        ElfNative_Rela *dylib_plt_rela;
    };
    size_t dylib_pltrelsz;
    enum Elf_DynTag dylib_hash_type;
    enum Elf_DynTag dylib_jmprel_type;
    const char *dylib_strtab;
    size_t dylib_strsz;
    Elf64_Addr* dylib_init_array;
    Elf64_Addr* dylib_fini_array;
    uint32_t dylib_init_array_size;
    uint32_t dylib_fini_array_size;
    const char *dylib_soname;
};

sysresult2_t dynld_link(void* base, ElfNative_Dyn dyn[], ElfNative_Phdr *phdrs,
                        size_t phnum, const char *file_name, bool no_relocate);


sysresult2_t dynld_sym_value(const char* sym, const struct DynLibraryEntry** ent_out);