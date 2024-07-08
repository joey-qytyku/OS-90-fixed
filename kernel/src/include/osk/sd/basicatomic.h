/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef BASICATOMIC_H
#define BASICATOMIC_H

#define STI() { ASM("sti" ::: "memory"); }
#define CLI() { ASM("cli" ::: "memory"); }

typedef struct { LONG _; } ATOMIC32;

#define ATOMIC32_INIT { 0 }

// The m constraint takes the address of the symbol passed to it.
// If we pass the name of the pointer rather than the theoretical
// thing that it points to, it will not interpret it as the address of
// the target object.
//
// This causes GCC to think that we are trying to access the pointer argument
// itself and uses the stack pointer as an address.

__attribute__((always_inline))
static inline VOID RELEASE_MUTEX(ATOMIC32 *m)
{
        ASM("btrl $0,%0" : "=m"(*m)::"memory");
}

__attribute__((always_inline))
static inline VOID ACQUIRE_MUTEX(ATOMIC32 *m)
{
        ASM(
            "spin%=:\n\t"
            "btsl $0,%0\n\t"
            "jc spin%="
            : "=m"(*m)
            : "m"(*m)
            : "memory");
}

__attribute__((always_inline))
static inline BOOL MUTEX_WAS_LOCKED(ATOMIC32 *m)
{
        BOOL ret;
        ASM(
            "cmpl $0, %1"
            : "=@ccz"(ret)
            : "m"(*m)
            : "memory", "cc");
        return ret;
}

__attribute__((always_inline))
static inline LONG SAVE_FLAGS(VOID)
{
        LONG ret;
        ASM(
            "pushf\n\t"
            "pop %0"
            : "=rm"(ret)
            :
            : "memory", "cc");
        return ret;
}

__attribute__((always_inline))
static inline VOID SET_FLAGS(LONG flags)
{
        ASM(
            "pushl %0\n"
            "popf"
            :
            : "rm"(flags)
            : "memory", "cc");
}

__attribute__((always_inline))
static inline VOID ATOMIC32_FENCED_STORE(ATOMIC32 *address, LONG value)
{
        ASM(
            "movl %1,%0"
            : "=m"(*address)
            : "n"(value)
            : "memory");
}

__attribute__((always_inline))
static inline LONG ATOMIC32_FENCED_LOAD(ATOMIC32 *address)
{
        LONG ret;
        ASM(
            "movl %1,%0"
            : "=r"(ret)
            : "m"(*address)
            : "memory");
        return ret;
}

__attribute__((always_inline))
static inline VOID ATOMIC32_FENCED_DEC(ATOMIC32 *address)
{
        ASM("decl %0" : "=m"(*address) : "m"(*address) : "memory");
}

__attribute__((always_inline))
static inline VOID ATOMIC32_FENCED_INC(ATOMIC32 *address)
{
        ASM("incl %0" : "=m"(*address) : "m"(*address) : "memory");
}

__attribute__((always_inline))
static inline BOOL ATOMIC32_FENCED_COMPARE32(ATOMIC32 *address, LONG imm)
{
        BOOL ret;
        ASM(
            "cmpl %1, %2"
            : "=@ccz"(ret)
            : "i"(imm), "m"(*address)
            : "cc", "memory");
        return ret;
}

#define PREEMPT_DEC() ATOMIC32_FENCED_DEC(&g_preempt_count)
#define PREEMPT_INC() ATOMIC32_FENCED_INC(&g_preempt_count)

extern LONG g_preempt_count;

#endif /* BASICATOMIC_H */
