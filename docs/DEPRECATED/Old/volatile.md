# The Basic Principle

If you think you need `volatile`, think again because you are very likely wrong.

# What is the Volatile Qualifier

Many people think they know what `volatile` is, but actually don't. It is not a panacea for any data race problem. `volatile` does not garauntee atomicity and it is not a memory fence. The compiler will actually optimize memory accesses by reordering. The only difference is that dead reads and writes are not eliminated.

`volatile` has only one purpose, which is to access memory locations that can change unexpectedly. This means that it will read memory location rather than any previous value in the registers.

# When to use volatile

Interrupt service routines for example, should use volatile for variables shared with other code since the compiler optimization could eliminate loads and stores it thinks are redundant. If an ISR has exclusive write access to the variable, there should be no need for volatile.

# When to NOT Use Volatile

Lock functions are serialized at the compiler level. Once a lock is held, the data being protected can be accessed for whatever purpose desired and there is simply no need for `volatile` because the data is safe from external modification. This means that `volatile` is useless in almost every situation and when used alongside a lock it just removes important optimizations.

Mutex locks theselves are not even volatile and do not need to be because only lock acqure and release macros will even modify them, and they will do so atomically. Volatile does nothing to garauntee thread safety.

If accessing data modified by an ISR, it is __NOT ENOUGH__ to use volatile. It is necessary to disable interrupts while accessing the information to ensure that nothing else is modifying the data.

# Volatile is Rarely Needed

Volatile pointers are unnecessary for accessing MMIO. There are dedicated inline functions for doing this provided by the OS/90 system headers that use it internally.
