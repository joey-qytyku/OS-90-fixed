#ifndef SV86_H
#define SV86_H

#include "SV86.H"

struct __ivt { SHORT ip:16; SHORT cs:16; };

static struct __ivt * const IVT = (PVOID)0;

typedef BOOL (*HV86)(PSTDREGS);

LONG INTxH(BYTE x, PSTDREGS r);

// Returns the vector of next INT
// Returns what if IRET?
LONG V86xH(PSTDREGS r);

#endif /* SV86_H */
