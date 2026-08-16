#pragma once

#include "sysresult.h"
#include <auxv.h>
#include <elf.h>
#include <got.h>

typedef void elf_init_t(char **, char **, auxv_t *);
typedef void elf_fini_t(void);

struct DynLibraryEntry {
    ElfNative_Dyn *dylib_dynamic_section;
    size_t dylib_dynamic_size;
    ElfNative_Phdr *dylib_phdrs;
    size_t dylib_phdrs_size;
    ElfNative_Sym *dylib_symtab;
    union {
        struct dt_gnu_hash *dylib_gnu_hash;
        struct dt_hash *dylib_hash;
    };

    union GotEntry *dylib_pltgot;
    union {
        ElfNative_Rel *dylib_plt_rel;
        ElfNative_Rela *dylib_plt_rela;
    };
    enum Elf_DynTag dylib_hash_type;
    enum Elf_DynTag dylib_jmprel_type;
    const char *dylib_strtab;
    elf_init_t **dylib_init_array;
    size_t dylib_init_array_size;
    elf_fini_t **dylib_fini_array;
    size_t dylib_fini_array_size;
    const char *dylib_soname;
};

sysresult2_t dynld_link(ElfNative_Dyn dyn[], ElfNative_Phdr *phdrs,
                        size_t phnum, const char *file_name, bool no_relocate);
