## Concurrency Safety Guidelines (IMPORTANT)

OS/90 has a fully preemptible kernel and almost everything is subject to the rules of concurrency.

All memory pages are locked in OS/90 by default. Ensure that locked memory is used for all kernel mode stacks and and code that needs to be called by a TI context.

API descriptions should describe the virtual memory safety. If locked memory must be used, such a fact will be specified. Changing page properties of any sort on pointers returned is usually illegal, especially if a constant pointer is returned.

Interrupts can cause page faults BEFORE they actually take place, but obviously not after.
