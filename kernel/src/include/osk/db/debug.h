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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include "printf.h"

#define _str(x) #x
#define _str2(x) _str(x)
#define LINE _str2(__LINE__)

#define BREAKPOINT() __asm__ volatile("xchgw %%bx,%%bx":::"memory");\

#ifdef NDEBUG

#define assert(exp)
#define debug_log(fmt, ...)

#else

// Memory fenced for accurate assertion in such events.

#define assert(exp)\
if (!(exp)) {\
  FENCE(\
  printf("ASSERT " #exp " FAILED AT %s" __FILE__ ":"  LINE "\n\r");\
  while(1);\
  );\
}

#define debug_log(fmt, ...)\
FENCE();\
  printf(ANSI_COLOR_YELLOW "%s:%i\t"\
  fmt ANSI_COLOR_RESET, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__);

#endif /* NDBUG */

#define _not_null(exp) assert((exp) != NULL)

#endif /* DEBUG_H */