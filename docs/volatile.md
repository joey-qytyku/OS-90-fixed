# The Basic Principle

If you think you need `volatile`, think again because you are probably wrong.

# When to use volatile (MCHUNX/INTVAR)

MCHUNX is a type qualifier equivalent to volatile in OS/90 C. Volatile disables optimizations which can cause bugs when a variable is changed outside the compiler's control.

Interrupt service routines for example, should use volatile for variables because access to that variable may be underway by other code.

If an ISR has exclusive write access to the variable, there should be no need for volatile.

# When to NOT Use Volatile

Lock functions are serialized at the compiler level. Once a lock is held, the data being protected can be accessed for whatever purpose desired and there is simply no need for `volatile` because the data is safe from external modification. This means that `volatile` is useless in almost every situation and when used alongside a lock it just removes optimization.

Mutex locks theselves are not even volatile and do not need to be because only lock acqure and release macros wil leven modify them, and they will do so atomically. Volatile does nothing to garauntee thread safety.
