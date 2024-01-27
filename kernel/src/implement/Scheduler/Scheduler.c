///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                     Copyright (C) 2023, Joey Qytyku                     ///
///                                                                         ///
/// This file is part of OS/90 and is published under the GNU General Public///
/// License version 2. A copy of this license should be included with the   ///
/// source code and can be found at <https://www.gnu.org/licenses/>.        ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////


// Note: Variables shared by ISRs and kernel must be volatile because
// they can change unpredictably

// The FPU registers only need to be saved when
// another process tries to use them
// Scheduler priority for FPU switch? No.
//
// NOTE: Interrupts can no longer happen during an ISR.
// This means that INTERRUPT is not a valid previous context.
// We only have kernel and user now

#include <Scheduler/Core.h>

#include <Platform/BitOps.h> /* Bit scan forward */
#include <Platform/8259.h>   /* Reading in service register */

#include <IA32/TSS.h>
#include <IA32/Segment.h>    /* Reading LDT segment descriptors */
#include <Misc/BitArray.h>   /* Bit array procedures for LDT managment */
#include <PnP/Resource/IRQ.h>

#include <Debug/Debug.h>
#include <Type.h>

// Organize using structs?

// Maybe we can generate a list of contructors in the headers?

ATOMIC preempt_count = ATOMIC_INIT;

// Why volatile? You will only access these when you have authority to do so.

P_PCB current_pcb;
P_PCB first_pcb; // The first process
U32   number_of_processes;

kernel VOID Preempt_Inc(VOID)
{
    Atomic_Fenced_Inc(&preempt_count);
}

kernel VOID Preempt_Dec(VOID)
{
    Atomic_Fenced_Dec(&preempt_count);
}

// BRIEF:
//      Are we in a preemptible context?
//
kernel BOOL Preemptible(VOID)
{
    return Atomic_Fenced_Compare(&preempt_count, 0);
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////// INTERRUPT HANDLING SECTION ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static inline void Send_EOI(U8 vector)
{
    delay_outb(0x20, 0x20);
    if (vector > 7)
        delay_outb(0xA1, 0x20);
}

//
// BRIEF:
//      Scheduler interrupt handler. This handles task switching.
//      todo: Consider doing this in assembly.
//
static VOID Handle_IRQ0(P_RD iframe)
{
}

__attribute__((regparm(1)))
VOID Interrupt_Dispatch(U32 diff_spurious)
{
    const U16 inservice16 = InGetInService16();
    const U16 irq         = BitScanFwd(inservice16);

    // 1. The ISR is set to zero for both PICs upon SpurInt.
    // 2. If an spurious IRQ comes from master, no EOI is sent
    // because there is no IRQ. if it is from the slave PIC
    // EOI is sent to the master only

    // Is this IRQ#0? If so, handle it directly
    if (BIT_IS_SET(inservice16, 0)) {
        // HandleIRQ0();
        return;
    }

    // Is this a spurious IRQ? If so, do not handle.
    if (diff_spurious == 7 && (inservice16 & 0xFF) == 0)
        return;
    else if (diff_spurious == 15 && (inservice16 >> 8) == 0)
        return;

    if (Get_IRQ_Class(irq) == IRQ_INUSE_32) {
        ;

        Send_EOI(irq);
    }
    else if (IRQ_RECL_16) {
        // TODO
    }
    else {
        // Critical error
    }
}

VOID Scheduler_Phase1(VOID)
{
    Configure_PIT();
}
