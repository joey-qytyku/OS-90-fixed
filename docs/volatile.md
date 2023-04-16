# When to use volatile (MCHUNX)

MCHUNX is a type qualifier equivalent to volatile in OS/90 C. Volatile disables optimizations which can cause bugs when a variable is changed outside the compiler's control.

Interrupt service routines, for example, must use volatile for variables because:
1. Access to that variable may be underway by other code
2. ISRs are themselves interruptible.

# Library Functions and Volatile Variables

The kernel exposes library functions to drivers that use data structures and variables. The question is, if other software can access API calls and indirectly modify variables, do API variables have to be marked volatile?

The answer is that such variables DO NOT NEED TO BE. Kernel code, including drivers, is non-interruptible. By the next time a function related to a data structure is called, all modifications will have been flushed. Even if an outside program modified a variable in the kernel, it would not be a problem if it is not part of an ISR, as all functions in the kernel are entered regularly. Putting volatile everywhere makes everything harder and causes "discarded qualifier" errors.

API calls which can be used inside an ISR will use volatile when appropriate. Most calls, however, cannot be called in the interrupt context, and generate fatal errors if this is done.
