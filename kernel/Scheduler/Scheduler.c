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

#include <Scheduler/Core.h>

#include <Platform/BitOps.h> /* Bit scan forward */
#include <Platform/8259.h>   /* Reading in service register */

#include <IA32/TSS.h>
#include <IA32/Segment.h>    /* Reading LDT segment descriptors */
#include <Misc/BitArray.h>   /* Bit array procedures for LDT managment */
#include <PnP/Resource.h>    /* Getting interrupt information */

#include <Debug.h>

#include <Type.h>

ALIGN(4) DWORD preempt_count = 0;

ALL_KERNEL_LOCKS g_all_locks = { 0 };

//
// Does this HAVE to be volatile?
//
INTVAR P_PCB current_pcb;
INTVAR P_PCB first_pcb; // The first process

INTVAR DWORD number_of_processes = 0;

// The mode that the PC switched from
// There are three modes:
// * Kernel
// * User
// Do not modify without memory fencing and interrupts off.
INTVAR BYTE last_mode = CTX_KERNEL;

// BRIEF:
//      Sometimes, you need to access process memory. This will take into
//      account segmentation and performs calculations automatically
//      based on the current processor mode
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

static                  V86_CHAIN_LINK v86_capture_chain[256];
static const    PDWORD  real_mode_ivt = (PVOID)0;
static          DWORD   dos_semaphore_seg_off; // ?
static          BYTE    iret_counter = 0;
static          BYTE    virt_io_strategy = 0;
// If 0: IOPB is used to direct IO
// If 1: All IO is emulated in protected mode.


static BYTE bPeek86(WORD seg, WORD off)
{
    return *(PBYTE)MK_LP(seg,off);
}

static WORD wPeek86(WORD seg, WORD off)
{
    return *(PWORD)MK_LP(seg,off);
}

static VOID bPoke86(WORD seg, WORD off, BYTE val)
{
    BYTE_PTR(MK_LP(seg, off), 0) = val;
}

static VOID wPoke86(WORD seg, WORD off, WORD val)
{
    BYTE_PTR(MK_LP(seg, off), 0) = val;
}

VOID KERNEL ScOnErrorDetatchLinks(VOID)
{
    C_memset(&v86_capture_chain, '\0', sizeof(v86_capture_chain));
}

VOID KERNEL ScHookDosTrap(
    BYTE                  vector,
    PV86_CHAIN_LINK       ptrnew,
    V86_HANDLER           hnd
){
    AcquireMutex(g_all_locks.v86_chain_lock);

    const PV86_CHAIN_LINK prev_link = &v86_capture_chain[vector];

    prev_link->next = ptrnew;
    ptrnew->handler = hnd;
    ptrnew->next = NULL;

    ReleaseMutex(g_all_locks.v86_chain_lock);
}

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
}

// BRIEF:
//      See documentation for more details. There are two strategies for
//      handling IO from supervisory virtual 8086 mode. One of them
//      is to direct all operations to the IOPB and run them directly.
//
//      The other is to deny all access and emulate the whole instruction.
//
//      The second option is very slow for individual port access, but provides
//      near native performance (maybe better) when string operations are used.
//
//      Switching to ring 0 from ring 3 takes approximately 100 clocks.
//
//      ECX value is not zeroed after running!
//
// Rewrite this in ASM?
//

static VOID EmulateDirectIO(
    PBYTE   opcode_addr,
    WORD    dx_val,     // Value of DX saved on stack
    DWORD   ecx_val,    // Number of iterations, must be zeroed by caller
    DWORD   edi_val,
    DWORD   es_val
){
    // We will read combine, no need for several if statements
    // Segment prefixes are not supported.
    // inw and outw are the only operations used for ATA drives,
    // so we mark them as most likely branch.

    if (likely(WORD_PTR(opcode_addr,0) == 0x6EF3)) // HEX: F3 6E
        rep_outsb(MK_LP(es_val, edi_val), ecx_val, dx_val);

    else if (WORD_PTR(opcode_addr,0) == 0x6FF3)
        rep_outsw(MK_LP(es_val, edi_val), ecx_val, dx_val);
    else {
        // Slow emulation. These are less speed critical.

    }
}

VOID ScMonitorV86(VOID)
{
}

STATUS V86CaptStub()
{
    return CAPT_NOHND;
}

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

VOID SystemEntryPoint(DWORD event, PDWORD trap_frame)
{
    if (EVENT_IS_EXCEPTION(event))
    {}
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
        delay_outb(0xA1, 0x20);
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

static VOID SetupIDT()
{}
// The PnP manager must be initialized before the scheduler
// Return value is the address to the first PCB
P_PCB InitScheduler()
{
    ConfigurePIT();
}
