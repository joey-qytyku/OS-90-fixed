#include <Scheduler/IoDecode.h>
#include <Platform/BitOps.h>

// This file contains helper functions for the scheduler code

//------------------------------------------------------------------------------
// Brief:
//  When a userspace process uses IO operations, it is necessary to decode it
//  and fully virtualize the instruction.
//
// opcode:
//  A pointer to the instruction to decode
// bits32:
//  Is the requesting mode 32-bit?
// dec:
//  Output structure
//
// Return:
//  A boolean, false if the instruction is IO, true if not
//  If not IO, contents of the decode struct are invalid
//
// Some opcodes, such as IO, have hidden parameter bits that are not
// documented in the manuals and are instead listed as entirely separate
// instructions.
//
// Layout of an IO opcode:
// |S|1|1|0|V|1|D|W|
//
// Bits if one:
// V: Variable (uses DX)
// S: String
// D: Output
// W: Word
//
BOOL ScDecodePortOp(
    PBYTE               opcode,
    BOOL                bits32,
    PDECODED_PORT_OP    dec
){
    BOOL  size_ovr = 0;
    BOOL  rep_prefix = 0;
    PBYTE op = opcode;

    // An assembler will usually emit the REP prefix before the.
    // The following do this:
    // - MASM
    // - NASM
    // - FASM
    // Most programs of the DOS era were built with MASM, so we should be okay
    // to assume that this is the order, though x86 does not require it

    // Do we have a REP prefix?
    // There will only be F3, the other rep prefix will not be appied
    if (*opcode == 0xF3)
    {
        rep_prefix = 1;
        dec->size_of_op++;
    }

    // Operand size prefix converts an instruction with W=1
    // to 16-bits in protected mode and 32-bits in real mode.
    // They are inverses of each other.
    if (*op == 0x66)
    {
        // We have a size prefix, so update the operand size

        // The only valid way to encode IO with a 66h prefix
        // in real mode is with W=1, this is assumed
        dec->operand_width = 2;
        op = op+1;

        if (bits32)
        {
            // This instruction is being made 16-bit in 32-bit mode
            dec->operand_width = 1;
        }
        else
        {
            // This instruction is being made 32-bit in 16-bit mode
            dec->operand_width = 2;
        }
    }

    // The binary sequence 01100100 represents the bits that are shared
    // by all IO opcode. We still do not know if the main opcode byte
    // is actually IO, so the bitmask is used and compared with the
    // bitmask itself to see if the bits are there.
    if (*op & 0x64 != 0x64)
    {
        // This is NOT an IO opcode.
        return 1;
    }

    // The bit index 3 is on if the port is variable (DX indexed)
    // Bit 0 is the W bit.
    // This applies to string ops
    dec->variable = BIT_IS_SET(*op, 3);

    if (!dec->variable)
        dec->imm8 = op[1];

    // Bit index 1 is on if the instruction is output
    // This also applies to string ops
    dec->in_or_out = BIT_IS_SET(*op, 1);

    // If it's a string op is determined by the top bit being off
    dec->string = !BIT_IS_SET(*op, 7);

    // IO string ops have the variable bit set, so we only need to check
    // that bit to determine the remaining size of the sequence
    if (dec->variable)
    {
        // One byte opcode
        dec->size_of_op++;
    }
    else {
        // Two byte opcode with 8-bit immediate value
        dec->size_of_op += 2;
        dec->imm8 = op[1];
    }
    return 0;
}

// By default, the PIT is set to pitifully slow intervals, clocking at
// about 18.4 Hz (or IRQs per second). This is unsuitable
// for pre-emptive multitasking. We must configure this to a
// more satifactory frequency. We will set it to something way higher.
//
// Setting it to the max is tempting, but that would mean the ratio of
// timer clocks to CPU clocks for an i386 would be about 1:12, which
// means that every 12 CPU clocks, the CPU is interrupted and forced to
// run potentially hundreds of clocks to switch tasks and whatnot.
//
// Instead, we will set it to something more reasonable (not in MHz range).
//
