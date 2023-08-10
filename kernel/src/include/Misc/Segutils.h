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
    SEG_GET_ACCESS_RIGHTS,
    SEG_GET_EXT_ACCESS_RIGHTS,
    SEG_GET_BASE_ADDR,
    SEG_GET_LIMIT,

    SEG_SET_ACCESS_RIGHTS,
    SEG_SET_BASE_ADDR,
    SEG_SET_LIMIT
};

static inline PVOID MK_LP(U32 seg, U32 off)
{
    return (PVOID)(seg * 16 + off);
}

PVOID KERNEL SegmentToLinearAddress(
    BOOL    use_pmode,
    PVOID   relative_to,
    U16     seg,
    U32     off
);

U32 SegmentUtil(
    U8  func,
    U16 seg,
    U32 operand
);

#endif /*SEGUTILS_H*/
