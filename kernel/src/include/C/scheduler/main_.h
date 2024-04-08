/*******************************************************************************
                        Copyright (C) 2023, Joey Qytyku

This file is part of OS/90 and is published under the GNU General Public
License version 2. A copy of this license should be included with the
source code and can be found at <https://www.gnu.org/licenses/>.
*******************************************************************************/


#pragma once

#include "StdRegs.h"

//
// These are fields in the TSS which have special meaning. The TSS is required
// when entering any kind of context to establish which SS:ESP to use when
// going back to ring 3.
//
//
enum {
  TSS_ESP0 = 1,
  TSS_SS0  = 2,
  TSS_SV86_EBP = 3,
  TSS_SV86_EIP = 4
};
extern int TSS[26];
