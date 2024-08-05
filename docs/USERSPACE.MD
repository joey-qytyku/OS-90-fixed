> This is probably nonsense.

# Userspace Environment

# Shell

The OS/90 command line shell is intended to be parsable without any sort of recursive descent whatsoever. It is fully space and tab delimited and uses semicolons to end lines (the interactive shell auto-inserts them).

## Examples

```
string x "Hello, world"
```

# Commands

Commands do not have any similarities in name to UNIX or DOS. There is a very serious UNIX knock-off vibe with the command names.

## lnend

Change line endings of stdin.

```
-D      DOS/Windows line ending
-U      Unix line ending
-M      Classic MacOS line endings
```

## of

Output file contents. No arguments except the file name.

## fcom

File compare. Lists each offset in the files that are different, along with the bytes in both

```
-A      Compare at aligned boundaries (only works with -D and -W)
-D      Compare double words
-W      Compare words
-T{n}   Convert tabs to spaces
```

Example output:
```
00000000 AB CD
```

Return code is 0 if the same and 1 if different.

## ofhex

Output file in hexadecimal format.

## cprintf

Identical to the UNIX printf command, but with a `c` to indicate that it is a C library call.

## mstat

## sss

Subsystem status. Lists all installed subsystems.

## cop

Copy.

## df

Delete file.

## ddr

Delete directory and all contents recursively.

## ddl

Delete directory files at first level only.

## page

Show one page at a time for input.

## driverctl

No arguments will list all drivers loaded, their resident memory usage, and other information.

```
-LD <name>      Load driver in default path
-RM <name>      Unload driver
-RS <name>      Restart driver
```

## irqctl

Lists all IRQ handlers and their driver owners with no arguments.

## clean

Clean a string.

```
-S      Collapse spaces into one space
-T      Convert tabs to spaces
-N      Replace line endings with space except last byte (all types)
```

## e90

OS/90 text editor. Its a REALLY good one. It is not a program exclusive to any subsystem and is implemented by a ring-0 driver.
