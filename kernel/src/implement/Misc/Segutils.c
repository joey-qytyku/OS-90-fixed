///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <Misc/Segutils.h>
#include <Type.h>

// BRIEF:
//      Convert a segment and offset to a more convenient linear address.
//
// use_pmode:       Use the LDT to get the base address if set.
//
// seg,off:         Segment and offset
//
// final_base_addr: Can be NULL if none, this is the base address to add
//                  to the final value. Can be the memory mirror of the process.
//
//
PVOID kernel Segment_To_Linear_Address(
    BOOL    use_pmode,
    PVOID   relative_to,
    U16     seg,
    U32     off
){
    U32 base_addr;
    if (use_pmode)
        base_addr = GetLdescBaseAddress(seg);
    else
        base_addr = seg * 16;

    base_addr += off;
    return relative_to + base_addr;
}

// BREIF:
//      This procedure is the compelte solution to all segment descriptor
//      problems. Get functions return a value. Set functions use the
//      additional operand.
//
//      Input is not checked for correctness.
//
U32 SegmentUtil(
    U8  func,
    U16 seg,
    U32 operand
){
    PVOID desc_ptr  = aqwLocalDescriptorTable + (seg & 0xFFF8);

    // The segment selector with the RPL and TI bits masked out are a valid
    // byte offset from the base of the table.

    switch (func)
    {
        // Access rights given by LAR will be in a confusing format
        // so we cannot use that. Also, it is really slow. Like 20 clocks.
        // We will just get the access U8normally.

        case SEG_GET_ACCESS_RIGHTS:
            return BYTE_PTR(desc_ptr, 5);
        break;
        // LSL will unscramble the segment limit, making it really easy
        // to get the value.
        case SEG_GET_LIMIT:
        {
            U32 seg_lim;
            __asm__ volatile (
                "lsl %0,%1"
                :"=r"(seg_lim)
                :"r"(seg)
                :"memory"
            );
            return seg_lim;
        }
        break;

        case SEG_SET_LIMIT:
            IaAppendLimitToDescriptor(desc_ptr, operand);
        break;

        case SEG_GET_BASE_ADDR:
        return IaGetBaseAddress(desc_ptr);

        case SEG_SET_ACCESS_RIGHTS:
            BYTE_PTR(desc_ptr, 5) = (U8)operand;
        break;

        case SEG_SET_BASE_ADDR:
            IaAppendAddressToDescriptor(desc_ptr, operand);
        break;

        default:
            return 0;
    }
}
