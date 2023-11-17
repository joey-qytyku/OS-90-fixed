#ifndef SCHEDULER_SYNC_H
#define SCHEDULER_SYNC_H

#include <Type.h>

#define _STI { __asm__ volatile ("sti":::"memory"); }
#define _CLI { __asm__ volatile ("cli":::"memory"); }

// Opaque type. 32-bit.
ALIGN(4)
typedef struct { U32 :32; } ATOMIC,*P_ATOMIC;

#define ATOMIC_INIT {0}

// The m constraint takes the address of the symbol passed to it.
// If we pass the name of the pointer rather than the theoretical
// thing that it points to, it will not interpret it as the address of
// the target object.
//
// This causes GCC to think that we are trying to access the pointer argument
// itself and uses the stack pointer as an address.

static inline VOID ReleaseMutex(P_ATOMIC m)
{
    __asm__ volatile (
       "btrl $0,%0":"=m"(*m)::"memory"
    );
}

static inline VOID AcquireMutex(P_ATOMIC m)
{
    __asm__ volatile (
        "spin%=:\n\t"
        "btsl $0,%0\n\t"
        "jc spin%="
        :"=m"(*m)
        :"m"(*m)
        :"memory"
    );
}

//
// To avoid the use of recursive mutexes, we have a function to check if the
// mutex is currently locked.
//
// REMOVE THIS, WE DONT NEED IT NOW
//
static inline BOOL MutexWasLocked(P_ATOMIC m)
{
    BOOL ret;
    __asm__ volatile(
        "cmpl $0, %1"
        :"=@ccz"(ret)
        :"m"(*m)
        :"memory","cc"
    );
    return ret;
}


static inline U32 SaveFlags(VOID)
{
    U32 ret;
    __asm__ volatile (
        "pushf\n\t"
        "pop %0"
        :"=rm"(ret)
        :
        :"memory", "cc"
    );
}

static inline VOID SetFlags(U32 flags)
{
    __asm__ volatile (
        "push %0"
        "popf"
        :
        :"rm"(flags)
        :"memory", "cc"
    );
}

__attribute__((always_inline))
static inline void AtomicFencedStore(P_ATOMIC address, U32 value)
{
    __asm__ volatile(
        "movl %1,%0"
        :"=m"(*address)
        :"n"(value)
        :"memory"
    );
}

__attribute__((always_inline))
static inline U32 AtomicFencedLoad(P_ATOMIC address)
{
    U32 ret;
    __asm__ volatile(
        "movl %1,%0"
        :"=r"(ret)
        :"m"(*address)
        :"memory"
    );
}

__attribute__((always_inline))
static inline VOID AtomicFencedDec(P_ATOMIC address)
{
    __asm__ volatile ( "dec %0" :"=m"(*address) :"m"(*address) :"memory");
}

__attribute__((always_inline))
static inline VOID AtomicFencedInc(P_ATOMIC address)
{
    __asm__ volatile ( "inc %0" :"=m"(*address) :"m"(*address) :"memory");
}

__attribute__((always_inline))
static inline _Bool AtomicFencedCompare(P_ATOMIC address, U32 imm)
{
    _Bool ret;
    __asm__ volatile (
        "cmp %1, %2"
        : "=@ccz"(ret)
        : "i"(imm), "m"(*address)
        : "cc", "memory"
    );
    return ret;
}

#define K_PreemptDec() AtomicFencedDec(&preempt_count)
#define K_PreemptInc() AtomicFencedInc(&preempt_count)

API_DECL(VOID, PreemptDec, VOID);
API_DECL(VOID, PreemptInc, VOID);

extern U32 preempt_count;
#endif /* SCHEDULER_SYNC_H */
