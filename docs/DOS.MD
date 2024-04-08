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

## Support for Virtual DOS Screens

Capturing writes to the VGA framebuffer is a complex subject since it could involve a video driver. There is no standard OS/90 video driver model at the moment.

OS/90 only supports 0xB8000 for each program's virtual screen and can handle 80x50 text mode by allocating extra memory. Switching video modes, which is sometimes done to clear the screen, is the resposibility of a video driver.
