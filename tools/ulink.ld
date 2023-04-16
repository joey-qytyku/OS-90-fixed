/*
 * This is the linker script for userspace programs for conversion to NXF
 * Make sure to run EXETOOL.
 * Do not remove the symbol or relocation tables, stripping is done by
 * EXETOOL.
**/

SECTIONS
{
    . = 0x110000;
    .all : ALIGN(4K)
    {
        *(.text)
        *(.data)
    }
    .bss : ALIGN(4K)
    {
        *(.bss)
    }
    /DISCARD/:
    {
        *(.comment)
    }
}
