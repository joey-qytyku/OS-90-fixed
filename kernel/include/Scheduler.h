/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Platform/IA32.h>
#include <Type.h>

#define ALLOWED_REAL_MODE_EXCEPTIONS 8

// This number must be obtained dynamically. It does not change.
#define _INTERNAL_MAX_WORKER_THREADS 24


// Get the byte[1] of the register, eg, GET_XH(EAX) == AH
#define GET_XH(reg_value) ((reg_value >> 8) & 0xFF)

enum {
    SCH_GET_CUR_PCB_ADDR,
    SCH_GET_CUR_PID,

    SCH_BLOCK_PID,
    SCH_BLOCK_CUR,

    SCH_GET_PROG_TYPE_CUR,
    SCH_GET_PROG_TYPE_PID,
    SCH_GET_VINT_STATE
};

/////////////////////////////
//     Synchronization     //
/////////////////////////////

#define KeUseCritical DWORD __critical_eflags

#define KeBeginCritSec \
    __asm__ volatile(              \
        "pushfd" ASNL              \
        "pop %0" ASNL              \
        "cli"                      \
        :"=rm"(__critical_eflags)::);

#define KeEndCritSec \
    __asm__ volatile(\
        "push %0" ASNL\
        "popfd"   ASNL\
        ::"rm"(__critical_eflags));

/////////////////////////////
// DPMI and scheduler defs //
/////////////////////////////

#define MASTER_PIC_VIRTUAL_BASE = 0x90,
#define SLAVE_PIC_VIRTUAL_BASE  = 0x98

#define CTX_KERNEL 0
#define CTX_USER   1


enum {
    PSTATE_DEAD    = 0,
    PSTATE_BLOCKED = 1,
    PSTATE_RUNNING = 2,

    PSTATE_FAKEIRQ_AWAIT = 3,
    PSTATE_FAKEIRQ_IN_PROGRESS = 4,

    PSTATE_EXCEPT_AWAIT = 5,
    PSTATE_EXCEPT_IN_PROGRESS = 6
};

typedef enum {
    VE_DE,
    VE_DB,
    VE_NMI,
    VE_BP,
    VE_OF,
    VE_BR,
    VE_UD,
    VE_NM,
    VE_DF,
    VE_SO,
    VE_TS,
    VE_NP,
    VE_SS,
    VE_GP,
    VE_PF,
    _VE_RES,
    VE_MF,
    VE_AC,
    VE_MC,
    VE_XM,
    VE_NUM_EXCEPT
}VEXC_VECTOR;

// Bitness of programs is determined by how they are initialized, not by their
// code segment descriptor size. If a program enters 32-bit DPMI, it will be
// recognized as 32-bit and some function calls are slightly different.

enum {
    PROGRAM_PM_32,      // DOS program in 32-bit PM
    PROGRAM_PM_16,      // DOS program in 16-bit PM
    PROGRAM_V86,        // DOS program in real mode
    PROGRAM_NATIVE,     // The program has access to system calls and not DOS
    PROGRAM_TYPES
};

// The previous context is of no concern to a driver.
// DPMI allows the PIC base vectors to be reported through the get version
// function call.
//
// 2.4.2 says that ints 0-7 turn off the virtual interrupt flag
//
// Hooking protected mode exceptions is done with a special function call
// that uses a specific range of indices. The client can, howver, hook
// a vector that would normally be an interrupt vector. It should not be doing
// this anyway, but the INT instruction will be virtualized.

enum {
    // If the program calls INT with this, reflect to DOS
    // using the global capture chain
    // The default for DOS except for INT 21H AH=4C and INT 31H
    //
    // PROTECTED MODE EXCEPTIONS ARE NOT REFLECTED TO REAL MODE
    // AND WILL RESULT IN PROGRAM TERMINATION IF UNHANDLED.

    LOCAL_PM_INT_REFLECT_GLOBAL = 0,

    // Same as last but it will disable interrupts upon entry.
    // Is this correct?
    LOCAL_PM_IRQ_REFLECT_GLOBAL,

    // This interrupt vector points to an ISR for a fake IRQ
    // It should not be called by INT or program crashes
    LOCAL_PM_INT_IDT_FAKE_IRQ,

    // This interrupt vector was modified by the program
    // to use a PM trap handler
    // If INT is called, it is caught by the #GP handler
    LOCAL_PM_INT_TRAP,

    // This interrupt reflects to a modified local real mode interrupt vector
    LOCAL_V86_INT_REFLECT,

    LOCAL_PM_EXCEPTION,
    LOCAL_PM_EXCEPTION_VINTS_OFF
};

typedef struct __PACKED
{
    DWORD   handler_eip     :29;
    BYTE    type            :3;
    WORD    handler_cseg;
}LOCAL_PM_IDT_ENTRY;

typedef VOID (*WORKER_THREAD_PROC);

typedef struct {
    WORKER_THREAD_PROC      procedures[_INTERNAL_MAX_WORKER_THREADS];
}KERNEL_THREAD_LIST;

// Performance sensitive. Remember to align data if needed since the structure
// is packed. This structure MUST be no larger than 4K.
//
typedef struct __PACKED
{
    DWORD   kernel_pm_stack;
    DWORD   context[RD_NUM_DWORDS];
    DWORD               user_page_directory_entries [64];
    // Exceptions share the same IDT. This might make standard compliance
    // a bit shaky, but it is more space efficient and a program should not
    // bother with these vectors anyway.
    LOCAL_PM_IDT_ENTRY  local_idt                   [256];

    // Real mode control section
    DWORD   rm_local_ivt[256];
    DWORD   rm_kernel_ss_esp;
    WORD    rm_subproc_exit_code;
    PID     psp_segment;

    DWORD   ctrl_c_handler_seg_off;
    DWORD   crit_error_seg_off;

    WORD    vpic_mask;  // By default, all IRQs are masked

    // Flags related to the process
    BYTE    program_type            :2; // [1]
    BYTE    virtual_irq_on          :1;
    BYTE    fake_irq_in_progress    :1;
    BYTE    fake_irq_pending        :1;
    BYTE    use87                   :1;
    BYTE    sched_state             :2;
    BYTE    protected_mode          :1;

    BYTE    vector_to_invoke:4; // [2]

    // [1]: The type of program. If the program enters protected
    // mode, it will permanently become a 16/32-bit protected mode
    // program. Raw mode switching will not change this, but it will
    // change protected_mode
    //
    // [2]: Vector for the scheduler tick to use for an exception or interrupt

    // Add subprocess stack. It contains:
    // * PSP
    // * Size of allocation
    // *

	PVOID   x87env;
    PVOID   next;
    PVOID   last;

}THREAD,*PTHREAD;

typedef struct {
    DWORD   regs[9]; // eax, ebx, ecx, edx, esi, edi, ebp, esp, eflags
}KERNEL_FIBER;

// static int x = sizeof(THREAD);
// *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
// Unfortunately, we cannot compress these into 24-bit linear addresses without
// losing the original code segment value.
//
// If a vector is zero, it is reflected to the actual real mode vector. If not
// we call the DPMI local vector.
//
// Vectors for IRQs can be modified, and doing so will invoke interrupt faking
// when in real mode. The same effect applies to the first eight exceptions.
//
// Exceptions are sometimes modified by software. For example, floating point
// or divide by zero. There is no reason for software to modify any 286/386
// vectors because they are never invoked in real mode and overlap with the
// PIC default vectors.

////////////////////////////
// V86 and Virtualization //
////////////////////////////

// V86 handler return values
#define CAPT_HND   0 /* Handled captured trap */
#define CAPT_NOHND 1 /* Did not handle */

// Return value of a V86 chain handler is zero if handled, 1 if refuse to handle
typedef STATUS (*V86_HANDLER)(PDWORD);
typedef VOID   (*EXCEPTION_HANDLER)(PDWORD);

typedef struct
{
    V86_HANDLER handler;  // Set the handler
    PVOID next;           // Initialize to zero
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

// When emulating port IO instructions, this structure is generated after
// decoding
typedef struct
{
    BYTE    operand_width:2;// Operand size, in order 8,16,32
    BYTE    in_or_out:1;    // Input if 0, output if 1
    BYTE    variable:1;     // Uses DX as address, implied by string
    BYTE    size_of_op:3;   // Number of bytes
    BYTE    string:1;       // INS/OUTS
    BYTE    repeat:1;       // Repeat CX times
    BYTE    imm8;           // Valid if !variable
}DECODED_PORT_OP, *PDECODED_PORT_OP;

typedef struct {
    PVOID   next;
    PIMUSTR screen_name;
    WORD    irq_bmp;
    WORD    io_base;
    BYTE    io_size;
}VIRTUAL_DEVICE;

////////////////////
// IMPORT SECTION //
////////////////////

#define ScContextWasV86() _bWasV86
#define ScGetExceptionIndex() _dwExceptIndex
#define ScGetExceptErrorCode() _dwErrorCode

extern VOID ScOnExceptRetReenterCallerV86(VOID);    // Defined in vm86.asm
extern VOID ScEnterV86(PDWORD); // Defined in vm86.asm

extern DWORD _dwErrorCode;
extern DWORD _dwExceptIndex;
extern BYTE  _bWasV86;

////////////////////
// EXPORT SECTION //
////////////////////

extern APICALL VOID ScHookDosTrap(
    VINT,
    PV86_CHAIN_LINK,
    V86_HANDLER
);

extern VOID ScOnErrorDetatchLinks(VOID);
extern VOID ScVirtual86_Int(PDWORD, BYTE);

////////////
// INLINE //
////////////

static inline PVOID MK_LP(WORD seg, WORD off)
{
    DWORD address = seg*16 + off;
    return (PVOID) address;
}

extern VOID APICALL_REGPARM(2) KeDisableBitArrayEntry(PDWORD, DWORD);
extern BOOL APICALL_REGPARM(2) KeGetBitArrayEntry(PDWORD, DWORD);
extern VOID KeEnableBitArrayRange(PDWORD, DWORD, DWORD);

extern STATUS KeAllocateBits(
    PDWORD,
    DWORD,
    DWORD,
    PDWORD
);

extern STATUS AllocateOneBit(
    PDWORD,
    DWORD
);

extern PDWORD _global_trap_frame;

#endif /* SCHEDULER_H */
