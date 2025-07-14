# Initialization

OS/90 is a complex piece of software in which different components must be initialized at certain times to satisfy dependencies.

Data structures that produce dependencies are mentioned here.

An unordered list of dependencies:
- Memory manager requires V86 to detect memory
- SV86 requires a working IDT and GDT and should not run with IF=0
- Scheduler needs a first process to execute (maybe live patch it?)
- Interrupts require the real mode switcher
- Memory manager and boot driver loader need configuration files
- Config parsing requires V86 to access FS
- Potentially more...

These are the first steps. Interrupts will be fully disabled until the end.
- i386
- SV86
- Init thread created
- Scheduler

The init thread does the rest of the initialization. The state that the kernel is in prior is unschedulable and has a temporary context that is permanently destroyed by the scheduler. A new stack will be used. The init thread task block is reclaimable.
- Config file parse
- Memory init
- Driver load and init code execution
- Kernel exec string
- Init thread terminates

## i386 Init

This stage inserts IDT entries, fills the GDT and loads the GDTR, and prepares the LDT. The PIC is remapped to the designated base vector.

The real mode swapper code is also copied into the HMA.

## SV86

SV86 does carry state and has dependencies for future use. This simply initialized the handler list to the default reflection stub handler that tells the V86 call interface to go to real mode.

There are doubts about its ability to work with interrupts disabled. It certainly cannot work with interrupts disbled with no active processes and the scheduler off.

## Init thread created

The task block for the init thread is created.

This is a simple solution for the problem of what the scheduler should do if there are no tasks. When the task is created, a new thread procedure runs and the main function exits.

The original invocation of main is lost forever. The scheduler destroys the state on the first IRQ#0.

Preemption and interrupts are enabled. The kernel init thread gets a time slice of 1000, using 100% of the CPU to start up. The idle thread is also created and ready to execute.

## Memory Init

> The exact procedure is explained in greater detail in the code.

The memory manager has a large bit of state that needs ot be initialized. It also depends on SV86 which was initialized earlier.

The page directory is kept in the HMA. Cache is enabled for the first 640K and the HMA, but not the UMA. The page directory table remains identity mapped as usual. The HMA uses ring-0 privileges so that V86 programs do not damage it.

An address space is reconstructed for the kernel with cache enabled, all pages locked, and ring-0 with no write protection.

By the end, the OS/90 heap manager and page allocator will both be ready to execute. Virtual memory is also ready but is not used by the kernel during bootstrap at all (and should not be).

Memory is finally allocatable at this stage.

## Driver Load

Drivers are loaded into the allocatable region and their entry points are executed.

## INIT Termination

The INIT thread cannot continue to execute since it uses 100% of the CPU. It is scheduled for termination.
