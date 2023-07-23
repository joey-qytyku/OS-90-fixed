////////////////////////////////////////////////////////////////////////////////
//                      This file is part of OS/90.
//
// It is dual licensed for GPLv2 and MIT.
//
////////////////////////////////////////////////////////////////////////////////

#include <Platform/BitOps.h>
#include <Type.h>

// WARNING:
//      All of these functions operate on little endian 32-bit values.
//      If the bits must be in literal order at byte aligned boundaries,
//      this will not help much.
//
//      Also, it is recommended that bit arrays are aligned at 4-byte boundaries
//      for high performance.
//
VOID KeEnableBitArrayEntry(PDWORD array, DWORD inx)
{
    DWORD bit_offset  = inx & 31;
    DWORD dword_index = inx >> 5;

    array[dword_index] |= 1 << bit_offset;
}

VOID KERNEL KeDisableBitArrayEntry(PDWORD array, DWORD inx)
{
    DWORD bit_offset  = inx & 31;
    DWORD dword_index = inx >> 5;

    array[dword_index] &= ~(1 << bit_offset);
}

BOOL KERNEL KeGetBitArrayEntry(PDWORD array, DWORD inx)
{
    DWORD bit_offset  = inx & 31;
    DWORD dword_index = inx / 32;

    return BIT_IS_SET(array[dword_index], bit_offset);
}

//
// Not for driver use
//
VOID KeEnableBitArrayRange(PDWORD array, DWORD base_inx, DWORD count)
{
    for (DWORD i = 0; i < count; i++)
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
    PDWORD  array,         // Address of array
    DWORD   array_bounds,  // Number of bits, multiple of 4, THE LIMIT, NOT NUMBER
    DWORD   to_alloc,      // Number to allocate
    PDWORD  out_base_index // Where to store the base index returned by function
){
    DWORD entry;

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
            DWORD free_found = 0;

            for (DWORD local_entry = entry; ; local_entry++)
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
    PDWORD array,
    DWORD  array_bounds,
    PDWORD out_index
){
    for (DWORD i = array_bounds-1; i != 0; i--)
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
