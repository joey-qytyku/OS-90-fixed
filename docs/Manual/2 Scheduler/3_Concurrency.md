## Concurrency Safety Guidelines (IMPORTANT)

OS/90 has a fully preemptible kernel and almost everything is subject to the rules of concurrency.

All memory pages are locked in OS/90 by default. Ensure that locked memory is used for all kernel mode stacks and and code that needs to be called by a TI context.

API descriptions should describe the virtual memory safety. If locked memory must be used, such a fact will be specified. Changing page properties of any sort on pointers returned is usually illegal, especially if a constant pointer is returned.

- Never lock or unlock a page that is not allocated by the memory API, especially real mode memory, as this can interfere with thread safety.

- Never hold a lock within an ISR ever, or use any other primitive. You may test a lock, however, though caution is advised.

- Never hold a lock in T0 or T1.

- Never yield in T0 or T1.

- Locked pages must be used for all kernel stacks and interrupt service routines. Never unlock such pages.

- Atomic data structures such as locks must always exist on locked pages. (RATIONALE NEEDED)
	- The rationale is that a page fault handler will run in a preemptible context and there is no guarantee that the lock will behave properly because the access is deffered and the lock could possbly be accessed by an exception handler.
- Only functions capable of running in a certain Tx should be used in a given context.

- Any user-provided atomics must call S_Yield somewhere.
