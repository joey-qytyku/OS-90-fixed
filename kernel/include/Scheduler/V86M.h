#ifndef V86M_H
#define V86M_H

#include <Type.h>

// V86 handler return values
#define CAPT_HND   0 /* Handled captured trap */
#define CAPT_NOHND 1 /* Did not handle */

// Return value of a V86 chain handler is zero if handled, 1 if refuse to handle
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

#endif /* V86M_H */
