# A Case for C

> Old because OS/90 is switching to assembly for most things.

In this document, I will explain why I picked the C language for OS/90.

# Modern Compilers

Using a modern C compiler to make an OS for old computers is cheating, and I am willing to admit that. GCC will spit out code multitudes better than a compiler from the 90's. Even modern java can sometimes outperform older C compilers. People wrote operating systems in assembly because C compilers were too clunky to be useful for that purpose and portability was not as important as performance. There was a clear divide between systems programming in assembly and userspace programming with C and Pascal.

This is no longer the case. High-level languages have replaced assembly in most areas.

# Calling Conventions

Most large functions will push registers onto the stack during operation to make room for other data until the original data is required later. Placing arguments into the stack right away is not a bad idea. x86 has always had highly optimized push instructions. The disadvantage may be that moving from the stack with an addressing mode is a more complex operation than simply popping a saved value from the stack to use it again.

x86 has push immediate instructions which means that the compiler does not need to emit `xor r32,r32` instructions to clear registers before loading a narrow value. Some tests done by me (trust me bro) using GCC global regparm compiler arguments dramatically increased code size. I am not sure why, though.

Finally, calling conventions can be mostly ignored by the compiler in the case of intramodular procedure calls. Functions can also be inlined to avoid the call overhead.
