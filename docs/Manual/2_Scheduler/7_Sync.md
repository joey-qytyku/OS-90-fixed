# Synchronization

OS/90 has a basic set of macros that can be used for certain atomic operations in `basicatomic.h`. The mutex lock macros should be totally avoided because OS/90 has a better set of functions for mutual exclusion that do not waste time slices.

OS/90 has an enhanced mutex which queues exactly one task in a pointer to hand of control to as soon as the lock is released. This allows for a resource to be locked and unlocked very fast when there is a lack of interdependency between that resource and other shared resources, making it acceptable for anything high demand. The memory manager uses enhanced mutexes because of the isolated design of the memory manager.

OS/90 also supports sempahore-guarded queue structures with more than one item. This is useful for when a thread is in the "background" and should handle a batch of requests rather than the low-latency enhanced mutex.

> The best way to handle concurrency optimally is to test the performance.

## Yield Mutex

Use the Ymutex if in doubt. It will yield if the lock is held, and has no other semantics.

## Enhanced Mutex

The enhanced mutex causes a yield to the next task on the list if locked to prevent wasting time slices; it does not hand control over to the other thing that is holding it for obvious reasons. If the lock is released, the task that is in the internal pointer, if active is immediately switched to.
