# Source Code

## Procedure Naming

The OS/90 kernel follows a strict naming procedure. A prefix indicates the thread/interrupt safety of a function, performance cahracteristics, what types of contexts it is safe to call in, and reentrancy.

It does not look nice, but is actually a grreat idea. It forces the programmer to think about important attributes of functions and makes wrong code look extremely wrong.

Callbacks also use this system.

Examples:
```
OsDpT12Sd_HookSV86ISR
OsDp0T12Sd_SVINT86
```

The full guidelines are as follows:
* IA32-related procedures will be prefixed with "i386." Example: i386SetLDTEntry
* Functions exposed to the kernel API will have it in the name now. "Os"
* Subsystem prefixes are back. If a function has no specific subsystem or could be used by multiple, can overlap or the prefix can be omitted.
* Pascal case is used
* Function return and inputs should be expressed as "To X from Y" if applicable.

Prefixes:
- i386: Low-level functions for CPU
- Os: Any API call
- Sd: Scheduler
- Isr: IRQ handler
- T0: Safe for interrupts off context caller
- T1: Safe for non preemptible context caller with interrupts on
- T2: Safe for preemptible context caller (may still not be thread safe)
- T{0,1,2}: Safe for any combination. For example, T01 or T12
- Tx: Safe for 0,1,2
- As: Asynchronous caller safe, or safe inside ISR or IRQ off section. This may be because it disables INTs. Just because a function is reentrant, does not mean an ISR should use it!
- Ts: Thread safe (does not imply reentrant nor T2).
- Re: Reentrant (implies safe in all cases, including As)
- Um: Uses mutex lock
- Xm: Expects mutex is held
- Xpd: Expects preemption disabled (may remove)
- Xid: Expects interrupts disabled (may remove)
- D{p,i} Disables preemption or interrupts

Os must take precedent over all in the ordering. Scheduling/context/threading characteristics come second and can be merged. Subsystem is the very last.

I highly recommend using this naming convention in all driver code.

## Variable Naming

Snake case is to be used for all variables. Global variables must be prefixed with `g_`.
