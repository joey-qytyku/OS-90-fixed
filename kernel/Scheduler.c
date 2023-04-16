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
#include <Scheduler.h>
#include <Farcall.h>
#include <PnP_Mgr.h>
#include <Debug.h>
#include <Type.h>

// Pointer to the current process control block.
INTVAR PTHREAD top_pcb;
INTVAR PTHREAD current_pcb;
INTVAR PTHREAD first_pcb; // The first process

// The mode that the PC switched from
// There are three modes:
// * Kernel
// * User

INTVAR BYTE last_mode = CTX_KERNEL;

// Brief:
//      Loop through each process entry. When the desired PID is found
//      return the address.
// Parameters:
//      pid: The PID
//  Return:
//      NULL if not found
//
// ONLY CALL WITHIN A CRITICAL SECTION
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
            cur = cur->next
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
        current_pcb->blocked = 1;
    break;

    case SCH_BLOCK_PID:
        proc_found = SearchForPID(code >> 16);
        if (proc_found == NULL)
            goto Fail;
        proc_found->blocked = 1;
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
    regs[RD_ESP] = current_pcb->kernel_real_mode_ss;
    regs[RD_SS]  = current_pcb->kernel_real_mode_sp;
}

// When entering an IRQ, EFLAGS.IF must be zero. Otherwise, it is the same as
// the caller.
VOID ScInitIrqStackFrame(PDWORD regs)
{
}

DWORD ScCurrentProgramInProtectedMode(VOID)
{
    return current_pcb->thread32;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////VIRTUAL 8086 MODE SECTION///////////////////////////
////////////////////////////////////////////////////////////////////////////////

static        V86_CHAIN_LINK v86_capture_chain[256];
static INTVAR BOOL supervisor_call = 0;
static INTVAR BOOL iret_is_reenter = 0;
static const  PDWORD real_mode_ivt = (PVOID)0;

static DWORD dos_semaphore_seg_off;

static BYTE bPeek86(WORD seg, WORD off) {return *(PBYTE)MK_LP(seg,off);}
static WORD wPeek86(WORD seg, WORD off) {return *(PWORD)MK_LP(seg,off);}

typedef VOID (FAR_CALL_HOOK_HANDLER_32)(PDWORD);

#define FAR_PROCEDURE_HOOK_REFLECT_DEFAULT 0
#define FAR_PROCEDURE_HOOK_DPMI_LOCAL      1
#define FAR_PROCEDURE_HOOK_KERNEL_GLOBAL   2

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

//------------------------------------------------------------------------------
// Brief:
//  When a userspace process uses IO operations, it is necessary to decode it
//  and fully virtualize the instruction.
//
// opcode:
//  A pointer to the instruction to decode
// bits32:
//  Is the requesting mode 32-bit?
// dec:
//  Output structure
//
// Return:
//  A boolean, false if the instruction is IO, true if not
//  If not IO, contents of the decode struct are invalid
//
// Some opcodes, such as IO, have hidden parameter bits that are not
// documented in the manuals and are instead listed as entirely separate
// instructions.
//
// Layout of an IO opcode:
// |S|1|1|0|V|1|D|W|
//
// Bits if one:
// V: Variable (uses DX)
// S: String
// D: Output
// W: Word
//
static BOOL ScDecodePortOp(
    const PBYTE         opcode,
    BOOL                bits32,
    PDECODED_PORT_OP    dec)
{
    BOOL  size_ovr = 0;
    BOOL  rep_prefix = 0;
    PBYTE op = opcode;

    // An assembler will usually emit the REP prefix before the
    // MASM emits REP first
    // NASM does this too
    // IDK about FASM
    // Most programs of the DOS era were built with MASM, so we should be okay
    // to assume that this is the order, though x86 does not require it

    // Do we have a REP prefix?
    // There will only be F3, the other rep prefix will not be appied
    if (*opcode == 0xF3)
    {
        rep_prefix = 1;
        dec->size_of_op++;
    }

    // Operand size prefix converts an instruction with W=1
    // to 16-bits in protected mode and 32-bits in real mode.
    // They are inverses of each other.
    if (*op == 0x66)
    {
        // We have a size prefix, so update the operand size

        // The only valid way to encode IO with a 66h prefix
        // in real mode is with W=1, this is assumed
        dec->operand_width = 2;
        op = op+1;

        if (bits32)
        {
            // This instruction is being made 16-bit in 32-bit mode
            dec->operand_width = 1;
        }
        else
        {
            // This instruction is being made 32-bit in 16-bit mode
            dec->operand_width = 2;
        }
    }

    // The binary sequence 01100100 represents the bits that are shared
    // by all IO opcode. We still do not know if the main opcode byte
    // is actually IO, so the bitmask is used and compared with the
    // bitmask itself to see if the bits are there.
    if (*op & 0x64 != 0x64)
    {
        // This is NOT an IO opcode.
        return 1;
    }

    // The bit index 3 is on if the port is variable (DX indexed)
    // Bit 0 is the W bit.
    // This applies to string ops
    dec->variable = BIT_IS_SET(*op, 3);

    if (!dec->variable)
        dec->imm8 = op[1];

    // Bit index 1 is on if the instruction is output
    // This also applies to string ops
    dec->in_or_out = BIT_IS_SET(*op, 1);

    // If it's a string op is determined by the top bit being off
    dec->string = !BIT_IS_SET(*op, 7);

    // IO string ops have the variable bit set, so we only need to check
    // that bit to determine the remaining size of the sequence
    if (dec->variable)
    {
        // One byte opcode
        dec->size_of_op++;
    }
    else {
        // Two byte opcode with 8-bit immediate value
        dec->size_of_op += 2;
        dec->imm8 = op[1];
    }
    return 0;
}

VOID APICALL ScHookDosTrap(
                      VINT                      vector,
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
    // IVT. During this process, the INT instruction

    // Changing the context is not enough, I actually need to
    // go to real mode using ScEnterV86, and I have to duplicate
    // the passed context because I should not be changing CS and EIP
    DWORD new_context[RD_NUM_DWORDS];
    C_memcpy(new_context, context, RD_NUM_DWORDS*4);

    new_context[RD_CS]  = real_mode_ivt[vector] >> 16;
    new_context[RD_EIP] = real_mode_ivt[vector] & 0xFFFF;

    IaUseDirectRing3IO();
    // Mem fence?
    supervisor_call = 1;

    ScEnterV86(new_context);

    // The function will return when the monitor initiates a re-entrance
    // After the INT call has been serviced, the monitor must raise an error
    // whenever a normal V86 program tries to use a supervisor opcode
    IaUseVirtualRing3IO();
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
{}

STATUS APICALL AllocateGlobalFarProcHook()
{}

// Far calls work a bit like V86 INT captures. If a 32-bit procedure does
// not replace it, the call will simply go to DOS directly.
//
// API available.
//
STATUS APICALL AllocateProcessLocalFarProcHook(WORD on_behalf_pid, DWORD )
{}

// The V86 monitor for 16-bit tasks, ISRs, and PM BIOS/DOS calls
// Called by GP# handler
VOID ScMonitorV86(IN PDWORD context)
{
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
        case 0xCF: /* IRETW */
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
            // Nuke this process
        break;
        }// END OF SWITCH
    }
    // Emulation of priviledged instructions for userspace
    else if (!supervisor_call)
    {
        PTHREAD pcb = ScGetCurrentPCB();

        switch (*ins)
        {
        case 0xFA: // CLI
            pcb->virtual_irq_on = 0;
        break;

        case 0xFB: // STI
            pcb->virtual_irq_on = 0;
        break;
        default:
            // This may have been an IO instruction. Decode it.
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

    __asm__ volatile ("":::"memory");
    dos_semaphore_seg_off = *(PDWORD)fct;

    for (DWORD i = 0; i<4096/2; i+=2)
    {
        fct[i] = 0xCD;
        fct[i+1]=0xFE;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////END VIRTUAL 8086 MODE SECTION///////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////EXCEPTION HANDLING SECTION//////////////////////////
////////////////////////////////////////////////////////////////////////////////

static VOID DivideByZero(PDWORD regs)
{
}

static VOID GeneralProtectionFault()
{
}

__attribute__((regparm(1)))
VOID ScExceptionDispatch(PDWORD regs)
{
    static EXCEPTION_HANDLER branch_table[VE_NUM_EXCEPT] =
    {
        DivideByZero
    };

    BYTE x = ScGetExceptionIndex();

    branch_table[x](regs);
}

////////////////////////////////////////////////////////////////////////////////// END EXCEPTION HANDLING SECTION //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////// INTERRUPT HANDLING SECTION //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static inline void SendEOI(BYTE vector)
{
    pic_outb(0x20, 0x20);
    if (vector > 7)
        pic_outb(0xA1, 0x20);
}

// IRQ#0 has two functions, update the uptime and switch tasks.
// Right now, task switching is done every 1MS, which means every time
// IRQ#0 is handled.
//
// Interrrupts stay off when handling IRQ#0
//
static VOID HandleIRQ0(PDWORD regs)
{
    static BYTE time_slice_in_progress;
    static WORD ms_left;

    // Update the DOS/BIOS time in BDA?
}

// The master interrupt dispatcher and ISR of all interrupts.
// EAX and EDX pass the arguments for speed
//
// The diff_spurious parameters is 7 if IRQ#7, and 15 if IRQ#15
// Otherwise, the in service register will tell me the IRQ
// and actual_irq will be zero
//
__attribute__(( regparm(2), optimize("align-functions=64") ))
VOID InMasterDispatch(PDWORD regs, DWORD diff_spurious)
{
    const WORD inservice16 = InGetInService16();
    const WORD irq         = BitScanFwd(inservice16);

    /// 1. The ISR is set to zero for both PICs upon SpurInt.
    /// 2. If an spurious IRQ comes from master, no EOI is sent
    /// because there is no IRQ. if it is from the slave PIC
    /// EOI is sent to the master only

    // Is this IRQ#0? If so, handle it directly
    if (BIT_IS_SET(inservice16, 0))
        HandleIRQ0(regs);

    // Is this a spurious IRQ? If so, do not handle.
    if (diff_spurious == 7 && (inservice16 & 0xFF) == 0)
        return;
    else if (diff_spurious == 15 && (inservice16 >> 8) == 0)
        return;

    if (InGetInterruptLevel(irq) == BUS_INUSE)
    {
        InGetInterruptHandler(irq)(regs);
        SendEOI(irq);
    }
    else if (RECL_16)
    {
        ScSimulateRealModeIRQ(irq);
    } else {
        // Critical error
    }
}

////////////////////////////////////////////////////////////////////////////////// DPMI SCHEDULER SUPPORT //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// LDT descriptors are allocated in contiguous blocks, but are freed
// individually. We can solve this problem by having a bit map of
// the LDT descriptors and scanning for free ranges until one is found
// this is not very efficient, but LDT allocation is not called much
// besides for startup and cleanup procedures.

static DWORD ldt_bmp[8192/4];

BOOL APICALL ScGetProcessVirtualInterruptFlag(VOID)
{
    return top_pcb->virtual_irq_on;
}

VOID KeEnableBitArrayEntry(PDWORD array, DWORD inx)
{
    DWORD bit_offset  = inx & 31;
    DWORD dword_index = inx / 32;

    array[dword_index] |= 1 << bit_offset;
}

VOID KeDisableBitArrayEntry(PDWORD array, DWORD inx)
{
    DWORD bit_offset  = inx & 31;
    DWORD dword_index = inx / 32;

    array[dword_index] &= ~(1 << bit_offset);
}

BOOL KeGetBitArrayEntry(PDWORD array, DWORD inx)
{
    DWORD bit_offset  = inx & 31;
    DWORD dword_index = inx / 32;

    return BIT_IS_SET(array[dword_index], bit_offset);
}

STATUS DpmiAllocateLdtEntries(DWORD to_alloc, PDWORD base_index)
{
    DWORD entry;

    // This procedure will probably not return correct input if the count
    // is zero. This is an invalid input.
    if (to_alloc == 0)
        return OS_ERROR_GENERIC;

    for (entry = 0; ; entry++)
    {
        if (entry > LDT_SIZE)
            return OS_ERROR_GENERIC;

        BOOL scan = KeGetBitArrayEntry(ldt_bmp, entry);

        if (scan == 0)
        {
            // Found a free LDT entry
            // We will check for more free LDT entries

            DWORD free_found = 0;

            for (DWORD local_entry = entry; ; local_entry++)
            {
                BOOL scan2 = KeGetBitArrayEntry(ldt_bmp, local_entry);

                // We do not want local entry to go out of bounds
                // if it ever does, we certainly failed to allocate
                if (local_entry > LDT_SIZE)
                    return OS_ERROR_GENERIC;

                if (free_found == to_alloc)
                {
                    // This range is correct. Now we can allocate this range
                    // original ENTRY is at the base of this block
                    for (DWORD i = entry; i< entry+to_alloc; i++)
                        KeEnableBitArrayEntry(ldt_bmp, i);
                    *base_index = entry;
                    return OS_OK;
                }

                if (scan2 == 0)
                    free_found++;

                // I do not need to check this if scan2 == 0 because this
                // will not be true in that case
                else if (scan2 == 1 && free_found != to_alloc)
                {
                    // If we reached the end of this free block and still have
                    // not found enough LDT descriptors, this is not it. This
                    // range of free LDT entries is not the one we need. We
                    // will continue scanning right after this incorrect block
                    // +1 for the terminating one
                    entry = entry + free_found + 1;
                    // If entry is out of bounds, the upper loop will terminate
                    break;
                }
            }
            // We will end up here if the inner loop broke out and could
            // not find the needed block. We need the upper loop to restart
            // now so that it can keep looking
            // -
            // When a for loop is restarted via continue, the update statement
            // will execute
            continue;
        } // END IF
    }// END FOR
    return OS_OK;
}

// Brief:
//      Allocating a DOS block will create at least one descriptor to access
//      the block. If the block is over 64K, several LDT descriptors
//      are created.
//      Why did they do this when a method of converting real mode segments to
//      protected mode descriptors exists? No idea.
//
// We have to allocate a contiguous list of descriptors.
//
// Return value is (the selector << 16) | real mode segment
//
DWORD DpmiAllocateDosBlock(WORD paragraphs)
{}

// Fails silently. Input must be checked before calling.
VOID DpmiFreeLdtEntry(DWORD number)
{
    KeDisableBitArrayEntry(ldt_bmp, number);
}

// DPMI programs can set 32-bit handlers for ^C, critical error, timer tick
// This  is what the spec says:
//
//            Most software  interrupts executed  in real mode will not be
//            reflected to  the protected  mode interrupt hooks.  However,
//            some software  interrupts are  also reflected  to  protected
//            mode programs when they are called in real mode.  These are:
//
//                       INT            DESCRIPTION
//
//                       1Ch    BIOS timer tick interrupt
//                       23h    DOS Ctrl+C interrupt
//                       24h    DOS critical error interrupt
//
// First sentence: What do they mean by "most" interrupts? I suppose all
// except the ones listed.
//
// Second sentance: Okay, if the program is in real mode and one of these are
// called, it will go directly to protected mode by default, so no hook is
// needed? DOS or a 32-bit driver may initiate one of these for a process,
// forcing it to handle it if a handler has been set.
//
// The real mode stack pointer saved on the stack is probably what we would
// use to get the actual
//
STATUS HandleCriticalError()
{
    // Has this program set its own critical error handler?
    PTHREAD cproc = ScGetCurrentPCB();

    if (cproc->ctrl_c_handler_seg_off != 0)
    {
        // In this case, we call INT 24H directly
    }
}

STATUS HandleControlBreak()
{}

// DPMI has a crazy requirement where all protected mode
// interrupt vectors must point to code that reflects it to DOS,
// with the exception of INT 21H AH=4CH and INT 31H (DPMI)
// To make this work, I have to make sure that each IDT entry that is not an IRQ
// or exception is a ring zero vector that points to nothing important.
// This will generate a #GP when called
//
VOID Init_DPMI_ReflectionHandlers()
{
    BYTE entry = NON_SYSTEM_VECTORS;
    BYTE iteration_max_bnd = 256 - entry;

    for (entry=NON_SYSTEM_VECTORS; entry < iteration_max_bnd; entry++)
    {
        MkTrapGate(entry, 0, 0);
    }
}

STATUS ScHandle_INT21H(PDWORD regs)
{
    BYTE ah = (BYTE)(regs[RD_EAX] >> 8);
}

// Brief:
//      This will virtualize an INT instruction when called by a DPMI
//      program. We will change the flow of the program context to do this.
//
//      We must get the descriptor size of the current program CSEG.
//
//
VOID DpmiVirtualIDT_INT(BYTE vector)
{
    PDWORD ctx = current_pcb->context;

}

// Brief:
//      DPMI requires a feature to call real mode routines
//      with a far call stack frame.
//
VOID DpmiFarCallRealModeRoutine(DWORD cs_ip)
{}

// Brief:
//  DPMI requires calling a real mode routine with
//  an interrupt stack frame. The return is an IRET. I am not sure why
//  a program would even use this function call. Maybe the goal is to take
//  control of an IDT IRQ and send it back to DOS.

//  This could be used for calling an interrupt service routine directly, and
//  that is problematic because it could bypass the capture chain. Of course,
//  "simulate real mode interrupt" is there for that purpose.
//
//
VOID DpmiIntCallRealModeRoutine(DWORD cs_ip)
{}

// Called by #GP, everything is handled here
// If the kernel caused the error, we don't care. The main handler
// deals with that.
VOID CheckAndHandleDPMI_Trap(PDWORD regs)
{
    DWORD error_index   = ScGetExceptErrorCode();
    DWORD caused_by_idt = (error_index >> 1)&1;
    BYTE vector = (error_index >> 3) & 0x1FFF;

    if (caused_by_idt)
    {
        // Good, this matters to us now, upper bits of the selector error code
        // represent the index to the IDT entry
        // Now we must handle the virtual PM IDT

        switch (current_pcb->local_idt[vector].type)
        {
        case LOCAL_INT_PM_TRAP:
            // Before we destroy EIP and CS of the program, we must first
            // save it for when IRET is called by the program
            // We are emulating the whole instruction. Nothing is on the stack.
            // The solution is to push values to the stack manually.
            //
            // In case it comes up, the V86 implementation of IRET, while it
            // does not save anything on the stack, EnterV86 does keep track,
            // making it nestable.
            //

            // Set the address to return when handler is done
            regs[RD_EIP] = current_pcb->local_idt[vector].handler_address;
            regs[RD_CS]  = current_pcb->local_idt[vector].handler_code_segment;
        break;

        default:
        break;
        }
    }
}

// When the DPMI program executes IRET, we must go back to the
// cause of the interrupt
VOID HandleDPMI_IRET()
{}

////////////////////////////////////////////////////////////////////////////////// END DPMI SCHEDULER SUPPORT //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// By default, the PIT is set to pitifully slow intervals, clocking at
// about 18.4 Hz (or IRQs per second). This is unsuitable
// for pre-emptive multitasking. We must configure this to a
// more satifactory frequency. I would like about 1000 Hz.
//
// The PIT has a base frequency of 1193800 Hz. We must set the division
// value to 1200 (0x4B0) to get an output frequency of 994.8 Hz.

static VOID ConfigurePIT()
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
