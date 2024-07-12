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
