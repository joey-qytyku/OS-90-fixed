# Native API

OS/90 excludes DPMI from its kernel and is not inherently DPMI compatible. It cannot use DOS extenders since they will raw switch to protected mode and change control registers.

OS/90 provides an API that allows the DPMI host to be implemented as a userspace stub.

Features:
- Setting the behavior of various events
- Making calls to DOS
- Loading drivers
- File IO
- Process control
- Memory

# Insertions

Because OS/90 uses a single address space for all programs, low-level libraries can be implemented by simple loading them into memory and calling them.

It is a block of 32-bit code that runs in a flat segmented 32-bit memory model and has an instance of it allocated for every process that instantiates it. Insertions can be instantiated on the behalf of any process.

Insertion entry points are obtained using a system call. This is a far call table. The first entry is the initialization routine. It recieves

An insertion cannot be removed once added.

Insertions have the file extension "INS" and are flat binaries.

## Where to Use

Insertions cannot be used to emulate devices because the kernel traps IO instructions.

# ABI

## Syscall Parameters

Registers are used to pass arguments to INT 41h, a reserved vector for OS/90 API calls.

AX is the function code. EBX, ECX, EDX, ESI are the subsequent parameters. ESI is the designated memory address. EDI, if a segment parameter is needed, serves as a segment parameter           . This makes V86 translation easy. A simple bitmap is used to decide if translations should occur.

Functions assume the data segment is used as the buffer base unless otherwise stated.

## Event Handlers


# API Listing

## 0AA55h: Get OS/90 Version

## Switch to Protected Mode

```
EAX = 0000h
```

## Execute Process

```
EAX = 0001h
DS:ESI = String path of executable
```

## Get Current PID

## Destroy Process by PID

## Add Dynamic Insertion

```
EAX =
ESI = String path of executable
```

## Instantiate Dynamic Insertion for Process with PID

32-bit DOS programs cannot execute unless the DPMI insertion is loaded.

## Create New Path Handle (FF00)

Paths are objects that need to be created with a handle. The path handle may or may not be a simple 32-bit pointer to a string containing the name of the file.

This method is inspired by Java NIO. It is an improvement because there is no need to deal with buffers and memory allocation for paths. There are, however, no file handles--only paths. They represent a location and a file at the same time or just a location.

The path is automatically deleted when the file bound to it is closed or deleted.

## Link Path to File, aka open file (FF01)

```
EAX = 0FF00h
EBX = Path handle (HPATH)
```

## Write to File (FF02)

```
EAX = 0FF02h
EBX = Path handle
ECX = Size of buffer
ESI = Buffer
```

## Read from File (FF03)

```
EAX = 0FF03h
EBX = Path handle
ECX = Size of buffer
ESI = Buffer
```
Output:
```
CF = 0 : if successful
CF = 1 : if failed
```

## Delete File (FF04)

Input:
```
EAX = 0FF04h
EBX = Path handle
```

Output:
```
CF = 0 : if successful
CF = 1 : if failed
```

This will delete the file at the path.

## Concatenate Paths

Input:
```
EAX = 0FF05h
EBX = Path 1 handle
ECX = Path 2 handle (-1 if using string)
ESI = String to concatenate with (*Must* be NULL if not using)
```

Output:
```
EAX = New path
CF = 0/1
```

Create new path using an existing path and concatenate it with another path or a string.

## Get Current Working Directory Path Handle

## Convert File Path Handle Into DOS Handle

IN:
```
EAX = 0FF06h
EBX = Path handle
```

This function will convert a 32-bit file handle into a 16-bit DOS file handle. If the filesystem is in 16-bit DOS, this function will do nothing. Behavior is configured by a kernel API call.

The file may be reopened and all buffers are expected to be flushed. Do not use the 32-bit handle anymore.

## Change Directory

This will change the directory of the current process. An absolute path handle or a string "offset" may be passed.

## Load Driver

## Create Mailbox

## System Shut Down

## Set Event Handler

## Convert SEG:OFF to Absolute Address

```
EAX =
ESI = Offset
EDI = Segment
```

Works as expected for real and protected mode.

## Allocate Segment Descriptor Range

## Load Entire Segment Descriptor

## Get Access Rights

## Get Extended Access Rights

## Allocate Memory

```
EAX =
EBX = Bytes to commit (rounded up to a block)
ECX = Bytes to not commit (rounded up to a block)
EDX = Address to map to (NULL if random)
```

The address must be above the HMA and below 0xC0000000.

## Reserve Linear Region

```
EAX =
EBX = Bytes to reserve
```

Outputs:
```
EAX = Linear address of region
CF = 0/1
```

## Uncounted Lock Region (TODO)

```
EAX =
EBX = Region base address
```

This function does not have a lock count for regions, which means it is incompatible with DPMI locking. Do not lock more than once.

If successful, the memory will not be swapped out. Limited utility since OS/90 only swaps when there memory is low.

## Uncounted Unlock Region

## Allocate DOS Memory

```
EAX =
EBX = Size/16 (must be < 65536)
```

Out:
```
EAX = Segment (< 65536)
CF = 0/1
```

> The segment returned is valid in the current mode. If in protected mode, a descriptor is allocated for it.

## Resize or Deallocate DOS Memory

In:
```
EAX =
EBX  = Size/16 (must be < 65536)
```

Out:
```
EDI = Segment (< 65536)
CF = 0/1
```
> Descriptors are automatically freed if in protected mode.

## Get Block and Page Size

Inputs:
```
EAX =
```

Outputs:
```
EAX = Block size in num bytes
EBX = Hardware page size (4096)
```
