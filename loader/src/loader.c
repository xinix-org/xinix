// Placeholder

#include "pointers.h"
#include <cmp.h>
#include <elf.h>
#include <stddef.h>
#include <stdint.h>
// FIXME: missing string.h on some targets
// #include <string.h>
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
#include <vmap.h>

#include <got.h>

#include <sysresult.h>

extern ElfNative_Dyn _DYNAMIC[];

extern union GotEntry _GLOBAL_OFFSET_TABLE_[];

void *image_base_addr(void) {
    return (void *)((uintptr_t)(&_DYNAMIC) -
                    _GLOBAL_OFFSET_TABLE_[0].got_value);
}

sysresult2_t loader_map_elf(const ElfNative_Ehdr *e_hdr,
                            ElfNative_Dyn **dyn_out,
                            ElfNative_Phdr **phdr_out) {
    SYSRESULT_TRY_SYSRESULT2(
        elf_validate_ident_native(&e_hdr->e_ident, ELFOSABINONE));

    if (e_hdr->e_machine != EM_NATIVE)
        return SYSRESULT2_ERROR(ERR_GENERIC);

    if (e_hdr->e_type != ET_DYN)
        return SYSRESULT2_ERROR(ERR_GENERIC);

    if (e_hdr->e_phnum != sizeof(ElfNative_Phdr))
        return SYSRESULT2_ERROR(ERR_GENERIC);

    ElfNative_Phdr *phdrs =
        (ElfNative_Phdr *)((char *)e_hdr) + (e_hdr->e_phoff);
    size_t phnum = e_hdr->e_phnum;

    ElfNative_Phdr *phdrs_end = phdrs + phnum;

    size_t last_rpa = 0;

    const ElfNative_Phdr *dyn_segment = nullptr;
    const ElfNative_Phdr *phdr_segment = nullptr;

    for (auto *pos = phdrs; pos != phdrs_end; pos++) {
        if ((pos->p_flags & (PF_W | PF_X)) == (PF_W | PF_X))
            return SYSRESULT2_ERROR(ERR_IMAGE_WX_SEG);
        if (pos->p_type == PT_PHDR)
            phdr_segment = pos;
        else if (pos->p_type == PT_DYNAMIC)
            dyn_segment = pos;

        if (pos->p_type != PT_LOAD)
            continue;

        last_rpa = max((pos->p_paddr + pos->p_memsz), last_rpa);
    }

    void *root = valloc((last_rpa + 4095) >> 12, PROT_NONE);

    for (auto *pos = phdrs; pos != phdrs_end; pos++) {
        if (pos->p_type != PT_LOAD)
            continue;

        enum valloc_flags _flags = 0;

        if (pos->p_flags & PF_R)
            _flags |= PROT_READ;
        if (pos->p_flags & PF_W)
            _flags |= PROT_WRITE;
        if (pos->p_flags & PF_X)
            _flags |= PROT_EXEC;

        auto base_rpa = pos->p_paddr & !4095;
        auto end_rpa = pos->p_paddr + pos->p_memsz;

        size_t total_len = ((end_rpa - base_rpa) + 4095) >> 12;

        void *page_begin = ((char *)root) + base_rpa;

        void *rbegin = vprotect(page_begin, total_len, PROT_READ | PROT_WRITE);

        void *mbegin = ((char *)rbegin) + (pos->p_paddr & 4095);

        const void *fbegin = ((const char *)e_hdr) + (pos->p_offset);

        memcpy(mbegin, fbegin, pos->p_filesz);

        vprotect(page_begin, total_len, _flags);
    }

    if (dyn_out && dyn_segment) {
        *dyn_out = launder_pointer(
            (ElfNative_Dyn *)(((char *)root) + dyn_segment->p_paddr));
    }

    if (phdr_out && phdr_segment) {
        *phdr_out = launder_pointer(
            (ElfNative_Phdr *)(((char *)root) + phdr_segment->p_paddr));
    }

    return SYSRESULT2_OK(root);
}
