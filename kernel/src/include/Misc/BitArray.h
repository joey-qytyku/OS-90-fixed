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

API_DECL(BOOL,   KeGetBitArrayEntry, PU32, U32);
API_DECL(STATUS, KeAllocateBits, PU32, U32, U32, PU32);
API_DECL(STATUS, AllocateOneBit, PU32, U32, PU32);
API_DECL(VOID,   KeDisableBitArrayEntry, PU32, PU32);

// These are NOT kernel exports.
VOID KeEnableBitArrayEntry(PU32,U32);
VOID KeEnableBitArrayRange(PU32,U32,U32);

#endif