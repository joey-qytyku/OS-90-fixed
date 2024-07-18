# Task Management

1.  GET_CURRENT_TASK
2.  VOID KTHREAD_PROC(PVOID args)
3.  VOID S_Terminate(PTASK pt)
4.  VOID S_ExecKernelThread(KTHREAD_PROC kp, PVOID pass_args)
5.  PTASK S_NewTask(VOID);
6.  VOID S_Yield(VOID)
7.  VOID S_Sched(PTASK pt)
8.  VOID S_Deactivate(PTASK pt)
9.  VOID S_PreemptOff(VOID)
10. VOID S_PreemptOn(VOID)
11. VOID S_IntsOn(VOID)
12. VOID S_IntsOff(VOID);
13. BOOL S_TaskInKernel(PTASK pt)
14. STAT S_IssueTimeSlices(PTASK to, PTASK time_provider, SHORT slices);
15. STAT S_RemoveTimeSlices(PTASK from, SHORT count)
16. LONG S_GetLoadAverage(VOID);
17. VOID HookIdle(IDLE newidle, PIDLE ptr_oldidle);
18. PTASK MapTaskBlock(PTASK task);

## Task Block Structure

Every task has a semi-opaque 4096-byte structure called a task block (TASK in C code). Because it contains a kernel mode stack, it is possible to find

Task blocks never move upon allocation because they are 4K (smallest memory unit). Upon creation, task blocks are permanently stuck to the virtual address where they are mapped and are in locked memory.

## GET_CURRENT_TASK

This is NOT a real kernel function, but a static inline. It gives the address of the task block of the currently running task.

It is paramout to not change the stack in a kernel mode task whatsoever beyond what is given by the kernel. Doing so would make it impossible to find the current task. If more stack space is needed, disable interrupts and change it only within that section.

## KTHREAD_PROC

Exiting the thread and terminating it is done by simply returning. This will self terminate. If in deeper procedure levels, use S_SelfTerminate.

## S_SelfTerminate

Recommended function for terminating the active thread. This is useful for managing KTHREADS as it does not need a task handle.

## S_ExecKernelThread

Execute a thread of type KTHREAD_PROC. The scheduler will switch to it.

## S_Yield

Use up all time slices and switch to next available task. This works with preemption off.

## S_Sched

Place task in the scheduled task chain and switch to a specific task immediately. This works for tasks that are already scheduled.
Internally works by removing that task from the list and re-inserting it directly after the current task, and follows this with a yield to switch to the task.

## S_Deactivate

Prevents the task from running. Internally it chops out the task block from the linked list. NEVER CALL ON THE CURRENT TASK. Self-deactivation makes no sense because another task would have to reschedue it, which is pointless when that other task could also deal with deactivating it. Doing so would also make it impossible for the scheduler to know which task is next.

## S_PreemptOn, S_PreemptOff

Enable or disable preemption while the current task remains current. Count is maintained. This can be used to enter critical sections with only voluntary preemption.
This was added in order to make contexts truly self-containing without limiting the capabilities of the scheduler.

## S_IntsOn, S_IntsOff

Tasks maintain an interrupt counter as with preemption. While interrupts can be enabled on disabled using the macros provided by basicatomic, this is the recommended function. It does not tell the compiler that the flags register was clobbered and does not require the use of automatic variables.

## S_TaskInKernel

Returns 1 if the task is in kernel mode. Redundant for the current task because it obviously is in the kernel if it is calling this.

## S_Terminate

Terminates any task and deallocates the task block. Do not use on current kernel thread.

## S_IssueTimeSlices

Issues a time slice to a task by taking from another. Fails if there are none available.

As described previously, this is a fraction of 1000. To give 10% of the CPU, 100 MS is the length of the slice.

Returns 0 if okay and a non-zero value if not indicating how many time slices are actually available from that task.

## S_RevokeTimeSlices

Revokes time slices and gives them to the idle task.

## S_GetLoadAverage

Returns the total number of time slices granted divided by the total number of running tasks. Result should be considered approximate. This function does not iterate through tasks.

The load average is a value that represents how much CPU the average task is using.

Subsystems can use this for advanced scheduling.

## HookIdle

This allows for adding extra things to the idle task. For example: setting some other low-power state. By default, the idle thread simply runs a set of HLT instructions (many of them are used to avoid branch predictor pollution).

> APM may have an idling feature that could be used.

## MapTaskBlock

Task blocks are allocated as single pages in physical memory. To make them actually accessible, they must be mapped. The current task block is always mapped to a reserved region in the HMA address space, which can be done directly from an interrupt service routine.

NEVER ACCESS A TASK BLOCK WITHOUT MAPPING IT FIRST. NEVER PASS A MAPPED TASK BLOCK TO A SCHEDULER TASK.

## Other TASK Topics

### Iterate Through Tasks

Iterating through tasks can be done using the current one as the base. Subsystems can add their own task links to make iteration faster for its own tasks, but this is optional and does not affect scheduling.
