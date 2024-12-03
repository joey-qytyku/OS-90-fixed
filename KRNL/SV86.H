/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef SV86_H
#define SV86_H

#include "sv86.h"

struct __ivt { SHORT ip:16; SHORT cs:16; };

static struct __ivt * const IVT = (struct __ivt * const)0;

typedef BOOL (*HV86)(PREGS);

LONG V86xH(BYTE v, PREGS r);

// Returns the vector of next INT
// Returns what if IRET?
LONG EnterV86(PREGS r);

VOID InitV86(VOID);

#endif /* SV86_H */
