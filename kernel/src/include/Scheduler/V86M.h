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

#ifndef SCHEDULER_V86M_H
#define SCHEDULER_V86M_H

#include <Type.h>

// V86 handler return values
#define CAPT_HND   0 /* Handled captured trap */
#define CAPT_NOHND 1 /* Did not handle */


//
// This is the structure used by EnterRealMode and as an input to Svint86.
//
typedef struct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
    U32 esi;
    U32 edi;
    U32 ebp;

    // RM Trap Frame
    U32 gs;
    U32 fs;
    U32 ds;
    U32 es;
    U32 ss;
    U32 esp;
    U32 eflags;
    U32 cs;
    U32 eip;

}SV86_REGS,*P_SV86_REGS;

typedef BOOL (*V86_HANDLER)(P_SV86_REGS);

typedef struct
{
    V86_HANDLER handler;  // Set the handler
    PVOID next;           // Initialize to zero
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

extern KERNEL VOID ScHookDosTrap(
    U8,
    PV86_CHAIN_LINK,
    V86_HANDLER
);

extern VOID KERNEL ScOnErrorDetatchLinks(VOID);
extern VOID KERNEL Svint86(P_SV86_REGS, U8);

extern VOID EnterRealMode(VOID);
extern SV86_REGS _RealModeRegs;

// We will not expose this.
//extern U32 _RealModeTrapFrame[9];

static inline PVOID MK_LP(U16 seg, U16 off)
{
    U32 address = seg*16 + off;
    return (PVOID) address;
}

extern U8 g_sv86;

// Change to a MOV instruction?
static inline VOID AssertSV86(VOID)
{
    FENCE;
    g_sv86 = 1;
    FENCE;
}

static inline VOID UnsignalSV86(VOID)
{
    FENCE;
    g_sv86 = 0;
    FENCE;
}

static BOOL WasSV86(VOID)
{
    FENCE;
    return g_sv86;
    FENCE;
}

#endif /* SCHEDULER_V86M_H */
