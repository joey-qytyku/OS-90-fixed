/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 2 of the License, or (at your option) any later
    version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with OS/90. If not, see <https://www.gnu.org/licenses/>.
*/


// Note: Variables shared by ISRs and kernel must be volatile because
// they can change unpredictably

// The FPU registers only need to be saved when
// another process tries to use them
// Scheduler priority for FPU switch? No.
//
// NOTE: Interrupts can no longer happen during an ISR.
// This means that INTERRUPT is not a valid previous context.
// We only have kernel and user now

#include <Platform/8259.h>
#include <Platform/IA32.h>
#include <BitArray.h>
#include <Scheduler.h>
#include <Farcall.h>
#include <PnP_Mgr.h>
#include <Debug.h>
#include <Type.h>

INTVAR PTHREAD current_pcb;
INTVAR PTHREAD first_pcb; // The first process

INTVAR DWORD number_of_processes = 0;

// The mode that the PC switched from
// There are three modes:
// * Kernel
// * User
// Do not modify without memory fencing and interrupts off.
INTVAR BYTE last_mode = CTX_KERNEL;

// Brief:
//      Loop through each process entry. When the desired PID is found
//      return the address.
// Parameters:
//      pid: The PID
//  Return:
//      NULL if not found
//
// ONLY CALL WITHIN A CRITICAL SECTION?
//
static PTHREAD SearchForPID(PID pid)
{
    PTHREAD cur = first_pcb;
    while (1)
    {
        if (cur->psp_segment == pid)
            return cur;
        if (cur->next == NULL)
            break;
        else
            cur = cur->next;
    }
    return NULL;
}

//==============================================================================
// code:       Bits 16-31 are the PID. Rest is the function code.
// exit_value: An extra parameter, not used for current process calls.
//
STATUS ASYNC_APICALL ScProcCtl(DWORD code, PVOID exit_value)
{
    PVOID   status = 0;
    PTHREAD proc_found;

    KeUseCritical;
    KeBeginCritSec;

    // It would be really inefficient to scan the processes outside
    // the individual case statements, even if there is less code as a result.

    switch (code)
    {
    case SCH_BLOCK_CUR:
        current_pcb->sched_state = PROC_BLOCKED;
    break;

    case SCH_BLOCK_PID:
        proc_found = SearchForPID(code >> 16);
        if (proc_found == NULL)
            goto Fail;
        proc_found->sched_state = PROC_BLOCKED;
    break;

    case SCH_GET_CUR_PCB_ADDR:
        proc_found = SearchForPID(code >> 16);
        if (proc_found == NULL)
            goto Fail;
        *(PDWORD)exit_value = (DWORD)current_pcb;
    break;

    case SCH_GET_PROG_TYPE_CUR:
        *(PDWORD)exit_value = current_pcb->program_type;
    break;

    case SCH_GET_PROG_TYPE_PID:
        proc_found = SearchForPID(code >> 16);
        if (proc_found == NULL)
            goto Fail;
        *(PDWORD)exit_value = proc_found->program_type;
    break;

    case SCH_GET_CUR_PID:
        *(PID*)exit_value = current_pcb->psp_segment;
    break;
    }
    return OS_OK;
Fail:
    return OS_INVALID_PARAMS;
    KeEndCritSec;
}


// Must be called before calling a virtual software INT. It sets
// SS and ESP before V86 is entered.
//
// When the kernel calls a DOS interrupt vector, a stack must be provided.
//
// The kernel has 32-bit stacks for each process (set in the TSS). Calling
// real mode software requires a stack too.
//
// The kernel may need to call software interrupts when the scheduler has not
// been yet initialized and no programs are running (with their own RM stacks).
//
// If this is the case, this function will detect if the scheduler has enabled
//
VOID ScInitDosCallStackFrame(PDWORD regs)
{
}

// When entering an IRQ, EFLAGS.IF must be zero. Otherwise, it is the same as
// the caller.
static VOID ScInitIrqStackFrame(PDWORD regs)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////VIRTUAL 8086 MODE SECTION///////////////////////////
////////////////////////////////////////////////////////////////////////////////

static        V86_CHAIN_LINK v86_capture_chain[256];
static INTVAR BOOL supervisor_call = 0;
static const  PDWORD real_mode_ivt = (PVOID)0;

static DWORD dos_semaphore_seg_off; // ?

static BYTE bPeek86(WORD seg, WORD off) {return *(PBYTE)MK_LP(seg,off);}
static WORD wPeek86(WORD seg, WORD off) {return *(PWORD)MK_LP(seg,off);}

typedef VOID (*FAR_CALL_HOOK_HANDLER_32)(PDWORD);

#define FPROC_HOOK_REFLECT_DEFAULT 0
#define FPROC_HOOK_DPMI_LOCAL      1
#define FPROC_HOOK_KERNEL_GLOBAL   2

#define NUM_FAR_PROCS 24

typedef struct __PACKED {
    FAR_CALL_HOOK_HANDLER_32 fphnd;
    BYTE    info;
    WORD    pid_owner;
}FAR_PROC;

FAR_PROC far_procedure_hooks[NUM_FAR_PROCS];

VOID ScOnErrorDetatchLinks(VOID)
{
    C_memset(&v86_capture_chain, '\0', sizeof(v86_capture_chain));
}

VOID APICALL ScHookDosTrap(
        VINT                  vector,
    OUT PV86_CHAIN_LINK       new,
    IN  V86_HANDLER           hnd
){
    const PV86_CHAIN_LINK prev_link = &v86_capture_chain[vector];

    prev_link->next = new;
    new->handler = hnd;
    new->next = NULL;
}

//==============================================================================
// Brief:
//  General purpose BIOS/DOS call interface
//  this function goes through capture and should be
//  used by drivers and the kernel to access
//  INT calls from protected mode.
//
//  This is also used to emulate the 16-bit INT instruction. This works because
//  EnterV86 can be nested. This means that an interrupt caused by V86 can call
//  that function again. The TSS is only set the to the current stack location.
//
// The problem is, what happens with the TSS? TSS.ESP0 will be set to the
// process kernel stack when a process is running. It does not need to be at
// an expected value.
//
// EnterV86 uses the stack to save registers and updates the TSS.ESP0 so that
// the process can exit V86.
//
// context:
//      The register params or the state of the 16-bit program.
//      Another function must be used to set the stack to a proper
//      location. This is not done here.
//
// context stack: Automatically set
// for supervisor calls
//
// The INT instruction does not change IF in real mode. The same thing happens
// here. Interrupts can happen, but the kernel is still NON PREEMPTIBLE.
//
// This can be called within an interrupt service routine. ??
//
VOID APICALL ScVirtual86_Int(PDWORD context, BYTE vector)
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

    // Fall back to real mode, in this case, the trap is of no
    // interest to any 32-bit drivers, and must go to the real mode
    // IVT. During this process, the INT instruction will directly
    // call the vector.

    // Changing the context is not enough, I actually need to
    // go to real mode using ScEnterV86, and I have to duplicate
    // the passed context because I should not be changing CS and EIP
    DWORD new_context[RD_NUM_DWORDS];
    C_memcpy(new_context, context, RD_NUM_DWORDS*4);

    new_context[RD_CS]  = real_mode_ivt[vector] >> 16;
    new_context[RD_EIP] = real_mode_ivt[vector] & 0xFFFF;

    IaUseDirectRing3IO();

    FENCE;
    supervisor_call = 1;

    ScEnterV86(new_context);

    // The function will return when the monitor initiates a re-entrance
    // After the INT call has been serviced, the monitor must raise an error
    // whenever a normal V86 program tries to use a supervisor opcode
    IaUseVirtualRing3IO();
    FENCE;
    supervisor_call = 0;
}

// Brief:
//      Calls a far procedure in real mode using a CALL FAR stack frame
//      RETF will terminate the call. This is like virtual INT. Can be nested.
//      This is NOT for DPMI and will call DOS code directly.
//
//      Input regs must be saved by caller if needed.
//
static VOID ScFarProcInvoke(PDWORD regs)
{
}

// Brief:
//      Calls a procedure with a 16-bit interrupt stack frame. This is required
//      by the DPMI specification. IRET already will return to the caller of
//      ScEnterV86, so this is easy to implement.
//
static VOID ScProcIframeInvoke(PDWORD regs)
{
}

// Brief:
//      This function will call an IRQ handler in real mode. The IRQ
//      parameter is 0-15. Input must be checked for correctness.
//
//      The ISR is terminated by an IRET. Interrupts will be OFF
//      when entering this context and will be restored to previous state
//      but there will be no use for this function outside of the master
//      dispatcher.
//
//      Note that there are no register parameters or outputs. The stack
//      and flags are set internally because ISRs do not return or take inputs.
//
static VOID ScSimulateRealModeIRQ(BYTE irq_vector)
{
    // Does regs work the way that I think?
    // How do outputs actually get there?
}

STATUS APICALL AllocateGlobalFarProcHook()
{}

// Far calls work a bit like V86 INT captures. If a 32-bit procedure does
// not replace it, the call will simply go to DOS directly.
//
// API available.
//
STATUS APICALL AllocateProcessLocalFarProcHook(WORD on_behalf_pid)
{}

// The V86 monitor for 16-bit tasks, ISRs, and PM BIOS/DOS calls
// Called by GP# handler
VOID ScMonitorV86(IN PDWORD context)
{
    // The EnterV86 caller expects that the register parameters reflect
    // the output of whatever just ran. This, of course, only happens
    // if it was in fact EnterV86 that caused us to enter V86 mode.
    //

    // After GPF, saved EIP points to the instruction, NOT AFTER
    PBYTE ins   = MK_LP(context[RD_CS], context[RD_EIP]);
    PWORD stack = MK_LP(context[RD_SS], context[RD_ESP]);

    // Software interrupts may be called by anything and must be emulated
    // if that interrupt has not been captured by a driver
    // Interrupt service routines may also call interrupts. Such nesting is
    // possible because ScVirtual86_Int and EnterV86 are thread safe
    // If the INT instruction is found in an ISR, all that has to happen is a
    // change in program flow, with IRET instead representing a return to the
    // 16-bit caller ISR.

    if (*ins == 0xCD)
    {
        ScVirtual86_Int(context, ins[1]);
        return; // Nothing else to do now
    }

    // The following is for emulating privileged instructions
    // These may be used by an ISR or by the
    // kernel to access DOS/BIOS INT calls.

    if (supervisor_call)
    {
        switch (*ins)
        {

        // I am not sure if EnterV86 can be safely called within a GP.
        // It should be able to because EnterV86 will never cause
        // #GP while it is running.

        case 0xCF: /* IRET */
            // Re-enter caller of ScEnterV86, this will not enter
            // immediately, only after the #GP handler returns.
            // Reflecting interrupts also uses EnterV86
            ScOnExceptRetReenterCallerV86();
            return;
        break;

        case 0xFA: /* CLI */
            IntsOff();
            context[RD_EIP]++;
        break;
        case 0xFB: /* STI */
            IntsOn();
            context[RD_EIP]++;
        break;

        case 0xCE: /* INTO */
        break;

        // INT3 and INTO?

        default:
            // IDK
        break;
        }// END OF SWITCH
    }
    // Emulation of priviledged instructions for userspace
    else if (!supervisor_call)
    {
        PTHREAD pcb = ScGetCurrentPCB();

        switch (*ins)
        {
        case 0xCF:
            // IRET encountered in a user program. This could mean:
            // * Program used IRET when it should not have
            // * Program used IRET to exit a fake ISR
            // We can tell based on a flag in the current PCB
            // that indicates if an interrupt is in-progress.
            if (pcb->fake_irq_in_progress && pcb->protected_mode)
            {
                // We now must exit the fake IRQ
            }
            else
            {
                // This program should NOT have used IRET. Terminate the program
                // immediately.
            }

        break;

        case 0xFA: // CLI
            pcb->virtual_irq_on = 0;
        break;

        case 0xFB: // STI
            // STI while in a fake ISR will not enable interrupts
            // the ISR must exit before interrupts can be enabled.
            if (!pcb->fake_irq_in_progress)
                pcb->virtual_irq_on = 0;
        break;
        default:
        break;
        }
    }

    // On return, code continues to execute or re-enters caller
    // of EnterV86
}

STATUS V86CaptStub()
{
    return CAPT_NOHND;
}


// Virtual 8086 mode cannot be used until this is called.
VOID InitV86(VOID)
{
     for (WORD i = 0; i<256; i++)
     {
        V86_CHAIN_LINK new = {
           .next = NULL,
           .handler = V86CaptStub
        };
        v86_capture_chain[i] = new;
     }

    // Setup the far call table. Each entry is an INT 254, INT 253
    // which is CD FE CD FD
    // INT 254 is the emulate far call signal
    // INT 253 is the reenter caller signal
    // The branch table will tell the kernel the index to a list of handlers
    // In this list of handlers, a handler can be local to a process
    //
    // If it is local to a process and another process tries to call it
    // it should go to real mode.
    //
    // The DOS sempahore seg:off is already stored in it, so we must save.

    PBYTE fct = KERNEL_RESERVED_MEM_ADDR;

    FENCE;
    dos_semaphore_seg_off = *(PDWORD)fct;

    for (DWORD i = 0; i<4096/2; i+=2)
    {
        fct[i] = 0xCD;
        fct[i+1]=0xFE;
    }
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////END VIRTUAL 8086 MODE SECTION////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////EXCEPTION HANDLING SECTION/////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define EVENT_FAKEIRQ 0
#define EVENT_EXCEPT  1

// ????
STATUS ScOnNextProcScheduleHandleAsyncEvent(BYTE type)
{}

static VOID DivideByZero(VOID)
{
}

static VOID GeneralProtectionFault(VOID)
{
}

__attribute__(( regparm(1) ))
VOID ExceptionDispatch(VOID)
{
}

// Global trap frame variable?

// Some exceptions will terminate the current program if it has no handler
// available for it in the current CPU mode. The following are those exceptions:
// * Divide by zero
// * Invalid Opcode
// * General Protection Fault
//
// The GP handler will only terminate if the program is not doing anything
// requiring emulation of privileged instructions.
//
// The following exceptions will cause a fatal system error (BSOD):
// * Invalid TSS
// * Double fault
//

////////////////////////////////////////////////////////////////////////////////
////////////////////// END EXCEPTION HANDLING SECTION //////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//////////////////////// INTERRUPT HANDLING SECTION ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static BOOL schedule_tasks = 0;

static inline void SendEOI(BYTE vector)
{
    pic_outb(0x20, 0x20);
    if (vector > 7)
        pic_outb(0xA1, 0x20);
}
//
// BRIEF:
//      Scheduler interrupt handler.
//
static VOID HandleIRQ0()
{
}


__attribute__((regparm(1)))
VOID InterruptDispatch(DWORD diff_spurious)
{
    const WORD inservice16 = InGetInService16();
    const WORD irq         = BitScanFwd(inservice16);

    /// 1. The ISR is set to zero for both PICs upon SpurInt.
    /// 2. If an spurious IRQ comes from master, no EOI is sent
    /// because there is no IRQ. if it is from the slave PIC
    /// EOI is sent to the master only

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
        InGetInterruptHandler(irq)();
        SendEOI(irq);
    }
    else if (RECL_16)
    {
        ScSimulateRealModeIRQ(irq);
    } else {
        // Critical error
    }
}


// BRIEF:
//      Fake an interrupt for a process. If it did not ever hook that vector,
//      this is ignored. This can be called inside an ISR.
//
//      If the program is currently servicing a fake IRQ, it will not recieve.
//
//      The fake IRQ must be directed toward a specific process.
//      This means we have to iterate through the task list.
//      Our function is also safe to call in an interrupt, so it does
//      its work with a critical section
//
//      The scheduler will handle the actual entrance. This procedure only
//      schedules it.
//
// RETURN:
//      True if the interrupt was scheduled successfully
//      False if VIF=0 or already handling a fake IRQ
//
BOOL ASYNC_APICALL ScGenerateFakeIRQ(BYTE irq_vector, PID pid)
{
    KeUseCritical;
    KeBeginCritSec;

    PTHREAD cproc = first_pcb;

    if (cproc->fake_irq_in_progress || !cproc->virtual_irq_on)
        return 0;

    // Now we iterate through every entry in the task chain
    // The bound for i is the number of active processes.
    for (DWORD i = 0; i < number_of_processes; i++)
    {
        if (cproc->psp_segment == pid)
        {
            switch (cproc->program_type)
            {
                case PROGRAM_PM_16:

                break;

                case PROGRAM_PM_32:
                break;

                case PROGRAM_V86:
                break;
            }
            return 1;
        }
    }

    KeEndCritSec;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// DPMI SCHEDULER SUPPORT //////////////////////////
////////////////////////////////////////////////////////////////////////////////

static DWORD ldt_bmp[8192/4];

// Brief:
//      Allocating a DOS block will create at least one descriptor to access
//      the block. If the block is over 64K, several LDT descriptors
//      are created.
//      Why did they do this when a method of converting real mode segments to
//      protected mode descriptors exists? Probably because protected mode
//      does not support segment arithmetic and the 286 is limited to 64K.
//
// We have to allocate a contiguous list of descriptors.
//
// Return value is (the selector << 16) | real mode segment
//
DWORD DpmiAllocateDosBlock(WORD paragraphs)
{}

STATUS HandleCriticalError(PDWORD trap_frame)
{
    // Has this program set its own critical error handler?
    PTHREAD cproc;

    if (cproc->crit_error_seg_off != 0 && cproc->program_type==PROGRAM_V86)
    {
        // Terminate the program. There is no handler to call.
        cproc->sched_state = PROC_DEAD;
    }
    //
    else if (
            cproc->program_type == PROGRAM_PM_32
        &&  cproc->local_idt[0x24].type == LOCAL_PM_INT_TRAP
        &&  cproc->protected_mode
    )
    {
        // This program is is protected mode, so we will reflect INT 24H
        // the to protected mode handler
    }
}

STATUS HandleControlBreak()
{}

VOID DPMI_HandleINT31H()
{}

// DPMI has a crazy requirement where all protected mode
// interrupt vectors must point to code that reflects it to DOS,
// with the exception of INT 21H AH=4CH and INT 31H (DPMI)
// To make this work, I have to make sure that each IDT entry that is not an IRQ
// or exception is a ring zero vector that points to nothing important.
// This will generate a #GP when called
//
VOID Init_DPMI_ReflectionHandlers(VOID)
{
    BYTE entry = NON_SYSTEM_VECTORS;
    BYTE iteration_max_bnd = 256 - entry;

    for (entry=NON_SYSTEM_VECTORS; entry < iteration_max_bnd; entry++)
        MkTrapGate(entry, 0, 0);
}

// Brief:
//      This will virtualize an INT instruction when called by a DPMI
//      program.
//
//      We must get the descriptor size of the current program CSEG.
//
//
VOID DpmiVirtualIDT_INT(BYTE vector)
{
    PDWORD ctx = current_pcb->context;
}

VOID DPMI_Handle_INT(VOID)
{
    const DWORD error_index     =   ScGetExceptErrorCode();
    const DWORD caused_by_idt   =   (error_index >> 1)&1;
    const BYTE  vector          =   (error_index >> 3) & 0x1FFF;
}

// By default, the PIT is set to pitifully slow intervals, clocking at
// about 18.4 Hz (or IRQs per second). This is unsuitable
// for pre-emptive multitasking. We must configure this to a
// more satifactory frequency. I would like about 1000 Hz.
//
// The PIT has a base frequency of 1193800 Hz. We must set the division
// value to 1200 (0x4B0) to get an output frequency of 994.8 Hz.

static VOID ConfigurePIT(VOID)
{
    const BYTE count[2] = {0xB0, 0x4};

    outb(0x43, 0x36);
    outb(0x40, count[0]);
    outb(0x40, count[1]);
}

// The PnP manager must be initialized before the scheduler
// Return value is the address to the first PCB
PTHREAD InitScheduler()
{
    ConfigurePIT();
    Init_DPMI_ReflectionHandlers();
}
