/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef IO_H
#define IO_H

#include <Type.h>

static inline void outb(WORD port, BYTE val)
{
    __asm__ volatile ("outb %0, %1": :"a"(val), "Nd"(port));
}

static inline BYTE inb(WORD port)
{
    BYTE ret;
    __asm__ volatile ("inb %1, %0" :"=a"(ret) :"Nd"(port) );
    return ret;
}

static inline void rep_insb(PVOID mem, DWORD count, WORD port)
{__asm__ volatile ("rep insb"::"esi"(mem),"ecx"(count),"dx"(port) :"esi","edi","dx");
}

static inline void rep_outsb(PVOID mem, DWORD count, WORD port)
{__asm__ volatile ("rep outsb"::"esi"(mem),"ecx"(count),"dx"(port) :"esi","edi","dx");
}

static inline void rep_insw(PVOID mem, DWORD count, WORD port)
{__asm__ volatile ("rep insw"::"esi"(mem),"ecx"(count),"dx"(port) :"esi","edi","dx");
}

static inline void rep_outsw(PVOID mem, DWORD count, WORD port)
{__asm__ volatile ("rep outsw"::"esi"(mem),"ecx"(count),"dx"(port) :"esi","edi","dx");
}

#endif /* IO_H */
