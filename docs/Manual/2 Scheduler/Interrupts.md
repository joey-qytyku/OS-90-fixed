## Interrupts

By default, all interrupts go to real mode. The OS actually switches to physical real mode to carry out the IRQ. This is what was done by Windows 3.x on standard mode. It is the responsibility of 32-bit drivers to reclaim interrupts and directly handle them.

Interrupts can be routed to tasks. The scheduler IRQ#0 handler checks for them before entering and emulates the stack as it should for an interrupt. The actual interrupt flag is always on, but the virtual interrupt implies further ones are blocked.

On OS/90, task-local interrupts can feel free to call any OS-level API they desire as long as it does not depend on an another interrupt, but that would never happen anyway.

The task block can only be accessed when interrupts are fully disabled within an Effective TI context. This allows the task links to be changed so that the interrupt is immediately handled.

If the task is in the kernel, accessing the saved registers of the user is a terrible idea and makes implementing APIs needlessly complex.
