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

OS/90 does not have a FS subsystem and relies completely on the DOS interface for it. The filesystem only works after (4).

## SchedulerInitPhase1

This is the first phase of the scheduler and the first to initialize. It enables virtual 8086 mode, which is critical for enabling the other subsystems.

System entry is also safe after this point, but only by V86.

Filesystem is not yet available.

## MemoryInit

MemoryInit uses virtual 8086 mode to get the amount of RAM.

Virtual memory will be operational, but it will not be used during initialization.

## PnP_Init

The PnP manager needs to access the PnP BIOS, hence why it needs (1). PnP_Init will use the BIOS to detect hardware and reserve resources.

## SchedulerInitPhase2

SchedulerInitPhase2 depends on MemoryInit and PnP_Init because it needs to allocate memory for process control blocks and to dispatch interrupts through the PnP subsystem.

This will access the filesystem using V86 in order to execute USER.EXE. The process is blocked for now.

## DriversInit

This step will load drivers into memory and run their initcode, and it requires a fully operational kernel.

Additionally, this procedure may parse configuration files.

## SchedulerInitPhase3

Drivers have set up their interrupt hook etc and can provide services to processes. USER.EXE is unblocked.
