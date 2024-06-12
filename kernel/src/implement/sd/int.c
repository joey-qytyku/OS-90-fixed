/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                   Copyright (C) 2023-2024, Joey Qytyku                     บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ  This file is part of OS/90 and is published under the GNU General Public  บ
  บ    License version 2. A copy of this license should be included with the   บ
  บ      source code and can be found at <https://www.gnu.org/licenses/>.      บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
#include <osk/sd/basicatomic.h>
#include <osk/sd/task.h>
#include <osk/sd/int.h>

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

        SET_FLAGS(trapframe->EFLAGS);
}
