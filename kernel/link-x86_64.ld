OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_kstart)

PHDRS
{
    phdrs PT_PHDR PHDRS;
    interp PT_INTERP;
    text PT_LOAD FILEHDR PHDRS;
    data PT_LOAD;
    data_relro PT_LOAD;
    dynamic PT_DYNAMIC;
    
}

SECTIONS
{
    . = 1024;
    .interp : {
        *(.interp)
    } :interp :text

    .text :{
        *(.text)
        *(.text.*)
    } :text

    .plt :{
        *(.plt)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .relr.dyn : {
        *(.relr.dyn)
    }

    .rela.dyn : {
        *(.rela.dyn)
    }

    .rela.plt : {
        *(.rela.pt)
    }

    . = ALIGN(4096);

    .got : {
        *(.got)
    } :data_relro

    .got.plt : {
        *(.got.plt)
    }

    . = ALIGN(4096);
    
    .data : {
        *(.data)
        *(.data.*)
    } :data

    .dynamic :{
        *(.dynamic)
    } :data :dynamic


    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } :data
}