# Standard IO Dillema

All programs will have access to standard IO, whether they be DOS or native. The desired behavior is that character IO can be interleaved between processes, so outputting DIR on one DOS window and running DIR in another would allow both to output text like this.

# Display Drivers

Display drivers must conform with this standard to be compatible with existing OS/90 software. This specification is designed to allow for multitasking of programs that request direct access to the framebuffer. Non-accelerated displays, including VESA, are supported by the display driver model.

## Framebuffer Management

Some framebuffers belong to DOS programs that have attempted to access programs. Any framebuffers controlled by a windowing system are none of the business of the display driver. The userspace can write to the main framebuffer after compositing the buffers. The contents of the virtual framebuffers VGA standards for text 80x25 text displays. For monochrome, the data is a bit map. For 640x480x16, the data is organized in 4-bit nibbles.

If a program needs direct access to the framebuffer, it should switch video mode instead of using the function, that way the OS knows that the program is going fullscreen.

|Mode|Color Depth|Size in bytes|
-|-
640x480 | 16  | 153600 bytes
640x480 | 2   | 38400 bytes
320x200 | 255 | 64000 bytes
80*25   | ~   | 4000 bytes

## ANSI.SYS

ANSI.SYS may be supported but does not have to be. Most programs that need advanced video features simply access the video BIOS directly. If this is invoked, the program will have to enter virtual framebuffer mode. If it is not supported, the multiplex for ANSI.SYS should be captured to report non-presence so that it is not accessed directly.

## Event Calls

The major code is 1 for kernel-to-kernel calls and 2 for userspace to kernel calls.

### MN_GET_FRAMEBUFFER_ATTR

Gets attrbutes about the framebuffer used by a process.

Returns a structure with the following format:
```c
typedef struct {
    BYTE    vga_mode;
    DWORD   ;
};
```

### MN_READ_FRAMEBUFFER

This function takes the PID of a process that has used the framebuffer and an address to write it to.

### MN_REPORT_STDOUT_HANDLE

It is necessary to inform the driver which special handle is being used to output text.
