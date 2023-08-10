# Subsystem Initialization

The OS/90 kernel has the following subsystems:
* IA32
* Scheduler/V86/Sysentry
* Memory Manager
* PnP Manager
* Driver loading

IA32 is set up automatically.

This is the order of initialization:
1. SchedulerInitPhase1
2. PnP_Init
3. MemoryInit
4. SchedulerInitPhase2
5. DriversInit

Some subsystem initialize internal functionality on their own, but that is not covered here.

## SchedulerInitPhase1

This is the first phase of the scheduler and the first to initialize. It enables virtual 8086 mode, which is critical for enabling the other subsystems.

System entry is also safe after this point, but only by V86.

Is the FS available? Does the BIOS work when interrupts are off? It will probably enable them on its own if not.

## MemoryInit

MemoryInit uses virtual 8086 mode to get the amount of RAM.

Virtual memory will be operational, but it will not be used during initialization.

## PnP_Init

The PnP manager needs to access the PnP BIOS, hence why it needs (1). Init will use the BIOS to detect hardware and reserve the resources.

## SchedulerInitPhase2

SchedulerInitPhase2 depends on MemoryInit and PnP_Init because it needs to allocate memory for process control blocks and to dispatch interrupts through the PnP subsystem.

This will access the filesystem using V86 in order to execute USER.EXE.

## DriversInit

This step will load drivers into memory and run their initcode, so it requires a fully operational kernel.
