# Introduction

The most powerful feature of OS/90 is the driver model. It is designed to be used for programming devices, buses (PCI, ISA, VLB, etc.), and anything requiring ring zero access to the system. The driver architceture allows bus driver to manage interrupts and other resosurces through the kernel. Device drivers can then communicate with the bus driver to control individual devices and recieve interrupts and events.

# What is a Driver in OS/90

Drivers are 32-bit relocatable NXF files. The entry point is the driver descriptor block. Drivers are loaded flat into the kernel space after relocation. Drivers can be inserted and removed at any time, or at least they will be in the future.

## The Job of Drivers

A driver usually implements the intended function of a certain device, real or virtual, and allows other parts of the system to access it. OS/90 is a hybrid 32/16-bit system. Because of this, it needs a uniform interface so that it decides which components should be used. This is analogous to Windows 9x VxD drivers, howver, the entire design is clean-house and has many differences. The driver model is what makes OS/90 a true operating system, rather than a protected mode extention to DOS.

## Comparison Between VxD and DM/90

* VxDs are designed around assembly language and require thunking to use services from C. OS/90 uses C calling conventions and supports C and assembly.
* VxDs and OS/90 use a practically identical method of hooking 16-bit INT calls.
* VxDs allow hooking exception vectors but OS/90 does not, though it lets programs set local exception vectors.
* OS/90 supports plug-and-play while VxD requires extra software for PnP.
* VxD has a concept of a system virtual machine and interrupts can be owned by a virtual machine. OS/90 has fake interrupts for processes that need them which can be scheduled by an actual IRQ.
* The VMM of Win386 and the KERNL386.EXE of OS/90 can both be described as non-reentrant.
* VMM appears to have a more complex scheduler and synchronization architecture.
* VxDs have a real mode initialization segment. OS/90 does not have this.
* VMM supports monotasking a single process while OS/90 only allows a process to be blocked, running, or terminated.
* VMM allows critical sections to service interrupts if specifically requested. OS/90 does not because it will never schedule other software within kernel mode.

# The Kernel API

The kernel API uses cdecl calling conventions. stdcall was considered but it turns out that x86 has an `add eax,imm8` instruction, which makes `ret imm16` useless except if we want a one-byte gain for every procedure and extra confusion when using different compiler parameters.

## Callbacks

All callbacks passed to and from the kernel use cdecl and should also be marked with `KERNEL`. Functions passed by callback can be `static` as long as they are marked `KERNEL` so that they are not inlined and are forced to use `cdecl` conventions.

## KLXTRN

KLXTRN sets the

# General Notes

Never assume the position of elements in structures. Always use provided structure definitions. Currently, only the C language is fully supported.

# Synchronization

Interrupt request handlers are called with interrupts disabled. The kernel can be interrupted at any time by an IRQ, and is non-reentrant.

Functions marked with KERNEL_ASYNC can be called within an IRQ.

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

In the ISA bus, each card is sent the pin signals and, hopefully, only one responds by transmitting data. The difference in address decode bits is problematic because a card with 10-bit IO decode will only get 10 bits, and the upper ones mean nothing. E.g. if the base is 2F8, the address AF8 would access the exact same address. This can become a source of conflict. To solve this, OS/90 limits the PnP allocatable port space to 1024 bytes.

This applies to memory as well. Because of the 24-bit addressing, accessing address zero would be equivalent to accessing 0x01000000 or 0xFF000000. That explains why computers with the ISA bus normally do not have more than 16 MB of RAM, since a chunk of the memory will have to be reserved for ISA devices.

The ISA PnP bus has more specifics that require specific design choices in the PnP manager. Devices can have certain memory addresses and ports which they can optionally be configured to use by software. For example, a parallel port card can be configured to the three standard ports and nothing else for compatibility.

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

### API

```
STATUS APICALL InInsertBusDispatchVector(
   PDRIVER_HEADER bus,
    PDRIVER_HEADER client,
    VINT vi,
    FP_IRQ_HANDLR handler
)
```

The point of `InInsertBusDispatchVector` is to obtain an interrupt vector that can be allocated to device drivers by a bus driver. This is will claim the interrupt as BUS_INUSE and set the owner to `client`, if it is given permission.

```
InAcquireLegacyIRQ();
```

This function will claim a BUS_FREE or RECL_16 as BUS_INUSE and set the owner to kernel. Plug-and-play drivers will know not to use this vector. This can be used to replace 16-bit interrupts to allow for a seamless transition between environments.

The driver should be passed a parameter to select the IRQ.

```
STATUS ScanFreeIRQ(PBUS_DRIVER, VINT vi);
```
Check if the virtual interrupt is free.

## Device Abstraction

Some devices are embedded to the system board, while others are attached to a bus (PCI, PCMCIA, ISA PnP). Most buses have their own DMA subsystems, while ISA PnP uses the standard AT DMA controller.

# Virtual Devices and Fake Interrupts

All programs are DOS virtual machines and can access the DOS interfaces. They can also access ports and memory-mapped IO, as well as set interrupt vectors that are local in scope. This is entirely emulated. Memory-mapped IO can only use direct buffers, so emulating devices that require getting each read/write is not supported.

See dosvm.md for the full specification.

## Methods of Emulating Device IO

The kernel never allows ports to be read or written to directly. All IO port instructions are decoded and executed in protected mode by the kernel on the behalf of a driver.

The first method of arbitrating IO would be first come first serve, and block any process that is trying to access the device besides the current user. This is easy to implement. A boolean lock and a PID variable are all that is needed. This type of emulation is not particularly fast despite the simplicity because port IO still has to be decoded and executed by the kernel and only after it has gone through the device chain.

The second method is to fully emulate the hardware interface. Separate emulation contexts must be created for each process that is accessing it. A queue can be used to store requests and send it to real hardware.

# Programming

There is a range of functions provided by the kernel that drivers can use. These are exposed through the kernel symbol tree, a list of absolute addresses with 28-character names and 32-bit addresses, making each entry 32 bytes in size. The symbol table is static and does not change.

The driver model permits dynamic loading and unloading, which could be implemented in the future. A driver should be prepared to handle an unload event.

## Kernel Fibers

OS/90 does not support kernel threads. It instead uses cooperatively scheduled fibers. The implementation details and API for fibers is specified in Scheduler.md. Here we will discuss the use cases for fibers.

### Terminal Output

Some DOS calls will block the system by default. For example, INT 21H AH=9. This is unacceptable for a multitasking operating system, especially when we have multiple windows with DOS programs open.

Reasons why this is bad:
* If we run DIR on both windows quickly, we have to wait for one to finish before the other one completes
* The system cannot do ANYTHING while we are running DIR besides interrupt requests.
* We do not know the X and Y of the terminal before we started OS/90, so even virtual framebuffers will have distorted outputs, so we have to trap terminal output anyway

This means we have to emulate terminal output. It involves nothing more than more than memory copying and writing to a virtual framebuffer.

If kernel threads were possible, we could create a separate thread for each program requesting the IO.

## ISA DMA

To use ISA DMA, memory has to be allocated under 16MB. There is no support for doing this with extended memory, so a conventional memory block has to be allocated that is up to 64K in size.

## Trap Capturing

Summary of defines:
```
CAPT_NOHND = 0 // Driver does not know how to handle this function
CAPT_HND   = 1 // Driver handled the function successfully.
// Eg: CAPT_HND
```

Capturing traps involves the creation of a chain of handlers. If one cannot handle the 16-bit trap (e.g. not the function code it wants) it is passed down to a driver that may handle it. A structure in the kernel already exists for all 256 real mode interrupt vectors. This structure is passed to the handler. If the handler cannot handle the requested, it must set the return status to CAPT_NOHND and yield to the kernel. The kernel will then call the next one upon realizing that the handler cannot perform the task.

If the kernel reaches the end of the chain with no handler returning successfully, it will call the real mode version.

The overhead with this feature would be similar to that of a 16-bit DOS function dispatcher, but with the added weight of the CALL instructions and entering through V86 mode to service the interrupt. Basically, this will a bit slower that real mode DOS, but it depends on the number of links. Range checking is the correct way to implement function dispatching.

To add a new capture, a function returning dword and taking a trap frame with external linkage must be written. A TrapCaptureLink variable should point to that variable, other fields are reserved and need not be set Chaining something that already has a handler cannot be prevented, but it is possible to check if it has been handled before.

Trap handlers are only called when DOS calls the INT instruction. IRQs are handled separately, although there is no reason to call an ISR with INT.

Note that the chain works differently than it would in DOS. Hooking an interrupt will not cause the new handler to run first, but last, thus hooking a vector cannot override another hook. Make sure to check the signature carefully to avoid problems.

## Callback Hooks

Some DOS APIs use far pointer callbacks rather than interrupts in order to achieve interoperability with high-level languages. These can be hooked as with interrupts. The implementation is described in dosvm.md.

```c
AllocateGlobalFarProcHook()
```

This function will return the seg:off value.

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

## Sending PnP Events

Events can be sent from kernel-to-kernel or they can be sent from a userspace process to the kernel.

## Handling PnP Events

PnP events are sent using a driver event packet. A major code is followed by a minor. Other parameters to the the event are stored as five double words.

PnP events are defined in the following format in documentation:
```
Major | Minor

<arg1> ...
<arg2> ...
<arg3> ...
<arg4> ...

OUT:
    ...
```

The exit value of an event is stored in the request header and can be anything not already defined by OS/90.

### Interrupt Hooks

Faking interrupts does not require a driver to own the real vector, as there is isolation between real and virtual devices. The kernel has a chain of virtual devices which can have their own interrupt vectors.

The local IVT will always be modified, but the driver must be informed so that it can create an emulation context and fake IRQs for the process.

Real mode IRQ hook:
```
BUS_PROC_IHOOK | BUS_PROC_IHOOK_RM

<CS:IP> Pointer to the new ISR in seg:off form.
<PID>   Process ID of the requesting process
<V>     The actual vector
<Bus>   Pointer to bus header of owner
OUT:
    EV_OK, if handled
```

For protected mode, CS and EIP are expanded into two values in the same location and RM is replaced with PM for the event code. This event must return EV_OK, even if the process made an invalid access to the IRQ.

```c
static VOID KERNEL EventHandler(PDRIVER_EVENT_PACKET evp)
{
    PDRIVER_EVENT_PACKET_IHOOK evp_ih = evp;

    // We will be rude to the process
    if (evp_ih->major == BUS_PROC_IHOOK)
    {
        ScProcCtl(SCH_TERMINATE_CUR, &NULL);
    }
}
```

# Initialization Order

Drivers must be loaded in a specific order in many cases. Drivers for non-PnP devices or PnP drivers with such features disabled must be loaded first to ensure that fixed resources are captured on startup and do not interfere with plug and play devices.
