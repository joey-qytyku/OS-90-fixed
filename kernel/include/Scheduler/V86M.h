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
            BYTE al;
            BYTE ah;
        };
        WORD ax;
    };
    union {
        struct {
            BYTE bl;
            BYTE bh;
        };
        WORD bx;
    };
    union {
        struct {
            BYTE cl;
            BYTE ch;
        };
        WORD cx;
    };
    union {
        struct {
            BYTE dl;
            BYTE dh;
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

tpkstruct {
    DWORD   ebp;
    DWORD   edi;
    DWORD   esi;
    DWORD   eip;
}V86_CALLER_STATE,*P_V86_CALLER_STATE;

typedef STATUS (*V86_HANDLER)(P_DREGW);
typedef VOID   (*EXCEPTION_HANDLER)(PDWORD);

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
extern DWORD RealModeRegs[7];
extern DWORD RealModeTrapFrame[9];

static inline PVOID MK_LP(WORD seg, WORD off)
{
    DWORD address = seg*16 + off;
    return (PVOID) address;
}

#endif /* SCHEDULER_V86M_H */
