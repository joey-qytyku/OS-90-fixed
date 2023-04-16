# System Calls

Native NXF applications have access to the system call interface. The interrupt vector is 80h. DPMI applications do not have access to this API.

Parameters are passed in the registers. Some types of arguments are placed in designated locations, but this does not apply to all functions.
```
EAX = Function code (AH=Functional group, AL=Subfunction, Rest is zero extended)
EBX, ECX, EDX, ESI, EDI = Arguments in specified order
```
This cannot use regparm and requires inline assembly to make C callable.

Return values:
EAX = Exit code
Any other registers are specified in the list.

# Functional Groups

* 0x00: Information
* 0x01: Filesystem
* 0x02: Processes
* 0x03: Memory

# Full List

## KE_VERSION
## KE_DOS_VERSION
## KE_GET_CPU_TYPE

## KE_CREATE_DEVICE_FILE
## KE_IOCTL

## KE_OPEN_FILE
## KE_CLOSE_FILE

## KE_MEMMAP_FILE

## KE_DUP
## KE_DUP2

## KE_DUP2_ON_BEHALF

Because programs memorize the aliases of the stdio file handles, it is necessary to provide a mechanism to intitate forced duplication on the behalf of a process. This can be used for virtual terminals.

* EBX: Handle to duplicate from
* ECX: Handle to duplicate to, belonging to the other process
* EDX: PID to do on the behalf of

If the PID is the current program's, it will behave as expected.

## KE_CREATE_SPECIAL_HANDLE

## KE_GET_SELF_PID
## KE_EXECUTE
## KE_TERMINATE
## KE_EXIT
## KE_SET_SIGNAL_HNDLR

## KE_ALLOC_PAGES
## KE_RESIZE_PAGES

# Examples

```
Main:
        mov     eax,KE_WRITE
        mov     ecx,14
        mov     edx,strHello
        int     80h

        mov     eax,KE_EXIT
        xor     ebx,ebx
        int     80h

strHello:
        DB      "Hello, world",10,13
```

