# System Entry Point

A system entry is the calling of any interrupt descriptor table entry from protected mode or virtual 8086 mode either by the INT family of instructions or by causing an exception.

A regular system entry does not terminate by an IRET instruction like exceptions or IRQs do. It will simply change the originally requesting process to USER and hang.

## Virtual 8086 Mode Monitor

The monitor is a procedure invoked only by the #GP handler if the exception was caused by virtual 8086. If the monitor returns an error, the process is terminated or a critical error is thrown.

When the virtual 8086 handler detects an INT opcode, it saves
