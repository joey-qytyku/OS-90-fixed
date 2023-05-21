#ifndef SYNC_H
#define SYNC_H

#include <Type.h>

extern WORD _wPreemptCount;

__ALIGN(4) typedef DWORD LOCK;

#define AcquireMutex(m)\
__asm__ volatile (\
    "spin%=:\n\t"\
    "btsl $0,%0\n\t"\
    "jc spin%="\
    :"=m"(m)\
    :"m"(m)\
    :"memory"\
)

#define ReleaseMutex(m)\
__asm__ volatile (\
    "btrl $0,%0":"=m"(m)::"memory"\
)

// Operations on _wPreemptCount outisde of CritSec.asm are
// * Check if zero
// That is all.
//
// Memory fences are not required here. We use memory fences if we want
// the compiler to generate writes in a certain order, but for reads,
// we do not need it. _wPreemptCount will never change during any kind of
// asynchronous event.
//

static inline BOOL GetPreemptEnabled(VOID)
{
    FENCE;
    return _wPreemptCount == 0;
}

#endif /* SYNC_H */
