/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/


/* Text section is smaller now. Something is terribly wrong */
SECTIONS
{
	. = 0xC0000000;

    .init :
    {
        *(.init)
    }
    . = ALIGN(4K);
    END_INIT = .;

	.text : AT(END_INIT)
	{
        *(.text)
	}
    . = ALIGN(4K);
    END_TEXT = .;

    .data : AT(END_TEXT)
    {
        *(.data)
    }
    . = ALIGN(4K);
    END_DATA = .;

    .rodata : AT(END_DATA)
    {
        *(.rodata)
        *(.rodata.*)
    }
    . = ALIGN(4K);
    END_RODATA = .;

	.bss : AT(END_RODATA)
	{
		*(.bss)
		BSS_END = .;
	}

    BSS_SIZE = BSS_END - END_DATA;

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
