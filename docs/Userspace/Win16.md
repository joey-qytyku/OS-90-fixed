# Win16 Subsystem for OS/90 (W16SO)

W16SO is an experimental implementation of the Win16 core so that 16-bit windows applications and even the graphical environment can execute on OS/90 while interfacing with the OS/90 native API as directly as possible.

W16SO implements winbase.h in the form of KRNL286.EXE. This contains core functions related to memory allocation and module/instance management.

W16SO is only capable of running a fullscreen win16 environment. To switch to other programs or a different UI the video mode would have to change. This is because the GDI attempts direct access to the display adapter through drivers which I do not know how to make.

# KRNL286 Implementation

KRNL286 is implemented in C. It is a 32-bit program that uses 16-bit thunks to perform API calls.

# Resources

https://source.winehq.org/source/include/wine/winbase16.h
215 functions to implement `:-(`. Good luck. Its less since the underscored ones are not exposed and mostly internal to Win95. Still check them though.

Some of the un-underlined ones are not found in the executable. Check every one of them before implementing.

