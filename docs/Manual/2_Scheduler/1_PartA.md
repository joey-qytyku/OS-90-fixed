# Scheduler

OS/90 has a fully preemptive reentrant kernel and time slicing scheduler. The kernel can schedule tasks while it is running without the need for event scheduling and is able to handle certain exceptions concurrently.

The scheduler is made up of the following components:
- Task scheduling
- Interrupt requests
- Exception dispatching
- Supervisor virtual 8086 mode (SV86)

## Context Types
Understanding the OS/90 scheduler requires knowing the meaning of a context. In OS/90, a context is the machine state of a thread of execution which may be currently in execution, entered, exited, or switched to another. Contexts are described by types in OS/90 based on how the scheduler interacts with them.

Functions provided by the kernel are specified to work in certain contexts.

TI or simply IRQ:
- Interrupt handler.
- Entered by external IRQ
- Interrupts are OFF and only approved functions may be used
- Any function called must be fully reentrant or internally CLI/STI guarded to prevent reentrancy problems.
- Preemption off.
- May never give control to any code made for other contexts. - In general, reentrant code is IRQ-safe but that may not always be the case depending on what the code does.
- Not schedulable or restartable like the others.
- MUST EXECUTE AND ACCESS LOCKED MEMORY ONLY. NO FAULTS EVER.

The other contexts are controlled by the transfer of state mechanism of the scheduler.

T0:
- Voluntary interrupts disabled section
- Entered from T1 or T2 by disabling interrupts
- Preemption off
- Exceptions are acceptable, including page faults.

T1: Preemption is disabled but interrupts remain the same. Entered by incrementing the local preemption counter.

T2:
- Preemption and interrupts are on
- Exceptions are safe to generate
- Mutexes are safe

TI, T0 and T1 are preemption-off contexts.

T0, T1, and T2 are thread local contexts. This means that yielding to another task from an interrupts off section is perfectly possible. The preemption counter is local to each process, so yielding from a preemption off thread will continue running other tasks normally.

TI is not a schedulable and restartable context.

There is ONE exception to this system. The FPU IRQ#13 handler represents more of a trap or system entry than an actual IRQ since it is a synchronous event caused only by user mode.

## Concurrency Safety Guidelines (IMPORTANT)

OS/90 has a fully preemptible kernel and almost everything is subject to the rules of concurrency.

All memory pages are locked in OS/90 by default. Ensure that locked memory is used for all kernel mode stacks and and code that needs to be called by a TI context.

API descriptions should describe the virtual memory safety. If locked memory must be used, such a fact will be specified. Changing page properties of any sort on pointers returned is usually illegal, especially if a constant pointer is returned.

Interrupts can cause page faults BEFORE they actually take place, but obviously not after.

## TASK Structure

In complete contradiction to common practice, OS/90 has a public task block structure with plenty of garauntees of proper behavior when doing so, but with some requirements.

The TASK structure uses some fields internally and such fields may change in meaning. The only ones that C code should ever access are the ones without underscores in the names. Furthermore, a task cannot be executed unless it has been properly prepared by the scheduler.

TASK structure fields that are defined never move, so all code that accesses the task structure is binary compatible.

Task blocks never move and are implicitly 4KB in size, with 2K being reserved to the scheduler and the subsystem block. 2K is for the defined stack, but an overflow may not always be fatal.

Task IDs exist as mere pointers, and functions that operate on tasks usually take a PTASK. The pointer is valid so long as the task is not terminated explicitly.

Casting a PTASK to PSTDREGS is sufficient for accessing the registers in all circumstances.

## Hooking

## Task Management

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

### GET_CURRENT_TASK

This is NOT a real kernel function, but a static inline. It gives the address of the task block of the currently running task.

It is paramout to not change the stack in a kernel mode task whatsoever beyond what is given by the kernel. Doing so would make it impossible to find the current task. If more stack space is needed, disable preemption and change it only within that section.

### KTHREAD_PROC

Exiting the thread and terminating it is done by simply returning. This will self terminate. If in deeper procedure levels, use

### S_SelfTerminate

Recommended function for terminating the active thread.

### S_ExecKernelThread

Execute a thread of type KTHREAD_PROC. The scheduler will switch to it.

### S_Yield

Use up all time slices and switch to next available task. This works with preemption off.

### S_Sched

Place task in the scheduled task chain and switch to a specific task immediately. This works for tasks that are already scheduled.
Internally works by removing that task from the list and re-inserting it directly after the current task, and follows this with a yield to switch to the task.
S_Deactivate
Prevents the task from running. Internally it chops out the task block from the linked list. NEVER CALL ON THE CURRENT TASK. Self-deactivation makes no sense because another task would have to reschedue it, which is pointless when that other task could also deal with deactivating it. Doing so would also make it impossible for the scheduler to know which task is next.

### S_PreemptOn,  S_PreemptOff

Enable or disable preemption while the current task remains current. Count is maintained. This can be used to enter critical sections with only voluntary preemption.
This was added in order to make contexts truly self-containing without limiting the capabilities of the scheduler.

### S_IntsOn,  S_IntsOff

Tasks maintain an interrupt counter as with preemption. While interrupts can be enabled on disabled using the macros provided by basicatomic, this is the recommended function. It does not tell the compiler that the flags register was clobbered and does not require the use of automatic variables.

### S_TaskInKernel

Returns 1 if the task is in kernel mode. Redundant for the current task because it obviously is in the kernel if it is calling this.
S_Terminate
Terminates any task and deallocates the task block.

## Virtual 8086 Mode

OS/90 can make calls to DOS, BIOS, and 16-bit drivers using the INT interface, and permits drivers to capture requests to 16-bit software and implement feature in a concurrent 32-bit environment.
SV86 is a non-preemptible context in which virtual 8086 mode is given special privileges to run as if it were true real mode. IO instructions execute directly and INT/IRET are emulated differently. It can recursively execute a virtual INT as with INTxH, and hooks apply each time.

This has NOTHING to do with multitasking DOS programs and how they handle INT calls. The DOS subsystem, and any subsystem for that matter, has the full authority to decide if it will handle a ring-3 system entry by calling V86. This is a driver-level interface that allows for the reimplementation of real mode software in protected mode.

> Anything done in 32-bit mode will automatically perform better than 16-bit mode. SV86 has to switch to ring-0, to ring-3 V86, back to ring-0, and finally, back to the ring-3 caller. It also has to copy register parameters one more time for the V86 entry.

Listing:
- VOID V_INTxH(BYTE vector, PSTDREGS regs)
- VOID V_HookINTxH(BYTE vector, V86HND hnew, V86HND *out_prev)

### V86HND

This function pointer type returns itself, or at least a type compatible with itself. It takes a PSTDREGS.
CONTEXT: May be T012 upon invokation.

### V_HookINTxH

Changes the current SV86 handler for that vector and outputs it to [out_prev]. To chain properly, the new handler must call the one it replaced.
Hooks should be applied when the driver is starting up to whatever vector it needs to control. It is not recommended to do this during runtime. V86 handlers also cannot be modified within a V86HND procedure.

The general procedure of a hook procedure is to check the appropriate registers to figure out if the driver has jurisdiction over this INT call; return the next handler address to pass to previous hook, return `(PVOID)(1)` to automatically reflect to real mode (not recommended) or return NULL to consume the INT and finish.

The correct segment register set to use is the one with v86 prefixes, though it would not have mattered. Simply using the name without anything else will work too.
V_INTxH

This is the general purpose V86 call interface. It calls a SV86 handler with hooks and returns nothing. The `PSTDREGS` argument is used as input and output, so save the input if it is still needed after execution.

If the stack pointer provided is ZERO (which is certainly invalid on 80x86), a stack will be provided automatically and is garaunteed to be 1024 bytes long. It is VERY important to set ESP to zero if this is desired. Using whatever garbage is in the stack from previous calls with the structure or anything else is likely to cause a fatal error. To not think about this, simply use INIT_V86R to initialize the structure.
	The DOS subsystem has its own function for hooking user-level INT calls which should be used for implementing anything that is actually DOS-level
FYI: floating point operations can at no point be done in a V86 handler. Period.

### V_SV86Call

Sometimes it is necessary to make direct calls to real mode without capturing. Subsequent calls to the INT instruction are not captured either.

This has limited applications. One example is a driver that uses an EMS card as a ramdisk that needs to call the DOS EMS driver.
The implementation of this will lead to execution in either in SV86 real mode or a preemption-off #GP exception handler. T2 is never entered, and therefore, the system can safely detach the V86 hook procedure for the requested INT in a T1 and call V_INTxH.
