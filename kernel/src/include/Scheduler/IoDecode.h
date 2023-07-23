////////////////////////////////////////////////////////////////////////////////
//                      This file is part of OS/90.
//
// OS/90 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 2 of the License, or (at your option) any later
// version.
//
// OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along
// with OS/90. If not, see <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_IODECODE_H
#define SCHEDULER_IODECODE_H

#include <Type.h>

#define IS_IO_OPCODE(op) ((op & 0b01100100) != 0)

// When emulating port IO instructions, this structure is generated after
// decoding
typedef struct
{
    BYTE    operand_width:2;// Operand size, in order 8,16,32
    BYTE    in_or_out:1;    // Input if 0, output if 1
    BYTE    variable:1;     // Uses DX as address, implied by string
    BYTE    size_of_op:3;   // Number of bytes
    BYTE    string:1;       // INS/OUTS
    BYTE    repeat:1;       // Repeat CX times
    BYTE    imm8;           // Valid if !variable
}DECODED_PORT_OP, *PDECODED_PORT_OP;

// TODO
// typedef struct { }VDEV_IOP_REQUEST_PACKET;

extern BOOL ScDecodePortOp(
    PBYTE,
    BOOL,
    PDECODED_PORT_OP
);

extern VOID IaUseDirectRing3IO(VOID);
extern VOID IaUseVirtualRing3IO(VOID);


#endif /* SCHEDULER_IODECODE_H */
