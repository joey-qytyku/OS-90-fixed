#ifndef BITOPTS_H
#define BITOPTS_H

#include <Type.h>

static inline BOOL x86BitTestD(DWORD value, BYTE bit_inx)
{
	BOOL ret;
	__asm__ volatile (
		"bt %1, %0"
		:"=ccc"(ret)
		:"ri"(bit_inx)
        :"cc"
		);
    return ret;
}

// At least one bit must be set for this to work properly
static inline DWORD BitScanFwd(DWORD val)
{
    DWORD ret;
    __asm__ ("bsfl %0, %1":"=r"(ret):"r"(val));
    return ret;
}

#endif /* BITOPTS_H */
