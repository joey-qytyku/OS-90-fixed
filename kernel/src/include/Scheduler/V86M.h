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
#include "Sync.h" /* ATOMIC type */


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

// SV86 and process V86 require radically different behavior and must be known
// by the handler. The register dump is also different.
//
// Regardless, a handler must be implemented for both
// A single front link is used.
//
// Returns 1 if swallowed the interrupt and 0 if passed down the chain.
//
typedef BOOL (*SV86_HANDLER)(P_SV86_REGS);
typedef BOOL (*UV86_HANDLER)(P_UREGS);

typedef struct
{
    SV86_HANDLER    if_sv86;
    UV86_HANDLER    if_uv86;
    PVOID next;
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

API_DECL(VOID, HookDosTrap,
    U8,
    PV86_CHAIN_LINK
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

extern ATOMIC g_sv86;

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
