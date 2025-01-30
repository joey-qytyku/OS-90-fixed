# C Library War Room

## Compatibility

C99 is my target. Most programs are compatible with C99.

## Files in C

printf and fprintf, or other related function calls need to operate on FILE streams.

FILE is an opaque type that is not supposed to be accessed directly. Technically it can be sometimes but that is not defined.

FILE is an abstraction for file descriptors and adds additional features:
- Buffering. The buffer length can be changed. Some streams can have immediate writeback or be line buffered.
- Orientation: byte characters or wide characters. Does not have to be supported fully.
- Binary or text mode
- End of file indicator
- Bunch of other stuff

Wide characters are not that bad. Basically, a wchar_t can be anything but is usually a UTF-16 character.

The purpose of wide functions in the C library is to allow easy conversion, although it is probably not very efficient.

For example, if I fprintf ascii/UTF-8 characters to a wide stream I can convert them directly to wide characters without problems.

Actually that is wrong. Per cppreference narrow and wide cannot coexist or errors occur.

But the premise is that the file can have IO done on it with strings and buffers that are wide or narrow.

Let's model the FILE structure:
```c
typedef struct {
    #define __STREAM_ORI_UNSET  0
    #define __STREAM_ORI_NRRW   1
    #define __STREAM_ORI_WIDE   2

    uint8_t     orient:2,
                binary:1;
                eof_reached:1;

    int         error_stat;
    fpos_t      file_pointer;

    void*       iobuff;
}FILE;
```

Localizing in/out/err can be done by making their file descriptors a special case.

The process context would include:
- The actual FILE structure of each default stream
- Malloc related pointers
- Localization and other things

## Testing and Build System

I need a proper build system to compile it fast. Almost every function will get a separate file for minimum size when static linking.

This will require a real makefile.

The source tree will be:
```
CLIB
	inc
	src
	bld
	Makefile
	1_pkg.sh
```

I of course can avoid makefiles entirely. Make uses time stamps. If the source file has a time stamp that is ahead of the object file or the object file does not exist, then the object file must be removed and the source file is recompiled.

I dislike makefiles because they are hard to read and are really just a domain specific way of doing everything the shell can, and I do not even need most of its features. I will just use the shell.

Making almost every function a separate file has the potential to greatly increase linking time. It should be done selectively because there are potentially hundreds of functions in the standard library.


> Add macro for LOCALE_INCAPABLE for functions that are not updated for locales.

## Stream Buffering

Consider not using buffering at all.

Well the advantage is that I can avoid having to perform a mode switch by calling INT 21h. This is not fast, so buffering reduces the number of filesystem calls, even if the filesystem implements its own buffer system.

The cache driver has to copy from its own aligned buffers to the unaligned buffers of the userspace.

## Compilation

DJGPP may allow a freestanding compilation. crt0 and the stub are supposed to enter protected mode and set up the C environment.
