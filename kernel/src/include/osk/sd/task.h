/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2028, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
#pragma once

#include <osk/sd/stdregs.h>

#define SUBSYS_BLOCK_SIZE 256

//
// Internally, this is treated as a pointer to the task block.
// Task blocks never move because they are 4K in size.
//
typedef LONG HTASK;

typedef PVOID (*PAGE_GET)(VOID);
typedef VOID (*KTHREAD_PROC)(PVOID);

typedef BOOL (*T0_TASK_PREHOOK)(STDREGS*);
typedef void (*T0_TASK_POSTHOOK)(STDREGS*);

// The task exit hook runs in T2, not T0.
typedef void (*T2_TASK_EXITHOOK)(STDREGS*);

typedef struct Task_ {
        STDREGS regs;
        void *next;
        SHORT _time_slices;
        SHORT _counter;
        LONG  _flags;

        SHORT subsystem;
        SHORT arch;

        T0_TASK_PREHOOK pre;
        T0_TASK_POSTHOOK post;
        T2_TASK_EXITHOOK onexit;

        char name[8]; // Need this? I guess for debugging.

        BYTE    subsystem_block[SUBSYS_BLOCK_SIZE];
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

next,prev: The tasks are a doubly-linked circular list. There is no definite
            beginning or end to it. When a task is terminated, the links are
            crossed over and the memory is deleted.

time_slices:  The number of time slices this task gets when scheduled.
              It is a count of how many times the task is rescheduled by
              IRQ#0 instead of switching to the next task.

counter:      A decrementing counter that represents the number of time
               slices left to give the process.

subsystem:  OS/90 provides the field as a standard way to determine what
            kind of operating system or environment this program belongs
            to, so that a subsystem driver can correctly service any
            events caused by the task.

arch:       The architecture being "emulated" by the subsystem or kernel.
            Does not mean a whole lot.

pre,post,onexit:        These are scheduler hooks. Scheduler hooks can be
                        used to simulate the existence of isolated address
                        spaces by quickly changing page mappings,
                        or to set the contents of the TSS IOPB.
                        Set to NULL and they will not run.

                        These run regardless of the process mode.

                        Note that `onexit` is called within a preemptible context.
                        The rest are atomic.

_flags: Provides some basic information about the task:
        First three bits are reserved as task states and are used by
        the scheduler only.

        Do not access flags directly.
*******************************************************************************/

/*******************************************************************************
TASK_DYING:     This task should not be scheduled or awakened. The
                termination chain is currently running.

TASK_ASLEEP:    Task that is not to be scheduled but may be awakened.

TASK_x_ON:      Currently executing.

Tasks do not get marked as busy because they switch modes and are serviced
by the kernel using a dedicated kernel thread.

It is not necessary to distinguish between kernel and user tasks because
the CS RPL gives this information.

*******************************************************************************/
enum {
        TASK_DYING,
        TASK_ON,
        TASK_ASLEEP
};

static inline PTASK GET_THREAD_TASK(VOID)
{
        register LONG e asm("esp");
        return (PTASK)(e & (~4095));
}
