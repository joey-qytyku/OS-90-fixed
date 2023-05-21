/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <Platform/IA32.h>

#define PG_SHIFT 12

// walkpgdir from xv6?

#define PG_DIRTY (1<<6)
#define PG_ACC   (1<<5) /* The page table/dir was used */
#define PG_CHDIS (1<<4)
#define PG_WRTH  (1<<3)
#define PG_USER  (1<<2)
#define PG_RDWR  (1<<1)
#define PG_PRES  (1<<0)

#define CH_USER        (1<<0)
#define CH_INIT_LOCKED (1<<1)
#define CH_PHYSCONT    (1<<2)

#define MAP_KPD (1<<0)
#define MAP_UPD (1<<1)
#define MAP_RD  (1<<2)

typedef struct __PACKED
{
    DWORD   b_idnum:28;
    BYTE    bf_free:1;
    BYTE    bf_frozen:1;
    BYTE    bf_phys_cont:1;
    BYTE    bf_user:1;
}BLOCK,*PBLOCK;

static inline VOID i486_invlpg(PAGE addr)
{
    __asm__ volatile ("invlpg (%0)"::"r"(addr) :"memory");
}

extern PVOID AllocDosMem(WORD);
extern VOID MmResizeDosMem(WORD);
#endif /* MEMORY_H */
