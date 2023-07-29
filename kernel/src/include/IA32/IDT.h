#ifndef IA32_IDT_H
#define IA32_IDT_H

#include <Type.h>

#define IDT_INT386  0xE /* IF=0 on entry */
#define IDT_TRAP386 0xF /* IF unchanged on entry */

#define MkTrapGate(vector, dpl, address)\
    {_SetIntVector(vector, 0x80 | dpl<<4 | IDT_INT386, address);}

#define MkIntrGate(vector, address)\
    {_SetIntVector(vector, 0x80 | IDT_INT386, address);}


extern VOID _SetIntVector(U32 vector, U32 _attr, PVOID address);

#endif /* IA32_IDT_H */
