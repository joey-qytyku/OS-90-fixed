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

enum {
SEG_GET_ACCESS_RIGHTS = 0,
SEG_GET_EXT_ACCESS_RIGHTS = 4,
SEG_GET_BASE_ADDR = 8,
SEG_GET_LIMIT = 12,

SEG_SET_ACCESS_RIGHTS = 16,
SEG_SET_BASE_ADDR = 20,
SEG_SET_LIMIT = 24
};

static inline PVOID MK_LP(U32 seg, U32 off)
{
    return (PVOID)(seg * 16 + off);
}

U32 Segment_Util(
    U8  func,
    U16 seg,
    U32 operand
);

#endif /*SEGUTILS_H*/
