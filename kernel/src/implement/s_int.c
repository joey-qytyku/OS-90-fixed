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

#include <OSK/SD/task.h>
#include <OSK/SD/int.h>

#define NUM_EXCEPTIONS 21

static ISR eh[NUM_EXCEPTIONS];

LONG g_preempt_count = 0;

VOID SV86_HandleGP(LONG error_code, PSTDREGS trapframe)
{

}

VOID SetException(BYTE index, ISR isr)
{
        PREEMPT_INC();
        eh[index] = isr;
        PREEMPT_DEC();
}

ISR GetException(BYTE index)
{
        PREEMPT_INC();
        return eh[index];
        PREEMPT_DEC();
}

//
// This procedure is the system entry point. It handles every exception.
//
VOID SystemEntryPoint(LONG index, LONG error_code, PSTDREGS trapframe)
{
        // Current state:
        // - Interrupts are off
        // - Preemption unknown, but disabled by proxy
        // - Trap frame contains user context

        // Maybe the current interrupt state should be updated to reflect the previous one
        // at the opportune time?

        // SET_FLAGS(trapframe->EFLAGS);
}
