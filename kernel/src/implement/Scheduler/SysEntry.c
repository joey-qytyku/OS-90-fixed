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
#include <Scheduler/IoDecode.h>
#include <Scheduler/Sync.h>

static EXCEPTION_HANDLER hl_exception_handlers[RESERVED_IDT_VECTORS];

//
// OS/90 cannot handle nested exceptions, or really any other asynchronous event
// happening within a non-preemptible context.
//
static U8 in_exception = 0;

//
// When the INT instruction is called in SV86, this number is incremented.
// IRET decrements it. After each dec/inc, the kernel checks if zero
// and if so, the original caller of virtual 8086 mode is entered.
//

static U8 rm_isr_entrance_counter = 0;

// >> A system entry must be preemptible or non-preemptible the entire time?
// Think about it. An exception will never enable preemption.
// The context must always be assumed and never manipulated within an exception
// handler. Some procedures are meant to be used this way and others are not.

// TODO: Get info about desciptor?

// BRIEF:
//      This is called by #GP handler only from virtual 8086 mode context.
//
//      SV86 is used with IOPL=0 at all times, which causes INT and others
//      to be trapped to the monitor in a non-preemptible context and separately
//      from userspace.
//
//      I considered using IOPL=3 for all operations, but this makes system entry
//      way more complicated and require to much knowledge of SV86 and UV86 in one
///     component. Instead, they take different paths.
//
//      We still need to know if it was SV86 because some instructions can be
//      emulated for userspace regardless of IOPL. CLI and STI also
//      behave differently too.
//
//      Emulated instructions: All IO, CLI/STI, INT, IRET
//
static VOID ScMonitorV86(P_IRET_FRAME iframe)
{
    const PU8  ins  = MK_LP(iframe->cs, iframe->eip);
    const BOOL sv86 = WasSV86();

    if (sv86) {
        if (*ins == OP_INT) {
            RmPushMult16(
                iframe->ss,
                &iframe->esp,
                3,
                iframe->eip+2,
                iframe->cs,
                iframe->eflags
            );
        }

        else if (*ins == OP_IRET) {
            // Check counter?
            // Each value is coppied to the pop location in x86 reverse order
            RmPopMult16(iframe->ss, &iframe->esp, 5, &iframe->ss);
        }
        else if (IS_IO_OPCODE(*ins) || IS_IO_OPCODE(ins[1])) {
            IoEmuSV86(ins);
        }
    }
    else {
        //
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
//      Exceptions and INT calls are handled by this function,
//      which includes userspace V86 and protected mode (32-bit and 64-bit).
//
// PARAMS:
//
//  iframe: Trap frame
//  event:  A code representing the exception index OR a special index called
//          SWI. Getting the interrupt vector that was invoked is local to
//          handling INT specifically.
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
    BOOL    thread_was_v86 = request_from->procflags;///////

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

        // 0. Enable interrupts
        _STI;

        // (1) Get the requesting process PCB
        request_from = GetCurrentPCB();

        // (2) Save iret frame to the user context
        CopyIframeToRing3Context(0, iframe, &request_from->user_regs);

        // (3) Save thread state.
        old_thread_state = request_from->thread_state;

        // (4) Block the process so it is not scheduled when preemption is enabled.
        request_from->thread_state = THREAD_BLOCKED;

        AtomicFencedDec(&preempt_count);

        // Now the process is blocked and we are free to service it

        // (5) Was it an exception? If so, dispatch to an exception handler.
        // Reserved vectors is 32. I should expand the table in Intr_Trap.asm
        // so that it includes them

        if (event < RESERVED_IDT_VECTORS)
            hl_exception_handlers[event](0); // TODO ERROR CODE?
        else {
            // INT family instruction caused the entry. INT more likely.
            // We do not
        }
    }

    // Now  will hang until reschedule. This kernel thread will be expunged later.
    if (!g_sv86)
        while (1) {
            __asm__("hlt":::"memory");
        }

    // Otherwise, return.
}
