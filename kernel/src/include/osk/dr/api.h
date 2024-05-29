/*******************************************************************************
                        Copyright (C) 2023, Joey Qytyku

This file is part of OS/90 and is published under the GNU General Public
License version 2. A copy of this license should be included with the
source code and can be found at <https://www.gnu.org/licenses/>.
*******************************************************************************/

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
