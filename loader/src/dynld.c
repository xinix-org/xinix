
#include "sysresult.h"
#include <elf.h>
#include <got.h>

#include <dynld.h>

#include <hash.h>
#include <stdatomic.h>

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

sysresult2_t dynld_link(ElfNative_Dyn dyn[], ElfNative_Phdr *phdrs, size_t phnum,
                       const char *file_name, bool no_relocate) {
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


    return SYSRESULT2_ERROR(-1);
}
