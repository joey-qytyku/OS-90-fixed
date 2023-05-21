# Special File Handles

The default file handles will write to the VGA text mode console using INT 21H and will block the system. A multitasking window manager will not do it this way, and will close the default handles and replace them with a special handle by requesting a kernel-mode driver to do so. Special handles are similar to device files, but are only handles.

Userspace does not have the ability to create special handles, so the display driver must handle it for STDOUT and STDERR.

```
HANDLE FsCreateSpecialHandle(SPECIAL_HANDLE_PROC);
```

```c
typedef struct {
    BYTE    cmd_code;
    PVOID   xlat_address;
    DWORD   bytes;

    union {
        struct {
            DWORD   seek_offset;
            DWORD   file_pointer_loc_ret;
        }seek_op;
        struct {
            PVOID   xlat_address;
            DWORD   num_bytes;
        }iobyte_op;
        struct {
            DWORD   dos_time_input;
            DWORD   dos_time_output;
        }time_op;
    };

    STATUS  exit_code;
}SPECIAL_HANDLE_REQUEST,
*P_SPECIAL_HANDLE_REQUEST;
```

The exit code is in the structure because we want to allow for the possibility of a threaded filesystem in the future, so relying on function returns is not viable. The DOS time substructure uses two values because we will use two double words anyway.

`xlat_address` is the address of the data being transfered. It is a 32-bit linear address translated from a real mode or protected mode segment.
`bytes` is the number of bytes to be read/written or the number of bytes to seek forward. R/W will normalize to a WORD. It can also contain a 32-bit DOS time record for setting the time.
`cmd_code` is the command to perform.

The current process is the requester.

Any operations for file handles are supported:
* SFH_COM_READ
* SFH_COM_WRITE
* SFH_COM_SEEK_SET, SFH_COM_SEEK_CUR, SFH_COM_SEEK_END
* SFH_SET_DATE
* SFH_GET_DATE

* SFH_COM_CLOSE
Special handles can be closed. This can be used to initiate cleanup by the driver if necessary. An SFH cannot be opened because it is initialized as open.

How will I add DUP2 support?

The following are the return codes:
* SFH_SELF_DESTRUCT
* SFH_OK
* SFH_ERROR

Example with a:
```
VOID SpecialHandleProc(P_SPECIAL_HANDLE_REQUEST rqst)
{
    KeLogf("Hello!");

    // Tell the kernel we succeeded
    rqst->out_status = SFH_OK;
}
```

Extended error codes are not supported. Internally, the carry flag will be used to return an error code. This is not of any importance to the SFH interface, however.
## Use Cases

This functionality is primarily for the possibility of a multitasking windowing system where program can output to independent console windows. This can also be used to implement device IO for userspace. A driver could recieve an event call from userspace.

# Display Drivers

Display drivers must conform with this standard to be compatible with existing OS/90 software. This specification is designed to allow for multitasking of programs that request direct access to the framebuffer. Non-accelerated displays, including VESA, are supported by the display driver model. The display driver model is userspace directed and provides useful functions to window managers.

## Mode Switch Behavior

When a video mode is switched using INT 10H, the display will actually be switched to that mode and the program will be fullscreen with direct access to the actual framebuffer. The interrupt call still has to be trapped to ensure that memory is mutually excluded and mapped appropriately.

If the mode switch is to monochrome or color (03H)

If a program attempts to access VGA registers, the driver must give it exclusive access.

## Pre-empting fullscreen programs

It would be hard to multitask if fullscreen-only programs could not release control of the screen until termination. For that reason, the display driver must support the ability for userspace to switch back to the default mode.

## Virtual Framebuffer Management

Some framebuffers belong to DOS programs that have attempted to access video memory. Any framebuffers controlled by a windowing system are none of the business of the display driver but instead that of the window manager. The userspace can write to the main framebuffer after compositing the buffers. The framebuffer can only contain text mode memory for either monochrome displays or color and only 80x25.

"DOS Boxes" can be implemented using virtual framebuffers, but the userspace must direct this process.

## ANSI.SYS

ANSI.SYS may be supported but does not have to be. Most programs that need advanced video features simply access the video BIOS directly. If this is invoked, the program will have to enter virtual framebuffer mode. If it is not supported, the multiplex for ANSI.SYS should be captured to report non-presence so that it is not accessed directly.

## Input Event Calls

The major code is 1 for kernel-to-kernel calls and 2 for userspace to kernel calls.

### MN_SET_ACTUAL_MODE

### MN_READ_VFB

This function takes the PID of a process that has used the framebuffer and an address to write it to.

The process calling this is not the one using the virtual framebuffer, so shared virtual address space is used.