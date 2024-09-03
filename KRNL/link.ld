/*
                     Copyright (C) 2023, Joey Qytyku

 This file is part of OS/90 and is published under the GNU General Public
 License version 2. A copy of this license should be included with the
 source code and can be found at <https://www.gnu.org/licenses/>.
*/

SECTIONS
{
        . = 0xC0000000;

/*
Explainiation on sections

The compiler emits sections that have an alignment as large as the highest
alignment requested by the generated assembly. The linker knows this and honors
the input section alignment rather than simply copying the section exactly
at the location.

This means it is rather safe to merge sections regardless of alignment.
The linker will insert padding where necessary, or reorder them optimally.

*/
	.text :
        {
                *(.init)
                *(.text)
                *(.data)
                *(.rodata)
                *(.rodata.*)
                . = ALIGN(4K);
	}=0
        END_CODE = .;

	.bss : AT(END_CODE)
	{
		*(.bss)
                . = ALIGN(4K);
	}
	END_BSS = .;

        BSS_SIZE = END_BSS - END_CODE;

        /*
                The linker compains about this and says it will be deprecated
                behavior. It is the only way I can get rid of extra non-code
                gargabe the GCC tries to put in.
        */
	/DISCARD/ :
	{
                *(*)
	}
}

/*
[1]

Strings are stored in different subsections depending on their alignment.
For example .rodata.str1.1. The linker can reference them as .rodata and
merge them.

But no?
*/