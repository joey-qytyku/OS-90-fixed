/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef IO_H
#define IO_H

#include <Type.h>

static inline void rep_insb(void* mem, int count, short port)
{
    __asm__ volatile(
        "cld" ASNL
        "rep insb"
        : "=D"(mem), "=c"(count)
        : "D"(mem), "c"(count), "d"(port)
        : "memory"
    );
}

static inline VOID rep_outsb(PVOID mem, U32 count, U16 port)
{
    __asm__ volatile(
        "cld" ASNL
        "rep outsb"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}
}

static inline VOID rep_insw(PVOID mem, U32 count, U16 port)
{
    __asm__ volatile(
        "cld" ASNL
        "rep insw"
        :
        : "D"(mem), "c"(count), "d"(port)
        : "edi", "ecx", "edx", "memory"
    );
}

static inline VOID rep_outsw(PVOID mem, U32 count, U16 port)
{
    __asm__ volatile(
        "cld" ASNL
        "rep outsw;"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}

#define _MAKE_PORT_IN(_ASM_TYPE_PREFIX, _TYPE)\
static inline _TYPE in##_ASM_TYPE_PREFIX(U16 port)\
{\
    _TYPE ret;\
    __asm__ volatile ("in" #_ASM_TYPE_PREFIX " %1, %0" :"=a"(ret) :"Nd"(port):"memory");\
    return ret;\
}


#define _MAKE_PORT_OUT(_ASM_TYPE_PREFIX, _TYPE)\
static inline VOID out##_ASM_TYPE_PREFIX (U16 port, _TYPE val)\
{\
    __asm__ volatile ("out" #_ASM_TYPE_PREFIX " %0, %1": :"a"(val), "Nd"(port):"memory");\
}

#define MAKE_MMIO_READ_FUNC(_TYPE)\
static inline _TYPE ReadIOMem ##_TYPE (PVOID addr)\
{\
    FENCE;\
    volatile P##_TYPE r = addr;\
    return *r;\
}

_MAKE_PORT_OUT(b, U8);
_MAKE_PORT_OUT(w, U16);
_MAKE_PORT_OUT(l, U32);

_MAKE_PORT_IN(b, U8);
_MAKE_PORT_IN(w, U16);
_MAKE_PORT_IN(l, U32);

static inline VOID delay_outb(U16 port, U8 val)
{
    outb(port, val);
    outb(0x80, 0); // Output to unused port for delay
}

static inline U8 delay_inb(U16 port)
{
    outb(0x80, 0);
    return inb(port);
}

#undef _MAKE_PORT_OUT
#undef _MAKE_PORT_IN


#endif /* IO_H */
