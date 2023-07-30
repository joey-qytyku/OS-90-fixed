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

// Keep this or nah?
tpkstruct {
    union {
        struct {
            U8 al;
            U8 ah;
        };
        U16 ax;
    };
    union {
        struct {
            U8 bl;
            U8 bh;
        };
        U16 bx;
    };
    union {
        struct {
            U8 cl;
            U8 ch;
        };
        U16 cx;
    };
    union {
        struct {
            U8 dl;
            U8 dh;
        };
        U16 dx;
    };
    U16 si;
    U16 di;
    U16 bp;

    U16 es;
    U16 ds;

    U16 ip;
    U16 cs;
    U16 flags;
    U16 sp;
    U16 ss;
}DREGW,*P_DREGW;

typedef STATUS (*V86_HANDLER)(P_DREGW);

typedef struct
{
    V86_HANDLER handler;  // Set the handler
    PVOID next;           // Initialize to zero
}V86_CHAIN_LINK,
*PV86_CHAIN_LINK;

extern KERNEL VOID ScHookDosTrap(
    U8,
    PV86_CHAIN_LINK,
    V86_HANDLER
);

extern VOID KERNEL ScOnErrorDetatchLinks(VOID);
extern VOID KERNEL ScVirtual86_Int(PVOID, U8);

extern VOID EnterRealMode(VOID);
extern U32 _RealModeRegs[7];
extern U32 _RealModeTrapFrame[9];

static inline PVOID MK_LP(U16 seg, U16 off)
{
    U32 address = seg*16 + off;
    return (PVOID) address;
}

#endif /* SCHEDULER_V86M_H */
