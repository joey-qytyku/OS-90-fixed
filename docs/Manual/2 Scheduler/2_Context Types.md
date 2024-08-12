# Context Types

Understanding the OS/90 scheduler requires knowing the meaning of a context. In OS/90, a context is the machine state of a thread of execution which may be currently in execution, entered, exited, or switched to another. Contexts are described by types in OS/90 based on how the scheduler interacts with them.

Functions provided by the kernel are specified to work in certain contexts.

Context types can be considered similar to Windows NT IRQ Levels (obviously much simpler) because it defines situations in which code in one context cannot be preempted by code in another, and determines the correct transfer of control flow.

TI or simply IRQ:
- Interrupt handler.
- Entered by external IRQ
- Interrupts are OFF and only approved functions may be used
- Any function called must be fully reentrant or internally CLI/STI guarded to prevent reentrancy problems.
- Preemption off.
- May never give control to any code made for other contexts. - In general, reentrant code is IRQ-safe but that may not always be the case depending on what the code does.
- Not schedulable or restartable like the others.
- MUST EXECUTE AND ACCESS LOCKED MEMORY ONLY. NO FAULTS EVER.

The other contexts are controlled by the transfer of state mechanism of the scheduler, which is implemented in TI itself, thereby making TI unschedulable.

T0:
- Voluntary interrupts disabled section
- Entered from T1 or T2 by disabling interrupts
- Preemption off
- Exceptions are acceptable, including page faults.
- Local to each process because of an interrupts disabled counter. It is safe to yield to another task while in T0.

T1: Preemption is disabled but interrupts remain the same. Entered by incrementing the local preemption counter.

T2:
- Preemption and interrupts are on
- Exceptions are safe to generate
- Mutexes are safe

TI, T0 and T1 are preemption-off contexts.

T0, T1, and T2 are thread local contexts. This means that yielding to another task from an interrupts off section is perfectly possible. The preemption counter is local to each process, so yielding from a preemption off thread will continue running other tasks normally.

TI is not a schedulable and restartable context.

There is ONE exception to this system. The FPU IRQ#13 handler represents more of a trap or system entry than an actual IRQ since it is a synchronous event caused only by user mode.

## How Do I Get the Current Context Type?

The answer: you don't. It is already known, and code should be specified for specific contexts.

For the purposes of error checking or any odd scenario where it is necessary to get the current context, there are functions for that. TI cannot be detected and must be known.
