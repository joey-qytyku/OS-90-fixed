////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_V86_H
#define SCHEDULER_V86_H

#include <Type.h>
#include "Sync.h"       /* ATOMIC type */
#include "Process.h"

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
typedef BOOL (*SV86_HANDLER)(P_RD);
typedef BOOL (*UV86_HANDLER)(P_RD);

typedef struct
{
    SV86_HANDLER    if_sv86;
    UV86_HANDLER    if_uv86;
    PVOID next;
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

API_DECL(VOID, Hook_Dos_Trap, U8, PV86_CHAIN_LINK);
API_DECL(VOID, On_Error_Detatch_Links, VOID);
API_DECL(VOID, Svint86, P_RD, U8);

extern VOID Enter_Real_Mode(VOID);
extern RD _RealModeRegs;

extern ATOMIC g_sv86;

static inline VOID Raise_SV86(VOID)
{
    Atomic_Fenced_Store(&g_sv86, 1);
}

static inline VOID Lower_SV86(VOID)
{
    Atomic_Fenced_Store(&g_sv86, 0);
}

static inline BOOL Was_SV86(VOID)
{
    return Atomic_Fenced_Compare(&g_sv86, 1);
}

VOID Init_V86(VOID);



#endif /* SCHEDULER_V86_H */
