OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0;

    TEXT = .;
    .text : { *(.text) }
    TEXT_SIZE = . - TEXT;

    DATA = .;
    .data : { *(.data) }
    DATA_SIZE = . - DATA;

    BSS = .;
    .bss  : { *(.bss)  }
    BSS_SIZE = . - BSS;

    RELA_TEXT = .;
    .rela.text : { *(.rela.text) }
    RELA_TEXT_SIZE = . - RELA_TEXT;

    RELA_DATA = .;
    .rela.data : { *(.rela.data) }
    RELA_DATA_SIZE = . - RELA_DATA;

    SYMTAB = .;
    .symtab    : { *(.symtab) }
    SYMTAB_SIZE = . - SYMTAB;

    STRTAB = .;
    .strtab    : { *(.strtab) }
    STRTAB_SIZE = . - STRTAB;

    .todo_hdr :
    {
        /* MAGIC */
        LONG(0xC3C3C3C3);

        /* RVA */
        LONG(TEXT);
        LONG(DATA);
        LONG(BSS);
        LONG(RELA_TEXT);
        LONG(RELA_DATA);
        LONG(SYMTAB);
        LONG(STRTAB);

        /* Size in bytes */
        LONG(TEXT_SIZE);
        LONG(DATA_SIZE);
        LONG(BSS_SIZE);
        LONG(RELA_TEXT_SIZE);
        LONG(RELA_DATA_SIZE);
        LONG(SYMTAB_SIZE);
        LONG(STRTAB_SIZE)

        /* Program entry point or driver header */
        LONG(_EntryPoint);
    }
    /DISCARD/ :
    {
        *(*)
    }
}
