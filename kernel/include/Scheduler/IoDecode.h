#ifndef SCHEDULER_IODECODE_H
#define SCHEDULER_IODECODE_H

#include <Type.h>

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

extern BOOL ScDecodePortOp(
    PBYTE,
    BOOL,
    PDECODED_PORT_OP
);

extern VOID IaUseDirectRing3IO(VOID);
extern VOID IaUseVirtualRing3IO(VOID);


#endif /* SCHEDULER_IODECODE_H */
