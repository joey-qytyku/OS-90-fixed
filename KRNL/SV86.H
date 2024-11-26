#ifndef SV86_H
#define SV86_H

#include "SV86.H"

struct __ivt { SHORT ip:16; SHORT cs:16; };

static struct __ivt * const IVT = (PVOID)0;

typedef BOOL (*HV86)(PREGS);

LONG V86xH(BYTE v, PREGS r);

// Returns the vector of next INT
// Returns what if IRET?
LONG EnterV86(PREGS r);

#endif /* SV86_H */
