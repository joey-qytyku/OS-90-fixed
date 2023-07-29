#ifndef BITOPTS_H
#define BITOPTS_H

#include <Type.h>

// This is a macro
#define BIT_IS_SET(num,bit) ((num & (1<<bit))>0)

// At least one bit must be set for this to work properly
static inline U32 BitScanFwd(U32 val)
{
    U32 ret;
    __asm__ ("bsfl %0, %1":"=r"(ret):"r"(val));
    return ret;
}

#endif /* BITOPTS_H */
