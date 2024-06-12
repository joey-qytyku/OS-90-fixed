/*
  ��������������������������������������������������������������������������ͻ
  �                  Copyright (C) 2023-2028, Joey Qytyku                    �
  ��������������������������������������������������������������������������͹
  � This file is part of OS/90 and is published under the GNU General Public �
  �   License version 2. A copy of this license should be included with the  �
  �     source code and can be found at <https://www.gnu.org/licenses/>.     �
  ��������������������������������������������������������������������������ͼ
*/

#ifndef TASK_H
#define TASK_H

#include "stdregs.h"

#define SUBSYS_BLOCK_SIZE 28

typedef PVOID (*PAGE_GET)(VOID);
typedef VOID (*KTHREAD_PROC)(PVOID);

typedef BOOL (*T0_TASK_PREHOOK)(STDREGS*);
typedef VOID (*T0_TASK_POSTHOOK)(STDREGS*);

// The task exit hook runs in T2, not T0.
typedef VOID (*T2_TASK_EXITHOOK)(STDREGS*);

typedef struct Task_ {
        STDREGS regs;
        PVOID   _next;
        PVOID   _prev;
        SHORT   _time_slices;
        SHORT   _counter;
        LONG    flags;
        LONG    preempt;

        T0_TASK_PREHOOK pre;
        T0_TASK_POSTHOOK post;
        T2_TASK_EXITHOOK onexit;

        char name[8]; // Need this? I guess for debugging.

        BYTE    _subsystem_block[SUBSYS_BLOCK_SIZE];
}TASK, *PTASK;

#define TFLAG_MASK 0b111

/*******************************************************************************
About the task structure

An entire task is represented by 4096 bytes of memory. The TASK structure
is followed by a 2048-byte stack space.

The ID of a process is the address of the TASK.

Fields:

regs: This is the active context of the thread. It can be kernel mode or
       user mode. The scheduler switches to this when the task is scheduled
       or otherwise switched to. After a system entry, the saved user
       registers are on the stack, but this is of no interest to the
       scheduler and only that of the task, which is now in ring-0.

next,prev:      The tasks are a doubly-linked circularlist. There is no definite
                beginning or end to it. When a task is terminated, the links are
                crossed over and the memory is deleted.

time_slices:    The number of time slices this task gets when scheduled.
                It is a count of how many times the task is rescheduled by
                IRQ#0 instead of switching to the next task.

counter:        A decrementing counter that represents the number of time
                slices left to give the process.

arch:           The architecture being "emulated" by the subsystem or kernel.
                Does not mean a whole lot.

pre,post,onexit:        These are scheduler hooks. Scheduler hooks can be
                        used to simulate the existence of isolated address
                        spaces by quickly changing page mappings,
                        or to set the contents of the TSS IOPB.
                        Set to NULL and they will not run.

                        These run regardless of the process mode.

                        Note that `onexit` is called within a preemptible context.
                        The rest are atomic.

preempt:                The preemption counter was once a global atomic variable
                        that blocked preemption when nonzero, but was made
                        task-local. This is because a thread that has preemption
                        disabled should be able to swith to another without
                        carrying over the same preemption state.
*******************************************************************************/

static inline PTASK GET_CURRENT_TASK(VOID)
{
        register LONG e asm("esp");
        return (PTASK)(e & (~4095));
}

VOID S_Terminate(PTASK pt)
VOID S_ExecKernelThread(KTHREAD_PROC kp, PVOID pass_args)
PTASK S_NewTask(VOID);
VOID S_Yield(VOID)
VOID S_Sched(PTASK pt)
VOID S_Deactivate(PTASK pt)
VOID S_SelfTerminate(VOID);

#endif /* TASK_H */
