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

#include <stdbool.h>

#define BIT_IS_SET(num,bit) ((num & (1<<bit))>0)

void enable_bit_array_entry(LONG *array, LONG inx)
{
    LONG bit_offset  = inx & 31;
    LONG Int_index = inx >> 5;

    array[Int_index] |= 1 << bit_offset;
}

void disable_bit_array_entry(LONG *array, LONG inx)
{
    LONG bit_offset  = inx & 31;
    LONG Int_index = inx >> 5;

    array[Int_index] &= ~(1 << bit_offset);
}

BOOL get_bit_array_entry(LONG *array, LONG inx)
{
    LONG bit_offset  = inx & 31;
    LONG Int_index = inx / 32;

    return BIT_IS_SET(array[Int_index], bit_offset);
}

void enable_bit_array_range(LONG *array, LONG base_inx, LONG count)
{
    for (LONG i = 0; i < count; i++)
        enable_bit_array_entry(array, base_inx+i);
}

// __attribute__((hot))
LONG alloc_bits(
    LONG*       array,
    LONG        bound,
    LONG        num
){
    LONG i = 0;
    LONG j = 0;
    LONG counter = 0;

    if (num == 0) {
        return -1;
    }

    while (1) {
        j = 0;
        if (i >= bound)
            goto fail;
        if (get_bit_array_entry(array, i) == 0) {
            j = i;
            counter = num;
            while (1) {
                if (get_bit_array_entry(array, j) == 0) {
                    j++;
                    if (j >= bound) {
                        goto fail;
                    }
                    counter--;
                    if (counter == 0)
                        goto success;
                }
                else {
                    goto restart_outer;
                }
            }
        }
        restart_outer:
        i = j + 1;
    }
    success:
    enable_bit_array_range(array, i, num);
    return i;

    fail:
    return -1;
}
