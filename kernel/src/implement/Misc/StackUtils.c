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

//
// Because stack operations do not modify the stack segment register, we
// do not need to provide a pointer to the SS value.
//
// A pointer is used because the stack may be emulated for a process
// control block register dump or for the interrupt stack frame.
//

VOID RM_Push_16(U16 ss, PU32 esp, U16 value)
{
    PU16 stack = MK_LP(ss, *esp);
    *esp -= 2;
    stack[-1] = value;
}

U16 RM_Pop_16(U16 ss, PU32 esp)
{
    PU16 stack = MK_LP(ss, *esp);
    *esp += 2;
    return *stack;
}

VOID PM_Push_16(U16 ss, PU32 esp, U16 value)
{
    PU16 stack = (PU16)GetLdescBaseAddress(ss) + *esp;
    *esp -= 2;
    stack[-1] = value;
}

U16 PM_Pop_16(U16 ss, PU32 esp)
{
    PU16 stack = (PU16)GetLdescBaseAddress(ss) + *esp;
    *esp += 2;
    return *stack;
}

// BRIEF:
//      Pushes values on the stack provided. Can take 1 or more 16-bit values
//
VOID RM_Push_Mult(
    U16         ss,
    PU32        esp,
    U32         num_to_push,
    const PU16  push_array
){
    // PU32 args = GET_VAR_LIST(num_to_push);

    // for (U32 i = 0; i<num_to_push; i++)
    //     RmPush16(ss, esp, args[i]);
}

VOID RM_Pop_Mult(
    U16     ss,
    PU32    esp,
    U32     num_to_pop,
    PU32    buff
){
    for (U32 i = 0; i < num_to_pop; i++) {
        buff[i] = RM_Pop_16(ss, esp);
    }
}

VOID PM_Push_32()
{}
