# Deprecation Note

OS/90 can run 16-bit windows applications by running KRNL286. There is no "true DPMI" compatibility planned currently for the 32-bit version to work.

# Win16 Subsystem for OS/90 (Win16/90)

W16SO is an experimental implementation of the Win16 core so that 16-bit windows applications and even the graphical environment can execute on OS/90 while interfacing with the OS/90 native API as directly as possible.

W16SO is only capable of running a fullscreen win16 environment. To switch to other programs or a different UI the video mode would have to change. This is because the GDI attempts direct access to the display adapter through drivers which I do not know how to make.

# Implementation Details

Win16 is a reserved subsystem type of a task. Each task represents one program and a kernel driver executes winbase functions.

Because far calls are used to obtain Win16 services, the kernel is able to capture all of the requests.

# Resources

https://source.winehq.org/source/include/wine/winbase16.h
215 functions to implement `:-(`. Good luck. Its less since the underscored ones are not exposed and mostly internal to Win95. Still check them though.

Some of the un-underlined ones are not found in the executable. Check every one of them before implementing.
