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

#include <Platform/BitOps.h> /* Bit scan forward */
#include <Platform/8259.h>   /* Reading in service register */
#include <Platform/IA32.h>   /* Segment descriptor procedures */
#include <Scheduler.h>
#include <BitArray.h>        /* Bit array procedures for LDT managment */
#include <PnP_Mgr.h>         /* Getting interrupt information */
#include <Debug.h>
#include <Type.h>

LOCK global_kernel_lock = 0;

INTVAR P_PCB current_pcb;
INTVAR P_PCB first_pcb; // The first process

INTVAR DWORD number_of_processes = 0;

// The mode that the PC switched from
// There are three modes:
// * Kernel
// * User
// Do not modify without memory fencing and interrupts off.
INTVAR BYTE last_mode = CTX_KERNEL;

#include <Platform/IA32.h>
#include <Type.h>

// BRIEF:
//      Sometimes, you need to access process memory. This will take into
//      account segmentation and performs calculations automatically
//      based on the current processor mode
//
//
PVOID KERNEL ProcSegmentToLinearAddress(
    P_PCB   proc,
    WORD    seg,
    DWORD   off
){
    PVOID final_address = 0;

    if (proc->protected_mode)
        final_address = (PVOID)(GetDescriptorBaseAddress(seg) + off);
    else
        final_address = (PVOID)((seg << 4U) + (WORD)off);

    final_address += (DWORD)proc->mem_mirror;

    return final_address;
}


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

VOID KERNEL ScOnErrorDetatchLinks(VOID)
{
    C_memset(&v86_capture_chain, '\0', sizeof(v86_capture_chain));
}

VOID KERNEL ScHookDosTrap(
    BYTE                  vector,
    PV86_CHAIN_LINK       ptrnew,
    V86_HANDLER           hnd
){
    const PV86_CHAIN_LINK prev_link = &v86_capture_chain[vector];

    prev_link->next = ptrnew;
    ptrnew->handler = hnd;
    ptrnew->next = NULL;
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
VOID KERNEL ScVirtual86_Int(P_DREGW context, BYTE vector)
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
    DREGW new_context;
    C_memcpy(&new_context, context, sizeof(DREGW));

    new_context.cs  = real_mode_ivt[vector] >> 16;
    new_context.ip =  real_mode_ivt[vector] & 0xFFFF;

    IaUseDirectRing3IO();

    FENCE;
    supervisor_call = 1; // Make wrapper function?

    EnterV86_16(&new_context);

    // The function will return when the monitor initiates a re-entrance
    // After the INT call has been serviced, the monitor must raise an error
    // whenever a normal V86 program tries to use a supervisor opcode
    IaUseVirtualRing3IO();
    FENCE;
    supervisor_call = 0;
}

// The V86 monitor for 16-bit tasks, ISRs, and PM BIOS/DOS calls
// Called by GP# handler
VOID ScMonitorV86(VOID)
{
#if 0
    // The EnterV86 caller expects that the register parameters reflect
    // the output of whatever just ran. This, of course, only happens
    // if it was in fact EnterV86 that caused us to enter V86 mode.

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
    }

    // On return, code continues to execute or re-enters caller
    // of EnterV86
#endif
}

STATUS V86CaptStub()
{
    return CAPT_NOHND;
}


// Virtual 8086 mode cannot be used until this is called.
// This can be called before initializing the scheduler.
VOID InitV86(VOID)
{
    // Add the V86 stub.
    for (WORD i = 0; i<256; i++)
    {
        V86_CHAIN_LINK new = {
            .next = NULL,
            .handler = V86CaptStub
        };
        v86_capture_chain[i] = new;
    }
}

VOID HandleIntV86Reflection(VOID)
{
    P_PCB   pcb;
    PBYTE   mirror;
    BYTE    vector;

    IntsOn();
    AcquireMutex(global_kernel_lock);

    pcb = GetCurrentPCB();
    mirror = pcb->mem_mirror;
    vector = mirror[pcb->user_thread_context.eip - 1];

    ReleaseMutex(global_kernel_lock);
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////END VIRTUAL 8086 MODE SECTION////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////EXCEPTION HANDLING SECTION/////////////////////////
////////////////////////////////////////////////////////////////////////////////

VOID ExceptionDispatch(VOID)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////// END EXCEPTION HANDLING SECTION //////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//////////////////////// INTERRUPT HANDLING SECTION ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static inline void SendEOI(BYTE vector)
{
    delay_outb(0x20, 0x20);
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
        InGetInterruptHandler(irq)();
        SendEOI(irq);
    }
    else if (RECL_16)
    {
        // TODO
    } else {
        // Critical error
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// DPMI SCHEDULER SUPPORT //////////////////////////
////////////////////////////////////////////////////////////////////////////////

static DWORD ldt_bmp[8192/4];

static VOID SetupIDT()
{}
// The PnP manager must be initialized before the scheduler
// Return value is the address to the first PCB
P_PCB InitScheduler()
{
    ConfigurePIT();
}
