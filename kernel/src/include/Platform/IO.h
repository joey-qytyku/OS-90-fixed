/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef IO_H
#define IO_H

#include <Type.h>

static inline VOID rep_insb(PVOID mem, DWORD count, WORD port)
{
    __asm__ volatile(
        "rep insb"
        :
        :"esi"(mem),"ecx"(count),"dx"(port)
        :"esi","edi","dx","memory"
        );
}

static inline VOID rep_outsb(PVOID mem, DWORD count, WORD port)
{
    __asm__ volatile (
        "rep outsb"
        :
        :"esi"(mem),"ecx"(count),"dx"(port)
        :"esi","edi","dx","memory"
        );
}

static inline VOID rep_insw(PVOID mem, DWORD count, WORD port)
{
    __asm__ volatile (
        "rep insw"
        :
        :"esi"(mem),"ecx"(count),"dx"(port)
        :"esi","edi","dx","memory"
        );
}

static inline VOID rep_outsw(PVOID mem, DWORD count, WORD port)
{
    __asm__ volatile (
        "rep outsw"
        :
        :"esi"(mem),"ecx"(count),"dx"(port)
        :"esi","edi","dx","memory"
        );
}

#define _MAKE_PORT_IN(_ASM_TYPE_PREFIX, _TYPE)\
static inline _TYPE in##_ASM_TYPE_PREFIX(WORD port)\
{\
    _TYPE ret;\
    __asm__ volatile ("in" #_ASM_TYPE_PREFIX " %1, %0" :"=a"(ret) :"Nd"(port):"memory");\
    return ret;\
}


#define _MAKE_PORT_OUT(_ASM_TYPE_PREFIX, _TYPE)\
static inline VOID out##_ASM_TYPE_PREFIX (WORD port, _TYPE val)\
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

_MAKE_PORT_OUT(b, BYTE);
_MAKE_PORT_OUT(w, WORD);
_MAKE_PORT_OUT(l, DWORD);

_MAKE_PORT_IN(b, BYTE);
_MAKE_PORT_IN(w, WORD);
_MAKE_PORT_IN(l, DWORD);

static inline VOID delay_outb(WORD port, BYTE val)
{
    outb(port, val);
    outb(0x80, 0); // Output to unused port for delay
}

static inline BYTE delay_inb(WORD port)
{
    outb(0x80, 0);
    return inb(port);
}

#undef _MAKE_PORT_OUT
#undef _MAKE_PORT_IN


#endif /* IO_H */
