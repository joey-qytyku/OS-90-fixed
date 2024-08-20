/*******************************************************************************
                      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/


#ifndef DEBUG_H
#define DEBUG_H


// RED     "\x1b[31m"   GREEN   "\x1b[32m"
// YELLOW  "\x1b[33m"   BLUE    "\x1b[34m"
// MAGENTA "\x1b[35m"   CYAN    "\x1b[36m"
// RESET   "\x1b[0m"

#define COLOR_STR(str) "\x1b[36m" str "\x1b[0m"

#include "printf.h"

// Memory fenced for accurate assertion in certain situatins.

#define kassert(exp)                                                    \
if (!(exp))                                                             \
{                                                                       \
        FENCE()                                                         \
        printf("%s:%i `%s` ASSRT FAILED\n\r",__FILE__,__LINE__,##exp);  \
        ASM("cli; hlt":::"memory");                                     \
        FENCE()                                                         \
}

#define not_null(exp) { assert((exp) != NULL); }

#ifdef NDEBUG
#undef kassert
#undef kdebug_log
#endif /* NDEBUG */

#endif /* DEBUG_H */
