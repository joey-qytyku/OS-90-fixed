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

API_DECL(VOID, Logf,        OUTPUT_DRIVER, const char*, ...);
API_DECL(VOID, FatalError,  U32);

VOID KeWriteAsciiz(OUTPUT_DRIVER, const char*);
VOID _KernelPutchar(char ch);

#define _str(x) #x
#define _str2(x) _str(x)

#define TRACE(...)\
    KeWriteAsciiz(_KernelPutchar,"\x1b[31m");\
    KeLogf(_KernelPutchar,\
    "[" __FILE__ ":" _str2(__LINE__) "] " \
    __VA_ARGS__\
    );\
    KeWriteAsciiz(_KernelPutchar,"\x1b[0m");

#define BREAK() __asm__ volatile ("xchgw %%bx,%%bx":::"memory")

#define kassert(x) {\
    if (!(x)) {\
        TRACE("ASSERT FAILED");\
    }\
}

#endif
