///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <Scheduler/V86M.h>

#include <Platform/IO.h>

#include <Debug/Debug.h>
#include <Type.h>

#include <Misc/String.h>

#include <stdarg.h>

//
// The following functions are probably slow, but there does not
// seem to be a perfect way of doing it, besides this one I found?
// https://www.quora.com/What-are-the-most-obscure-useless-x86-assembly-instructions?
//

// VOID KERNEL Hex32ToString(U32 value,  PU8 obuffer)
// {
// }

VOID _putchar(char ch)
{
    outb(0xE9, ch);
}


// This generates the blue screen. NOT A DEBUGGING FUNCTION.
// Only for error handling that production builds will have.
_Noreturn kernel VOID FatalError(U32 error_code)
{
    while (1);
}
