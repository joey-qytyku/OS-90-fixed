///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef STACKUTILS_H
#define STACKUTILS_H

#include <Type.h>

VOID RmPush16(U16 ss, PU32 esp, U16 value);
U16  RmPop16( U16 ss, PU32 esp);
VOID PmPush16(U16 ss, PU32 esp, U16 value);
U16  PmPop16( U16 ss, PU32 esp);

VOID RmPushMult16(
    U16     ss,
    PU32    esp,
    U32     num_to_push,
    PU16    push_array
);

VOID RmPopMult16(
    U16     ss,
    PU32    esp,
    U32     num_to_pop,
    PU32    buff
);
#endif /* STACKUTILS_H */
