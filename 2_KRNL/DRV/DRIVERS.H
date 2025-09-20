/*******************************************************************************
			Copyright (C) 2022-2024, Joey Qytyku
			Copyright (C) 2025, Joey Qytyku

This file is part of OS/90.

OS/90 is free software. You may distribute and/or modify it under
the terms of the GNU General Public License as published by the
Free Software Foundation, either version two of the license or a later
version if you chose.

A copy of this license should be included with OS/90.
If not, it can be found at <https://www.gnu.org/licenses/>

*******************************************************************************/

#ifndef DRIVERS_H
#define DRIVERS_H

/* ** Events **

Drivers send General Events to communicate various situtations that can occur
and need the attention of all drivers.

This is used for a few important features:
- Basic dependency loading/unloading
- Incompatibility detection
- Plug-and-play configuration
- Basic power management events and docking
- Single-tasking a program
- Performance advice

Events occur one-by one. Event handlers cannot cause other events directly but
can tell the kernel to initiate one after handling the first by chaining.

Events are sent to each driver in a linked-list manner with an undefined order.

General events allow OS/90 to scale and support new features with minimal
structural changes. Many are advisory or optional, but allow newer versions to
add improvements.

*/

typedef struct {
	// Name in all caps and with dot. Follows rules of FAT short names.
	// Terminated by a zero if less that 12 characters.
	char	name[12];
	// Human-readable description of the driver as a C string.
	// Must be ascii encoded and no more than 78 characters long.
	char*	desc;

}DH;

#define GRP_REQIRED(x, y) 0x0000

// 4-bit MSBits of a 16-bit event code define

// Why not make these function pointers.
#define REQ 0x0000
#define PWR 0x1000
#define ADV 0x2000
#define IOD 0x3000
#define PNP 0x4000


typedef enum {
	ASE_FN_AX,
	ASE_FN_BX,
	ASE_FN_CX,
	ASE_FN_DX,
	ASE_FN_WHOLE_INTF
}ASE_FNCODE_REG;

// Use something like Ge_drv_enable
// This will be used a lot.


// Can these be defined as functions for sending?
/*
DRV_HANDLE dh = DRV_HANDLE_NULL;
ASE_FNCODE_REG
GeIsServiceHooked(&dh, );
*/

// Why this?
// Mainly to separate the enums
// typedef union {
// 	enum _ge_drv drv;
// 	enum _ge_pnp pnp;
// }GE;

enum {
	GE_INIT = 0,
	GE_UNLOAD,

	// Example of how it works:
	// Removal of a dependent driver can lead to self-removal.
	//
	GE_DRV_ENABLE,
	GE_DRV_DISABLE,

	// Ask drivers if they implement extensions to an INT vector and an
	// associated register parameter. Only one is supported, but the register
	// can be anything from AX-DX.
	GE_INST_CHCK_ASE,

	// Same parameters as ASE
	GE_IS_VECTOR_HOOKED,

	// Rest are optional
	GE_EXIT_EARLY_BOOT,

	GE_PWR_ENTER_SYS_IDLE = 0x8000,
	GE_PWR_SHUTDOWN,

	GE_LAPTOP_LID_CLOSE,
	GE_LAPTOP_LID_OPEN,

	// NEW: Can a specific video mode be losslessly represented within the
	// current one? Video drivers respond only.
	GE_VGA_MODE_EMBEDDABLE,

	GE_PNP_DOCK,
	GE_PNP_UNDOCK,


	GE_PNP_ASSIGN_DONE,

	// Bus drivers are supposed to be loaded first. They may encounter a device
	// which has static resources and cannot necessarily be blocked out during
	// early startup because they are subordinate devices.
	GE_PNP_REPORT_STATIC_RSC,

	GE_PNP_RQ_DELETE_ALL_CFGINFO =,

	// The meaning of this event is flush all buffers to any disks, schedule
	// no further IO to any block device, and wait until further notice.
	// Argument is a bitmap of each drive letter.
	GE_OSK_FLUSH_BUFFERS_AND_INDEF_IO_LOCK,

	//
	GE_OSK_RELEASE_IO_LOCK,

	GE_OSK_MEMORY_LOW,
	GE_OSK_SWAP_LOW,

	GE_OSK_ENTER_EXCLUSIVE,
	GE_OSK_FORCE_TERMINATE,

	// Driver wants an input command string by keyboard.
	GE_USER_INTERV_RQ
};

// No error, continue normally to next driver
#define GESTAT_OK

// Error occured, the error may be recoverable or terminates the chain
// depending on the event.
#define GESTAT_ERR
#define GESTAT_

typedef struct void (DRIVER_ENTRY*)(PSTR cmdline);
typedef struct int (GENERAL_DISPATCH*)(LONG code, LONG arg);

#endif /* DRIVERS_H */
