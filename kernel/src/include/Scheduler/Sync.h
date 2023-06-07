#ifndef SCHEDULER_SYNC_H
#define SCHEDULER_SYNC_H

#include <Type.h>

#define LOCK_INIT (0)

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

tstruct {
    LOCK    real_mode_lock;
    LOCK    v86_chain_lock;
    LOCK    proc_list_lock;
}ALL_KERNEL_LOCKS;


#endif /* SCHEDULER_SYNC_H */
