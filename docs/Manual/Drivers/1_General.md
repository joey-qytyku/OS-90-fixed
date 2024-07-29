This section will describe the interface of all drivers, hence the title. This applies to subsystems and device drivers.

# Naming Conventions

OS/90 drivers use the file extension `.RZM`, which means ring-0 module.

# Call Mechanism

> Should I reconsider the permanent HMA? Perhaps.

Example code used to describe code in previous section is NOT CORRECT FOR DRIVERS! OS/90 uses a function pointer table with the address defined as a `static const` pointer which is at a fixed address in the HMA.

Hooking should always be logged to diagnose problems.

# Events

Drivers use a global event system. Any driver can post an event and all drivers must respond. This is done IN ORDER unless otherwise specified.

For example, GE_LOAD. This is called when a new driver is loaded. All other drivers are informed when a new driver is loaded, or at least the kernel tries to inform them. This can allow drivers to automatically detect incompatibilities with others if this is done on both ends, or to automatically unload if a dependency is removed.

Drivers are identified by name in all interfaces exposed to them that permit communication with or knowledge of other drivers. The name MUST be 8 characters long to conform with FAT case, and may later be extended to support the tilde notation.

Most of the events are not required for ordinary system operation and are there to allow for drivers to behave more optimally using certain features too complex to develop an entire API for. For example, we can handle APM or PnP events related to laptop lids or low-power states. Some are purely advisory and have no specific condition to be sent.

```
enum {
	GE_LOAD = 0,
	GE_UNLOAD,
	GE_DRV_ENABLE,
	GE_DRV_DISABLE,
	GE_ENTER_KCP,
	GE_EXIT_KCP,

	// Rest are optional

	GE_ENTER_SYSTEM_IDLE = 0x7000,
	GE_SYSTEM_POWEROFF,
	GE_FLUSH_BUFFERS,
	GE_LAPTOP_LID_CLOSE,
	GE_LAPTOP_LID_OPEN,
	GE_DOCK,
	GE_UNDOCK,
	GE_MEMORY_LOW,
	GE_SWAP_LOW,
	GE_SUBSYS_WANTS_DEV,
	GE_SUBSYS_RELEASE_DEV
};
```

## Required Events

GE_LOAD is called when a driver is loaded and must initialize. Dynamically loaded drivers will also cause this event to be sent. GE_LOAD can be masked if the driver does not care about further noticies, but only after it is set up (otherwise it would make no sense)

> Remember, ALL drivers get the GE_LOAD event and for every driver loaded. For example, PCI.90 will get G_LOAD when ATA.90 or FAT32.90 load after it.

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

```c

VOID EntryPoint(PEVENT_BLOCK e)
{
	switch (e->code)
	{
		case GE_LOAD:
			// Load stuff
		break;
		case GE_
	}
}
```

Cancellation prevents any other driver from receiving this event. This only works for non-required signals. Attempting to cancel a required event code will do nothing.

> Can I add filtering to the events? Allow driver to exclude another driver or prevent the event from being further handled? For example, the low memory thing could cause one driver to do enough to free memory that the others should run fine. This would lead to wasted code, however, since only the first one would actually do it. This feature is fine, but should only be conditional and cannot be allowed for certain events.
