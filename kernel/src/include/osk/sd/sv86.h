/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2028, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

#ifndef SV86_H
#define SV86_H

#include "stdregs.h"
#include "int.h"

#define V86R_INIT { .ESP = 0 }

/*
        If the call is consumed, returns NULL. Otherwise, it returns
        the next INT call to try. Can be anything, really, but is usually the
        last in the chain.
*/
typedef VOID* (*V86HND)(STDREGS*);

// Indicates to SV86 that there are no more procedures to run
#define SV86_RET_CONSUMED NULL

// Not recommended, but will signal an immediate reflection to SV86.
#define SV86_RET_REFLECT  ((PVOID)(1))

VOID V_HookINTxH(BYTE vector, V86HND hnew, V86HND *out_prev);

/*
        General purpose V86 INT service routine.
        If the stack in regparm is zero, the built in HMA SV86 stack
        is used.

        The register dump is modified after the call.

        This does NOT reflect interrupts to real DOS but can be used to call
        DOS services for DPMI-level support.

        Can be used to call a fake IRQ handler for a V86 user process. The task
        should be scheduled appropriately (exclusive tasking if needed) if
        direct hardware access is needed.

        Can be used with interrupts disabled, but not in an ISR.

        Preemption is blocked within SV86 if it actually reflects to real mode.
*/
VOID V_INTxH(BYTE vector, PSTDREGS regparm);


#endif /* SV86_H */
