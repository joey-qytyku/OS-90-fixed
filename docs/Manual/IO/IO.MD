# IO Manager

Last updated: 26 June, 2024

OS/90 has an IO manager based on thread pools and priority-based request grouping. It is designed to be highly asynchronous and be configurable by the user.

All IO requests first go to the kernel. An IO Directive Block is passed to the kernel. It contains two KFS handles, buffer offsets and sizes for both, flags, timeout and delay fields, and a cancelation routine.

In response, the kernel sends an IO ticket which contains information about the request. The IO is not started yet until it is initiated.

They can be chained together to ensure that the servicing thread waits for another request to come and services it too.

## Structures

The IO directive block is like the Windows NT IRP. It is not simple the IO request parameter block, but it in fact __IS__ the IO request itself.

This is the standard structure of the IO Directive Block.
```c
typedef struct {
        PVOID           next;
        LONG            kfs_handle_from;
        LONG            from_off;
        LONG            kfs_handle_to;
        LONG            from_off;
        LONG            size;

        PBYTE           requesting_drv_name;

        MUTEX           mutex;
        PVOID           kernel_caused;
        SHORT           ms_min_delay;
        SHORT           delay_counter;

        SHORT           param_com;
        SHORT           param_subcom;
        LONG            param_flags1;
        LONG            param_flags2;
        BUFF_BRK_PROC   param_buffbrk;
        CANCEL_PROC     cancel;

        BYTE            io_group_id;
}IO_DIR;
```

`next`: This field allows IO directives to be chained and executed as one unit by the same handler thread.

`kfs_handle_x, x_off`: Buffer information for the transfer. KFS_MEM uses the system memory as a buffer source.

`size`: Buffer size.

`mutex`: A mutex that protects the IO directive from being written to.

`kernel_caused`: Allows IO requests to be done on the behalf of another thread, though that is not the main purpose. The point is to make the directive self-contained. This should be the current thead task block pointer.

`param_com`: Command.

`param_subcom`: Subcommand.

`param_buffbrk`: Procedure that is used to handle line buffering or process each individual block of data sent.

`cancel`: Called if the IO request is canceled. Never call directly.

The timing stuff: This allows an IO request to be delayed.

## IO Directive Flags

There are 2 flags. Each pair of bits represents whether or not something is needed and what the specific paramter is. Each of these macros are function macros and an argument of 1 is to require. 0 is to not require but request.

Here is a full list of flags1:
- IODF_ASYNC:           The IO call should be done in a separate thread and not block the current one.
- IODF_EXCLUSIVE:       Preemption is prohibited while handling the request. Automatically disables ASYNC.

## IO Groups

IO groups are thread pools associated with a special type of IO. They take all IO requests from a specific class of device drivers.

These are the default groups. Further assignments must be given by a function call. Groups will also have screen names.

Groups are tied to the idea of an IO class, which all drivers must implement one of.

1. IOTYPE_NONE
6. IOTYPE_ANY
2. IOTYPE_BLOCKDEV_SYSDSK
3. IOTYPE_BLOCKDEV_LOWSPEED
4. IOTYPE_PHYSICAL_CHARDEV
5. IOTYPE_VIRTUAL_CHARDEV
7. IOTYPE_BUS_DEV
8. IOTYPE_BUS_DRV

OS/90 in its default configuration only creates these groups for each of these classes.

IO groups can be assigned any number of __jobs__ to any driver that uses the matching IO class. A job is a request to execute an IO directive, which may be done concurrently or sequentially depending on the thread resources.

### Implementation Details

IO Groups are essentially blocks of locked pages used to store task structures along with some book-keeping information.


## IO Tickets In Detail

The IO ticket is an object that certifies that an IO request is ready to be handled and has resources reserved for its execution. It must be later initiated for the request to be completed.

The ticket contains opaque data that is used internally. One of the reasons tickets exist is to provide linkage between the IO directive and the internal group structures.

## API

The following functions are used for IO:
- PVOID  IO_AllocDirective(VOID)
- VOID   IO_Request(IO_DIR *iod)
- VOID   IO_Await(IO_DIR *iod)
- VOID   IO_Chain()
- VOID   IO_Execute()
- VOID   IO_Close()
- SIGINT IO_Cancel()

IO_AllocDirective does not automatically fail if there are no directive block buffers available. It will try to get one and may use a low-latency timeout.

IO_Chain adds another IO request to the list and returns the address of the thing that was just inserted. Although it is probably known what was inserted into the list, using the return value is more flexible in general and reduces code size.

IO_Close tells the IOM that there are no more requests being chained. The ticket contains the handler thread and IOM automatically yields to it. Upon recieving this call, IOM will perform the necessary cleanup to close the IO request gracefully.

IO_Cancel is for when the IO request must be terminated unexpectedly, typically due to some sort of device failure.
