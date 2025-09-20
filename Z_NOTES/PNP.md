# OS/90 Plug-and-play Protocol

All drivers in OS/90 that are plug-and-play must follow the standard protocol for event handling, resource allocation, bus subordination, dynamic insertion and removal, among other features.

Drivers use the General Event system to notify each other of runtime PnP events and inform each other about potential resource conflicts and whether the driver is operating in PnP mode or not.

## Definition of PnP

OS/90 uses the following definition of plug-and-play and allows for all these capabilities.

- PnP devices can be
	- Recognized and detected at boot time
		- Unique IDs using the EISA format or something bus-specific
		- Report all resources that they use for immediate reservation

	- Inserted and removed at run time if supported by the driver stack
		- This includes:
			- physical insertion/removal of the device
			- the same thing but only the driver

	- Disabled, enabled, or both to restart, if supported by the device

	- When disabled, a device's resources may be considered free to use by all other drivers

	- Allow resources reported as configurable to be reassigned during the init process of a bus driver or while handling a later driver init
		- which includes:
			ISA DMA
			Memory
			Ports
			IRQs
		- Alignment, address decode, and data width requirements/refinements may also be imposed

	- Devices can be on the mainboard and are detectable with the PnP BIOS.

	- Optionally prefer certain resource ranges if the bus supports such hints

- Subordinate bus and devices may sometimes require static resources. Consider the VGA memory and port space, which PCI devices may mark as non-configurable.

A resource is defined like this:
- An IO port:
	- Decode type (10-bit, 12-bit, or 16-bit supported)
	- 8-bit, 16-bit, 32-bit, or unknown data decode
		- The ISA bus can only handle 16-bit data, so that would make it unsafe to use 32-bit operations like ind or outd

- Memory region
	- Aliasing is NOT considered, as most PCs do not drive the ISA bus for high addresses other buses are 32-bit.
	- Data width can be 8-bit, 16-bit, or 32-bit
	- Cache is enabled if the region permits it or the driver does it


In practice, you don't have to memorize all this stuff.
To put it simply:
- Detect+allocate, disable+deallocate
- Hot-swapping if allowed
- Resource requirements are fully reported even if non-configurable
- Standard PC devices are reported

## Bus drivers

The OS/90 strategy aims to have support for plug-and-play ISA and PCI support. Other buses are not on the roadmap, but those that follow the same principles should work, such as MCA or EISA.

A bus in OS/90 is an interconnect that contains enumerable PnP devices. Previously, an orthogonal bus driver model with considered, but instead, bus drivers get the freedom to decide how to handle their devices and how many PnP requirements or features they support.

The following guarantees are offered to make bus support easier:
- Bus drivers cannot be unloaded or disabled.
- Bus drivers load only at startup, not dynamically
- Bus drivers which use IRQ lines and are able to share them will load FIRST regardless of configuration

Buses must first enumerate all their devices and incrementally allocate all resources through the PnP resource manager. In the event of a conflict, the device that was just detected must be PnP-disabled.

IRQs are allocated similarly, however, any bus capable of sharing IRQs (such as PCI) must be loaded first so that it may be required to reconfigure sharing. Device drivers should not see this.

> PCI does not actually have a IRQ line register at all. It is a misconception and OSDev is WRONG. It is only used for information purposes.

## Resource Allocation API

> Updated September 6

### Allocate ports to a driver

```
int PnP_AllocPorts(	DH*		owner,
			const char*	info_str,
			shrt		base,
			ushrt		count,
			int		addr_decode,
			int		data_decode)
```


`owner` is the device driver (supposed to be bus) that owns it.

`info_str` is a human-reading description of which device it is. Recommended to be no longer than 32 characters.

`base` is positive zero for any base address, negative for any base address but with `abs(base)` equal to the desired alignment, or anything between 0x0000 and 0xFFFF for an exact position.

`count` is the number of bytes. The desired alignment is also used as the granularity. If the range is out of bounds, function fails.

`addr_decode` is literally just 10, 12, or 16. This is the number of bits decoded.

`data_decode` is 0 for unknown and {8,16,32} to represent the largest data that can be accessed in parallel. Likely redundant.

Returns a handle to the allocation. Negative is invalid and has the following meanings:
- -1: Forced resource conflict
- -

### Free IO ports

```
void PnP_FreePorts(int port_alloc_handle)
```

### Query port owner

### Allocate or reserve memory mapped IO range (TODO)

```
void PnP_AllocMMIO();
```

If this fails because RAM is at that location, it is necessary to use `M_PokeHole` to safely reserve the RAM without discarding contents. Such things should be reported to the user. Failing this, the device or bus being configured cannot work.


