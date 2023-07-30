/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef _8259_H
#define _8259_H

#include <Platform/IO.h>

#define ICW1 1<<4
#define LEVEL_TRIGGER 1<<3
#define ICW1_ICW4 1

#define ICW4_8086 1
#define ICW4_SLAVE 1<<3

// The in-service register is a bit mask with one turned on
static inline U16 InGetInService16(void)
{
    U16 in_service;

    in_service  = delay_inb(0x20);
    in_service |= delay_inb(0xA0) << 8;
    return in_service;
}

#endif /* _8259_H */
