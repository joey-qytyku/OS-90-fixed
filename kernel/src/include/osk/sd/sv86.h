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
/*
 * If the call is consumed, returns NULL. Otherwise, it returns
 * the next INT call to try. Can be anything, really, but is usually the
 * last in the chain.
 */
typedef VOID* (*V86HND)(STDREGS*);

#define SV86_RET_CONSUMED NULL

// Not recommended
#define SV86_RET_REFLECT  ((PVOID)(1))

VOID OS_HookINTxH(BYTE vector, V86HND hnew, V86HND *out_prev);

/*
        General purpose V86 INT service routine.
        If the stack in regparm is zero, the built in HMA SV86 stack
        is used.

        This does NOT reflect interrupts to real DOS but can be used to call
        DOS services for DPMI-level support.

        Can be used to call a fake IRQ handler for a V86 user process. The task
        should be scheduled appropriately (exclusive tasking if needed) if
        direct hardware access is needed.

        Cannot be used with interrupts disabled.

        Does not disable preemption when running. It has a lock that prevents
        more than one process from accessing SV86, which protects the SV86
        handler table as well.

        Preemption is blocked within SV86 if it actually reflects to real mode.

*/
VOID OS_INTxH_t12(BYTE vector, PSTDREGS regparm);

#endif /* SV86_H */
