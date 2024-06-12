OUTDATED. The new document is now a libreoffice document.

# Kernel API Handbook

The OS/90 kernel has the following distinct subsystems:
- Debugging (db)
- Drivers (dr)
- Low-level (ll)
- miscelaneous (mc)
- Memory manager (mm)
- Scheduler (sd)

The parentheses indicate the source code folder.

The OS/90 kernel is designed around the API provided by the kernel rather than the other way around. This document is authoritative for the entire API and supersedes any other documents..

# Scheduler

The scheduler is more broadly defined in OS/90. It refers to anything that controls the behavior of the changing of contexts, or machine states. This includes task scheduling, interrupts, and also the use of virtual 8086 mode.

## Virtual 8086 Mode

Listing:
- VOID INTxH_T012();
- VOID HookINTxH_T012(BYTE vector, V86HND hnew, V86HND *out_prev);

## Task Management

Listing:
- SIGLONG MkTask_T012(PTASK ts);
- SIGLONG Terminate_T012(PTASK ts);
- SIGLONG PutToSleep_T012();
- VOID SetTaskSlice_T012(PTASK t, SHORT val);
- SHORT GetTaskSlice_T012(PTASK t);

Use some kind of task create/change state parameter block?

## Thread Control

- VOID KTHREAD_PROC(PVOID args);
- VOID CreateKernelTask(PTASK ts, KTHREAD_PROC proc);
- VOID YieldKernelTask(VOID);
- VOID SwitchToTask(PTASK t);
- VOID PreemptInc(VOID);
- VOID PreemptDec(VOID);

## Task-local Exception Handling

## Interrupts

- VOID ISR(STDREGS*, LONG error);
- SHORT GetInService(VOID);
- SHORT GetIrqMask(VOID);
- ISR GetStage2ISR(VOID);
- VOID SetStage2ISR(ISR);

### TASK Structure

This structure is standardized for all members that have no underscore, and those that do are simply reserved for the scheduler to use how it wants.

A task should not be created directly however. First, it ust be initialized so that any fields that are kernel-reserved are properly initialized and unused memory is zeroed.

### VOID ISR(STDREGS*, LONG error)

A handler for both an exception or interrupt.

The error code is undefined for IRQ handlers. In the assembly stub, it may not even be passed.

### GetInService

Returns the index of the IRQ currently being serviced.

### GetIrqMask

Returns a 16-bit mask of which interrupts are blocked from being serviced.

### GetStage2ISR, SetStage2ISR

Gets or sets the stage 2 ISR. This is what drivers will use to set IRQ handlers.

# Memory Manager

The memory manager is bloken into the following components:
- Chain allocation
- Mapping allocation/generation
- Page attribute management
- Virtual memory and swapping
- Information query

> MM is a highly isolated subsystem. It uses a global lock to prevent concurrent access to its features. If it is necessary to call a MM function, the MM lock must be tested and if it is held, access to MM must be called off by the ISR.

## Chain Allocation

- LONG CAlloc(LONG bytes_commit, LONG bytes_uncommit, PLONG commit_head);
- VOID CFree(LONG chain);
- SIGLONG CResizeWithCommit(LONG chain, SIGLONG delta_bytes);

### CAlloc

commit_head is a pointer to a value that indicates how many committed bytes have been requested in total. This is changed when resizing. Can be null if not needed.

### CResizeWithCommit

This will cut into the uncommitted space if the delta is positive.

## Page Map Management

- PVOID CPMap(LONG chain, LONG using_attribs);

The attributes follow the EXACT same format as x86 pages do. Set any unused bits to zero.

The extra OS/90-specific page attributes may be used except for uncommitted.

### CPMap

Allocates a mapping in the allocatable region of the VAS and maps the entire chain there. Returns NULL if failed.

## Page Attribute Management

- VOID PStain
- VOID PClean
- VOID PLock
- VOID PUnlock
