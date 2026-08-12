
#include <auxv.h>
#include <random.h>
#include <memmap.h>
#include <framebuffer.h>
#include <stddef.h>
#include <cpuid.h>

[[noreturn]]
extern void kmain(int argc, char *argv[], char *envp[], auxv_t auxv[]);

extern void init_cpu_feature_array(void);


[[noreturn]]
void call_kmain(size_t hhdm_offset, framebuffer* fb, memmap* memmap, void* rsdp) {
    init_cpu_feature_array();

    char *argv[] = {"kernel", 0};
    char *envp[16] = {0};
    auxv_t auxv[16] = {
        {0},
    };

    auxv_t *auxtarg = auxv;

    char cpu_name[] = ARCH;

    *auxtarg++ = (auxv_t){.a_type = AT_PAGESZ, .a_un.a_val = 4096};
    *auxtarg++ = (auxv_t){.a_type = AT_PLATFORM, .a_un.a_ptr = cpu_name};
    *auxtarg++ = (auxv_t){.a_type = AT_XINIX_CPU_FEATURES_ARRAY, .a_un.a_ptr = (void*)(const void*)x86_feature_array};
    *auxtarg++ = (auxv_t){.a_type = AT_XINIX_CPU_FEATURES_LEN, .a_un.a_val = 38};
    uint8_t random[16];
    if (rand_slow_get_entropy(random) == 0)
        *auxtarg++ = (auxv_t){.a_type = AT_RANDOM, .a_un.a_ptr = random};

    *auxtarg++ = (auxv_t){.a_type = AT_KXINIX_HHDM_OFFSET, .a_un.a_val = hhdm_offset};
    *auxtarg++ = (auxv_t){.a_type = AT_KXINIX_MEMMAP, .a_un.a_ptr = memmap};

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

    // TODO: validate checksum

    *auxtarg++ = (auxv_t){.a_type = AT_KXINIX_RSDT_ADDR, .a_un.a_ptr = (void*)(rsdp_struct->rsdt_address + hhdm_offset)};

    if(rsdp_struct->revision >= 2)
        *auxtarg++ = (auxv_t){.a_type = AT_KXINIX_XSDT_ADDR, .a_un.a_ptr = (void*)(rsdp_struct->xsdt_address + hhdm_offset)};

}