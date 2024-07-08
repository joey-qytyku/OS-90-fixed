/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef TYPE_H
#define TYPE_H

/////////////////////////////////////////////////////////////////////////////
// Error Codes
/////////////////////////////////////////////////////////////////////////////

//
// OS/90 uses a complex error checking method similar to the DOS extended
// error code.
//
// There are four components:
// - Index      (typically called the "error code" and is 8-bit)
// - SRC        (where it came from)
// - Cause      (what caused it)
// - Action     (suggested action)
//
//
// These are all bit masks and can be or'ed together for various combinations
//
// These error codes are not intended to carry much meaning in programming.
// Only check non-binary error codes if
//

//
// EOK and EFAIL are global to all
//
#define EOK   0
#define EFAIL 1

#define SRC_KRNL        (1<<8)
#define SRC_MEM         (1<<9)
#define SRC_CHDEV       (1<<10)
#define SRC_FS          (1<<11)
#define SRC_BLKDEV      (1<<12)
#define SRC_DOS         (1<<13)

#define CAUSE_LIKELY_BUG (1<<14) /* Software using the function likely bugged*/
#define CAUSE_CONF_ERROR (1<<15)
#define CAUSE_DEVICE     (1<<16)
#define CAUSE_DRIVER     (1<<17)

// Change to index!
#define ACT_CALLER_TERMINATE    (1<<18)
#define ACT_USER_INTERV         (1<<18) /* User intervention */
#define ACT_WARNING             (1<<19) /* Inform the user or log a warning */
#define ACT_FATAL_ERROR         (1<<20) /* Bluescreen */
#define ACT_TRY_AGAIN           (1<<31)

//
// These are generic error codes for common problems with block devices.
// More advanced codes must be obtained in other ways.
//
enum {
        E_BLKDEV_XFER_ERROR=2,  /* Generic code for transfer failed */
        E_BLKDEV_MEDIA_REMOVED, /* Media was detached unexpectedly */
        E_BLKDEV_NO_MEDIA,      /* Drive selected has no media attached */
        E_BLKDEV_BAD_BLOCK,     /* The block selected is damaged */
        E_BLKDEV_INVALID_BLOCK, /* Block selected to access is invalid */
        E_BLKDEV_EQUIP_FAIL,    /* Some kind of very bad error, maybe broken hardware */
        E_BLKDEV_OTHER_ERROR,   /* Anything else. */
        E_BLKDEV_NUM_ERRS__     /* Do not use */
};

//
// These are intended to be returned by a filesystem driver.
// This means that anything related to permissions, file locking, or any other
// subsystem-specific filesystem semantics are not included.
//
enum {
        E_FS_NOT_FOUND=2,
        E_FS_HANDLE_NOT_OPENED,
        E_FS_MEDIA_UNFORMATTED,  /* A FS was initialized on unformatted drive */
        E_FS_DAMAGED_PBR         /* Data that hold FS information is broken */
};

enum {
        E_MEM_OUT=2,    /* No page frames available */
        E_MEM_NOMAP,    /* No more virtual address space*/
        E_MEM_NOSWP,    /* Memory evicted or swapped in cannot be due to no swap */
        E_MEM_NUM_ERRS__
};

/////////////////////////////////////////////////////////////////////////////
// I n l i n e   A s s e m b l y   a n d   A s s e m b l y   L i n k a g e //
/////////////////////////////////////////////////////////////////////////////

// We need to start using this for some ASM functions
#define STACK_PARAMS __attribute__(( regparm(0) ))

#define NORETURN _Noreturn

////////////////////////////////////////////////////
// F u n c t i o n   R e l a t e d   M a c r o s  //
////////////////////////////////////////////////////

#define noinline        __attribute__((noinline))
#define force_inline    __attribute__((always_inline))

// TODO
#define kernel_export

#define BYTESWAP(value) ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |\
((value & 0xFF0000)>>8) | ((value & 0xFF000000) >> 24)

#define UNUSED_PARM(x) (VOID)x

// API_DECL creates a type definition for a function
// so that it can be added to the procedure table

// TODO!
#define API_DECL(rtype, name, ...)

    // typedef rtype kernel (*_API_##name)(__VA_ARGS__);\
    // rtype kernel name(__VA_ARGS__)

///////////////////////////////////////////
// M i s c e l a n e o u s   M a c r o s //
///////////////////////////////////////////

#define ALIGN(x) __attribute__((aligned(x)))

#define FENCE() __asm__ volatile ("":::"memory")

#define NULL ((PVOID)0UL)

#define ASM(x) __asm__ volatile (x)

/////////////////////////////////////
// T y p e   D e f i n i t i o n s //
/////////////////////////////////////

typedef unsigned long  LONG;
typedef unsigned short SHORT;
typedef unsigned char  BYTE;

typedef LONG  *PLONG;
typedef SHORT *PSHORT;
typedef BYTE  *PBYTE;

typedef long  SIGLONG;
typedef short SIGSHORT;
typedef char  SIGBYTE;

typedef const BYTE *PCSTR;

#define VOID void
#define PVOID void*
#define BOOL _Bool

/////////////////////////////////////////////////////////////////////
// A d d r e s s + O f f s e t   A d d r e s s i n g   M a c r o s //
/////////////////////////////////////////////////////////////////////

// Fixed
#define BYTE_PTR(var, off) *(BYTE)  ((PVOID)(var) + (off))
#define WORD_PTR(var, off) *(SHORT) ((PVOID)(var) + (off))
#define DWORD_PTR(var,off) *(LONG)  ((PVOID)(var) + (off))

/////////////////////////////
// G C C   B u i l t i n s //
/////////////////////////////

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define memcpy(d, s, c) __builtin_memcpy((d), (s), (c));
#define memmove(d, s, c) __builtin_memmove((d), (s), (c));
#define strcmp(s1, s2) __builtin_strcmp((s1), (s2));
#define memset(a, val, count) __builtin_memset((a),(val),(count))

#endif /* TYPE_H */
