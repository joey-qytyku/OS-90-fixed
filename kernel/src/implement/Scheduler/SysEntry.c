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

#include <Platform/BitOps.h>

#include <Scheduler/Process.h>
#include <Scheduler/IoDecode.h>
#include <Scheduler/V86M.h>
#include <Scheduler/Sync.h>

#include <Misc/StackUtils.h>
#include <Misc/Segutils.h>

#include <Debug/Debug.h>

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
}

// SEP DO Procedures //

// Perform INT instruction for a real mode process.
// Procedure is:
// * Push flags
// * Push CS
// * Push EIP
//
// The stack is accurately emulated.
//
U32 IntSepDo_Realmode_16(P_IRET_FRAME ifr)
{
    assert(ifr->eflags & (1<<17));

    U16 ss  = ifr->ss;
    U32 esp = ifr->esp;

    // Maybe just optimize to a simple backward copy
    const U16 topush[3] = {ifr->eflags, ifr->cs, ifr->eip};

    RmPushMult16(ss, esp, 3, topush);
    return 0;
}

U32 IntSepDo_ProtMode(P_IRET_FRAME ifr)
{
    assert(ifr->eflags & (1<<17) == 0); // Not V86
    assert(ifr->cs & 0b11 != 0);        // Not kernel

    // Get bitness of the code segment
    const BOOL pm32 = (SegmentUtil(SEG_GET_ACCESS_RIGHTS, ifr->ss, 0) >> 5) & 3;

    // Popping a 16-bit value to eflags will only change the bottom half
    // Keep this in mind.

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

__attribute__((regparm(2), aligned(16)))
VOID SystemEntryPoint(
    U8           event,
    P_IRET_FRAME iframe
){
    U32     old_thread_state;
    P_PCB   request_from;
    request_from = GetCurrentPCB();


}
