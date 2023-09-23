#ifndef STRING_H
#define STRING_H

#include <Type.h>

VOID KERNEL StrCpy(PU8 dest, PU8 str);
U32  KERNEL StrLen(PU8 str);
VOID KERNEL StrLower(PU8 str);
VOID KERNEL StrUpper(PU8 str);
PU8  KERNEL Uint32ToString(U32 value, PU8 buffer);
VOID KERNEL Hex32ToString(U32 valie, PU8 buffer);

#endif /* STRING_H */
