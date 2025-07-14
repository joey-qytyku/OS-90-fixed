> This is good but not really operational at the moment.

# TODO

Remember that it may be better to use resizable arrays rather than linked lists. A resizable array will have faster lookup times in all cases.

Linked lists can be made faster by knowing the size and the middle of the list, cutting the worst case in half.

## Long Term Strategy

- Set up source code
- Get SV86 working and TEST IT

> Major and minor function code for things? For example major is "read" and minor says something about buffering?

- How do I do this? <https://en.wikipedia.org/wiki/SUBST>

# Plan

Phase 1:
- [REDO] Set up printf debugging
- [DONE] Update PIT frequency to 1000Hz
- [TODO] Make ring-0 threads fully work and demonstrate with a complex function
- Implement basic scheduler calls and test
- Write concurrency routines or update existing ones
- Test all concurrency routines with multiple threads.
- [DONE] Add IRQ reflection to real mode and fully confirm as working (with keyboard)
    - [DONE] Debug with single step
- Add ring-3 into the scheduler (not hard).
- Test ring-3 threads using V86 primarily.

Phase 2:
- [WIP-] Design and implement SV86
    - Includes IO port emulation and INT/IRET
    - Remeber the new design choices made to ensure nested T2 execution of hooks.
- Test SV86 rigorously, including with hooks
	- Try INT 10H first
	- INT 13H - Try to read a sector and validate the boot signature
- Implement configuration editing using the environment.
- Implement real mode DOS basic support functionality. This includes:
    - MZ executable loader and associated interface
    - Stdout callbacks (can be used to send error messages to user console btw)
    - Memory allocation restrictions (not that important)
    - INT reflection. Potentially use callbacks in DOS block to optimize.
- Reclaim bootloader memory
	- Copy the PSP of the startup program and free the boot program segment
	- Free the command shell as well by following the parent link.
	- The MCB is not technically a linked list. By adding the current MCB segment with the size of the allocated memory, it is possible to scan all the memory.
- Run DOS programs such as COMMAND, EDIT, and others. Fix all errors.
	- Direct C function calls to execute programs
	- Suggest using printf to log events

- Take a break and clean up code

Phase 3:
- Redesign MM for orthogoniality. Virtual memory will be an afterthough as it needs programs to test.
- Design and implement the page mapping features
- Design and implement automatic virtual memory allocation (if needed)
- Review and finalize the memory management design
- It will be tested further by running programs

Phase 4:
- Add XMS support
- Add INT reflection from protected mode
- Exception reflection
- Implement DPMI API
- Try to get FED to run.
- Log anything not supported and gradually implement
- Run DOOM

Phase 5:
- Work on finalizing the API.
- Design driver interfaces.
- Add the API structure.
- Finish executable format.
- IO port access hooking.
- Load first driver and verify correct operation

Phase 6:
- Develop mouse and keyboard drivers
	- No realistic way to separate these
	- COM mouse may be simpler to implement, but is not widely supported by emulators.
	- Best sample rate should be made available
- ISA DMA arbitration
- Test using hooked BIOS interface
- Write 32-bit COM and LPT
	- BIOS functions exist for this and should be made 32-bit
	- If a device wants control over a COM interface, it should get the whole controller.
	- All versions of UART must be supported, and all features.
	- LPT does not need to support ECP. This is an advanced feature.
	- Direct LPT access should also be supported.

Drivers to develop:
- LPT
- COM port (with configuration support)
- 32-bit disk access
- RTC
- VGA arbitration
- Keyboard and PS/2 mouse
- Serial mouse

Phase 7:
- Userspace specification
- Executable format design
- Develop toolchain
- ???
