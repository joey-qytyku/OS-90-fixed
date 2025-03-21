
phys = 0

/*
The first phase performs section merging as needed and discards any extras.

*/

SECTIONS
{
	.text {
		*(.text)
	}

	.data {
		*(.data)
	}

	.rodata {
		*(.rodata)
		*(.rodata.*)
	}

	/DISCARD/ {
		.comment
	}
}
