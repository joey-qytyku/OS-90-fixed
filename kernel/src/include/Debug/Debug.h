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

typedef VOID (*OUTPUT_DRIVER)(U8);

API_DECL(VOID, Logf,        const char*, ...);
API_DECL(VOID, FatalError,  U32);

API_DECL(VOID, WriteAsciiz, const char*);
API_DECL(VOID, Putchar, char ch);

#define _str(x) #x
#define _str2(x) _str(x)
#define LINE _str2(__LINE__)

#ifdef NDEBUG
    #define _assert(exp)
#else
    #define _assert(exp)\
        if (!(exp)) {\
            WriteAsciiz("Assert " #exp " failed in module " __FILE__  " at line "  LINE "\n\r");\
            FatalError(1);\
        }

#endif /* NDBUG */

#define _not_null(exp) _assert((exp) != NULL)

#endif /* DEBUG_H */
