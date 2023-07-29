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

#ifndef SCHEDULER_V86M_H
#define SCHEDULER_V86M_H

#include <Type.h>

// V86 handler return values
#define CAPT_HND   0 /* Handled captured trap */
#define CAPT_NOHND 1 /* Did not handle */

// In the 8086, if we fetch the little endian value
// AD DE from the memory, AH will be DE and AL will be AD.
tpkstruct {
    union {
        struct {
            U8al;
            U8ah;
        };
        WORD ax;
    };
    union {
        struct {
            U8bl;
            U8bh;
        };
        WORD bx;
    };
    union {
        struct {
            U8cl;
            U8ch;
        };
        WORD cx;
    };
    union {
        struct {
            U8dl;
            U8dh;
        };
        WORD dx;
    };
    WORD si;
    WORD di;
    WORD bp;

    WORD es;
    WORD ds;

    WORD ip;
    WORD cs;
    WORD flags;
    WORD sp;
    WORD ss;
}DREGW,*P_DREGW;

typedef STATUS (*V86_HANDLER)(P_DREGW);

typedef struct
{
    V86_HANDLER handler;  // Set the handler
    PVOID next;           // Initialize to zero
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

extern KERNEL VOID ScHookDosTrap(
    BYTE,
    PV86_CHAIN_LINK,
    V86_HANDLER
);

extern VOID KERNEL ScOnErrorDetatchLinks(VOID);
extern VOID KERNEL ScVirtual86_Int(P_DREGW, BYTE);

extern VOID EnterRealMode(VOID);
extern U32 _RealModeRegs[7];
extern U32 _RealModeTrapFrame[9];

static inline PVOID MK_LP(WORD seg, WORD off)
{
    U32 address = seg*16 + off;
    return (PVOID) address;
}

#endif /* SCHEDULER_V86M_H */
