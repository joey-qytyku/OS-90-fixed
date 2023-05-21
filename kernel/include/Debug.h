#ifndef DEBUG_H
#define DEBUG_H

#include <Type.h>

// What was this for? I made the function thread safe.
#define ERROR_IF_ISR()

typedef VOID (*OUTPUT_DRIVER)(BYTE);

extern VOID KERNEL Hex32ToString(DWORD, PBYTE);
extern VOID KERNEL Uint32ToString(DWORD, PBYTE);
extern VOID KERNEL KeLogf(OUTPUT_DRIVER, IMUSTR restrict, ...);
extern VOID KERNEL FatalError(DWORD);

extern VOID KeWriteAsciiz(OUTPUT_DRIVER, IMUSTR);
extern VOID _KernelPutchar(BYTE ch);

#define _str(x) #x
#define _str2(x) _str(x)

#define TRACE(...)\
    KeWriteAsciiz(_KernelPutchar,"\x1b[31m");\
    KeLogf(_KernelPutchar,\
    "[" __FILE__ ":" _str2(__LINE__) "] " \
    __VA_ARGS__\
    );\
    KeWriteAsciiz(_KernelPutchar,"\x1b[0m");

#define BREAK __asm__ volatile ("xchgw %%bx,%%bx":::"memory")

#endif
