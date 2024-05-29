# ELKS Compatiblity

> TODO

# Implementation

## Process Memory

## System Calls

System call emulation is done by translating the Linux calls into OS/90 calls and implementing anything that does not fit.

### Calling Conventions

An analysis of elks_sys.c shows that elks uses the following register parameter order: bx, cx, dx di, si. ax is the syscall code. The codes can be found at [1]. `elks.c` line 139 confirms that AX is the return code for all system calls.

### read, write, open, close

These calls end up using DOS handles or whatever the capturing driver uses. Parameters are piped as needed.

### mount, umount

## Signals

Signals are supported.

# Resources

List of system calls and their indices:
[1] https://github.com/ghaerr/elks/blob/master/elks/arch/i86/kernel/syscall.dat

This demonstrates the calling conventions and how the functions use their parameters.
[2] https://github.com/ghaerr/elks/blob/master/elksemu/elks_sys.c
