/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef DRIVERS_H
#define DRIVERS_H

enum {
        GD_INIT,
        GD_UNLOAD,
        GD_ENTER_SYSTEM_IDLE,
        GD_SYSTEM_POWEROFF,
        GD_FLUSH_BUFFERS,
        GD_LAPTOP_LID_CLOSE,
        GD_LAPTOP_LID_OPEN,
        GD_EXIT_EARLY_BOOT,
        GD_DRV_ENABLE,
        GD_DRV_DISABLE,
        GD_DOCK,
        GD_UNDOCK
};

typedef struct VOID (DRIVER_ENTRY*)(PSTR cmdline);
typedef struct VOID (GENERAL_DISPATCH*)(LONG code, LONG arg);

typedef struct {
};

#endif /* DRIVERS_H */
