////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef IO_H
#define IO_H

force_inline static inline void rep_insb(void* mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep insb"
        : "=D"(mem), "=c"(count)
        : "D"(mem), "c"(count), "d"(port)
        : "memory"
    );
}

force_inline static inline void rep_outsb(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep outsb"
        :
        : "S"(mem), "c"(count), "d"(port)
        : "esi", "ecx", "edx", "memory"
    );
}

force_inline static inline void rep_insw(void *mem, LONG count, SHORT port)
{
    __asm__ volatile(
        "cld\n"
        "rep insw"
        :
        : "D"(mem), "c"(count), "d"(port)
        : "edi", "ecx", "edx", "memory"
    );
}

force_inline static inline void rep_outsw(void *mem, LONG count, SHORT port)
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
force_inline static inline _TYPE in##_ASM_TYPE_PREFIX(SHORT port)\
{\
    _TYPE ret;\
    __asm__ volatile ("in" #_ASM_TYPE_PREFIX " %1, %0" :"=a"(ret) :"Nd"(port):"memory");\
    return ret;\
}


#define _MAKE_PORT_OUT(_ASM_TYPE_PREFIX, _TYPE)\
force_inline static inline void out##_ASM_TYPE_PREFIX (SHORT port, _TYPE val)\
{\
    __asm__ volatile ("out" #_ASM_TYPE_PREFIX " %0, %1": :"a"(val), "Nd"(port):"memory");\
}

#define MAKE_MMIO_READ_FUNC(_TYPE)\
force_inline static inline _TYPE ReadIOMem ##_TYPE (PVOID addr)\
{\
    FENCE; /*Fence both sides*/ \
    volatile P##_TYPE r = addr;\
    return *r;\
}

_MAKE_PORT_OUT(b, BYTE);
_MAKE_PORT_OUT(w, SHORT);
_MAKE_PORT_OUT(l, LONG);

_MAKE_PORT_IN(b, BYTE);
_MAKE_PORT_IN(w, SHORT);
_MAKE_PORT_IN(l, LONG);

force_inline static inline void delay_outb(SHORT port, BYTE val)
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