///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <Scheduler/V86.h>

#include <Platform/IO.h>

#include <Debug/Debug.h>
#include <Type.h>

#include <Misc/String.h>

#include <stdarg.h>

VOID _putchar(char ch)
{
    outb(0xE9, ch);
}

// This generates the blue screen. NOT A DEBUGGING FUNCTION.
// Only for error handling that production builds will have.
_Noreturn kernel VOID Fatal_Error(U32 error_code)
{
    while (1);
}
