////////////////////////////////////////////////////////////////////////////////
//                      This file is part of OS/90.
//
// OS/90 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 2 of the License, or (at your option) any later
// version.
//
// OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along
// with OS/90. If not, see <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#ifndef SYS_ENTRY_H
#define SYS_ENTRY_H

#include <Type.h>
#include "Interrupt.h"
#include "Sync.h"

// I am not sure if Intel reserved the first 32 vectors originally or if the
// first 32 being reserved was a result of extensions to the architecture.
// I will not make any assumptions on what exceptions are available and
// reseserve the first 32.

#define RESERVED_IDT_VECTORS 32

typedef VOID (*EXCEPTION_HANDLER)(PU32);

VOID SetHighLevelExceptionHandler(U8, EXCEPTION_HANDLER);

// Checking if the context was a supervisory virtual 8086 call
// is done by checking the value of the mutex lock for real mode.

#define OP_INT3 0xCC
#define OP_INT  0xCD
#define OP_INTO 0xCE
#define OP_IRET 0xCF

tpkstruct {
    U32   eax;
    U32   ebx;
    U32   ecx;
    U32   edx;
    U32   esi;
    U32   edi;
    U32   ebp;
    U32   eip;
    U32   cs;
    U32   eflags;

    U32   esp;
    U32   ss;

    U32   es;
    U32   ds;
    U32   fs;
    U32   gs;
}IRET_FRAME,*P_IRET_FRAME;


enum {
    VEC_SYS_RES_BASE= 252, // same as next
    VEC_INT_SV86    = 252,
    VEC_INT_RMPROC  = 253,
    VEC_IRET_SV86   = 254,
    VEC_IRET_RMPROC = 255
};
//
// Some of these are vectors that go into the imm8 field of an INT
// instruction. Others are replacements for the IRET instruction.
//
// The #GP handler replaces the INT ops imm8 into VEC_INT_SV86
// or VEC_INT_RMPROC. Then it memorizes the original byte.
//
// If it replaces the IRET instruction, it must save two bytes.

//
// INT VEC_IRET_SV86 is caught by SysEntry and causes it to go back to the caller of
// EnterRealMode.
//
// INT VEC_INT_SV86
//

#endif
