# Preface

KERNL386 has the following properties:
* Preemptive multitasking
* Non-preemtible kernel code
* Non-reentrant

# Stacks

# Process Control Block

The PCB is a structure called THREAD which contains the register dump and various information about the process. It is garaunteed to be 4096 bytes long and no larger. The PID is the program segment prefix of the program.

# Standard IO Handles

The standard handles can be redirected by the program. The force duplicate function call allows this to happen. Calls to dup2 are trapped so that the target alias descriptor are caught and saved to the PCB, and the interrupt is passed down the chain.

A special system call can be used to force duplicate on the behalf of another process, allowing standard IO to be redirected to a handle of the program's chosing.

# Interrupt Service Routines

ISRs are restricted contexts and cannot do any of the following:
* Cause a fault
* Access virtual memory
* Allocate memory
* Call any function that is not marked ASYNC

# Idea

What if we allow an interrupt to be delayed? If the kernel cannot be reentered, the ISR can simply save its context and continue once the function is available. When the ISR is in a paused state, it must be masked. Implementing this would be tricky. A single stub function would not be enough. Every function would need to have a reentrancy lock of some sort and signal once it completes. This might be a performance issue, but makes ISRs a lot more flexible.

```c
DWORD isr_ctx[RD_NUM];

VOID MyISR(PDWORD regs)
{
    ScAwaitReentrance(KeLogf, isr_ctx);

    else {
        KeLogf("Hello, world!");
    }
}
```

# Processes

A process can be a native executable or it can be a DOS program. When a program is running in native mode, the system call interface is available. For DPMI programs, INT 31H is available but 80h is not. Processes can be active, blocked, or terminated. There is no support for exclusive tasking.

When an asynchronous event takes place, the process ID or process control block of the process can be obtained by the driver. This process is garaunteed to be the one that was switched out of.

# Events

I should change the event system. Maybe I can make the kernel non-reentrancy less of an issue.

# Worker Threads

The kernel has a reserve of private worker threads. These threads are invisible to the rest of the system, but are executed with every scheduler tick. Worker threads can be requested by the kernel API. The callback will run once and is pre-emtible. The thread takes a single PVOID argument.

Worker threads can safely call kernel procedures with no concerns of reentrancy. System calls, however, should still never be used.

Some IO operations are tied to worker threads, which produces latency due to having to wait for the next IRQ#0 for it to be scheduled. The advantage is enhanced multitasking.

# Starting the Scheduler

During initialization, interrupts will be disabled. They are only enabled if a DOS/BIOS call is made and it enabled the interrupts. The flags are saved.

The kernel exits the KernelMain function and goes back to IA32.asm, with the stack being completely empty. Then, IA32.asm loads the context of the first process into registers and uses IRET to enter the process. From now on the kernel will only run upon an interrupt of some sort.

# IO Scheduling

DOS has the following predefined file handles:
* STDIN  (0)
* STDOUT (1)
* STDERR (2)
* STDAUX (3)
* STDPRN (4)
The direct console IO command probably uses a .SYS driver interface.

Character IO calls operate on one of these handles. The problem with the DOS IO functions is that they are obviously not multitasking and only one thread can safely perform IO. This would be okay if blocking the entire system and giving DOS complete control was an acceptable way to handle IO, which it is not for OS/90. When a program requests IO, only that program should be blocked. DOS will have no involvement whatsover in input and output processing by default except for normal files.

This requires special scheduler support. When a process is waiting for an IO operation, the scheduler should not execute it until it finishes.

For example: Process A is trying to print "Hello, world!" on the screen. Process B is waiting for user input using STDIO. When the INT 21H is sent, the kernel will trap it. The processes will be blocked after each call. But how should the IO actually be performed? Any attempt to access the default file handles is now the responsibility of OS/90 and DOS has no involvement.

## Worker Threads and IO

When a program is attempting to write to STDOUT, the display driver is invoked, which is expected to create worker threads and use critical sections when necessary.

# Control-C and Critical Error

For DOS VMs, these are segments or protected mode interrupt vectors for these interrupts.

# Sub-processes

DOS allows programs to create subprocesses. A subprocess can be "executed" simply by loading it in memory as an overlay or creating a new process with its own PSP. Despite having their own program segment prefixes, subprocesses are not independent programs and do not get their own process control blocks. This is the correct behavior because DOS subprograms run like a stack, where exiting a process is like popping from it.

Protected mode DOS programs cannot execute subprocesses and will be terminated in this case.

# Reenterancy of the Kernel

The kernel is non-reentrant, meaning only one thread can run in kernel mode. Non-reentrancy means that a piece of code will break if it is interrupted by another invokation of itself in another thread. ISR's are fully atomic and do not reenter, but exceptions can interrupt ISRs and vice versa. We need to be able to call certain functions within interrupt contexts that cannot be implemented in a pure reentrany way, so a simple critical section is used by the callee. Interrupts are blocked while inside, which makes reentrance completely impossible and allows non-reentrant routines to be called from interrupts. Some functions can be safely called from an ISR and are marked with an ASYNC_APICALL in their header declaraction. Within an interrupt service routine, only such functions may be called.

Critical sections are implemented as the following:
```c
#define KeUseCritical DWORD __critical_eflags

#define KeBeginCritSec \
    __asm__ volatile(              \
        "pushfd" ASNL              \
        "pop %0" ASNL              \
        "cli"                      \
        :"=rm"(__critical_eflags)::);

#define KeEndCritSec \
    __asm__ volatile(\
        "push %0" ASNL\
        "popfd"   ASNL\
        ::"rm"(__critical_eflags));

```

Example of this being used:
```c
PTHREAD ASYNC_APICALL ScGetCurrentPCB(VOID)
{
    PTHREAD ret;
    UseCritical;

    KeBeginCritSec;
    ret = current_pcb;
    KeEndCritSec;

    return ret;
}
```

Using a critical section to run a non-reentrant kernel function from an interrupt does NOT garauntee safety, because the non-reentrant function may have began to execute and interrupts were not off when it started. Any exception generated from a critical section will potentially jeopardize the atomic nature of the section.

Not all asynconous events have atomic context concerns. For example, the system call ISR can call whatever functions it wants to because it will only be called by the user.

Memory allocation, for example, cannot be invoked in an ISR, so it is not implemented with critical sections.
