#ifndef SCHEDULER_SYNC_H
#define SCHEDULER_SYNC_H

#include <Type.h>

#define _STI { __asm__ volatile ("sti":::"memory"); }
#define _CLI { __asm__ volatile ("cli":::"memory"); }

typedef DWORD LOCK;

tstruct {
    LOCK    v86_chain_lock;
    LOCK    proc_list_lock;
}ALL_KERNEL_LOCKS;

static inline VOID ReleaseMutex(LOCK *m)
{
    __asm__ volatile (\
    "btrl $0,%0":"=m"(m)::"memory"\
    );
}

static inline VOID AcquireMutex(LOCK *m)
{
    __asm__ volatile (\
        "spin%=:\n\t"\
        "btsl $0,%0\n\t"\
        "jc spin%="\
        :"=m"(m)\
        :"m"(m)\
        :"memory"\
    );
}

static inline BOOL MutexWasLocked(LOCK *m)
{
    BOOL ret;
    __asm__ volatile(
        "cmp $0, %1"
        :"=@ccz"(ret)
        :"m"(m)
        :"memory","cc"
    );
    return ret;
}


static inline DWORD SaveFlags(VOID)
{
    DWORD ret;
    __asm__ volatile (
        "pushf\n\t"
        "pop %0"
        :"=rm"(ret)
        :
        :"memory", "cc"
    );
}

static inline VOID SetFlags(DWORD flags)
{
    __asm__ volatile (
        "push %0"
        "popf"
        :
        :"rm"(flags)
        :"memory", "cc"
    );
}

extern ALL_KERNEL_LOCKS g_all_sched_locks;
extern DWORD preempt_count;

#define _Internal_PreemptDec()\
    __asm__ volatile ("decl %0":"=m"(preempt_count)::"memory")

#define _Internal_PreemptInc()\
    __asm__ volatile ("incl %0":"=m"(preempt_count)::"memory")

VOID KERNEL PreemptInc(VOID);
VOID KERNEL PreemptDec(VOID);


#endif /* SCHEDULER_SYNC_H */
