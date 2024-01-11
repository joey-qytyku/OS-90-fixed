///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef PNP_CORE_H
#define PNP_CORE_H

#define PDRIVER_HEADER PVOID
#define PDRIVER_EVENT_PACKET PVOID

#include "Resource.h"
#include "Bios.h"

#include <Type.h>

// Non-PnP drivers must be loaded first

extern VOID InitPnP(VOID);

#endif /* PNP_CORE_H */
