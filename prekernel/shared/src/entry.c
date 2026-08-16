
#include <cmp.h>
#include <pointers.h>
#include <auxv.h>
#include <cpuid.h>
#include <elf.h>
#include <framebuffer.h>
#include <memmap.h>
#include <random.h>
#include <stddef.h>
#include <vmap.h>
#include <sysresult.h>
#include <hcf.h>


typedef void kmain_t(int argc, char *argv[], char *envp[], auxv_t auxv[]);

extern void init_cpu_feature_array(void);

extern ElfNative_Ehdr _binary_target_xinix_kernel_so_start;

uintptr_t hhdm_offset;

static sysresult2_t loader_map_elf(const ElfNative_Ehdr *e_hdr,
                            ElfNative_Dyn **dyn_out,
                            ElfNative_Phdr **phdr_out, void*(*aligned_alloc)(size_t align, size_t sz)) {
    SYSRESULT_TRY_SYSRESULT2(
        elf_validate_ident_native(&e_hdr->e_ident, ELFOSABINONE));

    if (e_hdr->e_machine != EM_NATIVE)
        return SYSRESULT2_ERROR(ERR_GENERIC);

    if (e_hdr->e_type != ET_DYN)
        return SYSRESULT2_ERROR(ERR_GENERIC);

    if (e_hdr->e_phentsize != sizeof(ElfNative_Phdr))
        return SYSRESULT2_ERROR(ERR_GENERIC);

    ElfNative_Phdr *phdrs =
        (ElfNative_Phdr *)(((char *)e_hdr) + (e_hdr->e_phoff));
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

    void *root = aligned_alloc(4096, (last_rpa + 4095) & ~4095);
    if(!root)
        return SYSRESULT2_ERROR(ERR_GENERIC);

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

        auto base_rpa = pos->p_paddr & ~4095;
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


[[noreturn]]
void call_kmain(size_t _hhdm_offset, framebuffer *fb, memmap *memmap,
                void *rsdp, void *(*aligned_alloc)(size_t, size_t)) {
    init_cpu_feature_array();

    hhdm_offset = _hhdm_offset;

    auto res = loader_map_elf(&_binary_target_xinix_kernel_so_start, nullptr, nullptr, aligned_alloc);

    if(SYSRESULT2_CODE(res) < 0)
        hcf();

    uintptr_t root = SYSRESULT2_VALUE(res, uintptr_t); 
    

    ptrdiff_t offset = _binary_target_xinix_kernel_so_start.e_entry;

    kmain_t* kmain = (kmain_t*)(root + offset);

    char *argv[] = {"kernel", 0};
    int argc = 1;
    char *envp[16] = {0};
    auxv_t auxv[16] = {
        {0},
    };

    auxv_t *auxtarg = auxv;

    char cpu_name[] = ARCH;

    *auxtarg++ = (auxv_t){.a_type = AT_PAGESZ, .a_un.a_val = 4096};
    *auxtarg++ = (auxv_t){.a_type = AT_PLATFORM, .a_un.a_ptr = cpu_name};
    *auxtarg++ =
        (auxv_t){.a_type = AT_XINIX_CPU_FEATURES_ARRAY,
                 .a_un.a_ptr = (void *)(const void *)x86_feature_array};
    *auxtarg++ =
        (auxv_t){.a_type = AT_XINIX_CPU_FEATURES_LEN, .a_un.a_val = 38};
    uint8_t random[16];
    if (rand_slow_get_entropy(random) == 0)
        *auxtarg++ = (auxv_t){.a_type = AT_RANDOM, .a_un.a_ptr = random};

    *auxtarg++ =
        (auxv_t){.a_type = AT_KXINIX_HHDM_OFFSET, .a_un.a_val = hhdm_offset};
    *auxtarg++ = (auxv_t){.a_type = AT_KXINIX_MEMMAP, .a_un.a_ptr = memmap};
    *auxtarg++ = (auxv_t){.a_type = AT_BASE, .a_un.a_ptr = nullptr};

    struct {
        char signature[8];
        uint8_t checksum;
        char oemid[6];
        uint8_t revision;
        uint32_t rsdt_address;
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
    } *rsdp_struct = rsdp;

    if (fb)
        *auxtarg++ =
            (auxv_t){.a_type = AT_KXINIX_FRAMEBUFFER, .a_un.a_ptr = fb};

    // TODO: validate checksum

    *auxtarg++ = (auxv_t){
        .a_type = AT_KXINIX_RSDT_ADDR,
        .a_un.a_ptr = (void *)(rsdp_struct->rsdt_address + hhdm_offset)};

    if (rsdp_struct->revision >= 2)
        *auxtarg++ = (auxv_t){
            .a_type = AT_KXINIX_XSDT_ADDR,
            .a_un.a_ptr = (void *)(rsdp_struct->xsdt_address + hhdm_offset)};

    kmain(argc, argv, envp, auxv);
    __builtin_trap();
}
