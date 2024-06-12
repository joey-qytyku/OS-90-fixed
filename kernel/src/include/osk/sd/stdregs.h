/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2028, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
#include <Type.h>

#ifndef STDREGS_H
#define STDREGS_H

/*******************************************************************************
In x86, registers can be thought of as essentially being big endian, though
they do not technically have a specific byte order. When a word is loaded
from memory, it is automatically "byte swapped" in that AH corresponds with
the higher order byte of the hex representation.

So if we have the following bytes in memory: CD AB
And we load it in AX:
AH = AB
AL = CD

This means that the structure representation of the AX register
must define AH and AL in byte-swapped order.
*******************************************************************************/

// Create a register that has a 32-bit/16-bit/8-bit high or low version
#define _DWB_R(n)\
union {\
        LONG E##n##X;\
        union {\
                BYTE n##L;\
                struct { BYTE :8; BYTE n##H; };\
        };\
        SHORT n##X;\
}

// Create a register that can be 16-bit or 32-bit.
#define _DW_R(n)\
union {\
        LONG E##n;\
        SHORT n;\
}

/*******************************************************************************
This is the standard register dump structure. It is also known as a context
and contains every register that represents an execution state in any mode
of execution within the OS/90 protected mode environment.

Stdregs is the structure that is used by the low-level interrupt handler,
system entry, and task management.

It is important to understand that this is simply to promote the reuse of
code. Stdregs can be used in many situations that are not always related.

Stdregs can have various meanings and does not always have to be allocated
to its full extent. The V86 segment registers at the end are only generated
in cases when switching from a V86 context. alloca can be used in such
situations.

When entering V86, the V86 registers are used, but should be
accessed using the ES,DS,FS,GS,CS names since the v86 ones are a bit of an
implementation detail.
*******************************************************************************/
typedef struct {
        _DWB_R(A);
        _DWB_R(B);
        _DWB_R(C);
        _DWB_R(D);

        _DW_R(SI);
        _DW_R(DI);
        _DW_R(BP);

        LONG   pm_ES;
        LONG   pm_DS;
        LONG   pm_FS;
        LONG   pm_GS;
        _DW_R(IP);

        LONG   CS;
        _DW_R(FLAGS);
        _DW_R(SP);
        LONG   SS;

        union {LONG v86_ES; LONG ES;};
        union {LONG v86_DS; LONG DS;};
        union {LONG v86_FS; LONG FS;};
        union {LONG v86_GS; LONG GS;};
}STDREGS, *PSTDREGS;

#undef _DW_R
#undef _DWB_R

#endif /*STDREGS_H*/
