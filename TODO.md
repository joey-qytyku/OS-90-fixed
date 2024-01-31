Update header files and resource.c/h
V86 interface
Memory manager (per-process memory mapping and VMEM)

Should memory mapping use handles? Unix uses the address of the mapping.

Changing CR3 will always flush the cache.

If there is no FPU, coprocessor not present should terminate the process.

Add resource reserve feature (from Win95)

Sending interrupts to a multitasking DOS task?

When the page table is changed in the i386, the entire TLB must be invalidated.
On the i486 and above, the pages modified can be evicted from the TLB using INVLPG

======== 2023 Todo ========

* Finish exception handler code
* Test virtual 8086 mode

I need to have the differentiate parameter because the ISR is zero when there is a spurious interrupt. Do not change the current structure of the dispatcher.

Exceptions are now dispatched with a branch table. This is simpler and saves code size. The lower half handlers are apparently very complicated so I cannot just repeat them.

Leave the IOPB. I need it for V86. Or not?

I can merge DOS.c with Scheduler.

Tue March 28:

Am I linking old object files?

Sun Apr 2 2023

Look for anything that has a macro minus one. That is sus. <<<<<<<<<<<<<<<<<<

The for condition represents the count, not the limit of the index.

== 29 Apr 2023 ==

Now I need a proper plan so that I can finish OS/90 at some point and not turn it into a time sink.

The steps I will take:
* Get V8086 to work
* Get interrupt captures to work
* Make the scheduler and interrupt handling to work

== 2 May 2023

I must admit that I have a SERIOUS code quality problem in Scheduler.c. It is a complete mess. Maybe the comment subsections that I made are only creating more problems. It is time that I separate the file. V86 can stay in the same file, but other things have to go elsewhere.

== 7 Aug 2023 ==

Good stuff going on with the memory manager.

The scheduler though. I need to get it done.

TODO: I need to make the trap frame and user register dump IDENTICAL. There is no reason not to at this point. The rdump in the PCB can be whatever I want it to be, so if it is more convenient to make it the same as the register dump, I might as well. This allows for reusing and simplifying code.

The problems are mainly rooted in the fact that V86 autosaves the segment registers to a different place than I would if I pushed them after.

I can unify the two structures with PM and RM sregs, or I can abstract it all in assembly to save space.

Appears that my low syentry procedure is completely wrong and unfinished. It even destroys segment registers.

Okay, I will make a unified register dump structure that works regardless of the last mode.

What if the kernel changes the V86 mode in the flags register? I guess I would have to honor it.

Really? If we call a DPMI service to raw switch, the only thing that really happens is that the PCB is changed until the kernel thread terminates.

=== Dec 29, 2023 ===

I should make `include` and `implements` folders.

Additionally, I should make advanced debugging features, or OS/90 will be pure torture.

Ideas:
- The ability to debug/run a specific application with or without rebuilding the whole OS
- Support for OS breakpoints
- Console logging through the COM port
- Stack tracing
- Software exception handling with throw/catch? Can I make it a procedure within a procedure using inline ASM?
- Macros to throw fatal errors in certain situation (e.g. incorrect context type, function cannot be used in an ISR, ...)

```
    TRYING_FOR(MY_EXCEPTION, Catch, NotThrown)
        DoWhatever();
    Catch:
        return OS_GENERIC_ERROR;
    NotThrown:
        return OS_OK;
```
This would not exactly be high performance. You could just use setjmp.

Other plans:
- I need to reduce the reliance on external libraries in OS/90 drivers
- A real memory allocator like malloc(). I am sure I can come up with something nice.
  maybe use some sort of reference counting for efficient reshuffling.
  Maybe double pointers to replace references? IDK.

=== January 10 ===

Final todo list to finish OS/90 within 4 years in order of progression:

Kernel phase:
- Debugging (assertions, stack trace, logging, maybe breakpoints)
- Memory manager (virtual memory not a top priority for now)
- The dreaded scheduler
- Intensive testing of the aformentioned systems
- PnP subsystem and event dispatching

General improvements:
. Better build system for debugging drivers and applications


# January 19

Previous TODO list is kinda BS. Need an ACTUAL plan.

It looks like I need a mini TODO list for all the little things that need to be completed.

## Urgent TODO in order of importance

- Update the IDT code so we do not use the dumb copy table to put in the addresses. It clutters `IA32.asm`.

1. ~~IRQ_BASE: what will it actually be? It cannot match with the default ones. I think my headers say 0x90 but IA32.asm says 0xA0, and it sends this to the actual hardware. Why?~~
    - 0xA0 was chosen because it does not collide with any known APIs or programs. This does not matter much for real mode programs, but that is what I went with.

2. Potentially add a delay operation to the assembly PIC init code?

3. Clean up IA32.asm and make it look really nice. Do the same with all ASM code. Probably do this soon.
    - Reasses what services will be exposed by IA32.asm. Maybe create one function for all of them like segutil. The current services are messy and ugly.

4. Work on `Intr_Trap.asm`.
    - How will I handle exceptions with error code? Maybe use multi-byte nop or R/M pop instruction in its place.

5. How do we deal with ESP0? I guess the scheduler tick does. It should save the IRET frame ESP to the PCB and restore it to ESP later. How will I enter that process again?

- Consider doing Segutil entirely in assembly. It uses inline ASM anyway.

- PnP manager needs to have more than one phase. Complete redesign may be necessary.

- Read dosvm.md for any interesting information. Maybe update it.

- Documentation needs to be significantly improved. Most of it is outdated.

- DrvAPI.h needs to be updated or just delete everything and do it when you actually implement drivers

- Really just general organization.

- Underscores under internal structs. Dont put too many though or it will look bad.

## Future TODO

- Resource reserve feature
- PnP
    - Detecting COM ports and LPT?
    - Detecting free IRQs?
- How do we deal with a floating point unit?
    - FPU context in PCB
    - How do we do context switching?
    - CR0 bits, detection code
    - Most likely, I can just pretend the FPU does not exist for a while.
- CPU detection?
- Make printf `kernel` so it can be an API call.


## Ideas
- Can I have a notion of an actual IRQ being owned by process? Probably not.

- Stop adding code. Any code you add without testing is guaranteed to become obsolete or broken at some point.

We will have to check the instruction that caused a SE, so there is no need to avoid it. Unless the event code is >8-bit so that we can convery exception events.

I do not need to worry about the size of the low entry points because of our new IDT solution.

Should I go through with the whole bus idea? It seems that most operating systems let the bus software provide a good abstraction. My idea was to do some kind of strange ripoff of the Windows Driver Model by copying the idea of a bus driver but not with the endless inriquacies of WDM.

Focus on a more nanokernelish design. The focus should be on arbitrating the hardware in a way that avoids conflict. Let a bus driver determine how to dispatch IRQs and whatnot.

The kernel simply controls all the hardware and gives it to a driver. Everything else is up to the driver. The linux kernel does something like this, though the bus support is built-in. In my case, it will be a driver with its own API.

# Integrate With Documentation

The following provisions are necessary:
- Ownership of an IRQ by a driver, with an IRQ handler provided.
    - The handler needs to be there, but can be a null or error-throw handler if the IRQ has yet to be used

- Ownership of IO and memory resources by a single driver.
    - This will be required for PnP and non-PnP computers to avoid driver conflicts.

- The COM ports or the LPTs are classified as busses. We do not care much for how they handle their resources for drivers they service.

These are the classes of IRQs:
* RECL_16 :  16-bit reflect to SV86, default if IRQ not masked on startup
* INUSE_32:  Has a 32-bit PM handler
* FREE    :  Presumable not used by anything, can be taken, even shared with DOS if needed
* UNDEFINED: Undefined

RECL_16 means "reclaimable." They can also be surrendered back to real mode at any time.

The following services will be provided for IRQs:
- INTERRUPT_CLASS Get_IRQ_Class
- STATUS Acquire_IRQ(U8 irq, ISR isr);
    - Set the IRQ handler with a 32-bit handler. Works on RECL_16 and FREE_32.
- U32 Release_IRQ(U8 irq, U32 to);
    - If to = R2_16, set to RECL_16
    - If to = R2_32, set to FREE_32
- U16 Find_RECL_16(VOID);
    - Returns bitmap of free IRQs.
- U16 Find_FREE_32(VOID);
    - Returns bitmap of free IRQs.
    - No need for an inuse one. Just invert. `:-)`

## IO Ports and Memory

IO/mem ranges are not unlimited, and the limit may vary.

# January 22, 2024

TODO of last entry still holds and will be continually updated.

It would be possible to eliminate the kernel context from the process control block. Because we dont hang for reschedule and simply enter the process at will while it is still the current one. The thing is, this would make non-blocking IO completely impossible. It would also make it impossible to let a kernel thread sleep and give time to the process.
> Not necessarily.

New provision: Support for non-blocking IO by scheduling the kernel thread context and user thread context separately, with both having a idle/active state. This would allow both to execute in synch. Future proof, but not immediately useful.

We do not need to do it this way. A new process in kernel mode can be executed to do the IO. System entries go in and leave.

But what is the actual purpose of the kernel and user thread context? Why not just one? The last context saved on the PCB will always be that of the user. We are even free to read and alter it, so long as we sysexit with IRQs off. We are just arbitrarily deciding that we will save the context here instead of there.

> Does IRQ#0 have to clean the stack? YES. Or not because it will IRET to next process? And where does context go?

> If you had a dumb kernel without reentrancy, then maybe this could work.

The only reason we need a context is so that it can be copied from the PCB into the IRQ#0 stack so that we can jump to the next process.

# January 25

It will be essential to complete the V86 interface and update it to fit the new RD structure. SV86 must be fully working.

# January 27

OS/90 is officially being assemblified. I added the NASMX library into the includes and I am currently converting the scheduler to full ASM.

ASSEMBLIFICATION TODO:
- Segment Util (DONE)
- Program loader

What I will not do in assembly is the memory manager and the bit array features, since that would be too hard to do.

Assembly in some ways is easier to debug, especially with an emulator. I can single-step through instructions with no additional debugging support.

# January 28

I have an idea: WIN16 emulation.

We create our own 32-bit KRNL286.EXE that thunks all the 16-bit calls and performs all the windows-related tasks expected of it.

KRNL286.EXE will implement the entire 16-bit winbase API.

DPMI support could potentially be cut out of the OS/90 kernel and implemented as a stub application that handles all requests in userspace, thus freeing some of my time to work on other things. OS/90 then would implement a native API through INT 41h that allows for more advanced features which DPMI can be built on top of. This would get around the inflexibility of the DPMI interface.

Any occurance of the INT instruction, exceptions, or other events are processed by an event handler.

Fake IRQs are a topic I have not addressed a whole lot. They must work in real mode (IRET terminated) and in protected mode (I decide termination method). A real mode only program needs to be able to modify vectors, set local handlers, recieve fake IRQs, and have a virtual IF. This can be fully handled by the DPMI insertion.

## On the Documentation...

The docs are very outdated. My memory and the source code are far better than it. Maybe I can be like a sort of prophet of OS/90 and let eveyone else write down what I say. Eh, its inevitable.

I probably should refrain from premature API documentation until I fully understand what I am doing.

> TURN IN THE SPANISH ASSIGNMENTS AND PRACTICE THE SEGMENTS!

## Back to Scheduler Implementation, etc.

In the immediate future, the following tasks must be completed:
- Remove all SEPDO functionality and anything DPMI-related
- Make Svint86, Enter_Real_Mode, and all the other things work
- Grind the scheduler for a bit
- Implement insertions
- Consider how I am going to deal with forking processes and loading DOS executable.

The V86 capture thing remains unchanged. It only applies to when the program calls an INT in real mode by default. In protected mode, it will be an event that can be reflected. If no 32-bit handler exists, we go to SV86 and handle it.


# January 29

Note: Remeber to look at previous entries.

We need to have an actual IO subsystem built into the kernel in order to implement the API and for other high-level features.

Lets refine what I am already doing in IO.md.

## Special File Handles

The file handles for standard IO are automatically closed and never to be used. After initializing the filesystem, subsequent IO to stdio handles will have the handle replaced by a process-local one stored in the process control block.

The process local handle is where SFH is involved. The SFH allows drivers to recieve the data perform the necessary operations. This has to be done within the kernel, however. The display server must communicate with a stdio driver to virtualize IO for several DOS programs.

Sounds good. This would have to work at the DOS level in the low-level IO code.

But how do we deal with device files and other things? I guess we give the STDIO handles special treatment. High-level IO code can deal with such abstractions, such as mounting filesystem and whatnot. It will be a mere extension to the existing DOS filesystem that does not break compatibility.

## Memory Manager

I have discovered that there is no reason to use blocks of multiple pages.

If we have 32MB of memory, there are 8192 pages total. A PBT with 8-byte entries would be 64KB. This is 0.1953125 percent of the total memory, or a 1:512 ratio.

# January 30

Working on the memory manager now.

You need some way of setting the memory window to the page table entry.

So how do I do that? This requires the ability to set the window to reference the page table entry in the allocated buffer. This is why there is Set_Memory_Window_To_Chain_Local_Block. This is supposed to take the default page table chain as a parameter.

The page table chain is a simple chain that grows in size as more page tables are needed. We need a way to delete page tables once they are no longer needed. The page directory references all of the page tables.

When a virtual region is deallocated, the associated page tables should somehow be marked for deletion, perhaps by zeroing the first entry or something.

The PD can be used to find any page tables that are ready for reuse. Shrinking is necessary too. We could find some way to compact the page tables and adjust the page directory accordingly.

Do we even have to deallocate page tables? We will only need as many as a 2GB address space can handle. OS/90 virtual memory is very basic and can only swap out parts of chains to save up memory. In practice, we will never use anywhere near 2GB of addressing space on most hardware configurations, and if we do, why bother deallocating page tables?

Okay, suppose we have 2MB of RAM, the lowest end memory setup that OS/90 wont crash on.

1441792 bytes are available which are not conventional memory, or 352 pages. One page table is enough to represent THE ENTIRE MEMORY, with no need for a chain at all! Memory used for page tables is always lower than the memory available.

Suppose we have 32MB, 33554432 bytes or 8192 pages. That requires a whopping TWO page tables to represent.

We do not need more because OS/90 has a stupid virtual memory manager. If uncommitted memory is allocated, maybe __then__ we need more page tables. But if uncommitted memory is committed, it will just extend the chain it happens to be part of to match the linear address accessed by the program. The memory that is on-the-spot allocated can NEVER exceed the total.

In fact, OS/90 should not even allow a user to uncommit more pages than the kernel can allocate. It cannot uncommit individual pages. The whole point of uncommitted memory is to not allocate it all at once, not to avoid using RAM, the point is we WILL use it all, but not now. Swapping takes care of using less physical memory.

Conclusion:
- Memory manager must remain extensible and support more advanced VM functionality not currently planned

- Uncommitted memory will stay because it is good for avoiding wasted memory for allocations that are larger than needed.

- Swapping will be for when it hits the fan and the computer cant just crash. This can currently only work at the level of chains.

- There will be page tables for all physical memory and page tables for the kernel address space.

- There will currently be no support for memory mapping to arbitrary locations. No DOS application will ever use such a feature and without isolated address spaces it is quite useless.

- Virtual address spaces are allocated and released with dedicated functions. The size must be known when freeing.

Individual pages CAN be swapped out. Chains are not swapped entirely, but a chain local index is required for evicting memory.


# Reconsider Single Address Space and Other Thoughts

Maybe make local DOS memory possible? Return to multiple address spaces?

I want OS/90 to be BETTER that VMM32. Getting rid of what made Win386 work well for the time makes OS/90 much less impressive as a project.

Local DOS memory should be done. Perhaps I can copy the system VM idea and have it do the supervisory calls. That way EVERY interrupt call is locally managed for processes with an independent capture chain (which could be shared too). Then we can also make processes capable of entering critical sections. Would the latency be acceptable in such a case?

We can now discard the entire OS/90 native interface.

> Does OS/90 need a near-total rewrite? The documentation needs to go. It is probably what is holding me back. A lot of the code I have written may look nice but I think it is the right time to start fresh.

> I can make the kernel API in the function code style. That way, it is more thought-out with nothing non-essential.
