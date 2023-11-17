///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
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
#include <PnP/Resource.h>    /* Getting interrupt information */

#include <Debug.h>
#include <Type.h>

ATOMIC preempt_count = {0};
ATOMIC g_sv86 = {0};

volatile P_PCB current_pcb;
volatile P_PCB first_pcb; // The first process
volatile U32 number_of_processes;

// A flag that indicates if the last context was SV86. Must be set with
// preemption disabled.

VOID KERNEL PreemptInc(VOID)
{
    AtomicFencedInc(&preempt_count);
}

VOID KERNEL PreemptDec(VOID)
{
    AtomicFencedDec(&preempt_count);
}

BOOL KERNEL Preemptible(VOID)
{
    // TODO
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////VIRTUAL 8086 MODE SECTION///////////////////////////
////////////////////////////////////////////////////////////////////////////////

alignas(4) static V86_CHAIN_LINK v86_capture_chain[256];
alignas(4) static U32 dos_semaphore_seg_off; // ???

VOID KERNEL ScOnErrorDetatchLinks(VOID)
{
    C_memset(&v86_capture_chain, '\0', sizeof(v86_capture_chain));
}

VOID KERNEL HookDosTrap(
    U8                    vector,
    PV86_CHAIN_LINK       ptrnew
){
    AtomicFencedInc(&preempt_count);

    const PV86_CHAIN_LINK prev_link = &v86_capture_chain[vector];

    prev_link->next = ptrnew;
    ptrnew->next = NULL;

    AtomicFencedDec(&preempt_count);

}

////////////////////////////////////////////////////////////////////////////////
// BRIEF:
//      A general purpose function for calling virtual 8086 mode INT calls.
//
//      This is called any time the kernel is trying to emulate INT.
//      To avoid confusion: there is ZERO relation with the trap frame here.
//
// WARNINGS:
//      This function does not provide a stack.
//
VOID KERNEL Svint86(P_SV86_REGS context, U8 vector)
{
    PV86_CHAIN_LINK current_link;

    // A null handler is an invalid entry
    // Iterate through links, call the handler, if response is
    // CAPT_NOHND, call next handler.
    current_link = &v86_capture_chain[vector];

    // As long as there is another link
    while (current_link->next != NULL)
    {
        STATUS hndstat = current_link->handler(context);
        if (hndstat == CAPT_HND)
            return;
        else {
            current_link = current_link->next;
            continue;
        }
    }

    // FALLBACK TO REAL MODE USING IVT
    // In this case, we change the stack itself using the reg buffer

    // We must lock real mode when accessing this structure
    AtomicFencedInc(&preempt_count);

    AssertSV86();

    // Copy parameters to the context.
    C_memcpy(&_RealModeRegs, context, sizeof(SV86_REGS));

    // EIP and CS are not set properly. Get them from the IVT.
    _RealModeRegs.eip = WORD_PTR(0, vector * 4);
    _RealModeRegs.cs  = WORD_PTR(0, vector * 4 + 2);

    // Fall back to real mode.
    EnterRealMode();

    DeassertSV86();
    // Write back results
}

STATUS V86CaptStub()
{
    return CAPT_NOHND;
}

VOID InitV86(VOID)
{
    // Add the V86 stub.
    for (U16 i = 0; i<256; i++)
    {
        V86_CHAIN_LINK new = {
            .next = NULL,
            .handler = V86CaptStub
        };
        v86_capture_chain[i] = new;
    }
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////END VIRTUAL 8086 MODE SECTION////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//////////////////////// INTERRUPT HANDLING SECTION ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static inline void SendEOI(U8 vector)
{
    delay_outb(0x20, 0x20);
    if (vector > 7)
        delay_outb(0xA1, 0x20);
}

//
// BRIEF:
//      Scheduler interrupt handler. This handles task switching.
//
static VOID HandleIRQ0(P_IRET_FRAME iframe)
{
}


__attribute__((regparm(1)))
VOID InterruptDispatch(U32 diff_spurious)
{
    const U16 inservice16 = InGetInService16();
    const U16 irq         = BitScanFwd(inservice16);

    // 1. The ISR is set to zero for both PICs upon SpurInt.
    // 2. If an spurious IRQ comes from master, no EOI is sent
    // because there is no IRQ. if it is from the slave PIC
    // EOI is sent to the master only

    // Is this IRQ#0? If so, handle it directly
    if (BIT_IS_SET(inservice16, 0))
    {
        HandleIRQ0();
        return;
    }

    // Is this a spurious IRQ? If so, do not handle.
    if (diff_spurious == 7 && (inservice16 & 0xFF) == 0)
        return;
    else if (diff_spurious == 15 && (inservice16 >> 8) == 0)
        return;

    if (InGetInterruptLevel(irq) == BUS_INUSE)
    {
//        InGetInterruptHandler(irq)();
        SendEOI(irq);
    }
    else if (RECL_16)
    {
        // TODO
    } else {
        // Critical error
    }
}

VOID SchedulerPhase1(VOID)
{
    ConfigurePIT();
}
