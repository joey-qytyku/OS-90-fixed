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
//      If the bits must be in literal order at U8aligned boundaries,
//      this will not help much.
//
//      Also, it is recommended that bit arrays are aligned at 4-U8boundaries
//      for high performance.
//
VOID KeEnableBitArrayEntry(PU32 array, U32 inx)
{
    U32 bit_offset  = inx & 31;
    U32 U32_index = inx >> 5;

    array[U32_index] |= 1 << bit_offset;
}

VOID KERNEL KeDisableBitArrayEntry(PU32 array, U32 inx)
{
    U32 bit_offset  = inx & 31;
    U32 U32_index = inx >> 5;

    array[U32_index] &= ~(1 << bit_offset);
}

BOOL KERNEL KeGetBitArrayEntry(PU32 array, U32 inx)
{
    U32 bit_offset  = inx & 31;
    U32 U32_index = inx / 32;

    return BIT_IS_SET(array[U32_index], bit_offset);
}

//
// Not for driver use
//
VOID KeEnableBitArrayRange(PU32 array, U32 base_inx, U32 count)
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
// TODO:
//      Maybe rewrite in assembly.
//
STATUS KERNEL KeAllocateBits(
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

        BOOL scan = KeGetBitArrayEntry(array, entry);

        if (scan == 0)
        {
            U32 free_found = 0;

            for (U32 local_entry = entry; ; local_entry++)
            {
                BOOL scan2 = KeGetBitArrayEntry(array, local_entry);

                if (local_entry > array_bounds)
                    return OS_ERROR_GENERIC;

                if (free_found == to_alloc)
                {
                    KeEnableBitArrayRange(array, entry, entry+to_alloc);
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
STATUS KERNEL AllocateOneBit(
    PU32 array,
    U32  array_bounds,
    PU32 out_index
){
    for (U32 i = array_bounds-1; i != 0; i--)
    {
        BOOL bit = KeGetBitArrayEntry(array, i);
        if (!bit)
        {
            KeEnableBitArrayEntry(array, i);
            *out_index = i;
            return OS_OK;
        }
    }
    // We iterated through the whole list and found nothing
    // Return error
    return OS_ERROR_GENERIC;
}
