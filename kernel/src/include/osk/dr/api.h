/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#pragma once


/*
 * startup stub is essentially:
 * jmp [0x8000_0006] ; 6 bytes
 * DD _start         ; 4 bytes
*/

// This structure is 4K in size and at the beginning of the kernel address
// space.
struct _systab {
    char _startup_stub[16];
};
