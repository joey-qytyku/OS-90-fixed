# Virtual Device Protocol

The virtual device protocol is a specification for OS/90 that allows multiple subsystems to access an emulated or arbitrated device through a similar virtual interface. This can be done without the driver knowing anything about installed subsystems.

VDP is capable of emulating port IO and access to RAM-like memory regions in a way that is safe for the OS/90 shared address space by using scheduler hooks internally.

The DOS subsystem uses the VDP to implement the mode 3 text display. Additional video drivers could also allow DOS programs to even use graphical modes simultaneously.

> If no driver can emulate direct port or memory accesses, requests may be carried out directly.

# Device List

Devices have virtual IO and memory ranges specified in an array that must always be present in locked memory. Devices are given 8-character names (other characters are ignored), and this name does not have to be the same as the driver creating it.

VDs are global to the entire OS.

# Device Subscription

Virtual devices can be added to a process into a limited set of slots. Adding can always fail.

# MMIO

OS/90 cannot currently emulate real MMIO with each operation being emulated. It can only simulate RAM-like regions like framebuffers.

VDP allows for deciding if the memory is cache-enabled or cache-disabled. The memory does not have to be present and can be swapped out or uncommitted.

# IO Handler Routine

Devices can only operate with the oversight of the subsystem driver. If the subsystem catches an IO instruction, it decides if it wants to send it to VDP.

HDEVIO_t2 is the routine that handles all IO port access and other events.

HDEVIO_t2 takes a pointer to this block:

```
typedef struct {
        SHORT   function.

        SHORT   port;
        BYTE    str_direction;
        BYTE    in_or_out;
        LONG    iterations;
        LONG    datum;
        PVOID   which_range;
}DEVIO_REQ_PACKET;
```

`datum` represents the data that is being sent or the data that has been outputted.

`function` is DEVIO_FUNC_IOPORT for IO access and DEVIO_FUNC_

# API Listing


```
int VDP_RegisterDevice_t12(const char *drv, const char *name, HDEVIO_t2 hio, SHORT *pio, LONG *mmio);
int VDP_DeleteDevice_t12(const char *drv, const char *name, const char *devname);

int VDP_CheckIO_t2(PSTDREGS);

VDP_DispatchIO_t2(PSTDREGS);

int VDP_Attach_t12(LONG pid, const char *devname);
int VDP_Detach_t12(LONG pid, const char *devname);
```

The format for the MMIO list is the base address followed by [15..0] as the size in pages. The rest of the bits are reserved.

# Example

```
const char vdname[] = "V8042";

static SHORT pio[] = {
        2
        0x60,1,
        0x64,1
};

VOID HandleIO_t2()
{}

void Example()
{
        VDP_RegisterDevice_t12(g_devname, "V8042", HandleIO, io, NULL);
}
```
