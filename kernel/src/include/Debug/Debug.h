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

#include <Type.h>

#include "printf.h"

typedef kernel VOID (*OUTPUT_DRIVER)(char);

API_DECL(VOID, Logf,        const char*, ...);
API_DECL(VOID, FatalError,  U32);

API_DECL(VOID, WriteAsciiz, const char*);
API_DECL(VOID, Putchar, char ch);

#define _str(x) #x
#define _str2(x) _str(x)
#define LINE _str2(__LINE__)


#define BREAKPOINT() __asm__ volatile("xchgw %%bx,%%bx":::"memory");\

#ifdef NDEBUG
    #define _assert(exp)
    #define KLogf(fmt, ...)
#else
    #define assert(exp)\
        if (!(exp)) {\
            FENCE;\
            printf("ASSERT " #exp " FAILED AT %s" __FILE__ ":"  LINE "\n\r");\
            while(1);\
        }

    #define KLogf(fmt, ...) printf("%s:%i>> " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif /* NDBUG */

#define _not_null(exp) assert((exp) != NULL)

#endif /* DEBUG_H */
