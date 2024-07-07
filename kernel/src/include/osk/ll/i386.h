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

#ifndef I386_H
#define I386_H

STACK_PARAMS PVOID i386GetDescriptorAddress(PVOID descptr);
STACK_PARAMS VOID  i386SetDescriptorAddress(PVOID descptr, LONG newaddr);
STACK_PARAMS VOID  i386SetDescriptorLimit(PVOID descptr, LONG lim);

STACK_PARAMS VOID i386AllowRing3IO(VOID);
STACK_PARAMS VOID i386MonitorRing3IO(VOID);

#endif /* I386_H */
