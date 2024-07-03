///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

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

//
// ALWAYS use this instead of printf. This will memory fence and block
// interrupts, ensuring no garbled output.
//
#define kdebug_log(fmt, ...)                                                                    \
{                                                                                               \
        ASM("pushf; cli":::"memory");                                                           \
        printf(COLOR_STR( "%s:%i\t" fmt),  __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__);       \
        ASM("popf":::"memory");                                                                 \
}                                                                                               \

#define not_null(exp) { assert((exp) != NULL); }

#ifdef NDEBUG
#undef kassert
#undef kdebug_log
#endif /* NDEBUG */

#endif /* DEBUG_H */