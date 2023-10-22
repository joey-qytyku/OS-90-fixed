///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



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
extern char LKR_STARTTEXT;
extern char LKR_ENDTEXT;
extern char LKR_STARTDATA;
extern char LKR_ENDDATA;
extern char LKR_STARTBSS;
extern char LKR_END;

#endif
