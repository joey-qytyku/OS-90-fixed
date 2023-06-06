/*
 * Note that we do not pad the end of the file. That is a kernel, specific
 * thing. The relocations are copied
*/

SECTIONS
{
    . = 0

	.text : AT(START_PROG_DATA_NONINIT)
	{
		START_PROG_DATA_NONINIT = .;

        LONG()
		*(.text)
		. = ALIGN(4K);

		*(.data)
		. = ALIGN(4K);

		*(.rodata)
		. = ALIGN(4K);

		*(.rodata.*)
        RELOC_ADDR = .
		*(.rela.dyn)    /* Insert relocations into the resulting executable */
		LKR_END_PROG_DATA = .;
	}
	.bss : AT(LKR_END_PROG_DATA)
	{
		*(.bss)
		. = ALIGN(4K);
		LKR_END = .;
	}

    /DISCARD/
    {
        *(*)
    }
}
