# INT 21H Monitor Implementation Details

This document explains how each function in the INT 21H interface is implemented or monitored under OS/90. It is an effort to establish correct behavior in writing.

## AH=00h

This is the old way to terminate a program. It it illegal in protected mode.

Passes control to INT 22H if not the only subprocess, otherwise terminates the session.

## AH=01h

Reads a character from stdin.

The JFT is used to implement redirection, and it is globally shared between concurrent DOS processes.

## AH=02h

Writes to standard output.

^C is not really a major problem. It is set to an invalid value in the local PSP so that ^C crashes if it goes unhandled. It is set by the shell normally.

## AH=03h

Read from stdaux. No remarks.

## AH=06h

Prints a character to stdout. Not sure why it exists. DOS 1.0 was not capable of redirecting it.

## AH=07h

## AH=4Bh

The entire MZ loader is implemented with 32-bit code for performance reasons.

> Described more in the other doc.

## FCB-related

OS/90 does not use FCBs at all. FreeDOS emulates them. OS/90 will probably prohibit FCBs.

## AX=3305h

Get boot drive. Redirected as normal.



## AX=3700h

Gets the command line switch character. This is a process-local context in theory but it is almost always slash.

## AX=4400h


