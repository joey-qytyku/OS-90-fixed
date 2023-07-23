#ifndef STACKUTILS_H
#define STACKUTILS_H

#include <Type.h>

VOID RmPush16(PWORD ss, PDWORD esp, WORD value);
WORD RmPop16(PWORD ss, PDWORD esp);
VOID PmPush16(PWORD ss, PDWORD esp, WORD value);
WORD PmPop16(PWORD ss, PDWORD esp);


#endif /* STACKUTILS_H */
