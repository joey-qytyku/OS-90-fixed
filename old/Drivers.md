# Driver structure

Drivers are PE-COFF object files with ending with .DRV. They are linked to the kernel address space and use the same page directory as the kernel.

## Driver Parameter Block

```c
struct {
        dword flags;
        int (entry*)(int);
}DPB,*PDPB;
```

Flags: [DMA|INT|BUS]

BUS: Bus driver
INT: can arb. interrupts for kernel
DMA: can arb. ISA DMA channels

# Kernel API

KAPI is the application programming interface provided by the kernel to ring 0 programs called __drivers__. All functions use __cdecl__. That means the stack is cleared by the CALLER, in this case, the kernel.

Critical sections are not required for most kernel functions unless specified, even if they modify interrupts and other volatile tasks because it is implied.

## Int86(pvoid mem, page mem4k, GeneralRegdump rd)

Call a 16-bit interrupt. A pointer to the memory is required. If mem == NULL the real mode memory is used and page mappings are not changed, as this is the default for the kernel.

## int PnP(word func, word node ...)

Call protected mode PnP call. This function copies values to a 16-bit stack segment and calls PnP BIOS functions

## AllocatePages(dword size, ?)

Allocate page frames and map to a 4096 byte aligned location.

# Bus Drivers

The main idea as that the bdev arbitrates system resources with the supervision of the kernel and dispatches hardware events. The kernel assists with this process.

Resources should be reassigned by the bus driver if possible.

bdevs can use other bdevs, such as USB using PCI.

Bus drivers generate a list of attached devices which are identfied by a universal 128-bit number. It is basically whatever ID the bus provided that specifies the exact type of device and vendor, the latter can be ignored.

For PCI, this includes the vendor ID, device ID, class code, and PROG IF. All these are concatenated into a 64-bit number internally.

## Interrupts

This is further elaborated in other sections. When an interrupt happens, only one ISR is called in the kernel. The master handler then calls the associated high-level handler of an interrupt. This handler can be owned by a bus driver. If that is the case, the driver will then call the device driver.

Not all busses support sharing IRQs so the extra arbitration step may not be necessary.

When OS/90 boots, it scans the interrupt vector table IRQ sections for any changes.

## Query Devices

## Bus_RequestLines(dword handle, byte lines)

When  line is taken, dispatching must be handled by the driver so that a proper ISR is called.

When an IRQ is sent, the kernel will call the bus driver. The bdev then calls a device driver handler.

The kernel only has one ISR for all IRQ vectors. It reads the in service register and calls the owner of the interrupt, which can be both types of drivers.

The RequestFixedLines function is for drivers that use a standard interrupt line. The implementation is almost exactly the same.

If the bus wants to, it can have different handlers for different interrupts by requesting several times.

## Bus_ReportDeviceList(PDevice, word length)

Stores the address of the device list so that the kernel can access it later.

## Busdev_RequestDevice(, )

This requests that the bus driver install an ISR for when the device needs attention. The details are hidden from the busdev. It does not have to be a real interrupt and it can be shared.

The procedure that is installed is called by the bus driver.

DEV_ID_NOT_FOUND

## Busdev_RequestSolitaryIRQ(dword handle);

Some drivers may want a solitary IRQ (no share). Sound cards, for example, frequently broadcast interrupts and sharing becomes inefficient.

Device drivers must support all three cases.

returns:
DEV_YES
DEV_NO
DEV_NOT_SUPPORTED

# Advice

Drivers should make sure that the device is able to run under any condition because they cannot truly terminate and will end up wasting memory. Drivers can give up initialization but this should be used sparingly. For example, a sound drv should not give up because it did not get a solitary IRQ.

# Examples
## PCI.DRV

This driver is preincluded.

## VMM.DRV

This is the default virtual machine manager. It uses a 32-bit interrupt vector 3Ch to configure virtual machines from userspace.

DOS VMs can share STDIO with the 32-bit OS but only through the VMM.
