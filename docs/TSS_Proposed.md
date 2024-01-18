# Hardware Multitasking?

> Threads are bimodal. This presents a major problem for HMT because we would need two TSSes per process.

Hardware multitasking could be used for OS/90. It could simplify some parts of the scheduler in various ways.

I have to redesign the whole idea of a task. SV86 would be a TSS.

Using a TSS for exceptions means that the previous context can be autosaved.

> Add option to disable Logf. It can be fully removed from the kernel, but not from drivers.
> Make a macro for the kernel and let drivers use the function.

## The GDT, LDT, IDT

## Exceptions

The exception handler is a ring-0 task. In it executes an infinite loop that has an IRET instruction at the end. IRET works in an unusual way with x86 HMT. It saves the current running context and goes to the back linked task. The task can be entered again and executes directly after IRET.

Can this even work? A double fault cannot work this way because tasks are not reentrant. We could do special handling for the double fault since it is unrecoverable.

The OSDev wikie recommends this specifically for reliable double fault handling, but that is literally wrong.

## SV86

SV86 is an independent TSS.

## Context Switching

Context switching is done with the JMP instruction.

## System Entry

## IRQs

IRQs work the normal way. They are not a task, but a regular interrupt context entered by an int gate. Going through the hardware switching process for an IRQ is not worth the effort.
