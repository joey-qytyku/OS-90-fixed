#ifndef SCHEDULER_INT
#define SCHEDULER_INT

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

#endif /* SCHEDULER_INT */
