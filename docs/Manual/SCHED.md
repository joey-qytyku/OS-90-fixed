# Scheduler

OS/90 has a fully preemptive reentrant kernel and time slicing scheduler. The kernel can schedule tasks while it is running without the need for event scheduling and is able to handle certain exceptions concurrently.

The scheduler is percentage based. Time slices are 1 milisecond long. Time slices are taken from a pool of 1000 miliseconds and each task gets at least 1 MS. This means that OS/90 has a true concept of "CPU usage" as a fraction. If there is only one task with a time slice of 10 MS, then 1% of the CPU is being used. The remaining 99% are given to the idle thread.

The reason why this is done is because the number of time slices given to a task in an isolated context is not an accurate way to ensure accurate scheduling. Percentage-based scheduling should not be construed as permitting real-time programming.

## Context Types

Understanding the OS/90 scheduler requires knowing the meaning of a context. In OS/90, a context is the machine state of a thread of execution which may be currently in execution, entered, exited, or switched to another. Contexts are described by types in OS/90 based on how the scheduler interacts with them.

Functions provided by the kernel are specified to work in certain contexts.

Context types can be considered similar to Windows NT IRQ Levels (obviously much simpler) because it defines situations in which code in one context cannot be preempted by code in another, and determines the correct transfer of control flow.

### TI
- Usually an external interrupt handler.
- Interrupts are OFF and only approved functions may be used
- Any function called must be fully reentrant or internally CLI/STI guarded to prevent reentrancy problems.
- Preemption off.
- May never give control to any code made for other contexts.
- In general, reentrant code is IRQ-safe but that may not always be the case depending on what the code does.
- Not schedulable or restartable like the others.
- MUST EXECUTE AND ACCESS LOCKED MEMORY ONLY. NO FAULTS EVER.
- No faults should be caused while trying to enter TI (e.g. unlocked memory)

The other contexts are controlled by the transfer of state mechanism of the scheduler, which is implemented in TI itself, thereby making TI unschedulable.

### T0:
- Voluntary interrupts disabled section
- Entered from T1 or T2 by disabling interrupts
- Preemption off
- Exceptions are acceptable, including page faults.
- It is safe to yield to another task while in T0
- Iff no yielding takes place within the handler, T0 is an __effective TI__.

Be careful with this.

### T1: Preemption is disabled but interrupts remain the same. Entered by incrementing the local preemption counter.

### T2:
- Preemption and interrupts are on
- Exceptions are safe to generate
- Mutexes are safe

TI, T0 and T1 are preemption-off contexts.

T0, T1, and T2 are thread local contexts. This means that yielding to another task from an interrupts off section is perfectly possible. The preemption counter is local to each process, so yielding from a preemption off thread will continue running other tasks normally.

TI is not a schedulable and restartable context.

There is ONE exception to this system. The FPU IRQ#13 handler represents more of a trap or system entry than an actual IRQ since it is a synchronous event caused only by user mode.

### How Do I Get the Current Context Type?

The answer: you don't. It is already known, and code should be specified for specific contexts.

### Yield Semantics In Detail

Mutex locks and other synchronization primitives all have implicit yield characteristics to avoid wasting CPU time. This means that no-preemption and no-interrupts sections have weak guarantees that depend on what function calls are used

Each API call has this specified somewhere.

> Any changes to the API will not break compatibility with a previous standard version. If the concurrency safety changes to be more relaxed, old code will not be effected.

## Concurrency Guidelines

OS/90 has a fully preemptible kernel and almost everything is subject to the rules of concurrency.

All memory pages are locked in OS/90 by default. Ensure that locked memory is used for all kernel mode stacks and and code that needs to be called by a TI context.

API descriptions should describe the virtual memory safety. If locked memory must be used, such a fact will be specified. Changing page properties of any sort on pointers returned is usually illegal, especially if a constant pointer is returned.

- Locked pages must be used for all kernel stacks and interrupt service routines.
- Atomic data structures such as locks must always exist on locked pages. (RATIONALE NEEDED)
- Only functions capable of running in a certain Tx should be used in a given context.
- Never hold a lock within an ISR ever, or use any other primitive.
- T1 and T2 are quite interchangable due to yield semantics, but not always. Check the CT of every callback.
- Any user-provided atomics must call S_Yield somewhere. They must work in T0, T1, and T2.


## Task Control

- VOID S_Terminate(PTASK pt)
- VOID S_ExecKernelThread(KTHREAD_PROC kp, PVOID pass_args)
- PTASK S_NewTask(VOID);
- VOID S_Yield(VOID)
- VOID S_Sched(PTASK pt)
- VOID S_Deactivate(PTASK pt)

Advanced Features:
- BOOL S_TaskInKernel(PTASK pt)
- VOID S_WaitForExitKernel(PTASK task)

### Task Block Structure

Every task has a semi-opaque 4096-byte structure called a task block (TASK in C code). Because it contains a kernel mode stack, it is possible to find using a simple stack calulation.

Task blocks never move upon allocation because they are 4K (smallest memory unit). Upon creation, task blocks are permanently stuck to the virtual address where they are mapped and are in locked memory.

__Concurrency Protocol__

Interrupts must be off while accessing the task block. Interrupt redirection depends on this to reduce redirection latency once IRQ#0 schedules the simulated IRQ.

A fake IRQ can only go to a task that is NOT in the kernel. More on this in the interrupts section.

### Task Registers

There are a few scenarios:
- Kernel mode accesses the user context of the current task
- Kernel mode accesses the task context of a non-current task in user
- Kernel mode accesses the context of another kernel task

In the first scenario, the correct thing to do is to get the address of the entry stack frame. This is an STDREGS structure that is popped off the stack to go back to user.

In the second and third scenario, the save buffer in the start of the task block is used.

This is quite complicated so get ready. The scheduler uses bimodal threads just like most other multitasking operating systems. This means that one schedulable entity can represent a context in ring-0 or ring-3. A task is both ring-0 and ring-3, but not at the same time.

The task block, which includes the stack, has a register save buffer for when the task is switched out by the timer interrupt. It also has a return to user stack frame which serves no purpose other than switching back to user mode within a safe E-TI context in the system exit.

The register save buffer in the task block is one of these:
- The context of a kernel mode thread that IS NOT CURRENTLY RUNNING (of course, only one is running at once and there is no reason to access your own buffer since it will do nothing.)
- The context of a user mode thread that IS NOT CURRENTLY RUNNING

The RSB is ONLY used when switching tasks and is the image of the registers while the task was running before the IRQ#0 interrupt.

For SV86 hooks, the STDREGS pointer is provided automatically. When accessing the registers of the current task for other situations, it is correct to access the register image on the stack frame.

The register save buffer very rarely has to be used. One situation would be providing some kind of service using one thread but for multiple tasks, or using a debugger.

> Accessing the saved stack registers of a thread that is in kernel should 100% be avoided. It could of course work, but there is essentially no need. If a worker thread is needed to perform a job, it should be passed the PSTDREGS only.

An example of accessing user registers would be the DPMI implementation. It has to do this a lot. It uses the saved stack variables to return outputs of function calls.

Anyway, here are the calculations used to get both:
```
GET_OTHERS_RSAVE /* Nothing, it already points to it */
GET_MY_UREGS     ESP & (~4095) + (4096 - 80)
```

### S_SelfTerminate

Recommended function for terminating the active thread. This is useful for managing KTHREADS as it does not need a task handle and is reentrant.

About:
- T0, T1, T2
- Effectively reentrant
- Disables interrupts

### S_ExecKernelThread

Execute a thread of type KTHREAD_PROC. The scheduler will switch to it.

### S_Yield

Use up all time slices and switch to next available task. This works with preemption off.

About:

### S_Sched

Place task in the scheduled task chain and switch to a specific task immediately. This works for tasks that are already scheduled.
Internally works by removing that task from the list and re-inserting it directly after the current task, and follows this with a yield to switch to the task.

About:
- T1, T2
- Turns off interrupts

### S_Deactivate

Prevents the task from running. Internally it chops out the task block from the linked list. __NEVER CALL ON THE CURRENT TASK__. Self-deactivation makes no sense because another task would have to reschedue it, which is pointless when that other task could also deal with deactivating it. Doing so would also make it impossible for the scheduler to know which task is next.

About:

### S_TaskInKernel

Returns 1 if the task is in kernel mode. Redundant for the current task because it obviously is in the kernel if it is calling this.

### S_Terminate

Terminates any task and deallocates the task block. Do not use on current kernel thread.

## Basic Concurrency Control

- VOID S_PreemptOff(VOID)
- VOID S_PreemptOn(VOID)
- VOID S_IntsOn(VOID)
- VOID S_IntsOff(VOID)

### S_PreemptOn, S_PreemptOff

Enable or disable preemption while the current task remains current. Count is maintained. This can be used to enter critical sections with only voluntary preemption.
This was added in order to make contexts truly self-containing without limiting the capabilities of the scheduler.

### S_IntsOn, S_IntsOff

Tasks maintain an interrupt counter as with preemption. This allows a task to run with interrupts off. The yield operation is fully working, which makes it possible to hold locks within T0 so long as the standard ones are used.

> NEVER use CLI or STI unless you are ready to run code as if it were TI, since anything that acquires locks or is intended to be multithreaded will CRASH THE WHOLE SYSTEM.

### VOID S_WaitForExitKernel(PTASK task, HEXIT_KERNEL h)

This function will wait for another thread to not be in the kernel and the function pointer is called when true. The handler is not an instant event and has significant latency.

Internally, this waits for the thread to be out of the kernel from within a non-preemptible context, and yields periodically to the specified process until it does. Expect significant delays for other processes if the operation the initial VM is busy with takes too long.

If making DOS calls, this is useful when performing the call on the behalf of a task and its process instance. Using NULL as the task will use the initial task.

## Time distribution

- STAT S_IssueTimeSlices(PTASK to, PTASK time_provider, SHORT slices)
- STAT S_RemoveTimeSlices(PTASK from, SHORT count)
- LONG S_GetLoadAverage(VOID);
- VOID S_HookIdle(IDLE newidle, PIDLE ptr_oldidle);


### SHORT S_IssueTimeSlices(PTASK to, PTASK time_provider, SHORT slices)

Issues a time slice to a task by taking from another. Fails if there are none available.

Return:
- Actual issued slices
- Zero if none left

> If high precision is needed, disable preemption before checking load averages or the slices left in the idle thread.

### STAT S_RemoveTimeSlices(PTASK from, SHORT count)

Revokes time slices and gives them to the idle task. Essentially the inverse of S_IssueTimeSlices.

### LONG S_GetLoadAverage(VOID);

Returns the total number of time slices granted divided by the total number of running tasks. Result should be considered approximate. This function does not iterate through tasks and is not slow.

The load average is a value that represents how much CPU the average task is using.

### VOID S_HookIdle(IDLE newidle, PIDLE ptr_oldidle);

This allows for adding extra things to the idle task. Idle handler takes no arguments and returns nothing.
