////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_V86M_H
#define SCHEDULER_V86M_H

#include <Type.h>

// V86 handler return values
#define CAPT_HND   0 /* Handled captured trap */
#define CAPT_NOHND 1 /* Did not handle */


//
// This is the structure used by EnterRealMode and as an input to Svint86.
// It is independent of anything trap frame related because it is a separate
// buffer for entering SV86.
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

//
// TODO:
//      Is SV86_REGS really what I want to pass? Can it depend?
//      Remeber that the behavior can be radically different depending
//      on whether it is SV86 or not. I should probably pass it as a
//      parameter.
//
typedef BOOL (*V86_HANDLER)(P_SV86_REGS);

typedef struct
{
    V86_HANDLER handler;  // Set the handler
    PVOID next;           // Initialize to zero
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

API_DECL(VOID, HookDosTrap,
    U8,
    PV86_CHAIN_LINK,
    V86_HANDLER
);

API_DECL(VOID, ScOnErrorDetatchLinks, VOID);
API_DECL(VOID, Svint86, P_SV86_REGS, U8);

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
    AtomicFencedStore(&g_sv86, 1);
}

static inline VOID DeassertSV86(VOID)
{
    AtomicFencedStore(&g_sv86, 0);
}

static inline BOOL WasSV86(VOID)
{
    return AtomicFencedCompare(&g_sv86, 1);
}

#endif /* SCHEDULER_V86M_H */
