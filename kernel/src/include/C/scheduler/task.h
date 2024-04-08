/*******************************************************************************
                        Copyright (C) 2023, Joey Qytyku

This file is part of OS/90 and is published under the GNU General Public
License version 2. A copy of this license should be included with the
source code and can be found at <https://www.gnu.org/licenses/>.
*******************************************************************************/

#pragma once

#include "stdregs.h"

typedef LONG HTASK;

typedef bool (*TASK_PREHOOK)(void*);
typedef void (*TASK_POSTHOOK)(void*);
typedef void (*TASK_EXITHOOK)(void*);

// Anything without an underscore is fair game.
typedef struct Task_ {
    STDREGS regs;
    void *next;
    void *prev; // May remove

    SHORT subsystem;
    SHORT arch;

    TASK_PREHOOK pre;
    TASK_POSTHOOK post;
    TASK_EXITHOOK onexit;

    SHORT _time_slices;
    SHORT _counter;
    LONG  _flags;

    char name[8]; // Need this? I guess for debugging.
}TASK, *PTASK;

#define TFLAG_MASK 0b111

/*******************************************************************************
About the task structure

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

pre,post,onexit:  These are scheduler hooks. Scheduler hooks can be
                   used to simulate the existence of isolated address spaces
                   by quickly changing page mappings, or to set the contents
                   of the TSS IOPB. Set to NULL and they will not run.

                   Note that `onexit` is called within a preemptible context.
                   The rest are atomic.

_flags: Provides some basic information about the task:
         First three bits are reserved as task states and are used by
         the scheduler only.

         Do not access flags directly.
*******************************************************************************/

enum {
  TASK_KRNL_DYING = 0,
  TASK_ASLEEP,
  TASK_ON
}TASK_STATE_E;

/*******************************************************************************
TASK_DYING:   This task should not be scheduled or awakened. The
              termination chain is currently running.

TASK_ASLEEP: Task that is not to be scheduled but may be awakened.

TASK_ON:     Currently executing.
*******************************************************************************/


