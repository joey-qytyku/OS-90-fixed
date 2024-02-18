///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Type.h"

// Doubleworld, word, byte
#define _DWB_R(n)\
union {\
  uint E##n##X;\
  union {\
    unsigned char n##L;\
    struct { unsigned char :8; unsigned char n##H; };\
  };\
  ushort n##X;\
};

// double, word
#define _DW_R(n)\
union {\
  uint E##n;\
  ushort n;\
}

// Standard register dump structure with convenient fields for accessing
// high and low registers.
struct stdregs {
  _DWB_R(A);
  _DWB_R(B);
  _DWB_R(C);
  _DWB_R(D);

  _DW_R(SI);
  _DW_R(DI);
  _DW_R(BP);

  _DW_R(IP);

  uint   CS;
  _DW_R(FLAGS);
  _DW_R(SP);
  uint   SS;

  uint   pm_ES;
  uint   pm_DS;
  uint   pm_FS;
  uint   pm_GS;

  uint   v86_ES;
  uint   v86_DS;
  uint   v86_FS;
  uint   v86_GS;
};

#undef _DW_R
#undef _DWB_R

typedef void (*sched_hook_f)(void);

typedef void (*context_consumer_f)(struct stdregs);

enum context_mode { IN_KERNEL, IN_USER }; // Remove?

// Anything without an underscore is fair game.
struct task_desc_blk {
  struct stdregs regs;
  struct task_desc_blk *next;
  struct task_desc_blk *prev;

  ushort time_slices;
  ushort counter;
  ushort subsystem;
  ushort arch;

  sched_hook_f pre;
  sched_hook_f post;
  sched_hook_f onexit;

  uint _flags;
};

#define TFLAGS_MASK 0b111

//// About the task descriptor block ////
//
// The ID of a process is the address of the TDB.
//
// Fields:
//
//  regs: This is the active context of the thread. It can be kernel mode or
//        user mode. The scheduler switches to this when the task is scheduled
//        or otherwise switched to. After a system entry, the saved user
//        registers are on the stack, but this is of no interest to the
//        scheduler and only that of the task, which is now in ring-0.
//
//  next,prev: The tasks are a doubly-linked circular list. There is no definite
//             beginning or end to it. When a task is terminated, the links are
//             crossed over and the memory is deleted.
//
//  time_slices:  The number of time slices this task gets when scheduled.
//                It is a count of how many times the task is rescheduled by
//                IRQ#0 instead of switching to the next task.
//
//  counter:      A decrementing counter that represents the number of time
//                slices left to give the process.
//
//  subsystem:    OS/90 provides the field as a standard way to determine what
//                kind of operating system or environment this program belongs
//                to, so that a subsystem driver can correctly service any
//                events caused by the task.
//
//  arch:         The architecture being "emulated" by the subsystem or kernel.
//                Does not mean a whole lot.
//
//  pre,post,onexit:  These are scheduler hooks. Scheduler hooks can be
//                    used to simulate the existence of isolated address spaces
//                    by quickly changing page mappings, or to set the contents
//                    of the TSS IOPB. Set to NULL and they will not run.
//
//                    Note that `onexit` is called within a preemptible context.
//                    The rest are atomic.
//
//  _flags: Provides some basic information about the task:
//          First three bits are reserved as task states and are used by
//          the scheduler only.
//
//          Do not access flags directly.
//
enum {
  TASK_KRNL_DYING = 0,
  TASK_ASLEEP,
  TASK_ON
};
//// About task states ////
//
// TASK_DYING:   This task should not be scheduled or awakened. The
//               termination chain is currently running.
//
// TASK_ASLEEP: Task that is not to be scheduled but may be awakened.
//
// TASK_ON:     Currently executing.
//

#endif /* SCHEDULER_H */
