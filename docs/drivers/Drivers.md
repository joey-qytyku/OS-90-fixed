# Introduction

The most powerful feature of OS/90 is the driver model. It is designed to be used for programming devices, buses (PCI, ISA, VLB, etc.), and anything requiring ring zero access to the system. The driver architceture allows bus driver to manage interrupts and other resosurces through the kernel. Device drivers can then communicate with the bus driver to control individual devices and recieve interrupts and events.

# The Job of Drivers

A driver usually implements the intended function of a certain device, real or virtual, and allows other parts of the system to access it. OS/90 is a hybrid 32/16-bit system. Because of this, it needs a uniform interface so that it decides which components should be used. This is analogous to Windows 9x VxD drivers, however, the entire design is clean-house and has many differences. The driver model is what makes OS/90 a true operating system, rather than a protected mode extention to DOS.

# The Kernel API

OS/90 has an API for access to each of the subsystems.

## ABI

The kernel API uses modified cdecl calling conventions. stdcall was considered but it turns out that x86 has an `add eax,imm8` instruction, which makes `ret imm16` useless except if we want a one-byte gain for every procedure and extra confusion when using different compiler parameters. This also makes variadic functions more complicated.

Any function with KERNEL in its header prototype is part of the kernel API and will remain as such. KERNEL __must__ be used for any functions intended to be called by the kernel inself. That also includes the __main method__ and __any callback__.

## System Table

`System` is a C struct variable that is linked into every driver on startup. All API functions are organized into substructures. Always use it to call them. Never call a function by the header definition or the driver will not compile.

> Only `DrvAPI.h` has to be included to use the entire kernel API. There is no need to include anything else.

## Callbacks

## Register Clobbers

The parameters `-fcall-used-esi` and `-fcall-used-edi` are used to compile the kernel. This means that ESI and EDI are not callee preserved as they normally would be. It is done to improve performance and make linking C with assembly easier and faster. All kernel-mode software must be compiled this way.

Experiments have shown an improvement in code density. In one sample, the size dropped from 2349 to 2301, likely because the code does not need to spill registers to the stack between procedure calls.

## Warnings

Never modify the system table. By default, it is externed as a const structure.

Do not assume the position of elements in structures. Always use provided structure definitions. Currently, only the C language is fully supported.

# Synchronization

Interrupt request handlers are called with interrupts disabled.

# Plug-and-Play Support

Plug-and-play is a key feature of OS/90.

PnP is defined as such:
On system startup, after the kernel has finished initializing,
* Devices are disabled, in other words
  * No interrupts, all are masked
  * Resources cannot be disabled at this point, but that is part of the idea
* The PnP BIOS enumerates the mainboard and lists devices

Bus drivers act on subordinate devices as the kernel acts on all devices globally:
* Subordinate devices are disabled
  * Resources are not assigned or reconfigured outside the BIOS defaults
  * Interrupts are disabled in the per-device configuration
* If the bus is connected to another one, it may send interrupts after init
* All devices on the bus are to be enumerated and reported in the devfs

Resources are always owned by a bus and lent to other devices. Subordinate device/bus drivers cannot access resources they are not permitted to use by the parent bus.

The kernel is technically a bus driver that controls all system resources. If an independent device driver (LPT, COM, PS/2 mouse) takes an interrupt, it will call to the "kernel bus". This is to keep consistency.

# Resource Management

A resource is a DMA chanel, IO port range, IRQ line, or memory mapped IO location. The kernel has a list for all resources, IRQ lines, and DMA channels.

System device nodes from the PnP BIOS report resources with the PnP ISA format.

The owner is identified with a pointer to BUSDRV_INFO. This structure contains the name of the device, as well as a unique identification. It also contains information about the segment it belongs to.

## PnP BIOS and Motherboard Resources

The PnP resource information is reported as a tag-based structure also used for ISA cards. The IRQ and DMA resource items indicate which lines and channels can be used by the device. If a device has an interrupt mask of FFFF, it can be configured to use any IRQ. Same applies with DMA.

This means that resource usage is indicated only if one of the IRQ or DMA bits is turned on, informing the OS that the device can only use that interrupt if it is to be operational.

Some devices like the PIC or DMA controller are reported through PnP, but they cannot be configured.

System board devices are not automatically configured as there is little reason to do this. A user can manually reconfigure them if desired.

The PnP BIOS does not report everything. It will not report PCI devices. PCI VGA cards are hardcoded to use A0000h-BFFFFh and use BARs for SVGA VRAM.

## Problems

### Limited Configurability and Decode

The ISA bus is 16-bit so it does not support 32-bit memory access or addressing (24-bit). The ISA PnP specification seems to support this anyway, but the physical interface cannot possible handle that. ISA also can use 10-bit port decode or 16-bit decode, which could vary between cards because the ISA bus multiplexes the address pins for port access.

In the ISA bus, each card is sent the pin signals and, hopefully, only one responds by transmitting data. The difference in address decode bits is problematic because a card with 10-bit IO decode will only get 10 bits, and the upper ones mean nothing. E.g. if the base is 2F8, the address AF8 would access the exact same address. This can become a source of conflict. To solve this, OS/90 limits the PnP allocatable port space to 1024 bytes. Other ports can be accessed directly, such as the PCI conf space.

This applies to memory as well. Because of the 24-bit addressing, accessing address zero would be equivalent to accessing 0x01000000 or 0xFF000000. That explains why computers with the ISA bus normally do not have more than 16 MB of RAM, since a chunk of the memory will have to be reserved for ISA devices.

Computers that supported PCI and 16MB+ of RAM probably had measures to prevent problems. Perhaps they could refuse to assert the address pins of the address is above the ISA range. Internal devices like VGA (implemented in a chip) can support a higher internal decode.

The ISA PnP bus has more specifics that require specific design choices in the PnP manager. Devices can have certain memory addresses and ports which they can optionally be configured to use by software. For example, a parallel port card can be configured to the three standard ports and nothing else for compatibility.

> Conclusion: OS/90 limits port decode to ten bits and does not impose any limits on memory decode or assume the existence of aliasing.

### User Configuration

The user may need to change certain settings. Configurations must be maintained by the userspace.

## Interrupts

### Concept

The 8259 PIC is abstracted by the interrupt subsystem. Instead, a 32-bit virtual interrupt request, or VINT, are assigned to interrupt vectors, and VINT is local to a specific bus. As previously mentioned, the kernel is the low-level bus driver that controls all resources on startup.

The VINT is physical to the kernel bus, so VINT 15 is the same is the actual IRQ 15. The kernel can be modified to support IOAPIC.

### Rules

Interrupt-related routines cannot be within an ISR. This is undefined behavior.

The kernel modifies interrupt entries within a critical section.

### Types

An interrupt can be:

BUS_FREE:
This interrupt is managed by a bus and cannot be taken if accessed through the bus. The utility pointer refferences the bus driver header. Upon startup, this IRQ was not used by DOS in any way.

BUS_INUSE:
It is taken by a bus-subordinate driver. IRQ sharing is possible, but only under the terms of the bus driver. The kernel bus does not permit this.

For the kernel, this IRQ was detected by the PnP BIOS and is statically assigned to a plug-and-play system board device. It cannot be taken for use by any driver.

RECL_16:
A reclaimable IRQ which is sent to real mode. Typically used with DOS drivers for non-PnP hardware. A bus driver should only take it if the interrupt line is used by a bus device. Utility pointer has no meaning.

The utility pointer has a different meaning depending on the interrupt type.

All interrupt ownership requests go through a bus driver, which can be the kernel, in which case the kernel simply handles it directly without event handling.

### Interrupt Callbacks

The kernel calls the owner of the IRQ and the owner has to handle it. It decides how it will notify the driver that an interrupt has occured.

### Legacy Support

OS/90 does what old versions of Windows do. The interrupt mask register is all ones on startup, and inserting handlers unmasks them. Because of this, we can assume an interrupt has a handler if the initial mask indicates it as unmasked.

A 32-bit driver for an non-PnP ISA card uses InAcquireLegacyIRQ to change it from BUS_FREE or RECL_16 to BUS_INUSE, with the owner being the kernel.

### Interrupt API

## Device Abstraction

Some devices are embedded to the system board, while others are attached to a bus (PCI, PCMCIA, ISA PnP). Most buses have their own DMA subsystems, while ISA PnP uses the standard AT DMA controller.

# Virtual Devices and Fake Interrupts

All programs are DOS virtual machines and can access the DOS interfaces. They can also access ports and memory-mapped IO, as well as set interrupt vectors that are local in scope. This is entirely emulated. Memory-mapped IO can only use direct buffers, so emulating devices that require getting each read/write is not supported.

See dosvm.md for the full specification.

## Methods of Emulating Device IO

The kernel never allows ports to be read or written to directly. All IO port instructions are decoded and executed in protected mode by the kernel on the behalf of a driver. This is done within the __non-preemptible context__ of an exception handler.

The first method of arbitrating IO would be first come first serve, and block any process that is trying to access the device besides the current user. This is easy to implement. A boolean lock and a PID variable are all that is needed. This type of emulation is not particularly fast despite the simplicity because port IO still has to be decoded and executed by the kernel and only after it has gone through the device chain. Some devices are probably not going to be used directly by multiple processes anyway, so this could be acceptably simple.

The second method is to fully emulate the hardware interface. Separate emulation contexts must be created for each process that is accessing it. A queue can be used to store requests and send it to real hardware after processing. Counting semaphores can be used to provide limited emulation slots to several processes and queue the requests.

# Programming

The driver model permits dynamic loading and unloading, which could be implemented in the future. A driver should be prepared to handle an unload event.

## ISA DMA

To use ISA DMA, memory has to be allocated under 16MB. There is no support for doing this with extended memory, so a conventional memory block has to be allocated that is up to 64K in size.

The region of memory MUST be marked with cache disabled. On an i386, this may not be a problem as cache was typically outside the CPU, but on certain models of the i386 and the i486 or better processors, cache coherency is not garaunteed. By default, cache is allowed for all pages under 1MB except for the BIOS ROM. This means that a driver must disable cache using a memory management for all of the pages. This can be tricky due to the paragraph granularity of DOS memory, but OS/90 provides a simple function for this. It will cause collateral damage and some memory may be needlessly cache disabled, but is garaunteed to CD everything requested.

```c
VOID DisableCacheUnder1MB(PVOID location, WORD bytes);
```

The base address will be rounded down by a page. The paragraph count will be ceiled to a page boundary.

## Trap Capturing

Summary of defines:
```
CAPT_NOHND = 0 // Driver does not know how to handle this function
CAPT_HND   = 1 // Driver handled the function successfully.
// Eg: CAPT_HND
```

Capturing traps involves the creation of a chain of handlers. If one cannot handle the 16-bit trap (e.g. not the function code it wants) it is passed down to a driver that may handle it. A structure in the kernel already exists for all 256 real mode interrupt vectors. This structure is passed to the handler. If the handler cannot handle the requested, it must set the return status to CAPT_NOHND and yield to the kernel. The kernel will then call the next one upon realizing that the handler cannot perform the task.

If the kernel reaches the end of the chain with no handler returning successfully, it will call the real mode version.

The overhead with this feature would be similar to that of a 16-bit DOS function dispatcher, but with the added weight of the CALL instructions and entering through V86 mode to service the interrupt. Basically, this will a bit slower that real mode DOS, but it depends on the number of links. Range checking is the correct way to implement function dispatching. To use a call table, the lower bound can be subtracted and used as an index.

To add a new capture, a function returning dword and taking a trap frame with external linkage must be written. A TrapCaptureLink variable should point to that variable, other fields are reserved and need not be set Chaining something that already has a handler cannot be prevented, but it is possible to check if it has been handled before.

Trap handlers are only called when DOS calls the INT instruction. IRQs are handled separately, although there is no reason to call an ISR with INT.

Note that the chain works differently than it would in DOS. Hooking an interrupt will not cause the new handler to run first, but last, thus hooking a vector cannot override another hook. Make sure to check the `AH` signature carefully to avoid problems.

### Example

```
#include <DrvAPI.h>

BOOL KERNEL HookProc(PVOID _)
{
    UNUSED_PARM(_);
    System.Debug.Logf("Trying to use INT 21H!");
    return CAPT_NOHND;
}

VOID KERNEL Init()
{
    System.Scheduler.HookDosTrap(0x21, HookProc);
}

```

### Notes on Kernel Reentrancy

Virtual 8086 mode protected mode hooks are usually preemtible contexts but are not always this way. V86 hooks can be invoked by multiple threads and can be entered by multiple kernel threads, which means that synchronization may be needed for certain tasks. It is up to the driver to do this safely.

Kernel callbacks provided by drivers generally need to be thread safe.

Check if mutex locks are already acquired before acquiring them to avoid locking the system, especially when debugging deadlocks. If a mutex lock is already acquired within a possibly non-preemptible context, the correct course of action is to pass and not acquire it at all.

## Callback Hooks

TODO

## Allocating Memory

Allocating memory must be done with page frame allocation or INT 21H. It is possible to allocate conventional memory, but this should be avoided because conventional memory is a scarce resource and the system will need a decent amount of it to run normally. As a general rule of thumb, there should be at least 72 KB free at all times in case the kernel itself needs to use it.

Page frame allocation should be prefered for large structures. It can be safely used with heaps. Contiguous memory that is safe for DMA can also be allocated.

## Interrupt Handling

Handling interrupts is possible with the bus/device model of driver communication. A device driver can be subordinate to the bus driver and recieve interrupts through it.

A bus can claim any free interrupts that it needs for IRQ steering. RECL_16 cannot be claimed. STANDARD_32 can be "stolen" by a device driver but never by a bus, so that standard hardware can be implemented in the kernel and in drivers.

Wait what about RECL_16?

Interrupts owned by the bus are signaled by the kernel, and the bus uses callbacks to signal the final handler function.

Make the ID a pointer?

Device drivers can request a device on a bus by signaling the bus driver with the appropriate ID. The exact format of the ID can be anything that fits in a 256-bit number and must be known by the busdrv and devdrv. A driver can support the same device with different capabilities, such as native mode IDE vs legacy IDE. It can call the request function several times to probe the capabilities, but this should be avoided and the driver should support all possible cases.

Duplicate devices are possible and must be handled by the bus. It does not technically matter which one a device ends up using, but the duplicates should be accounted and reported so that the device driver can control both devices if possible. In the case of PCI, the vendor and device ID would classify a device, rather than the prog-if of capabilities. Example, two PCI-IDE controllers.

# Synchronization

OS/90 has a reentrant kernel and multiple programs can be in kernel mode. Callbacks provided by drivers to the kernel could generally reentered by another kernel-mode thread except under certain conditions.

V86 hooks, for example, can be entered by multiple processes because they are called within a preemtible context. This requires locking if a certain resource needs to be shared.

# Driver-User Communication and Events (WIP)

OS/90 uses a single addressing space, so programs can easily communicate with drivers. DOS interrupt hooks can be used, but the easiest method is shared memory. This is used for the event system.

The event subsystem is for general communication between components and is not good for anything that is speed critical or requires direct hardware access. The response latency is the time it takes for a context switch to the thread, which in the very worst case will be equal to the number of active threads times their timelices.

## Basic commands: Send, Signal, Forward,

```
tstruct {
    U16     function;
    U8      payload_size;
    U8      completion_code;
    ATOMIC  lock;
}EVENT_PACKET,*P_EVENT_PACKET;
```

EVHND is a handle to an event. Internally it is a pointer, but the implementation is opaque and subject to change.

```c
// Kernel
VOID Example(EVHND self)
{
    EVENT_PACKET evp = {};
    evp.payload_size = 0;
    evp.sender       = self;
    evp.function     = GENERIC_CONNECT;
    evp.interrupt    =;

    PnP_Send(&evp);
}
```

CONNECT is a basic function that will decide if the program requesting has a right to send further packets. Packets not sent between CONNECT and DISCONNECT should be ignored and are not garaunteed to be recieved.

PnP events are passed by reference in a message stack. If the stack is overflowed, a special callback is initiated to deal with the overflowing request specifically. Threads trying to access it are stalled.

PnP events are not asynchronous. The client has to put them in a thread if that is desired as with sockets. Multiple requests can and should be handled properly, but the request sender will not continue executing until the event finishes.

## Forwarding requests

Requests can be forwarded to make it look like the event came from a different client. This requires a mediator.

## Thread Safety

Thread safety is garaunteed to the extent needed for the event system to work. Any time a dispatch or interrupt routine needs to access a shared resource, a locking mechanism is needed.

The event packet is a read-only structure that is never to be modified at any point in time outisde initialization. It can point to mutable data, but it must never be modified. For this reason, syncrhonization does not need to be used to access it at all.

## Full Event API Outline

```c
VOID PnP_RegisterDriver(P_EVCONFIG);
VOID PnP_Send(P_EVENT_PACKET);
P_EVENT_PACKET PnP_GetEventPointer(EVHND);
EVHND PnP_GetHandleFromPointer
```

## VOID PnP_Interrupt(EVHND)

Interrupt a request. PNP_EVINT_CANCEL is the only one that is formally defined.

Implementing control C behavior could be done with this type of function.

## BOOL PnP_IsPacketFromSupervisor();

## Example

```c
ATOMIC disp_lock;

// The dispatch routine runs periodically.
VOID Dispatcher(P_EVENT_PACKET ev)
{
    // Kelogf is thread safe but we do not want to interleave output
    AcquireMutex(&disp_lock)
    if (ev.function == PNP_EV_CONNECT)
    {
        Kelogf("Connecting");
    }
    ReleaseMutex();
}

VOID DriverInit()
{
    KEV_CONFIG evc = {
        .disp = Dispatcher
        .drvh =
    };
    PnP_RegisterDriver(&evc);
}

```

## Dispatch Thread

A single thread deals with dispatching. The dispatch routine is a single iteration of the "check mailbox" operation.

# Multithreading

The userspace has difficulties with multiple threads, but it is easy for the kernel. A special procedure does this.

```
KTHREAD_INFO ExecKernelThread(KTHREAD_PROC);
VOID TerminateKernelThread(KTHREAD_INFO);
```

KTHREAD_INFO is a 32-bit opaque type. Internally, it is a pointer to a PCB. Because the process is always in kernel mode, it has no real mode PSP and must be terminated by the pointer.

The signature of a thread procedures
```c
VOID ThreadProc(VOID)
{
}
```
The entire interface is very barebones. The thread procedure does not return or take arguments and must manually terminate itself. It should be marked with _Noreturn.

## HLT

It is highly recommended to place an HLT instruction at the end of kernel threads if there is no useful work to be done. OS/90 does not have scheduler yielding.

HLT can be used anywhere else in the kernel that is expected to take a long time or needs to wait for some external event. Because the scheduler has only milisecond resolution, placing a halt can save up to a milisecond's time from consuming CPU power.

# Initialization Order

Drivers must be loaded in a specific order in many cases. Drivers for non-PnP devices or PnP drivers with such features disabled must be loaded first to ensure that fixed resources are captured on startup and do not interfere with plug and play devices.
