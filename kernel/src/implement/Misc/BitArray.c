///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <Platform/BitOps.h>
#include <Type.h>

// WARNING:
//      All of these functions operate on little endian 32-bit values.
//      If the bits must be in literal order at byte aligned boundaries,
//      this will not help much.
//
//      Also, it is recommended that bit arrays are aligned at 4-byte boundaries
//      for performance.
//
VOID Enable_Bit_Array_Entry(PU32 array, U32 inx)
{
    U32 bit_offset  = inx & 31;
    U32 U32_index = inx >> 5;

    array[U32_index] |= 1 << bit_offset;
}

VOID kernel Disable_Bit_Array_Entry(PU32 array, U32 inx)
{
    U32 bit_offset  = inx & 31;
    U32 U32_index = inx >> 5;

    array[U32_index] &= ~(1 << bit_offset);
}

BOOL kernel Get_Bit_Array_Entry(PU32 array, U32 inx)
{
    U32 bit_offset  = inx & 31;
    U32 U32_index = inx / 32;

    return BIT_IS_SET(array[U32_index], bit_offset);
}

//
// Not for driver use. Why not?
//
VOID Enable_Bit_Array_Range(PU32 array, U32 base_inx, U32 count)
{
    for (U32 i = 0; i < count; i++)
        KeEnableBitArrayEntry(array, base_inx+i);
}

// BRIEF:
//      This procedure returns an index to a bit array where to_alloc number
//      of bits can be found contiguously and it enables those bits.
//      It fits many different situations, such as IO ports, memory,
//      and LDT descriptors.
//
// NOTE:
//      Ignore references to the LDT in comments. This is for any bit array.
//
//      I need to do some testing.
//
// WARNINGS:
//      Insanely good algorithm.
//
// TODO:
//      Maybe rewrite in assembly. Make it return -1 on failure.
//
kernel STATUS Allocate_Bits(
    PU32  array,         // Address of array
    U32   array_bounds,  // Number of bits, multiple of 4, THE LIMIT, NOT NUMBER
    U32   to_alloc,      // Number to allocate
    PU32  out_base_index // Where to store the base index returned by function
){
    U32 entry;

    // This procedure will probably not return correct input if the count
    // is zero. This is an invalid input.
    if (to_alloc == 0)
        return OS_ERROR_GENERIC;

    for (entry = 0; ; entry++)
    {
        if (entry > array_bounds)
            return OS_ERROR_GENERIC;

        BOOL scan = Get_Bit_Array_Entry(array, entry);

        if (scan == 0)
        {
            U32 free_found = 0;

            for (U32 local_entry = entry; ; local_entry++)
            {
                BOOL scan2 = Get_Bit_Array_Entry(array, local_entry);

                if (local_entry > array_bounds)
                    return OS_ERROR_GENERIC;

                if (free_found == to_alloc)
                {
                    Enable_Bit_Array_Range(array, entry, entry+to_alloc);
                    *out_base_index = entry;
                    return OS_OK;
                }

                if (scan2 == 0)
                    free_found++;

                else if (scan2 == 1 && free_found != to_alloc)
                {
                    entry = entry + free_found + 1;
                    break;
                }
            }
            continue;
        } // END IF
    }// END FOR

    return OS_OK;
}

// BRIEF:
//      Sometimes, we only want to allocate a single bit.
//      This procedure scans the array as it is most likely to find a free
//      entry at the end for most situations.
//
kernel STATUS Allocate_One_Bit(
    PU32 array,
    U32  array_bounds,
    PU32 out_index
){
    for (U32 i = array_bounds-1; i != 0; i--)
    {
        BOOL bit = Get_Bit_Array_Entry(array, i);
        if (!bit)
        {
            Enable_Bit_Array_Entry(array, i);
            *out_index = i;
            return OS_OK;
        }
    }
    // We iterated through the whole list and found nothing
    // Return error
    return OS_ERROR_GENERIC;
}
