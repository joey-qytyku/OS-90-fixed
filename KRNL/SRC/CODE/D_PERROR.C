/*******************************************************************************
		      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

#include <OSK/DB/printf.h>

void _putchar(char c);

char eok[]   = "Operation successful";
char efail[] = "Generic error";

char *blkdev[E_BLKDEV_NUM_ERRS__] =
{
[EOK]                     =     eok,
[EFAIL]                   =     efail,
[E_BLKDEV_XFER_ERROR]     =     "Data transfer error",
[E_BLKDEV_MEDIA_REMOVED]  =     "Media was detached unexpectedly",
[E_BLKDEV_NO_MEDIA]       =     "Drive has no media attached",
[E_BLKDEV_BAD_BLOCK]      =     "The block accessed was damaged",
[E_BLKDEV_INVALID_BLOCK]  =     "Block selected to access is invalid",
[E_BLKDEV_EQUIP_FAIL]     =     "Critical device error",
[E_BLKDEV_OTHER_ERROR]    =     "Anything else"
};

//
// These are intended to be returned by a filesystem driver.
// This means that anything related to permissions, file locking, or any other
// subsystem-specific filesystem semantics are not included.
//
// enum {
// E_FS_NOT_FOUND=,
// E_FS_HANDLE_NOT_OPENED  = "File handle does not exist",
// E_FS_MEDIA_UNFORMATTED  = "Drive is unformatted",
// E_FS_DAMAGED_MBR        = "Data that hold FS infroamtion is broken",
// };

char *memdev[E_MEM_NUM_ERRS__] = {
[EOK]           = eok,
[EFAIL]         = efail,
[E_MEM_OUT]     = "No page frames available",
[E_MEM_NOMAP]   = "No more virtual address space",
[E_MEM_NOSWP]   = "Memory evicted or swapped in cannot be due to no swap"
};

VOID PrintError()
{

}
