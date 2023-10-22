#ifndef STRING_H
#define STRING_H

#include <Type.h>

API_DECL(VOID, StrCpy, char *dest, const char *str);
API_DECL(U32,  StrLen, const char *str);
API_DECL(VOID, StrLower, char *str);
API_DECL(VOID, StrUpper, char *str);
API_DECL(PSTR, Uint32ToString, U32 value, char *buffer);
API_DECL(VOID, Hex32ToString,  U32 value, char *buffer);

#endif /* STRING_H */
