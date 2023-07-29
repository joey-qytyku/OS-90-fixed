///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <Misc/StackUtils.h>
#include <Misc/Segutils.h>

VOID RmPush16(PU16 ss, PU32 esp, U16 value)
{
    PU16 stack = MK_LP(*ss, *esp);
    *esp -= 2;
    stack[-1] = value;
}

U16 RmPop16(PU16 ss, PU32 esp)
{
    PU16 stack = MK_LP(*ss, *esp);
    *esp += 2;
    return *stack;
}

VOID PmPush16(PU16 ss, PU32 esp, U16 value)
{
    PU16 stack = GetLdescBaseAddress(ss) + *esp;
    *esp -= 2;
    stack[-1] = value;
}

U16 PmPop16(PU16 ss, PU32 esp)
{
    PU16 stack = GetLdescBaseAddress(ss) + *esp;
    *esp += 2;
    return *stack;
}
