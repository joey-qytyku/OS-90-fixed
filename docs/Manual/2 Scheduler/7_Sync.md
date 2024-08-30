# Synchronization

OS/90 has an enhanced mutex which queues exactly one task in a pointer to hand of control to as soon as the lock is released. This allows for a resource to be locked and unlocked very fast when there is a lack of interdependency between that resource and other shared resources, making it acceptable for anything high demand. The memory manager uses enhanced mutexes because of the isolated design of the memory manager.

OS/90 also supports sempahore-guarded queue structures with more than one item. This is useful for when a thread is in the "background" and should handle a batch of requests rather than the low-latency enhanced mutex.

The PAUSE instruction is encoded in all builds of OS/90. It is a `REP NOP` on older systems and significantly improves Pentium performance.

> The best way to handle concurrency optimally is to test the performance.

## Yield Mutex

Use the Ymutex if in doubt. It will yield if the lock is held, and has no other semantics.

The reason why yielding is used in the syncrhonization primitives is to allow T1 to acquire locks. Otherwise it would deadlock since nothing would release it.

T0 is permitted to lock too because it is capable of yielding.

## Enhanced Mutex

The enhanced mutex causes a yield to the next task on the list if locked to prevent wasting time slices; it does not hand control over to the other thing that is holding it for obvious reasons. If the lock is released, the task that is in the internal pointer, if active is immediately switched to.

The first task to try to acquire while the pointer is zero does not simply yield, however, and is also disabled (chopped out of chain) so the scheduler does not even see it.

It may be enhanced and usually better, but contention is even worse as other tasks may struggle to run if too many threads want the lock. Additionally, if the tasks using the same resource are dependent on other events, the performance is unpredictable.

## Semaphore

The OS/90 semaphore is a counter-based lock with a fixed-length FIFO queue. It is declared in C as an array containing the following 32-bit values:

- Counter
- Counter max
- Queue entries

The structure MUST be aligned at a 4-byte boundary.

Semaphore functions are not simply called. They are GENERATED using a function so that they work with a certain queue depth. 32 bytes at a 4-byte aligned boundary is needed to make this work.

Listing:
- S_CreateSemaphoreWait(PVOID where, LONG depth)
- S_CreateSemaphoreSignal(PVOID where, LONG depth)

> There is no definite limit to the depth, but no more than 32 is recommended.

Calling a semaphore operation is not done with a procedure. This is done using inline assembly, where EAX is set to the semaphore base address.
