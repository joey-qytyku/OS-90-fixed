////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


#ifndef TYPE_H
#define TYPE_H

///////////////////////////////////////////////
// K e r n e l   A P I   E x i t   C o d e s //
///////////////////////////////////////////////

// Are we even going to use all these?

enum {
    OS_OK,
    OS_ERROR_GENERIC,
    OS_DOS_ERROR,
    OS_INVALID_FUNCTION,
    OS_INVALID_PARAMS,

    OS_ACCESS_DENIED,
    OS_FILE_NOT_FOUND,
    OS_INVALID_HANDLE,
    OS_IS_DIRECTORY_NOT_FILE,
    OS_IS_FILE_NOT_DIRECTORY,
    OS_TOO_MANY_OPEN,
    OS_NO_SPACE_ON_DISK,
    OS_FEATURE_NOT_SUPPORTED,
    OS_OUT_OF_MEMORY
};

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

#define ASNL "\n\t"
#define ASM_LINK __attribute__(( regparm(0) ))

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

#define kernel  __attribute__(( regparm(0), cdecl ))
#define kernel_async kernel
#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

// stdarg? Pfffff. Too bloated. We know we are using cdecl and that the callee
// is going to clean up the arguments, so might as well use a pointer.
// On the stack, each item is going to be low-to-high in memory since cdecl
// goes in reverse (it has to for variadic arguments to work reasonably).
//
// So no need for va_start and va_end!

// The procedure returns a pointer with a predefined boundary
// This allows GCC to optimize code with more clever assumtions

// Drivers cannot possible know the alignment of certain
// things like memory blocks. bound should NEVER be a macro!!!!!
#define RETPTR_WITH_FIXED_BOUND(bound) __attribute__((alloc_align(bound)))

#define ARGPTR_RDO(...)  __attribute__( (access(read_only,  __VA_ARGS__ )) )
#define ARGPTR_WRO(...)  __attribute__( (access(write_only, __VA_ARGS__ )) )
#define ARGPTR_RDWR(...) __attribute__( (access(read_write, __VA_ARGS__ )) )

#define __todoremove_GET_VAR_LIST(last_fixed) (&(last_fixed) + 4)

// API_DECL creates a type definition for a function
// so that it can be added to the procedure table

#define API_DECL(rtype, name, ...)\
    typedef rtype kernel (*_API_##name)(__VA_ARGS__);\
    rtype kernel name(__VA_ARGS__)

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

#define ALIGN(x) __attribute__( (aligned(x)) )

#define FENCE __asm__ volatile ("":::"memory")

#define offsetof(st, m) ((U32)&(((st *)0)->m))

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

#define VOID void
#define PVOID void*

#define BOOL _Bool
#define STATUS U32
#define HANDLE I32

#define PID U16

typedef __UINT32_TYPE__ U32,*PU32;
typedef __UINT16_TYPE__ U16,*PU16;
typedef __UINT8_TYPE__  U8, *PU8;

typedef __INT32_TYPE__ S32,*PS32;
typedef __INT16_TYPE__ S16,*PS16;
typedef __INT8_TYPE__  S8, *PS8;

typedef __UINT64_TYPE__ U64,*PU64;

#define NULL ((PVOID)0UL)

#define tstruct   typedef struct
#define tpkstruct typedef struct __attribute__((packed))

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

#define BYTE_PTR(var, off) *(PU8) ((var)+(off))
#define WORD_PTR(var, off) *(PU16) ((var)+(off))
#define DWORD_PTR(var,off) *(PU32)((var)+(off))

#define PTR2INT(ptr) ((U32)(ptr))
#define INT2PTR(Int, ptrtype) ((ptrtype)(Int))

// Consider removing
#define CAST(val, type) ((type)(val))

/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

static inline VOID *C_memcpy(VOID *d, VOID *s, U32 c)
{
	return __builtin_memcpy(d, s, c);
}

static inline VOID *C_memmove(VOID *d, VOID *s, U32 c)
{
	return __builtin_memmove(d, s, c);
}

static inline int C_strcmp(char *s1, char *s2)
{
	return __builtin_strcmp(s1, s2);
}

static inline VOID C_memset(PVOID a, U32 val, U32 count)
{
    __builtin_memset(a,val,count);
}

#endif /* TYPE_H */
