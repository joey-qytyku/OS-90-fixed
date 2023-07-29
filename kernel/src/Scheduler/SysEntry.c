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
* Stack frame simulation for opcodes (Sim_xxx)
* Interrupt and exception behavior for SV86, user V86, and protected mode.
  (Do_xxx)

In SV86:
    IRET will return to caller if isr counter reaches zero.
    INT will accurately emulate the stack but the original
    emulation of INT will not push anything to it.

In Protected Mode:

*/

#include <Scheduler/SysEntry.h>
#include <Scheduler/V86M.h>
#include <Scheduler/Process.h>
#include <Platform/BitOps.h>
#include <Scheduler/Sync.h>

static EXCEPTION_HANDLER hl_exception_handlers[RESERVED_IDT_VECTORS];

//
// When the INT instruction is called in SV86, this number is incremented.
// IRET decrements it. After each dec/inc, the kernel checks if zero
// and if so, the original caller of virtual 8086 mode is to be called.
//

static U8rm_isr_entrance_counter = 0;

// TODO: Get info about desciptor?

static U8global_orig_vector;
static U8global_orig_byte_after_iret;

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
    PU8ins = MK_LP(iframe->cs, iframe->eip);

    const BOOL sv86 = MutexWasLocked(&g_all_sched_locks.v86_chain_lock);

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
VOID SetHighLevelExceptionHandler(U8e, EXCEPTION_HANDLER handler)
{
    hl_exception_handlers[e] = handler;
}

tstruct {
    P_PCB   caused_process;
    U32   event_id;
    P_IRET_FRAME iframe;
}HANDLE_EVENT_INFO;

//
// Exceptions that occur in real mode will go through the per-process IVT.
// They can also nest, so accurate stack emulation is necessary.
//
static VOID DoRealModeException(HANDLE_EVENT_INFO info)
{
    WORD ip;
    WORD cs;

    // Was this vector modified? If it was NOT modified, the process should
    // be terminated.

    FAR_PTR_16 jump_to = info.caused_process->rm_local_ivt[info.event_id];

    if (U32_PTR(&jump_to, 0) == 0)
    {
        // The process will be terminated because it caused an exception
        // for which it did not set a handler.
    }

    ip = info.caused_process->rm_local_ivt[info.event_id].off;
    cs = info.caused_process->rm_local_ivt[info.event_id].seg;
}

////////////////////////////////////////////////////////////////////////////////
//                     I N T / I R E T   E m u l a t i o n                    //
////////////////////////////////////////////////////////////////////////////////

static VOID DoIretSv86(HANDLE_EVENT_INFO info)
{
    // EIP now points after the new INT imm8 instruction.
    // One U8before is where we want to write back our value.
    PU8next_ins = MK_LP(info.iframe->cs, info.iframe->eip);

    next_ins[-1] = global_orig_byte_after_iret;

    // Now that we wrote back the U8after IRET, we need to emulate the
    // stack by popping the necessary values into the IRET frame.
    // Remember that IRET is not a termination code. It is emulated like any
    // other privileged instruction.
    PWORD stack = MK_LP(info.iframe->ss, info.iframe->esp);
    info.iframe->eip = stack[0]; // ?
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
}

// BRIEF:
//
// ENTRY STATE:
//      Interrupts are off
//      Preemption is off by proxy, but the preempt counter can be anything
//      Registers have not been saved anywhere, they are just on the stack
//
VOID SystemEntryPoint(
    U8           event,
    P_IRET_FRAME    iframe
){
    U32   old_thread_state;
    P_PCB   request_from;
    BOOL    was_sv86;

    // First, we need to copy the registers on the stack into
    // a context into the PCB corresponding with the thread that just entered
    // this function instance.

    //
    // Normally, it is not okay to modify the PCB without acquiring the lock
    // but a system entry cannot be caused more than once by the same process
    // and interrupts are completely off right now, so access is exclusive.
    //

    was_sv86 = MutexWasLocked(&g_all_sched_locks.v86_chain_lock);

    // We will ONLY save registers to the PCB if it was a process and not SV86.
    if (!was_sv86)
    {
        // Prevent scheduling of tasks because this task is about to be blocked
        // and we do not want it to run when we enable interrupts.
        _Internal_PreemptInc();

        // Enable interrupts
        _STI;

        // Get the requesting process PCB
        request_from = GetCurrentPCB();

        // Save iret frame to the user context
        CopyIframeToRing3Context(0, iframe, &request_from->user_regs);

        // Save thread state.
        old_thread_state = request_from->thread_state;

        // Block the process.
        request_from->thread_state = THREAD_BLOCKED;

        //
    }

    // If we reached here, it may be an exception or an INT.

    // We will return only if it was an SV86.

    AcquireMutex(&g_all_sched_locks.proc_list_lock);

    request_from->thread_state =
        request_from->program_type == PROGRAM_V86 ? PROGRAM_V86 : THREAD_RUN_PM;

    ReleaseMutex(&g_all_sched_locks.proc_list_lock);

    // Now we will hang until reschedule.
    if (was_sv86)
        here: goto here;
}
