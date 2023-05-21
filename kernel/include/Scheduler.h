/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Platform/IA32.h>
#include <Platform.BitOps.h>
#include <Platform/IO.h>
#include <Type.h>

#define ALLOWED_REAL_MODE_EXCEPTIONS 8

/////////////////////////////
// DPMI and scheduler defs //
/////////////////////////////

//
// This MUST be the same as IRQ_BASE in IA32.asm
// The index specified here and the following 15 vectors
// are reserved for interrupt requests, both real and fake.
//
// The reason this is done is that the DPMI program can find out which vectors
// are to be used for IRQs. They can never be called by the INT instruction.
// This means that it is safe to reserve these vectors as they will only be
// used for interrupt requests by the OS and software.
//
// Vectors commonly used by software usually started with a number 0-9 for
// programming convenience. INT A0-AF is unused according to RBIL.
//
#define IRQ_RESERVED_VECTOR 0xA0

#define CTX_KERNEL 0
#define CTX_USER   1

enum {
    PS_ACTIV,
    PS_BLOCK,
    PS_KERNL,
    PS_FINTS,
    PS_FIINP,
    PS_FXNTS,
    PS_FXINP
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

// The previous context is of no concern to a driver.
// DPMI allows the PIC base vectors to be reported through the get version
// function call.
//
// 2.4.2 says that ints 0-7 turn off the virtual interrupt flag
//
// Hooking protected mode exceptions is done with a special function call
// that uses a specific range of indices. The client can, however, hook
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

    // The program set an IRQ vector in real mode and expects it to be reflected
    // to real mode.
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

    // Interrupt state is always off while handling exceptions, regardless of
    // the DPMI spec.
    LOCAL_PM_EXCEPTION
};

// I literally cannot compress this any further :(

typedef struct PACKED
{
    DWORD   handler_eip     :28;
    BYTE    type            :3;
    WORD    handler_cseg;
}LOCAL_PM_IDT_ENTRY;

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

//////////////////////////////////
// I M P O R T   S E C T I O N  //
//////////////////////////////////

#define ScContextWasV86() _bWasV86
#define ScGetExceptionIndex() _dwExceptIndex
#define ScGetExceptErrorCode() _dwErrorCode

extern VOID ScOnExceptRetReenterCallerV86(VOID);    // Defined in vm86.asm
extern VOID EnterV86_16(P_DREGW); // Defined in vm86.asm

extern DWORD _dwErrorCode;
extern DWORD _dwExceptIndex;
extern BYTE  _bWasV86;

extern BOOL ScDecodePortOp(
    PBYTE,
    BOOL,
    PDECODED_PORT_OP
);

//////////////////////////////////
// E X P O R T   S E C T I O N  //
//////////////////////////////////

/////////////////
// I N L I N E //
/////////////////

static inline PVOID MK_LP(WORD seg, WORD off)
{
    DWORD address = seg*16 + off;
    return (PVOID) address;
}

static inline P_PCB GetCurrentPCB(VOID)
{
    register DWORD sp __asm__("sp");
    return (P_PCB)(sp & (~0x1FFF));
}

#endif /* SCHEDULER_H */
