///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef SEGUTILS_H
#define SEGUTILS_H

#include <IA32/Segment.h>

static inline PVOID MK_LP(U32 seg, U32 off)
{
    return (PVOID)(seg * 16 + off);
}

#endif /*SEGUTILS_H*/
