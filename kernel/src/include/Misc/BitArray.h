///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef BIT_ARRAY_H
#define BIT_ARRAY

#include <Type.h>

VOID   KERNEL   KeDisableBitArrayEntry(PU32,U32);
BOOL   KERNEL   KeGetBitArrayEntry(PU32,U32);
STATUS KERNEL   KeAllocateBits(PU32,U32,U32,PU32);
STATUS KERNEL   AllocateOneBit(PU32,U32,PU32);

// These are NOT kernel exports.
VOID KeEnableBitArrayEntry(PU32,U32);
VOID KeEnableBitArrayRange(PU32,U32,U32);

#endif