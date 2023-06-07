/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef _LINKER_H
#define _LINKER_H

// The kernel is loaded at 1/3 of the virtual address space.
// 1M is identity mapped to the start of memory on startup
// because the kernel binary is relocated to higher half
// subtraction is needed to go to low memory

//
// phys() should be used for symbols, not absolute addresses (aka integer cast)
// Absolute addresses are not relocated by linker.
//
#define HIGHER_HALF 0xC0000000
#define phys(a) ((PVOID)(a-HIGHER_HALF))

/* In C, get these linker symbols using address-of operator (&) */
extern int LKR_STARTTEXT;
extern int LKR_ENDTEXT;
extern int LKR_STARTDATA;
extern int LKR_ENDDATA;
extern int LKR_STARTBSS;
extern int LKR_END;

#endif
