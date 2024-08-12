# Notes

Some operations, such as getting the extended error information, require per-VM contexts.

Such could be with some filesystem operations.

The Disk Transfer Area which is used for scanning directory entries for example is decided by the program and must keep a persistent state.

There is a default DTA at offset 80h in the PSP.

# DOS Support Subsystem

## DOS Multitask Region (DMR)

A region of memory of a configurable size is allocated at the very end of the conventional memory at a garaunteed 4K boundary. It is always a multiple of 64K.

Allocating it is done using the DOS API.

- Find out how many paragraphs are left on the system
- If not enough, DOS subsystem fails to start, otherwise, continue
- Subtract max memory block size with requested DMR size
- Allocate that number of bytes (converted to paragraphs)
- Free the "bubble" buffer

This region of memory is garaunteed to be page aligned since there will be no space whatsoever for anything else to be allocated.

The DMR is used to multitask 16-bit DOS programs using isolated address spaces, with the rest of real mode memory being globally shared. The DMR is bank switched upon task entry with the program's memory.

The contents of the physical DMR without mapping anything do actually exist and can be used for one program.

If the program does not fit inside the DMR, there is no problem. Such programs are NON-CONFORMANT and have only one difference: they are loaded in global conventional memory instead of the DMR. They can actually be faster than DMR programs because they do not need to have the DMR mapped in.

## Subprogram Extended Memory Swap

Similar to what FreeDOS COMMAND.COM does, DSS swaps the DMR to extended memory when subprograms run. This allows programs like COMMAND.COM to have a very small conventional memory footprint and leave a lot of address space for other things. The VM remains the same and additional tasks are not created.

Subprograms are infeasible using other means. Making them non-conformant to the DMR wastes address space. Allocating them inside the DMR either requires the user to reserve more space for the DMR or the fork may fail.

The only problem is greatly increased extended memory use. A single DOS box will require 128K for the DMR (if not using physical), 8K for the VRAM, and possibly 2K for the VM block. 138K is a lot.

## Support for Virtual DOS Screens

Capturing writes to the VGA framebuffer is a complex subject since it could involve a video driver. There is no standard OS/90 video driver model at the moment.

OS/90 only supports 0xB8000 for each program's virtual screen and can handle 80x50 text mode by allocating extra memory. Switching video modes, which is sometimes done to clear the screen, is the resposibility of a video driver if one exists, or a UI environment which knows to keep track of video modes.

# INT Hooks

There will be program segment prefixes that will be exactly the same because the memory being used also is. The call for fetching the PSP does not need to be trapped, but anything related to the termination or creation of a process needs to be.

When a program is spawned, OS/90 will capture the entire call and cause the task that requested it to switch program flow to the newly allocated region.

# Subsystem Block

The subsystem block is a single pointer to a separately allocated piece of data large enough to contain the necessary information. It is no larger than 2048 bytes long.

```
FAR_POINTER_16

typedef struct PACKED {}LIDT_ENTRY;

typedef struct {
        LIDT_ENTRY      leh[DPMI_SUPPORTED_LEHS];
        LIDT_ENTRY      lidt[256];
        FAR_PTR_16      rm_livt[256];
        FAR_PTR_16      rm_service_stack;
        LONG            virtual_if;
        PVOID           virtual_device_list[32];

        SHORT           ee_code;
        BYTE            ee_class;
        BYTE            ee_action;
        FAR_PTR_16      disk_xfer_area;
        PVOID           dmr; // NULL if using the "real" one
        PVOID           subproc_dmr_copy_stack[8]; // No index, NULL indicates available
}VDOS_BLOCK;
```
The program segment prefix is not registered with DOS in any way (not that it would be, it is just a structure) and is created by the DOS subsystem. A maximum of eight subprogram nest levels are supported by using a stack.

Executing subprograms can only be done using memory outside the DMR.

# DPMI

OS/90 supports DPMI 0.9 with some 1.0 features and reports 0.9 to software.

This part of the document lists each DPMI feature and how OS/90 implements them.

# MS-DOS Extensions

https://docs.pcjs.org/specs/dpmi/1991_03_11-MSDOS_DPMI_EXTENSIONS.pdf
