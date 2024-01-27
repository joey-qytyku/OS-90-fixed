#ifndef SCHEDULER_DEBUG_H
#define SCHEDULER_DEBUG_H

#include <Debug/Debug.h>
#include <Scheduler/Sync.h>

#define T0 0
#define T1 1
#define T2 2
// Add a special message for this, do not assert
#define _assert_T0()
#define _assert_T1()
#define _assert_T2()

#define assert_ctx(Tx) require_##Tx

#endif /* SCHEDULER_DEBUG_H */
