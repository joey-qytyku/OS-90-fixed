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

VOID RmPush16(PWORD ss, PU32 esp, WORD value);
WORD RmPop16(PWORD ss, PU32 esp);
VOID PmPush16(PWORD ss, PU32 esp, WORD value);
WORD PmPop16(PWORD ss, PU32 esp);


#endif /* STACKUTILS_H */