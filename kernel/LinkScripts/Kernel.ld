/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

SECTIONS
{
	. = 0xC0000000;

	.init : AT(LKR_START_PROG_DATA)
	{
		LKR_START_PROG_DATA = .;
		*(.init)
		. = ALIGN(4K);
	}
	.text : AT(START_PROG_DATA_NONINIT)
	{
		START_PROG_DATA_NONINIT = .;
		*(.text)
		. = ALIGN(4K);
		*(.data)
		. = ALIGN(4K);
		*(.rodata)
		. = ALIGN(4K);
		*(.rodata.*)
		. = ALIGN(4K);
		LKR_END_PROG_DATA = .;
	}
	.bss : AT(LKR_END_PROG_DATA)
	{
		*(.bss)
		. = ALIGN(4K);
		LKR_END = .;
	}
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
