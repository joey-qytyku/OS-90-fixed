This section will describe the interface of all drivers, hence the title. This applies to subsystems and device drivers.

# Naming Conventions

OS/90 drivers use the file extension `.RSX`, which means "resident system extension."

# Call Mechanism

# Events

Drivers use a global event system. Any driver can post an event and all drivers must respond. This is done IN ORDER unless otherwise specified.

For example, GE_LOAD. This is called when a new driver is loaded. All other drivers are informed when a new driver is loaded, or at least the kernel tries to inform them. This can allow drivers to automatically detect incompatibilities with others if this is done on both ends, or to automatically unload if a dependency is removed.

Drivers are identified by name in all interfaces exposed to them that permit communication with or knowledge of other drivers. The name MUST be 8 characters long to conform with FAT case, and may later be extended to support the tilde notation.

Most of the events are not required for ordinary system operation and are there to allow for drivers to behave more optimally using certain features too complex to develop an entire API for. For example, we can handle APM or PnP events related to laptop lids or low-power states. Some are purely advisory and have no specific condition to be sent.

```
// This is more up-to-date right now.
enum {
	GE_LOAD = 0,
	GE_UNLOAD,
	GE_DRV_ENABLE,
	GE_DRV_DISABLE,

	// A DOS VM instance wants to change the vector of an IRQ.
	// A virtual device driver should respond to this if interrupts must be simulated.
	//
	// Interrupts cannot go directly.
	//
	GE_SOFTWARE_WANTS_IRQ,

	//
	// A DOS VM has terminated and it happened to control the IRQ lines
	// specified by the mask.
	//
	GE_IRQ_HOOKER_DONE,

	// Rest are optional

	GE_ENTER_SYSTEM_IDLE = 0x7000,
	GE_SYSTEM_POWEROFF,
	GE_FLUSH_BUFFERS,
	GE_LAPTOP_LID_CLOSE,
	GE_LAPTOP_LID_OPEN,
	GE_DOCK,
	GE_UNDOCK,
	GE_MEMORY_LOW,
	GE_SUBSYS_WANTS_DEV,
	GE_SUBSYS_RELEASE_DEV
};
```

## Required Events

GE_LOAD is called when a driver is loaded and must initialize. Dynamically loaded drivers will also cause this event to be sent. GE_LOAD can be masked if the driver does not care about further noticies, but only after it is set up (otherwise it would make no sense)

> Remember, ALL drivers get the GE_LOAD event and for every driver loaded. For example, PCI.RSX will get G_LOAD when ATA.RSX or FAT32.RSX load after it.

GE_UNLOAD is sent when a driver is removed from the memory. It can be refused by returning an error code of 1, but it is highly recommended that drivers do everything they can to permit dynamic removal.

## Event Masking

## Show me the Code!

The entry point specified in the executable is used as the event signal function. It uses a STAT error code and takes a function code and a pointer to an EVENT_BLOCK.

```c
#define GE_RSLT_OK  0
#define GE_RSLT_ERR 1
#define GE_RSLT_REFUSED 2
#define GE_RSLT_CANCELLED 3

typedef struct {
	SHORT   code;
	SHORT   detail;
	union { PVOID parg; LONG uiarg; };
	LONG    result;
}EVENT_BLOCK,*PEVENT_BLOCK;
```

Cancellation prevents any other driver from receiving this event. This only works for non-required signals. Attempting to cancel a required event code will do nothing.
