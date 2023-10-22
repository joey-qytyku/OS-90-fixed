# Special File Handles (Update)

The file handles for standard IO are automatically closed and never to be used. After initializing the filesystem, subsequent IO to stdio handles will have the handle replaced by a process-local one stored in the process control block.

The process local handle is where SFH is involved. The SFH allows drivers to recieve the data perform the necessary operations. This has to be done within the kernel, however. The display server must communicate with a stdio driver to virtualize IO for several DOS programs.

## Note

Research how stdio works if more/less bytes are recieved than requested. If I read 10 characters from stdin, will it just stop once it reads 10 or will it save the data for the next read?

It seems that it allows for partial reads and writes because the INT 21H function will return how many bytes were read.

DOS will terminate the read command for stdin once it finishes recieving the number of requested characters. A special characters are also be sent, such as newlines.

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

This functionality is primarily for the possibility of a multitasking windowing system where programs can output to independent console windows. This can also be used to implement device IO for userspace. A driver could recieve an event call from userspace.

# Display Drivers

There is no specification on the display driver model. It needs to have a protocol that can communicate with the display controller.

# The Display Controller

The DC is a program which controls access to the display. It directs the CONIO driver to create special handles to manage IO for DOS programs and instructs the video adapter driver. The DC must support the protocol of the VD and can support several different driver architectures if necessary.

A DC needs a video driver in order to create local framebuffers for DOS programs that require it. The actual framebuffer can be accessed directly.

The DC must declare itself as a DC so that the memory region for the framebuffer is the real one.

## Fullscreen Video Mode Switching

If a program switches video mode, the BIOS routine for that purpose must be called. The only issue is with this is how we go back to the original one, since DOS programs will assume the 80x25 mode.

Local video mode table?
