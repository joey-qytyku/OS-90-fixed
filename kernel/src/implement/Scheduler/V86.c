///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                     Copyright (C) 2023, Joey Qytyku                     ///
///                                                                         ///
/// This file is part of OS/90 and is published under the GNU General Public///
/// License version 2. A copy of this license should be included with the   ///
/// source code and can be found at <https://www.gnu.org/licenses/>.        ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

#include <Scheduler/V86.h>
#include <Misc/StackUtils.h>
#include <Scheduler/ScDebug.h>

ATOMIC g_sv86 = ATOMIC_INIT;

ALIGN(4) static V86_CHAIN_LINK v86_capture_chain[256];
ALIGN(4) static U32 dos_semaphore_seg_off; // ???

kernel VOID On_Error_Detatch_Links(VOID)
{
    C_memset(&v86_capture_chain, '\0', sizeof(v86_capture_chain));
}

kernel VOID Hook_Dos_Trap(
    U8                    vector,
    PV86_CHAIN_LINK       ptrnew
){
    Atomic_Fenced_Inc(&preempt_count);

    const PV86_CHAIN_LINK prev_link = &v86_capture_chain[vector];

    prev_link->next = ptrnew;
    ptrnew->next = NULL;

    Atomic_Fenced_Dec(&preempt_count);

}

// BRIEF:
//      A general purpose function for calling virtual 8086 mode INT calls.
//
//      This is called any time the kernel is trying to emulate INT. SV86 is
//      used here.
//      To avoid confusion: there is ZERO relation with the trap frame or the
//      PCB register dump. A separate buffer is used.
//
// WARNINGS:
//      This function does not provide a stack.
//
kernel VOID Svint86(P_RD context, U8 vector)
{
    _not_null(context);

    PV86_CHAIN_LINK current_link;

    // A null handler is an invalid entry
    // Iterate through links, call the handler, if response is
    // CAPT_NOHND, call next handler.
    current_link = &v86_capture_chain[vector];

    // As long as there is another link
    while (current_link->next != NULL) {
        STATUS hndstat = current_link->if_sv86(context);
        if (hndstat == CAPT_HND)
            return;
        else {
            current_link = current_link->next;
            continue;
        }
    }

    Atomic_Fenced_Inc(&preempt_count);
    Raise_SV86();
    {
        _RealModeRegs = *context;

        // EIP and CS are not set properly. Get them from the IVT.
        _RealModeRegs.EIP = WORD_PTR(0, vector * 4);
        _RealModeRegs.CS  = WORD_PTR(0, vector * 4 + 2);

        // Fall back to real mode.
        Enter_Real_Mode(); // Will this inc automatically?


        // Write back results
        C_memcpy(context, &_RealModeRegs, sizeof(SV86_REGS));

    }
    Lower_SV86();
    Atomic_Fenced_Dec(&preempt_count);
}

//
// This is now invalid because I undid my decision to make preemptible
//

// Only to be called from master dispatch
VOID Service_RECL_16(U8 irq_vector)//TODO
{

    const U16 topush[3] = {
        _RealModeRegs.EFLAGS,
        _RealModeRegs.CS,
        _RealModeRegs.EIP
    };

    RM_Push_Mult(
        _RealModeRegs.SS,
        &_RealModeRegs.ESP,
        3,
        topush
    );

    if (Atomic_Fenced_Compare(&g_sv86,1)) {
    } else {
        // Svint86();
    }
}

VOID Do_IRET_RECL_16()
{}

// Works for UV86 and SV86 because we do not read the arguments.
kernel static STATUS V86_Capt_Stub(PVOID unused)

{
    UNUSED_PARM(unused);
    return CAPT_NOHND;
}

VOID Init_V86(VOID)
{
    // Add the V86 stub.
    for (U16 i = 0; i<256; i++)
    {
        V86_CHAIN_LINK new = { NULL, V86_Capt_Stub, V86_Capt_Stub };
        v86_capture_chain[i] = new;
    }
}
