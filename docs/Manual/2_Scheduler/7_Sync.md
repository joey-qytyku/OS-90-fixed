# Synchronization

OS/90 has a basic set of macros that can be used for certain atomic operations in `basicatomic.h`. The mutex lock macros should be totally avoided because OS/90 has a better set of functions for mutual exclusion that do not waste time slices.


