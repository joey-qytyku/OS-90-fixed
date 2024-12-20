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

#ifndef IO_H
#define IO_H

static inline void rep_insb(void* mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep insb"
        : "=D"(mem), "=c"(count)
        : "D"(mem), "c"(count), "d"(port)
        : "memory"
    );
}

static inline void rep_outsb(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep outsb"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}

static inline void rep_insw(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep insw"
        :
        : "D"(mem), "c"(count), "d"(port)
        : "edi", "ecx", "edx", "memory"
    );
}

static inline void rep_outsw(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep outsw;"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}

#define _MAKE_PORT_IN(_ASM_TYPE_PREFIX, _TYPE)\
static inline _TYPE in##_ASM_TYPE_PREFIX(SHORT port)\
{\
    _TYPE ret;\
    __asm__ volatile ("in" #_ASM_TYPE_PREFIX " %1, %0" :"=a"(ret) :"Nd"(port):"memory");\
    return ret;\
}


#define _MAKE_PORT_OUT(_ASM_TYPE_PREFIX, _TYPE)\
static inline void out##_ASM_TYPE_PREFIX (SHORT port, _TYPE val)\
{\
    __asm__ volatile ("out" #_ASM_TYPE_PREFIX " %0, %1": :"a"(val), "Nd"(port):"memory");\
}

_MAKE_PORT_OUT(b, BYTE);
_MAKE_PORT_OUT(w, SHORT);
_MAKE_PORT_OUT(l, LONG);

_MAKE_PORT_IN(b, BYTE);
_MAKE_PORT_IN(w, SHORT);
_MAKE_PORT_IN(l, LONG);

static inline void delay_outb(SHORT port, BYTE val)
{
    outb(port, val);
    outb(0x80, 0); // Output to unused port for delay
}

static inline BYTE delay_inb(SHORT port)
{
    outb(0x80, 0);
    return inb(port);
}

#undef _MAKE_PORT_OUT
#undef _MAKE_PORT_IN

#endif /* IO_H */
