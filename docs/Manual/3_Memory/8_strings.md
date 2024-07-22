# String Operations

The kernel provides optimmized string operations for memory blocks, and C or Pascal strings.

On modern processors, inline MOVSB should be prefered for any situation where unrolling is too expensive. On older processors, a naive `REP MOVSB` can quadruple the processing time in the worst case.

For that reason, OS/90 provides optimized kernel procedures for this purpose. Standard names are used to permit inlining.
