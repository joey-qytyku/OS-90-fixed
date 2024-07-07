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

#include <osk/sd/basicatomic.h>
#include <osk/sd/stdregs.h>
#include <osk/sd/sv86.h>
#include <osk/sd/int.h>

#include <osk/db/debug.h>

extern VOID M_Init(VOID);

static STDREGS r;

VOID KernelMain(VOID)
{
        kdebug_log("Hello, world!\n");
}
