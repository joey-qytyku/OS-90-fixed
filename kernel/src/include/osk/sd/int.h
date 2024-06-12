/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2028, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

#ifndef INT_H
#define INT_H

//
// Not all exceptions can be directly handled by a subsystem and some are
// abstracted, but the virtual exceptions allow for effectively the same
// amount of control by the subsystem.
//
// This abstracts the FPU error IRQ (which is always reserved).
// If the FPU is external, the IRQ handler actually calls HL exception
// handler for floating point errors directly. This may seem to violate
// the concept of preemptible exception handlers, but is actually safe.
// The only context in which the FPU can ever be used is a preemptible
// user one.
//
// The FPU cannot be used by SV86 and doing so causes a system crash.
//
enum {
        VE_DIVZ         = 0,
        VE_DEV_NA       = 7,
        VE_SEG_NP       = 11,
        VE_STKF         = 12,
        VE_GP           = 13,
        VE_TOTAL
};

#include "stdregs.h"
/*
        ISR handles exceptions and IRQs.
*/
typedef void (*ISR)(STDREGS*, LONG error);

/*
        Get in-service register as 16-bit mask, each index corresponds with a
        real IRQ vector.
*/
SHORT GetInService(VOID);

/*
        Get interrupt mask register.
*/
SHORT GetIrqMask(VOID);

/*
        Get a high-level IRQ handler pointer. Can be used for hooking.
*/
ISR GetStage2ISR(VOID);

/*
        Set the ISR.
*/
VOID SetStage2ISR(ISR);

#endif /*INT_H*/
