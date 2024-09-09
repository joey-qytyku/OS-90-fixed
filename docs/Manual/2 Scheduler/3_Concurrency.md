## Concurrency Safety Guidelines (IMPORTANT)

OS/90 has a fully preemptible kernel and almost everything is subject to the rules of concurrency.

All memory pages are locked in OS/90 by default. Ensure that locked memory is used for all kernel mode stacks and and code that needs to be called by a TI context.

API descriptions should describe the virtual memory safety. If locked memory must be used, such a fact will be specified. Changing page properties of any sort on pointers returned is usually illegal, especially if a constant pointer is returned.

- Locked pages must be used for all kernel stacks and interrupt service routines.
- Atomic data structures such as locks must always exist on locked pages. (RATIONALE NEEDED)
- Only functions capable of running in a certain Tx should be used in a given context.
- Never hold a lock within an ISR ever, or use any other primitive.
- T1 and T2 are quite interchangable due to yield semantics, but not always. Check the CT of every callback.
- Any user-provided atomics must call S_Yield somewhere. They must work in T0, T1, and T2.


