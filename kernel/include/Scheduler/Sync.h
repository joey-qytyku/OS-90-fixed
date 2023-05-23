#ifndef SCHEDULER_SYNC_H
#define SCHEDULER_SYNC_H

#include <Type.h>

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

ALIGN(4) typedef DWORD LOCK;

extern WORD _wPreemptCount;
extern VOID EnterCriticalRegion(WORD);

static inline BOOL GetPreemptEnabled(VOID)
{
    FENCE; // Keep
    return _wPreemptCount == 0;
}

#endif /* SCHEDULER_SYNC_H */
