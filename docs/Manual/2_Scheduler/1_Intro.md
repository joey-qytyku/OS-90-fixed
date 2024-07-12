# Scheduler

OS/90 has a fully preemptive reentrant kernel and time slicing scheduler. The kernel can schedule tasks while it is running without the need for event scheduling and is able to handle certain exceptions concurrently.

The scheduler is made up of the following components:
- Task scheduling
- Interrupt requests
- Exception dispatching
- Supervisor virtual 8086 mode (SV86)

The scheduler is percentage based. Time slices are 1 milisecond long. Time slices are taken from a pool of 1000 miliseconds and each task gets at least 1 MS. This means that OS/90 has a true concept of "CPU usage" as a fraction. If there is only one task with a time slice of 10 MS, then 1% of the CPU is being used. The remaining 99% are given to the idle thread.

The reason why this is done is because the number of time slices given to a task in an isolated context is not an accurate way to ensure accurate scheduling.

Percentage-based scheduling should not be construed as permitting real-time programming. OS/90 is not intended to be a real time operating system. Do not rely on any kind of single-milisecond precision and keep in mind that most drivers yield slices and can make RT scheduling impossible.

> This should never come up, but never trust OS/90 for anything important. If a surgical patient dies or a plane crashes with a one-off mistake in timing, do not use OS/90.

## TASK Structure

In complete contradiction to common practice, OS/90 has a public task block structure with plenty of garauntees of proper behavior when doing so, but with some requirements.

The TASK structure uses some fields internally and such fields may change in meaning. The only ones that C code should ever access are the ones without underscores in the names. Furthermore, a task cannot be executed unless it has been properly prepared by the scheduler.

TASK structure fields that are defined never move, so all code that accesses the task structure is binary compatible.

Task blocks never move and are implicitly 4KB in size, with 2K being reserved to the scheduler and the subsystem block. 2K is for the defined stack, but an overflow may not always be fatal.

Task IDs exist as mere pointers, and functions that operate on tasks usually take a PTASK. The pointer is valid so long as the task is not terminated explicitly.

Casting a PTASK to PSTDREGS is sufficient for accessing the registers in all circumstances.

## Hooking
