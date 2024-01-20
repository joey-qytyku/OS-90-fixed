# TODO

Note: I need to have the ability to delete unused PCBs from memory. I suppose if there are none inside a block, the block can be deallocated.

- Use IRET frame instead of trap frame. Sounds way cooler and DPMI uses it too.
- Because of your direct IDT decision, do you need to reintroduce the IOPB?
    - IOPB will be all zero to deny all. This is by placing the IOPB pointer outside of the TSS bounds.
- Consider soft IRQ
- Maybe do system entry in assembly! It would be so cool!

# Preface

OS/90 features a fully-preemptible and reentrant kernel. The scheduler is the most complex part of OS/90 because of the high degree of DOS compatibility and modern features built on top of it.

The scheduler subsystem does not involve tasks/processes in particular. Instead, it involves everything relating to:
* How processes enter and exit the kernel
* How tasks are switched
* Synchronization
* Sending user-trapped events like IRQs and exceptions
* Virtual 8086 mode

# Interrupts and Exceptions

## Exceptions

Exceptions are interruptible and non-preemptible. They use a upper-half handler that can decide to terminate or block the task by its return value.

Exceptions cannot nest and should not cause another exception unless it is a double fault, which is an unrecoverable error.

## Exceptions in Real Mode

Some programs trap exceptions such as divide by zero or FPU error. The local vectors are used for this if the entry has been set up properly.

IRQ#13 may be used to recieve FPU errors.

# Processes

## The Process Control Block

The PCB is 8192 bytes large and naturally aligned. It includes the kernel stack and information about the process, both being 4096 bytes. This allows two PCB's to exist inside a memory block on the default configuration. This is handled by using the flags related to whether or not the PCB is valid.

The PCB is opaque to all drivers and should not be considered at all. It may change at any time and likely will.

> The begining of the stack area plus 4 is the IRET/trap frame.

## Process Thread States

A process can be in the following states

- TH_KERNEL: The thread is active in ring-0. The scheduler knows to enter its ring-0 context. When the kernel is attempting to service a thread, it will be in this state.

- TH_BLOCKED: Thread is blocked for any reason. This is useful for stopping a particular process from running. It will not be scheduled for execution in kernel or user mode. It simply wont run. The process should not be blocked while in service, because the kernel thread will not be able to reschedule after the next tick.

- TH_ACTIVE: The thread is running in ring-3. The scheduler knows to load the ring-3 context.

- TH_DEAD: The process is not running and can be discarded.

Thread states have nothing to do with protected mode or V86.

# The IRET Frame

When a switch from ring-3 to ring-0 takes place, the kernel will have an system entry procedure that will save registers onto the stack and call a higher level procedure.

This is a complicated matter. If a switch happens from V86, the segment registers will automatically be saved onto the stack before the rest can be. This means that the IRET frame structure will have two sets of segment registers.

This is not a problem. Regardless of the previous mode, segment registers are always pushed and popped after the system entry is finished. This can have the effect of restoring the ring-3 protected mode state's segment registers or zeroing them out as specified by V86. IRET will check EFLAGS and decide based on that.

What this all means is that code that modifies the trap frame directly--particularly the segment registers--must know what mode the context was in. The VM flag is all this is needed, though checking the process type ID works too.

> This avoids a branch penalty.

## For IRQs and Exceptions

Exceptions use system entry and are subject to the same rules as software interrupts. The only difference is that exceptions are non-preemptible.

- An exception does not need to block the process that caused it because it is a T1.
- It may not modify the IRET frame, but it may read it.
- It will return to the process, terminate the process, or raise a critical error.

IRQs use the same entry/return method but do not go through system entry.

# Elevated Virtual 8086 Mode (SV86)

SV86 is a non-preemptible context that is entered by the kernel to perform a task in real mode with direct hardware access. INTs can be captured, but IO is always direct.

Interrupt captures must support both SV86 calls and UV86 (user) calls.

# Bimodal Threads

A single process has one register dump for both kernel and user. It can be in kernel or in userspace (V86 or PM).

When the kernel thread is done its work, it can go back to the original process. It does not need to unblock it, switch to its previous execution state and wait for reschedule. just as the user thread can enter the kernel thread with only one single non-preemptible context between them, the same can happen in reverse. This is done by entering a NPC, setting up the PCB as necessary, and copying the ring-3 regdump to the IRET frame before returning (which will reset IF).

This avoids scheduling latency for relatively basic system entries.

> Wait, really? In our current design, the kernel thread simply stalls until it is terminated and the userspace process runs. I could find a way to avoid this. I am not sure if I can just enter the userspace process. I guess I could disable interrupts, set up the PCB, and enter. Only one process can do this at once, but it should not be a problem.

# Scheduler Tick Interrupt

The scheduler tick interrupt is the core of the scheduler that makes decisions about which processes will run. It concurrently switches between the active threads of each process in the list.

Interrupts are completely turned off within this ISR as with all other IRQs. The low half saves all registers to a trap frame and passes a pointer to it. When the ISR is done, it pushes all the reigsters off of the stack. These registers could have been modified to run a different process.

> Remember that a T0 is allowed to change the IRET frame and return.

## Process Hook Procedures

A process hook is a procedure called in an atomic context with the folowing signature:
```
PH_RET ProcessHook(PID pid)
```

PH_RET is a type which can be `PH_SKIP` or `PH_CONT`.

A function called InsertProcessHook(PID pid) inserts the hook procedure into the specified process. The reason the procedure takes the PID even though it is known when assigning the proc hook is because we may want to reuse the same code for several processes requesting the same service.

Process hooks should never block the process or change anything relating to its execution state. If it is blocked, how can you expect to unblock it? Instead, return PH_SKIP so that the scheduler does not run the process.

Process hooks are a hack meant to allow for simulating multiple addressing spaces in an OS that does not allow for them. It is primarily designed for emulating memory mapped IO for individual processes.

# Controlling Interrupts, Syncrhonization, and Preemption

OS/90 supports the ATOMIC type which behaves outwardly as a counter or boolean lock, depending on how it is used.

The following operations are supported and are done in a single instruction for atomicity:
* Increment: AtomicFencedInc
* Decrement: AtomicFencedDec
* Load:      AtomicFencedLoad
* Store:     AtomicFencedStore
* Compare:   AtomicFencedCompare
* Acquire:   AcquireMutex
* Release:   ReleaseMutex

These are implemented as macros/inline.

# Preemption Counter in Detail and Uses in Kernel

The preeption counter is not really a recursive mutex. It is a single atomic variable that has the special meaning of blocking all threads except the currently running one (kernel).

Decrementing the preemption counter is a viable synchronization method if the critical section is relatively short.

If the critical section is rather complex, a lock is better since it allows other tasks to run until they access the same resource.

## SV86 Blocks Preemption (TODO)

SV86 cannot be accessed by multiple tasks, so they will race to increment the preempt counter atomically.

A global variable called `g_sv86`  determines if the kernel should handle INT/IRET and other opcodes for SV86 or for V86 processes.

The reason why a lock is not used is because there is no need to allow other tasks the opportunity to run at all since they will be blocked later on anyway.

SV86 cannot be entered unless IOPL=0. This is different than the usual IOPL=3 for regular processes and garauntees that only the #GP handler will deal with INT opcodes within its non-preempible context. This separates SV86 from the more complicated handling of INT for protected mode.

### SV86 IRQ (TODO HERE)

SV86 can be interrupted by SV86. The interrupt handler will call NOT call `EnterV86` since it cannot nest, but instead will change the control flow of the SV86 context with stack emulation.

The process is as follows:
* Kernel enters SV86
* Interrupt is recieved and kernel needs to reflect
* ISR handler realizes saved context is SV86
* ISR handler calls a function that emulates the stack behavior of an IRQ.
* ISR handler returns and executes the real mode ISR. It has no control after this point.
* Real mode ISR executes `INT VEC_IRET_SV86`
* IRET stack behavior
* Computer continues executing in SV86

Stack emulation for a RECL_16 IRQ while inside SV86 is different because it requires IF=0 on entry, but is identical otherwise.

## Process List

Disabling preemption will acquire a critical section for the thread that increments the counter first. It will ensure that any non-ISR code will not access the data.

When a process enters the system, it will be blocked with interrupts disabled. It will be automatically be unblocked by the system entry procedure.

# The Rules for Contexts and Thread Safety

> This section is very important.

There are types of context preemption:
- T0: Interrupts are disabled and preemption is by proxy or by increasing the preempt counter.
    - Nothing can preempt the current section.
- T1: Interrupts are enabled but the preemption counter is non-zero, blocking preemption.
    - Interrupts may preempt but a process will not.
- T2: Interrupts are enabled and counter is zero. Fully preemptible context.
    - An IRQ or a process can preempt the active thread.

__The following are the general rules regarding contexts:__

- A T0 may read or alter the trap frame and enter that context directly by returning from an interrupt.

- A T0 context may alter or read a process control block under specific rules. Such operations must be restricted to copying between register dumps and the trap frame. The IRQ#0 handler is restricted to such instances.
- A T0 may never call a function that is not explicity stated to be `kernel_async`.

- A T1 (such as an exception handler) may read or alter a process control block so long as it is blocked or in-kernel.

- A T1 context may alter the trap frame and return to the context theirin and does not need to change the process state whatsoever.

- T2 may not under an circumstances access or alter a process control block or the trap frame. What if blocked?

## Debugging Contexts

`SchedulerDBG.h` defines macros for asserting context types.

```c
assert_T0();
assert_T1();
assert_T2();
```

# INT Capture

INT capturing must work with SV86 and regular processes. In fact, the INT capture structure contains a handler for SV86 and user V86. Both __must__ be provided. It cannot be the same procedure.

The register parameter structures are completely different. One is a SV86 parameter block and the other is a process register dump.

# Far Call Capture

...

# System Entry Point

The system entry point is where exceptions and INT calls go. It starts in T0 and enters T1 for exceptions or T2 for INT calls.

The SEP is invoked by the following:
- Userspace V86
- Userspace 32-bit or 16-bit PM
- Exceptions in kernel or user

The reason exceptions are included is because they may be redirected to process-local handlers.

## The Problem

UV86 may call the INT instruction. It is not acceptable to cause an exception because exception handlers are not meant to be reentrant. To do this, OS/90 uses IOPL=3 for all V86 processes and IOPL=0 for everything else, including SV86. The affect of this is that execution of an INT instruction will cause a system entry. The only issue now is what to do after, because a system entry has different sources.

The IDT, with the exception of the IRQ handlers, will be filled with INT lower half handlers or exception lower half handlers that go to the system entry.

The solution could involve a bunch of nested conditionals. Lets start with the if-statement approach.

```c
if SEP event was INT
    // SEP INT cannot be caused by the kernel. We know it is a process and
    // that the PCB of the current process has the necessary information.
    if current process is V86:
        // This will call the UV86 handler.
        => Call a handler using the V86 handler chain but NOT Svint86.
    if current process is PM 32:
        if there is a local IDT handler assigned
            => Transfer control flow to it with 32-bit stack emulation
            => Exit back to the process

        if there is NOT a local IDT handler assigned
            // In accordance with DPMI
            => Reflect to V86 handler chain

    if current process is PM 16
        if there is a local IDT handler assigned
            => Transfer control flow to it with 16-bit stack emulation

        else if there is NOT a local IDT handler assigned
            Reflect to V86 handler chain
if SEP event was exception:
    if RPL of saved CS is 0, aka last mode was KERNEL:
        => call a kernel-mode handler right away
    else if RPL of saved CS is 3, aka last mode was USER
        if current process is UV86:
            if process modified IRQ#13 and exception is FPU error
                => change control flow to the local real mode IRQ#13 handler
            else if current process has a virtual IVT entry for exception
        if current process is protected mode
            if an exception handler (not the LIDT, separate handlers!) exists
                switch control flow to it
            else
                Terminate process
    Exit to process or kernel code
```
> Rest in peace branch target buffer 💀.

> The procedure for finding out if it was an INT or exception is not included here. More on it later.

## The Solution

This is not bad because the situation can be broken down into independent scenarios that can be implemented separately as polymorphic procedures. his is actually better for performance because branch prediction is not strained and also because switches between execution modes are not frequent enough to necessitate constant checking.

Each process will have an EXC_SEPINIT and INT_SEPINIT function pointer.
> Change INIT to DO
The INT_SEPDO signature is `U32 (P_IRET_FRAME)` There are different INT_SEPINIT procedures:
* IntSepDo_Realmode_16
* IntSepDo_ProtMode

These will return 1 if there was an error that requires the process to be terminated. Calling an IRQ ISR with INT is one such condition.

Protected mode could be in 16-bit or 32-bit execution mode. This is not the same as the DPMI startup mode, which is defined when entering the program for the first time in protected mode. This is the bitness of the currently loaded code segment selector for that process.

## Exception Handling

Exceptions will have a different method of being handled. There are three that apply to user processes and not the kernel.

* ExcSepinit_RealMode_16
* ExcSepinit_ProtMode

Can a SEP instance alter the trap frame and return? Only if it is an exception.

## Fake IRQs

Fake IRQs also use a sepinit procedure.

## Setting a Process Execution Mode

These procedures do not apply to the way DPMI sees it, but how the scheduler does.

Switching a process to V86 will set the IOPL in the PCB EFLAGS to 3 so that INT does not cause an exception and instead goes directly to the PCB. The `intsepinit` is set to `&IntSepinit_Realmode_16`.

Switching a process to protected mode execution involves setting `intsepinit`.

## Exiting the SEP

Exceptions are permitted to exit the system entry point by interrupt returning because a T1 is permitted to do that safely.

When SEP is in a T2 context for any other system call, it is more complicated. We could just mark the kernel thread for termination and reentrace to user, and wait for a reschedule. This would incur an unacceptable system call latency. We need to be able to directly leave the system call and maximize the time where the CPU can do useful work.

The process has to be unblocked, but T0 must be entered so that preemption is blocked UNTIL SEP exits through IRET, where it can automatically be enabled. A T1 is inadequate since no other code will decrement the counter.

> Think about this.
Going back to the process requires changing the process state to KERNEL.

This allows for low-latency system calls that do not stall when completed.

> Changing the links of the PCB requires interrupts to be fully disabled

## Compliance with DPMI

DPMI requires that all interrupt vectors are modifyable by the client. By default, every vector except for `INT 21h` in the special case of `AH=4Ch` will reflect to 16-bit DOS. The only exception would be the DOS error handlers, which can cause an error if not caught on most DPMI servers.

Exceptions use a separate table and function for configuring them because of the inherent collision with real mode INT calls.

This makes things difficult for the system entry point. We will end up having to check the imm8 field of the INT call because simply getting the IDT index with preincluded thunks will not be enough to tell apart an exception.

The only way to know if it was an exception is to confirm it was not an INT or any variant of it.

# Handling IRET

Handling of IRET can be done within an exception handler. It is necessary for virtual 8086 mode and SV86. Protected mode applications initiate a RETF to exit an ISR.

Because exception handlers are non-preemptible, they can perform the IRET service for the process directly with the IRET frame.

> IRET uses function pointer. IRQ uses it too?

> TODO: Add context debugging features.

# Execution and Termination

For a process to fully terminate and leave the memory permanently, several steps need to be taken that require the cooperation of several subsystems.

All memory must be deallocated. Locked pages are forcefully unlocked and removed from the swap file.
