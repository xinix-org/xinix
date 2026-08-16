#pragma once

#include <elf.h>
#include <sysresult.h>

sysresult2_t loader_map_elf(const ElfNative_Ehdr *e_hdr,
                            ElfNative_Dyn **dyn_out, ElfNative_Phdr **phdr_out);

void *image_base_addr(void);
