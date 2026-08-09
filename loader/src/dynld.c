
#include "sysresult.h"
#include <elf.h>
#include <got.h>

#include <dynld.h>

#include <hash.h>
#include <stdatomic.h>

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

static constexpr size_t lock_bit = 0x80000000;

static constexpr size_t max_size = 8192;

struct DynldState {
    alignas(4096) struct DyldHeader {
        _Atomic(size_t) dynld_lock_and_offset;
        _Atomic(struct DynldState *) dynld_next;
        void *_pad[6];
    } header;

    struct DynLibraryEntry
        entries[(max_size / sizeof(struct DynLibraryEntry)) - 1];
};

struct DynldState dynld_state;

sysresult_t dynld_link(ElfNative_Dyn dyn[], ElfNative_Phdr *phdrs, size_t phnum,
                       const char *file_name) {
    ElfNative_Dyn *dynent = dyn;

    struct DynLibraryEntry ent_build = {};

    while (dynent->d_tag != DT_NULL) {

        switch ((enum Elf_DynTag)dynent->d_tag) {

        case DT_NULL:
        case DT_NEEDED:
        case DT_PLTRELSZ:
        case DT_PLTGOT:
        case DT_HASH:
        case DT_STRTAB:
        case DT_SYMTAB:
        case DT_RELA:
        case DT_RELASZ:
        case DT_RELAENT:
        case DT_STRSZ:
        case DT_SYMENT:
        case DT_INIT:
        case DT_FINI:
        case DT_SONAME:
        case DT_RPATH:
        case DT_SYMBOLIC:
        case DT_REL:
        case DT_RELSZ:
        case DT_RELENT:
        case DT_PLTREL:
        case DT_DEBUG:
        case DT_TEXTREL:
        case DT_JMPREL:
        case DT_BIND_NOW:
        case DT_INIT_ARRAY:
        case DT_FINI_ARRAY:
        case DT_INIT_ARRAYSZ:
        case DT_FINI_ARRAYSZ:
        case DT_RUNPATH:
        case DT_FLAGS:
        case DT_ENCODING:
        case DT_PREINIT_ARRAYSZ:
        case DT_LOOS:
        case DT_HIOS:
        case DT_GNU_HASH:
        case DT_LOPROC:
        case DT_HIPROC:
            break;
        }

        dynent++;
    }
}
