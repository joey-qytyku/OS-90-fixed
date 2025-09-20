#ifndef LOGGER_H
#define LOGGER_H

#define LOG(fmt, ...)

/*
The kernel uses its own logger because printf has extra features that are not
necessary for the kernel.

` is the format escape. `` prints the caret once and has no further arguments.

The following conversions exist:

`i	Integer
`u	Unsigned integer
`x	Hexadecimal (capitalized in MASM notation)
`s	Null-terminated string

All conversions use 32-bit values unless they are strings.

> Consider using pascal format strings internally for performance.
> This can be done transparently for libc on printf

*/


#endif /*LOGGER_H*/
