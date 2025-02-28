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

/*
General events are situations that do not have very defined behavior
unless otherwise stated. Behaviors resulting from the calls are
up to the driver. They just have a certain code reserved for them so that
the OS is not incomplete and can properly handle advanced events without
having to provide a specification.

Any part of the kernel may send such an event. Every driver will recieve
general events.

Userspace can cause some of these events.
*/

enum {
	GE_INIT = 0,
	GE_UNLOAD,
	GE_DRV_ENABLE,
	GE_DRV_DISABLE,

	// Rest are optional

	GE_PWR_ENTER_SYS_IDLE = 0x1000,
	GE_PWR_SHUTDOWN,

	GE_OS_FLUSH_BUFFERS,
	GE_LAPTOP_LID_CLOSE,
	GE_LAPTOP_LID_OPEN,
	GE_EXIT_EARLY_BOOT,

	GE_PNP_DOCK,
	GE_PNP_UNDOCK,

	GE_OSK_MEMORY_LOW,
	GE_OSK_SWAP_LOW,
	GE_OSK_ENTER_EXCLUSIVE,
	GE_OSK_FORCE_TERMINATE,

	// Driver wants an input command string by keyboard.
	GE_USER_INTERV_RQ
};

typedef struct VOID (DRIVER_ENTRY*)(PSTR cmdline);
typedef struct STAT (GENERAL_DISPATCH*)(LONG code, LONG arg);

#endif /* DRIVERS_H */
