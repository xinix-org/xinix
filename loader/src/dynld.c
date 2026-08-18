
#include "pointers.h"
#include "sysresult.h"
#include <elf.h>
#include <got.h>

#include <dynld.h>

#include <auxfuncs.h>
#include <hash.h>
#include <stdatomic.h>

static constexpr size_t lock_bit = 1ul << ((8 * sizeof(size_t)) - 1);

static constexpr size_t max_size = 8192;

struct DynldState {
    alignas(4096) struct DyldHeader {
        _Atomic(size_t) dynld_lock_and_offset;
        _Atomic(struct DynldState *) dynld_next;
        void *_pad[16 - 2];
    } header;

    struct DynLibraryEntry
        entries[(max_size / sizeof(struct DynLibraryEntry)) - 1];
};

struct DynldState dynld_state;

static inline size_t read_max_ents(struct DynldState *state) {
    auto val = atomic_load_explicit(&state->header.dynld_lock_and_offset,
                                    memory_order_acquire) &
               ~lock_bit;
    return val;
}

#define DYN_PTR(dynent, T) (T *)(((char *)base) + (dynent).d_ptr)
#define DYN_STR(dylib, offset) ((dylib).dylib_strtab + (offset))

static inline ElfNative_Offset addend_rel(Elf64_Rel *r) { return 0; }

static inline ElfNative_Offset addend_rela(Elf64_Rela *r) {
    return r->r_addend;
}

// #define DREL_JUMP_SLOT R_X86_64_JUMP_SLOT
// #define DREL_GLOB_DAT R_X86_64_GLOB_DAT
// #define DREL_COPY R_X86_64_COPY
// #define DREL_RELATIVE R_X86_64_RELATIVE
// #define DREL_IRELATIVE R_X86_64_IRELATIVE

// #define DREL_TLSDESC R_X86_64_TLSDESC
// #define DREL_DTPMOD R_X86_64_DTPMOD64
// #define DREL_DTPOFF R_X86_64_DTPMOD64

#define APPLY_RELOC_DEFERRED 1

static inline sysresult2_t apply_rel(void *offset, ElfNative_Reloc reloc,
                                     uint64_t addend, bool read_add,
                                     const Elf64_Sym *sym, bool resolve_defered,
                                     bool skip_non_defered,
                                     const struct DynLibraryEntry *ent) {
    const Elf64_Sym *val;
    const struct DynLibraryEntry *sym_ent;
    if (!sym) {
        val = sym;
        sym_ent = ent;
    } else if (sym->st_shndx && ((ST_BIND((*sym)) == STB_LOCAL) ||
                                 (ST_VISIBILITY(*sym) != STV_DEFAULT))) {
        val = sym;
        sym_ent = ent;
    } else {
        const char *name = DYN_STR(*ent, sym->st_name);
        auto res = dynld_sym_value(name, &sym_ent);
        SYSRESULT_TRY_SYSRESULT2(SYSRESULT2_CODE(res));
        val = SYSRESULT2_VALUE(res, const Elf64_Sym *);
    }
    switch (reloc) {
    case DREL_JUMP_SLOT: {
        void **addr = (void **)offset;
        void *rrel = ((char *)sym_ent->dylib_base) + val->st_value + addend +
                     (read_add ? (size_t)(*addr) : 0);
        if (ST_TYPE(*val) == STT_GNU_IFUNC) {
            if (resolve_defered) {
                void *(*res)(void) = launder_pointer((void *(*)(void))rrel);
                void *vaddr = res();
                *addr = vaddr;
                return SYSRESULT2_OK(rrel);
            }
        } else if (!skip_non_defered) {
            *addr = rrel;
            return SYSRESULT2_OK(rrel);
        }

        break;
    }
    case DREL_GLOB_DAT:
        if (!skip_non_defered) {
            void **addr = (void **)offset;
            void *rrel = ((char *)sym_ent->dylib_base) + val->st_value +
                         addend + (read_add ? (size_t)(*addr) : 0);
            *addr = rrel;
            return SYSRESULT2_OK(rrel);
        }
        break;
    case DREL_RELATIVE:
        if (!skip_non_defered) {
            void **addr = (void **)offset;
            void *rrel = ((char *)sym_ent->dylib_base) + addend +
                         (read_add ? (size_t)(*addr) : 0);
            *addr = rrel;
        }
        break;
    case DREL_IRELATIVE: {
        void **addr = (void **)offset;
        if (resolve_defered) {
            void *rrel = ((char *)sym_ent->dylib_base) + addend +
                         (read_add ? (size_t)(*addr) : 0);
            void *(*res)(void) = launder_pointer((void *(*)(void))rrel);
            void *vaddr = res();
            *addr = vaddr;
            return SYSRESULT2_OK(vaddr);
        }
    } break;
    default:
        return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
    }

    return SYSRESULT2_OK(nullptr);
}

sysresult2_t dynld_link(void *base, ElfNative_Dyn dyn[], ElfNative_Phdr *phdrs,
                        size_t phnum, const char *file_name, bool no_relocate) {
    ElfNative_Dyn *dynent = dyn;

    struct DynLibraryEntry ent = {};

    ent.dylib_base = base;

    int32_t soname = 0;

    ElfNative_Addr *preinit_array = nullptr;
    size_t preinit_array_sz = 0;

    ElfNative_Dyn *first_needed = nullptr;
    ElfNative_Dyn *last_needed = nullptr;

    union {
        ElfNative_Rel *rel;
        ElfNative_Rela *rela;
    } rel = {};

    enum Elf_DynTag relty = DT_NULL;

    size_t relcount = 0;

    bool bind_now = false;

    ElfNative_Addr *relr = nullptr;
    size_t relr_count = 0;

    while (true) {
        switch ((enum Elf_DynTag)dynent->d_tag) {
        case DT_NULL:
            goto next;
        case DT_NEEDED:
            if (!first_needed)
                first_needed = dynent;
            else
                last_needed = dynent;
            break;
        case DT_PLTRELSZ:
            ent.dylib_pltrelsz = dynent->d_val;
            break;
        case DT_PLTGOT:
            ent.dylib_pltgot = DYN_PTR(*dynent, union GotEntry);
            break;
        case DT_HASH:
            if (ent.dylib_hash_type != DT_GNU_HASH) {
                ent.dylib_hash = DYN_PTR(*dynent, struct dt_hash);
                ent.dylib_hash_type = DT_HASH;
            }
            break;
        case DT_STRTAB:
            ent.dylib_strtab = DYN_PTR(*dynent, const char);
            break;
        case DT_SYMTAB:
            ent.dylib_symtab = DYN_PTR(*dynent, ElfNative_Sym);
            break;
        case DT_RELA:
            if (no_relocate)
                return SYSRESULT2_ERROR(ERR_IMAGE_INVALID_RELOC);
            rel.rela = DYN_PTR(*dynent, ElfNative_Rela);
            relty = DT_RELA;
            break;
        case DT_RELASZ:
            if (no_relocate)
                return SYSRESULT2_ERROR(ERR_IMAGE_INVALID_RELOC);
            relcount = dynent->d_val / sizeof(ElfNative_Rela);
            break;
        case DT_RELAENT:
            if (dynent->d_val != sizeof(ElfNative_Rel))
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            break;
        case DT_STRSZ:
            ent.dylib_strsz = dynent->d_val;
            break;
        case DT_SYMENT:
            if (dynent->d_val != sizeof(ElfNative_Sym))
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            break;
        case DT_INIT:
            if (ent.dylib_init_array)
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            ent.dylib_init_array = &dynent->d_val;
            ent.dylib_init_array_size = 1;
            break;
        case DT_FINI:
            if (ent.dylib_fini_array)
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            ent.dylib_fini_array = &dynent->d_val;
            ent.dylib_fini_array_size = 1;
            break;
        case DT_SONAME:
            soname = dynent->d_val;
            break;
        case DT_RPATH:
            break;
        case DT_SYMBOLIC:
            break;
        case DT_REL:
            if (no_relocate)
                return SYSRESULT2_ERROR(ERR_IMAGE_INVALID_RELOC);
            rel.rel = DYN_PTR(*dynent, ElfNative_Rel);
            relty = DT_REL;
            break;
        case DT_RELSZ:
            if (no_relocate)
                return SYSRESULT2_ERROR(ERR_IMAGE_INVALID_RELOC);
            relcount = dynent->d_val / sizeof(ElfNative_Rel);
            break;
        case DT_RELENT:
            if (dynent->d_val != sizeof(ElfNative_Rel))
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            break;
        case DT_PLTREL:
            ent.dylib_jmprel_type = dynent->d_val;
            break;
        case DT_DEBUG:
            // TODO:
            break;
        case DT_TEXTREL:
            return SYSRESULT2_ERROR(ERR_IMAGE_WX_SEG);
        case DT_JMPREL:
            ent.dylib_plt_rel = DYN_PTR(*dynent, ElfNative_Rel);
            break;
        case DT_BIND_NOW:
            bind_now = true;
            break;
        case DT_INIT_ARRAY:
            if (ent.dylib_init_array)
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            ent.dylib_init_array = DYN_PTR(*dynent, ElfNative_Addr);
            break;
        case DT_FINI_ARRAY:
            if (ent.dylib_fini_array)
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            ent.dylib_fini_array = DYN_PTR(*dynent, ElfNative_Addr);
            break;
        case DT_INIT_ARRAYSZ:
            ent.dylib_init_array_size = dynent->d_val;
            break;
        case DT_FINI_ARRAYSZ:
            ent.dylib_fini_array_size = dynent->d_val;
        case DT_RUNPATH:
            break;
        case DT_FLAGS:
            // TODO:
            break;
        case DT_PREINIT_ARRAY:
            preinit_array = DYN_PTR(*dynent, ElfNative_Addr);
            break;
        case DT_PREINIT_ARRAYSZ:
            preinit_array_sz = dynent->d_val;
            break;
        case DT_GNU_HASH:
            ent.dylib_gnu_hash = DYN_PTR(*dynent, struct dt_gnu_hash);
            break;
        case DT_RELRSZ:
            relr_count = dynent->d_val;
            break;
        case DT_RELR:
            relr = DYN_PTR(*dynent, ElfNative_Addr);
            break;
        case DT_RELRENT:
            if (dynent->d_val != sizeof(ElfNative_Addr))
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            break;
        case DT_SYMTABSZ:
            if (!ent.dylib_hash_type) {
                ent.dylib_symcount = dynent->d_val;
                ent.dylib_hash_type = DT_SYMTABSZ;
            }
            break;
        }

        dynent++;
    }

next:

    if (soname) {
        ent.dylib_soname = DYN_STR(ent, soname);
    } else {
        ent.dylib_soname = file_name;
    }

    for (; first_needed != last_needed; first_needed++) {
        if (first_needed->d_tag == DT_NEEDED) {
            const char *needed = DYN_STR(ent, first_needed->d_val);
            // TODO: search for entries
            return SYSRESULT2_ERROR(-1);
        }
    }

    if (!no_relocate) {
        auto relrend = relr + relr_count;

        ElfNative_Addr *current = nullptr;

        for (; relr != relrend; relr++) {
            auto val = *relr;
            if (!(val & 1)) {
                current = (ElfNative_Addr *)(((char *)base) + val);
                *current += (ElfNative_Addr)base;
            } else if (!current)
                return SYSRESULT2_ERROR(ERR_IMAGE_VALIDATION_ERROR);
            else {
                do {
                    val >>= 1;
                    current++;
                    if ((val & 1))
                        *current += (ElfNative_Addr)base;
                } while (val != 0);
            }
        }
#define DO_REL(reltag, reladd_f, read_add, defered)                            \
    do {                                                                       \
        auto *_rel = (reltag);                                                 \
        auto *_rel_end = _rel + relcount;                                      \
        bool _has_defered = false;                                             \
        bool _needs_defered = (defered);                                       \
        bool _read_add = (read_add);                                           \
        for (; _rel != _rel_end; _rel++) {                                     \
            auto _syment = ELFNATIVE_R_SYM(_rel->r_info);                      \
            auto _sym = ent.dylib_symtab + _syment;                            \
            auto _relty = ELFNATIVE_R_TYPE(_rel->r_info);                      \
            auto _offset = ((char *)base) + _rel->r_offset;                    \
            auto _addend = (reladd_f)(_rel);                                   \
            /* sysresult2_t apply_rel(void* offset, ElfNative_Reloc reloc,     \
             * uint64_t addend, bool read_add, const Elf64_Sym* sym, bool      \
             * resolve_defered, bool skip_non_deffered, const struct           \
             * DynLibraryEntry* ent) */                                        \
            auto res = apply_rel(_offset, _relty, _addend, _read_add, _sym,    \
                                 _needs_defered, _needs_defered, &ent);        \
            SYSRESULT_TRY_SYSRESULT2(SYSRESULT2_CODE(res));                    \
            if (!SYSRESULT2_VALUE(res, void *))                                \
                _has_defered = true;                                           \
        }                                                                      \
        (defered) = _has_defered;                                              \
    } while (0)

        bool rel_defered = false;
        switch (relty) {
        case DT_NULL:
            break;
        case DT_REL: {
            DO_REL(rel.rel, addend_rel, true, rel_defered);
        } break;
        case DT_RELA: {
            DO_REL(rel.rela, addend_rela, false, rel_defered);
        } break;
        default:
            return SYSRESULT2_ERROR(-999);
        }

        bool jmprel_defered = false;
        if (bind_now) {
            switch (ent.dylib_jmprel_type) {
            case DT_NULL:
                break;
            case DT_REL: {
                DO_REL(ent.dylib_plt_rel, addend_rel, true, jmprel_defered);
            } break;
            case DT_RELA: {
                DO_REL(ent.dylib_plt_rela, addend_rela, false, jmprel_defered);
            }
            default:
                return SYSRESULT2_ERROR(-999);
            }
        }

        if (rel_defered) {
            switch (relty) {
            case DT_NULL:
                break;
            case DT_REL: {
                DO_REL(rel.rel, addend_rel, true, rel_defered);
            } break;
            case DT_RELA: {
                DO_REL(rel.rela, addend_rela, false, rel_defered);
            } break;
            default:
                return SYSRESULT2_ERROR(-999);
            }
        }

        if (bind_now && jmprel_defered) {
            switch (ent.dylib_jmprel_type) {
            case DT_NULL:
                break;
            case DT_REL: {
                DO_REL(ent.dylib_plt_rel, addend_rel, true, jmprel_defered);
            } break;
            case DT_RELA: {
                DO_REL(ent.dylib_plt_rela, addend_rela, false, jmprel_defered);
            }
            default:
                return SYSRESULT2_ERROR(-999);
            }
        }
    }

    size_t ent_lock;

    while ((ent_lock = atomic_fetch_or_explicit(
                &dynld_state.header.dynld_lock_and_offset, lock_bit,
                memory_order_acquire)) &
           lock_bit)
        spin_loop_hint();

    size_t ent_off = ent_lock & ~lock_bit;

    if (ent_off ==
        (sizeof(dynld_state.entries) / sizeof(dynld_state.entries[0])))
        return SYSRESULT2_ERROR(-1);

    auto eref = &dynld_state.entries[ent_lock];

    *eref = ent;

    if (eref->dylib_pltgot) {
        eref->dylib_pltgot[1].got_address = eref;
        eref->dylib_pltgot[2].got_address = nullptr; // Add a resolver later
    }

    atomic_store_explicit(&dynld_state.header.dynld_lock_and_offset, ent_off,
                          memory_order_release);

    return SYSRESULT2_OK(eref);
}

sysresult2_t dynld_sym_value(const char *sym,
                             const struct DynLibraryEntry **ent_out) {

    return SYSRESULT2_ERROR(-1);
}
