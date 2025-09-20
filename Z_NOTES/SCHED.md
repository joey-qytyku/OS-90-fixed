# Scheduler

Information in sched.txt is valid for this document. This is focused on implementation details.

## Task Block

Threads are called TASK in OS/90. They represent a bimodal restartable and schedulable entity. Task structures are 4K aligned and non-moving structures with no less than 3K of stack space and the rest is used for the kernel mode context.

It looks something like this, although different versions can change some things:
```
After exit registers | Available kernel stack space | Userspace trap frame | V86 call registers
```

The first region is filled with the context after a switch to the next is happening. This happens regardless of what mode the thread is in, kernel or user.

The last region is the trap frame generated when a user program causes a trap, usually from a fault.

Tasks have more information which is not directly accessible. A PDE for the first 4M is included (INVLPG is used where available).

## Switch Actions

OS/90 uses a very high timer interval of 1MS for scheduling. This is suitable for modern computers, but is a challenge on the 80386.

For that reason, a specialized method of context switching exists based on a Switch Action. This is a specific scenario that maps to one single specialized procedure that switches tasks in the most optimized way.

Switch actions are dependent on the next task and are updated automatically for each task insertion or removal. This is efficient because changes to the task list are much less frequent than accesses to the task list as part of switching.

The idea is the switch action specifies:
- r0-r0
- r0-r3pm
- r0-r3vm

- r3pm-r0
- r3pm-r3pm
- r3pm-r3vm

- r3vm-r0
- r3vm-r3pm
- r3vm-r3vm

There are 9 procedures in the kernel that do the switch.

Direct register access with minimal pushing and no copying is used to avoid the usual overhead.

## Time slices

1MS is the fixed quantum of the scheduler.

Time slices are granted to a task from a provider. This is usually the idle thread, which normally runs HLT instructions in a loop until all time slices expire.

It is possible to give tasks time slices from any other task that has them, which maintains a balance of 1000 time slices granted to any program, which is one second. This limits the number of threads, but allows for accurate timing relative to a second.

There is also the idea of percentage of utilization. All tasks have at least 1MS to run, or 0.1% of the CPU.

## Preemption enable and disable.

Programs are allowed to use the critical section interface provided by Windows to disable preemption. This gives the program 100% control of the CPU and 100% utilization.

This works by changing the IRQ handler to a simpler one that does not clobber/save/restore all the registers.

Screen drawing in a UI is a useful application of this. It is more commonly done in the kernel as a form of synchronization without using a lock.

Preemption can be disabled but it is imperative that a program does not yield while doing it.

OS/90 does NOT support exclusive tasking like Win386 does. Instead, program information should request higher time slices and put other threads in the background. It is mainly the responsibility of the userspace interface.
