///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/*

This module handles entry and exit of kernel mode as well as emulation
of certain instructions involved in this process for V86 mode.

Classes of function used here:
* Stack frame simulation for opcodes
* Interrupt and exception behavior for SV86, user V86, and protected mode.

In SV86:
    IRET will return to caller if isr counter reaches zero.
    INT will accurately emulate the stack but the original
    emulation of INT will not push anything to it.

In Protected Mode:

*/

#include <Scheduler/SysEntry.h>
#include <Scheduler/Process.h>
#include <Platform/BitOps.h>
#include <Misc/StackUtils.h>
#include <Scheduler/V86M.h>
#include <Scheduler/Sync.h>

static EXCEPTION_HANDLER hl_exception_handlers[RESERVED_IDT_VECTORS];

//
// When the INT instruction is called in SV86, this number is incremented.
// IRET decrements it. After each dec/inc, the kernel checks if zero
// and if so, the original caller of virtual 8086 mode is entered.
//

static U8 rm_isr_entrance_counter = 0;

// TODO: Get info about desciptor?

static U8 global_orig_vector;
static U8 global_orig_byte_after_iret;

// BRIEF:
//      This is called by #GP handler only from virtual 8086 mode context.
//
//      Emulating INT and IRET is done by writing a INT operation
//      entirely and memorizing the original vector. The return EIP is adjusted.
//      After handling, it must be written back.
//
//
static VOID ScMonitorV86(P_IRET_FRAME iframe)
{
    const PU8 ins = MK_LP(iframe->cs, iframe->eip);

    const BOOL sv86 = WasSV86();

    switch(*ins)
    {
        case OP_INT:
            global_orig_vector = ins[1];
            ins[1]             = sv86 ? VEC_INT_SV86 : VEC_INT_RMPROC;
            iframe->eip       -= 2;
        break;

        case OP_IRET:
            global_orig_byte_after_iret = ins[1];
            ins[0]                      = OP_INT;
            ins[1]                      = sv86 ? VEC_IRET_SV86 : VEC_IRET_RMPROC;
            iframe->eip                -= 2;
        break;

        default:
        // Wtf?
    }
}

// Reduces cross-referencing in code at a small cost of density
// Not available to drivers.
VOID SetHighLevelExceptionHandler(U8 e, EXCEPTION_HANDLER handler)
{
    hl_exception_handlers[e] = handler;
}

VOID CopyIframeToRing3Context(
    BOOL            dir,
    P_IRET_FRAME    iframe,
    UREGS*          uregs
){
    PU32 source;
    PU32 dest;

    // IFRAME -> UREGS
    source = offsetof(IRET_FRAME, eax);
    dest   = offsetof(UREGS, eax);

    if (dir == 1)
    {
        PU32 temp = source;
        dest = source;
        source = dest;
    }
    *source = *dest;
}

// BRIEF:
//
//      Exceptions and INT calls are handled by this function.
//
//
//
// PARAMS:
//
//  iframe: Trap frame
//  event:  The interrupt vector being invoked.
//
// ENTRY STATE:
//      Interrupts are off
//      Preemption is off by proxy, but the preempt counter can be anything
//      Registers have not been saved anywhere, they are just on the stack
//
__attribute__((regparm(2), aligned(16)))
VOID SystemEntryPoint(
    U8           event,
    P_IRET_FRAME iframe
){
    U32     old_thread_state;
    P_PCB   request_from;

    // We may not NEED this, but I will predictively fetch this value. It is
    // a cheap calculation.
    request_from = GetCurrentPCB();

    // First, we need to copy the registers on the stack into
    // a context into the PCB corresponding with the thread that just entered
    // this function instance.

    // Normally, it is not okay to modify the PCB without acquiring the lock
    // but a system entry cannot be caused more than once by the same process
    // and interrupts are completely off right now, so access is exclusive.

    // We will ONLY save registers to the PCB if it was a real or protected
    // mode process and not SV86.
    if (!g_sv86)
    {
        // Prevent scheduling of tasks because this task is about to be blocked
        // and we do not want it to run when we enable interrupts.
        AtomicFencedInc(&preempt_count);

        // Enable interrupts
        _STI;

        // Get the requesting process PCB
        request_from = GetCurrentPCB();

        // Save iret frame to the user context
        CopyIframeToRing3Context(0, iframe, &request_from->user_regs);

        // Save thread state.
        old_thread_state = request_from->thread_state;

        // Block the process so it is not scheduled when preemption is enabled.
        request_from->thread_state = THREAD_BLOCKED;

        AtomicFencedDec(&preempt_count);
    }
    else {
        // This was SV86. We will now dispatch the event code.
        switch (event)
        {
            // INT from real mode process. Generic stack emulation using
            // the PCB

            // Fun fact: The compiler can optimize repetitive arguments
            // by moving directly to the stack. This is an
            // advantage that only caller cleaning has.

            case VEC_INT_RMPROC:
            {
                U32 push_ip, push_cs, push_flags;

                // The segment register dillema persists. I think branching
                // can be used to save some space and get better locality.

                // Direct access to PCB while process is inactive.
                push_ip    = iframe->eip; // EIP saved is AFTER the INT code.
                push_cs    = iframe->cs;
                push_flags = ;

                RmPushMult16(
                    request_from->user_regs.ss,
                    &request_from->user_regs.esp,
                    3,                              // IP + CS + FLAGS
                    push_ip,
                    push_cs,
                    push_flags,
                );
            }
            break;

            // INT from SV86. Direct access to the trap frame.
            case VEC_INT_SV86:
            break;

        }
    }


    // We will return only if it was an SV86.

    // Now  will hang until reschedule.
    if (!g_sv86)
        while (1);

    // Otherwise, return.
}
