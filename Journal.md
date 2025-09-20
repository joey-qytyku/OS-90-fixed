
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


## Reconsider Single Address Space and Other Thoughts

Maybe make local DOS memory possible? Return to multiple address spaces?

I want OS/90 to be BETTER that VMM32. Getting rid of what made Win386 work well for the time makes OS/90 much less impressive as a project.

Local DOS memory should be done. Perhaps I can copy the system VM idea and have it do the supervisory calls. That way EVERY interrupt call is locally managed for processes with an independent capture chain (which could be shared too). Then we can also make processes capable of entering critical sections. Would the latency be acceptable in such a case?

We can now discard the entire OS/90 native interface.

> Does OS/90 need a near-total rewrite? The documentation needs to go. It is probably what is holding me back. A lot of the code I have written may look nice but I think it is the right time to start fresh.

> I can make the kernel API in the function code style. That way, it is more thought-out with nothing non-essential.

## New Protocols

ASM is not very include-heavy. Usually include files are used to define macros and other things.

I will not:
- Put extern/global declarations in headers
I will:
- Import all procedures in the imports section
- Use double lines for code sections, use single lines for procedures

Can I give procedures function/instruction notation?

Find a way to pre-process assembly with python code. That would be pretty sick.

# Jan 31

I think we should add exception hooking or something. We need to split up as much of the virtualization as possible. Chaining exception handlers could be effective. Maybe split user and kernel exceptions.

That way, we could emulate virtual IO.

We can keep the RECL_16, INUSE_32, etc... instead of copying whatever Win386 does. Actaully, I have an idea. We can give the handler a choice. It can decide to emulate INT and IRET behavior for a process, or simply run a regular INT procedure.

Separate the expected stack behavior from the kernel if you can. Make OS/90 modular. Just like your insertion idea, but at the kernel level.

## Part 2

Process-specific hook behavior should be the responsibility of the driver alone. We will not make them local by default.

If the system process deals with SV86 calls, it will cause significant latency but will make a lot of sense from a design perspective. We can add a feature to switch to a process manually if needed.

There will be a need for process-specific critical sections which have to at least be interruptible or it will never exit.

# Feb 2

I think I should avoid the whole python assembly thing. I already have NASMX macros, which should be more than enough.

## Microkernel?

Just an idea.

Because I am starting fresh, I can completely throw out whatever I was previously doing and make OS/90 a microkernel OS. I have thought about how drivers can be used to divide the OS components. It seems like the natural progression is a microkernel design.

The compatibility with DOS does not actually make it that much harder. It is simply a different way or organizing the protected mode components.

### Possible Design

The scheduler would have to be very different. I think I will create a "Virtual Machine Manager" subsystem, but unlike VMM32 of Win386, it is a ring-3 server that time slices its own virtual machines. The VMM will get the most real time slices since it runs userspace processes.

The VMM is capable of implementing address space swapping simply by using the kernel API features.

Some parts simply have to be part of the kernel, such as memory management and whatnot. It will not be a true microkernel design, but it is microkernel-inspired.

### Events, Messages, etc.

We need queues and asynchronous events.

Events may include occurances of the INT instruction, exceptions, etc. The IDT should go to something similar to my system entry idea so we can enter a T2 regardless of V86 or other things.

We could also consider reentrant exception handlers. Probably not though. The advantages are just not really there since most exception handlers will lock a critical resource. We can still allow different types of exceptions to be serviced concurrently, though servicing the same one is definetely impossible. Occurances of the same exception would require locking to work properly.

The issue is about what events actually are. Where do they come from and how are they serviced?

Does the process send it or does the kernel? Do we need a central dispatcher?

The process is the server. VMM raises an event because it executes userspace code. Maybe split VMM from run environment?

Runtime Server and Emulation Server? (TASKRUN.SRV, EMUBRIDG.SRV)

Will there be a notion of event "ports" and plugging them? Requiring some of them to be used or driver will not run?

> Maybe do something with the concept of an object like in WinNT

> Sending events requires context switches. We need the ability to immediately switch context. Remember, it can just be a jump location.

We need to save the trap frame of the requesting server into a requester register dump and enter the target server by setting up the trap frame for it and.

> Mirror process memory AND process registers in high mem?

> Priority queues. Also, we need to have a real scheduler algorithm.

> Coop schedule servers in same address space? Other programs are other processes? Yield using windows-style message dispatching.


# Feb 3

## Cooperative Multitasking

It is technically not impossible. If a DOS program waits for input, that is essentially a hint to let other processes run. Even DOS programs that are fullscreen will poll the keyboard using the BIOS.

The process can be in kernel and user more, and both can yield.

The kernel can be entered by multiple threads, locking is different. If a lock is held, an implicit yield is required.

V86 hooks can decide if they should yield or complete and go back to the process. IO should 100% yield in a tight loop while simple information calls or installation checks can be completed entirely.

Preemptive multitasking can also have yielding but this has the potential to be just as good as preemptive multitasking, at least if round robin is used, since every process will do IO.
> We can go back to C?

Priority scheduling is possible and a good idea. If a thread acquires a lock, it should have its priority drastically reduced to prevent excessive context switching from the tight yield loop. Priority is a simple counter of how many times the yield operation will be a simple no-op when currently in the thread. Maybe we can make it even lower by having a negative number, where abs(priority) is the number of times to skip scheduling

# Feb 4

No idea where this project is going anymore.

My kernel is going to transition to a very minimalistic exo kernel-esque design. Almost everything will be done using drivers. Even exceptions will be hookable events.

The whole thing will be implemented as one single executable file.

Okay, separate the initcode from the IA32 stuff. Make a procedure and call it. Or dont and do something else with sections?

I do not want to use the bss sections whatsoever. I need to find a way to have a macro that reserves to bss.

The idea of having code close to the data that it uses is a brilliant way to reduce TLB pressure and it should happen. BSS, however, should be ignored entirely and be placed all the way at the end so that it takes up no space and can be zeroed.

This way, we do not need to even know about text and bss.

## General Design Ideas

OS/90 will move toward an exokernel design. This means the kernel provides minimal abstraction and allows software to take direct control over hardware and implement abstractions that are more appropriate.

The kernel:
- Does not handle or dispatch interrupts except for the timer.
    - Instead, a function for getting the base vector of the IRQ handlers and setting them is presented.
    - A special driver must arbitrate the IRQ lines.
- Does not deal with events like INT calls
- Exceptions are hookable events that can have a chain of handlers. Their default behavior is to terminate the process if a handler does not "swallow" the exception.

## API Calling Conventions

Use the stack.

# Feb 5

OS/90 will not technically be an exokernel, since it does not "securely" multiplex hardware, but it does multiplex hardware.

Exokernel does not necessarily mean 100% minimalism. For example, we expose a low level API for the PIC so that drivers can set their own IRQ vectors. We do not need a driver for something like that. We could even make the scheduler and memory manager into drivers. But why do this? There is no benefit to such separation.

Exokernel means that direct hardware access to userspace must be emphasized. I still need to figure out how this will work.

I can go back to the idea of implementing DPMI in userspace using special services. This type of feature requires kernel mode to regulate access, and that is where the nanokernel design is involved.

Processes can be given prehooks to set the TSS IO permissions and emulate devices, or give direct access to one of the devices. For example, if a program directly accesses the COM port rather than use the BIOS API for it. In such a case, the COM port interface must be fully given to the program. Another program trying to use the BIOS call for COM output can be terminated or preferrably blocked until the directly-accessing process terminates.

## Memory Manager

I am not sure if I want to do isolated address spaces. While it greatly increases the amount of virtual memory space, there is a lot of TLB flushing.

Isolated address spaces allow for massive allocations and for the the majority of the addressing space to be completely virtual. In the SAS design, pages can be individually swapped out, but this will only happen when under pressure. We will never be able to swap out more than can be allocated.

A scenario with 2MB of RAM and 16MB of swap would not function. We cannot allocate memory that does not exist.

> Am I interpreting this correctly?

In the previous design, individual pages can be evicted and marked as not present. They can then be removed from the chain. The issue is that a chain is pointed to by an index rather than a handle.

This means that we cannot swap out the begining or the entirety of a chain. The first page must be allocated because it points to all the other ones.

If you give each PFE structure an ID, you can cut up the chain however you like as long as the links are preserved. You can even have a chain that does not exist at all and we know that it needs to be allocated the moment we access memory pertaining to it.

The actual swapping mechanism is another thing to consider. If a page is swapped out, the memory it occupied is free to use for other allocations. When a page fault is caused and in needs to be replaced, the page is allocated and put back in the chain it belonged to. But how?

In my previous design, the PFE entry contains a local index. This allows for uncommitted memory to be possible because #PF can find out how many pages to allocate and put back in the chain. This requires being able to allocate with an existing ID. It also requires a way of finding out which chain a page belongs to, or at least used to.

It looks like PFE entries need:
- An ID (U16)
- Local index (U16, can be smaller)
- Next (U16)
- Prev (U16)

This makes entries quite large.

FREE and LAST_IN_CHAIN can be interpreted as the next being zero (logically impossible) for the last and the previous being zero. Or use all ones interpretation?

We may also want the ability to access allocated memory without mapping it anywhere. Could we make the raw memory available at a certain address? YES. We could access linked list structures with ease and never have to change any mappings. Its perfect! This mapping will be permanent and never change and is the size of the main memory.

Also note the self-referencing page directory trick. It allows for changing the PD entries

> We may want to support the global bit.

### Single or Multiple Address Spaces

I do not have any ideas for multiple address spaces. We may have to do it though. Think.

If we do this, process memory should be accessible in a shared address space. This allows the kernel to get the advantages of a SASOS without being one, though IPC requires kernel involvement.

## Vectors and String Pools

OS/90 will have highly efficient pool/slab allocation. It will support dynamic arrays with n-length. Associative arrays may also be implemented.

## HMA

The high memory area can be allocated by drivers. Only one can have it. 65520 bytes are safe to access and use for DMA.

## Plan for MM

The scheduler will be cooperative and most of the emulation/translation stuff is done outside the kernel. The hard part will be the memory manager. I need to fully write down all the ideas and make this possible.

I will go with separate address spaces for userspace processes and a shared address space for the kernel.

There will be a kernel page directory and every process will get 16 or so page directory entries for 64MB of addressing space. These are copied to the kernel PDE using a pre-hook. The self-referencing page directory trick is useless in OS/90 since there is only one page directory which we always know the location of.

The kernel and bootloader will be adjusted to execute at address 8000_000h. The kernel and drivers receive no more than 1GB of addressing space and the rest, which starts at 0C000_0000h, is the raw memory region (RMR). The raw memory region is useful for traversing linked list structures and other memory management data which does not need to be mapped anywhere in particular. The RMR region is generated according to the amount of extended memory on the system but includes the entire real mode-addressable memory. Only the necessary number of page tables are used.

The problem with the RMR is cache polution. Because associative caches use virtual addresses, aliasing occurs and cache may be wasted. We do not want to disable caches or invalidate TLB entries either because that would make functions that use the RMR slow.

The most basic requirement of the memory manager is the ability to allocate page frames. This can be done using a giant list of pages. With the new design, we are looking at 8-byte page frame entries.

Little changes. A very tiny section of memory is used by the pfmap. We may make a provision to exclude the real mode memory entirely from the map since it will be taken anyway.

Suppose we have 2MB of memory. Minus 640K and there are 1441792 bytes or 352 page frames. 2816 bytes are used to store the pfmap.

pfmap entries can form chains using back and front links. This is necessary for implementing virtual memory and uncommitted memory.

### Address Spaces

The kernel permits the reservations of page ranges in the kernel and userspace memory to ensure that nothing else uses allocated virtual address spaces. Bit array allocation can be used to do this.

It is not enough to delete the ID and break the chain to release memory if it is mapped. The kernel does address space and page frame allocation separately and does not remember the size of a region. This means the size of an allocation, its chain ID, and its virtual base address need to be kept track of by a high-level allocator. Size and base address are not atypical

Sometimes, we want to change the mappings of a process, perhaps without it being the currently executing one.

> Note: we need at least SV86 to work. Exception handlers need to handle SV86.

> Are there available bits in the local descriptor table entries that can be used for allocation free/in-use?

### List Implementation

Lists use the RMR for all transfers and are intended for data of power-of-two sizes smaller than a page. They are highly space efficient and have optimal data alignment whenever possible. Lists never have to fully reallocate like std::vector in C++.

Lists are created with driver API calls and return handles. The handles must be passed by reference and not by value since they may actually change to hold certain information, such as a memoized size to avoid traversal of the list. This is fine because the address of a variable can be pushed to the stack.

The current design features a 20-bit size and 12-bit ID inside the handle. This allows over a million elements and 4096 lists.

Lists support remove and insert, with push and pop being special cases that work with the top. There are special list types suited for different data types.

List_64 is implemented as a doubly-linked list and stores 64-bit integers. Each entry has a pointer to the next and previous integer.

```
struct {
    U64     value
    PVOID   next;
    PVOID   prev;
}
```
This structure is 16 bytes long.

# Feb 6

I will continue to write on the previous entry.

## Scheduler

Should I really go with cooperative? There are advantages.

I can simply implement yield functionality inside a preemptive scheduler, as well as priority levels. This would allow much better locking semantics without having to implement that weird implicit yield idea.

Well its not actually an implicit yield. It is explicit, but transparent to userspace. The idea is that drivers can decide how much priority they need and if their task needs a certain amount of time. The fine-grained control is much greater than what even time slices would allow.

## Naming

Maybe I should use "virtual machine" instead of task or process? That fits better.

Or no. The kernel could make its own tasks. Why should I name it VM in that case?

## Memory Manager

Do PFEs need to have an associated TID? I think they should so that they can be deallocated when the process terminates. The same does not apply to virtual address spaces, however, so this begs the question of should I even bother doing this.

Probably not. The system service responsible for the subsystem should be implementing the memory allocation method as is appropriate for it. If such behavior is specifically required, than let it be implemented separately. Termination hooks can take care of everything else. No forced abstractions.

Local addressing spaces are going to be implemented. This requires allocating arbitrary numbers of pages.

## Kernel Build

I should probably just go back to using ELF. I will not be able to link C code with it properly (for printf support). I can redo KERNEL.ASM since its nothing but copy and paste of existing code.

Linker script can remain as is. The text section will be used for all ASM code. `.init` should be reintroduced.

## Userspace Interrupts

I think it is of high importance to allow userspace and not just drivers to directly access hardware. This is per the design of exokernel operating systems. For example, if a program wants direct access to the keyboard by using IO ports rather than a BIOS call or DOS IO, it should be able to control the IRQ for it as well.

Of course, x86 does not allow ring-3 to handle IRQs. This can be achieved by finding a way to DIRECTLY call an ISR within a ring-0 context, except it is running untrusted userspace code.

There are lots of potential problems with this, however. The ISR would have to be very well behaved, accessing ONLY one device at a time, never causing any additional system event, and never re-enabling interrupts since OS/90 fully disables them. It may send a direct EOI signal. A line of defense against IRQs being re-enabled is to mask all of them and restore the original mask.

The advantage of this approach is rapid IRQ response, though with the cost of switching CR3 twice. This is still less costly than a threaded IRQ or scheduling a simulated IRQ.

We may need a way to do a "softer" IRQ that is scheduled rather than executed in an unsafe context. Still, this idea is conformant with the exokernel design and works well with DOS. Programs that directly access hardware are usually the only programs that need to.

## Real Mode Interrupts

Sending interrupts to real mode is important default behavior that needs to be present in order for DOS drivers to work unimpeded. They should also be reclaimable by 32-bit drivers.

The mask register of the PIC can be used to determine this. On startup, every IRQ is masked except for IRQ#0 for the timer and perhaps the floppy controller and other standard hardware. Free IRQs can be detected simply be checking which interrupts remain masked.

### BIOS and Cascade

IRQ#2 is sent to IRQ#9 or something.

## Floating Point Unit

We must support the 80387 FPU and later revisions. They both handle exceptions differently. The 287 (which can be installed with an i386) and 387 both use an IRQ to signal exceptions.

The i486 changed this and added an exception vector, but maintained compatiblility. CR0 has a bit called NE for numeric error.

> Reentrant exception handlers cannot handle multiple INTs.

# Feb 7

Same as yesterday. I will write to earlier journals. This TODO is becoming a developer diary. That is great, because I can write down any ideas without cluttering the official documentation.

## System Entry Design

Reentrant exception handlers are possible and necessary for OS/90 to function properly. This means multiple tasks must be able to enter an exception handler, and that yield calls should work normally. There is no need to separate exceptions from INT calls. Is OS/90, the INT instruction will have no default behavior and will cause an exception (each IDT entry is marked as ring-0).

Multiple instances of the same exception should be possible as long as critical resources are locked or the instance does not yield.

This makes me question the role of mutex locks in the first place. One is needed to guard the kernel from IRQ reentrance, but is probably going to be more of a counter.

Contention over a resource could be handled by not yielding during the critical section. This has the same problem that disabling preemption does on a preemptive multitasking OS. It does not let other tasks run because they could access the same resource.

> Each RD structure will have an entry for the yeild and system entry location for easy access.

## Organization

I will use multiple files for the kernel now, but I will NOT use multiple directory levels.

There will be no .text or .data to avoid confusion. All initialized data, executable or not, will go in a section called `.CODE`

## TSS

I can make the TSS part of the TCB. It will not be used the normal way. The register fields beside ESP0 and SS0 are simply the RD structure.

This allows each process to have a local IOPB with no additional costs. The TSS also contains the stack pointer/segment and a LINK field which we can use as intended but in software.

Just remember that the stack contains the actual user registers. The TCB contains the saved registers. It does not need to overlap with the TSS totally, as long as it can be copied flat into it when task-switching.

We have one major problem though. A full IOPB is 8192 byte long, which is WAY larger than a TCB can possibly be. With a size like that, it might as well be allocated separately. I think it might be best left to the subsystem drivers to decide how IO should be dealt with.

But no. IO port access should be handled by drivers, and some of them need to decide how ports are accessed.

## OS/90 Boot Low

Could I allow OS/90 to be booted low or in the HMA? This would require two separate bootloaders, but is doable. I no longer expect the OS/90 kernel to be particularly large. It may not even be larger than abput 20K.

# Feb 8

## Interrupt Handling

OS/90 will have the ability to direct interrupts to userspace by direct calling. This must work for all processor modes.

x86 has no restrictions when it comes to which ring can handle interrupts. The only problem is with real mode. Pentium VME could make it possible, but we must support i386 and i486.

It looks like only protected mode will be able to handle interrupts. It will be a non-interruptible context that cannot generate system entries. IO port access MUST be completely direct for whatever device it needs to access within this context.

A way around this is to use TSSes for the interrupts, but I am not really interested in that.

Oh wait, there is a problem if you just use an interrupt gate. CR3 is not automactically updated. Maybe you SHOULD do single address space?

## Resources

Could OS/90 be APM or even ACPI compatible? ACPI is very complicated. It has a whole system for resource allocation. The kind of information that shows in the device manager interface. In such a case, should OS/90 return to supporting resource reservation ranges?

Interrupts (and potentially DMA channels) would have to be enumerated too.

The thing is it has to be ACPI or PnP/APM BIOS. We cannot really do both. Interrupt ownership may become obsolete as an idea.

The point of PnP and ACPI is plug and play functionality by automatic discovery of devices and driver loading.

ACPI and PnP use completely different methods of conveying hardware information. A driver may have to know about it. Interrupt classifications should be rentered only around behavior and not ownership.

Interrupts may become hookable, so level 2 handlers are needed.


## Compact Lists

The kernel keeps a so-called "compact list" which is essentially a List64 that contains information about other internal lists. It is wasteful to have to allocate 4K just to store some integers.

# Feb 9

## The TCB Structure

I will standardize and document the TLB enough that direct access is possible. We do not want to want to clutter the API with too many getters and setters.

## Compact List

Make it work the same as a chain. It can store 32-bit values instead.

## Kernel Stacks

3K of stack space just isn't enough to cover all situations.

The kernel stack will have to be larger. This means potentially expanding the TCB structure back to 8K but at a 4K boundary.

## Boot Up Information

Show memory statistics.

Also, should I keep track of which process owns what memory? This seems potentially useful for diagnostic purposes.

## Bus Abstractions

The whole idea of the exokernel does not require there to be absolutely no abstractions. The emphasis is on direct access being easily facillitated by a driver. THAT is the point of the exokernel, or at least that is how it seems.

We can create a bus driver that provides an API for a particular bus, like PCI or ISA. The device driver is supposed to latch on to access attempts by the userspace.

But how can a disk, for example, be accessible directly if a userspace process cannot enumerate the PCI bus? Will it be allowed to? I know some utilities may do this.

Some programs try to access standard resources that could be reconfigured. In such a case, only virtualization of the hardware is possible, or even flat out termination of the process. If the ATA disk was remapped, direct acess simply cannot be allowed.

## Inquiry Into Segmented Memory Model

It is possible to "trick" the GNU linker into generating executables of a two-section code/data memory model. A separate stack segment is unfortunately impossible because passing pointers to automatic variables as arguments to procedures will only pass the offset and thus be incorrect.

Using the segmented model means does not change the location where the kernel is mapped.

Trying to remember why I thought about this. It had something to do with copying a Win16 design concept. In Win16, there is something called an instance. Basically, a DLL or EXE has a single code segment but can have multiple data segments. This allows them to execute using the same code segment without having to use TLS or something like that and avoid excess memory allocation.

Somehow, I though such a feature would be valuable to OS/90. Perhaps there could be a section of data that acts as TLS.

We could just avoid segmentation entirely and implement a real TLS. A region of data could be automatically replicated and accessed using a segment override.

```
INSTFIELD LSEG_REL PU32 LSEG_REL example;
```
This is a pointer inside the instance to another value inside the instance.

### Disadvantages

I am not sure why the kernel itself would need to use the segemnted models and I can think of reasons why it should not. Accessing the RMR would be much easier without segments. It is also better to avoid having to use far pointers.

Implementing task local segments cannot be done easily without using a segmented model for drivers.

The only potential issue with local segments is that I need to find a way to optimally allocate them. We also need a way to free them when the task using it is done.

> What will the name be?

## Pool Allocation and Objects

> Can we just trash task local segments and use pool allocation instead? YES WE WILL. It woul make memory management way too hard.

The only problem we face is the fact that we have to start the pool with at least 4K automatically. If we create many different pools for different objects, it would add up fast.

What about pool element handles? Maybe make it work like Win32 objects? Call them objects too?

## Throwing Exceptions?

In assembly, throwing an excpetion is as simple as jumping to a label. It may be an easier way to handle errors.

## Heaps

It is unlikely that we need proper heaps since the memory facillities of OS/90 should take care of most use cases for the kernel.

If I can make a very efficient heap manager with lock/unlock operations, the rest of the features may not even be needed.

# Feb 10

## Heap Manager

I would like to make a heap manager based on handles rather than pointers.

### The Use Case

Memory compaction would be done for the same reason as memory swapping. Most operating systems will only swap out pages when main memory is under pressure since the hard disk is much slower.

In theory, the OS could swap out things that are the least used and "cache" other things. Windows is known for doing this, and that is why it has notoriously high memory usage. Thing is, the kernel really does not deal with any of that. Userspace has access to low-level functions that allow for precise control over virtual memory. The filesystem in Windows is accessed through the windows API, so a userspace file caching system is totally feasible and that is probably what is already done.

Anyway, back to handle-based memory allocation. We will only compact if there is no more memory to allocate on the heap. OS/90 can impose a soft restriction on the heap and attempt to pop bubbles to fit it.

If the allocation is very large, perhaps greater than a page, we might as well allocate off-heap entirely. Large allocations in the midst of smaller ones are the cause fragmentation, so we should avoid that and allocate multi-page memory in regions that accomodate that size of data.

Popping bubbles when a threshold is exceeded periodically is viable because it saves memory.

### Far Pointers

If we needed to point to allocated data within, suppose an array of structures with those pointers, it would be impractical to freeze every block since that would take way the main advantage of handle-based allocation.

## Virtual Memory

I think I need to fully understand how "swapping" actually works. For the virtual memory system to actually work, we need the ability to allocate move virtual memory that physical memory.

## New List64 Op

List64_Iterate

## Thinking About The Design

What is the role of DOS in OS/90? I think we need to have an SV86 capture capability. Subsystem drivers are expected to make calls to DOS because OS/90 does not implement any of its features. FS, IO, etc. are specified in the DOS interface. For that reason, we need the ability to hook this functionality for the low-level drivers to potentially emulate this behavior.

The other thing that must be considered is how we manage to get these subsystems to work together. Part of that is already taken care of by using the DOS interface. OS/90 and drivers are essentially a translation interface.

On startup, we will need a task that serves as INIT. What subsystem will it have? Or we can have a kernel-mode user interface?

## Driver for GUI?

A driver can implement the GUI. There is no need for complications with user-kernel communication in order to execute programs and whatnot.

I still am not sure how the GUI will look.

## Win16

Win16 is easier than I thought. It is not much more than a fullscreen DOS application and can be done simply by running `WIN.COM`. KRNL286 is 16-bit DPMI and will not touch anything 32-bit.

# Feb 11

I have a few ideas.

## Do it in C?

We are obviously rewriting the whole OS. Strange how much I, as a programmer, enjoy removing code more than writing it. The world needs more code deleters.

If I do it in C, some special considerations need to be made. I have changed the whole API paradigm to prefer a small set of functions that basically never change under any circumstance. This will not change.

## Pascal Strings

It is possible to generate pascal strings in C using this macro that I made. Pascal strings are far superior to C strings in most ways.
```C
#define PSTRING(name, value)\
    asm(".data\n");\
    asm(#name "_len: .word " #name "_end" "-" #name "\n");\
    asm(#name ": .ascii " "\"" value "\"" "\n");\
    asm(#name "_end:");
```

Or we can switch over to Pascal and use that. No, I like C more.

## Clarification on Exceptions and Events

Exceptions that occur in kernel mode are handled separately from user events, though they do use the system entry procedure.

Every INT is a fault when the user does it. An exception handler is never directly invoked either and also causes a fault, though it is possible to find out which vector was invoked through the error code of #GP.

The handler chain will cooperate to handle the exception. If an INT is encountered (this MUST be checked first) in [CS:EIP], then it is an INT and the sysentry is serviced and terminated.

> Swallowing a handler means simply not passing to the next one.
> Also not that handlers for SV86 and system entries, while unrelated, can modify inputs. A SV86 handler can modify parameters and then proceed to bypass the whole chain.

## SV86

It must be possible to:
- Pass to the previously installed handler
- Execute an SV86 INT call with or without the capture chain
- Swallow the INT, which is NOT done by system exit

SV86 could be implemented as a regular task using an SV86 subsystem, though the subsystem would not matter much.

# Feb 12

Wow, the bit array allocator was quite hard to make in assembly. Still does not work. I think I should do it in C at this point.

What else should I do in C? The scheduler is probably best done in assembly because of all the jumping stuff. The complex algorithms (which should be isolated when possible) can be done in C to avoid wasting time

## Assembly Debugging

We will need the ability to log to the console. It should be able to print out the contents of registers.

ASSERT will also be necessary. It can be implemented using the NASMX IF macros, which I do not think require the use of PROC/ENDPROC.

I plan to (ab)use assert as much as possible in debug builds.

```
; Examples

LOG "Hello, world %i", ebx

ASSERT expr
```

printf must be called directly because INVOKE can only be used in a PROC/ENDPROC block.

The printf used here is reentrant, so it can be called in an ISR. Interrupts are disabled when calling printf to avoid interleaving the output.

Implementations
```

%macro LOG 1+

%rep %0
    %ifstr %1
        jmp %%over
        %%thestr DB %1
        %%over:
        push %%thestr
    %endif

    %ifidni %1 == eflags
        pushfd
    %else
        push %1
    %endif

    %rotate 1
%endrep

%endmacro


```
# Feb 13

I implemented the bit array allocator once more in C but in a more assembly style.

Making the helper procedures static caused inlining to happen and the generated code looked quite nice. I see no reason to do it in assembly. It is better than my old code since it does not have an additional pointer return and using only a single return value, making it a bit faster.

Now I can move on to the parts of the OS that matter and can actually be done in assembly.

## What Needs to be Done

A new TODO is needed because of the rewrite. The kernel is only a small part of the OS now.

## Event Handler and Subsystem

Separating the subsystem from the event handler in the TCB does not make sense. Instead, we will replace the subsystem and event handles with the SSDB, or the subsystem descriptor block. The SSDB will contain the actual event handler.

## IRQ Handling

I would like tasks to be able to implicitly hook an IRQ and directly recieve it. This is not really possible since the IDT can only have one entry.

## An Idea

Can we make OS/90 preemptible with a yield feature? Cooperative tasking just isn't predictable enough.

Now for an even crazier idea. 100% preemptible and reentrant interrupt service routines. Even the scheduler tick. Everything.

With the number of spinlocks this will necessitate, a proper scheduler with not only yielding, but also priority adjustment, will be necessary.

Or we can use cooperative scheduling and do the exact same thing. It does not matter as long as I get what I want.

### A New Context?

If I want interrupts to be preemptible, they have to somehow be linked to the concept of a task. This means we need an interrupt context (since the stack saved registers and TCB RD will not be enough) and we need to properly keep track of what the last mode was.

An IRQ could preempt the kernel context or the user context. The thing is, it will use the same exact stack. IA32 does not have interrupt stacks.

I think there is a reason why most operating systems do not make interrupts preemptible, at least not without scheduling the creation of a tasklet or other threaded context and simply returning. IRQs are supposed to be fast. If not, they can just schedule a task.

### Thinking About IRQs

What makes an IRQ so different from a software interrupt? The difference is that it is not immediately known where the CPU was before it happened. Software interrupts can be declared to be a ring-3 affair that will always generate a system entry.

In practically every operating system, IRQs are either done with interrupts totally off or they are an atomic context. In the context of linux, interrupts always block preemption.

### Conclusion

Just make interrupts IF=0.

## Scheduler

I am still not sure about multitasking. Preemptive has its advantages. If I can add a yield functionality or just generally implement the idea of "switch to this task now" as a procedure, it can work just fine.

What is to stop an interrupt service routine from calling a yield itself? If the yield point disables interrupts, it can only run once. In such a case, an IRQ can call it. In such a case, the IRQ might as well be able to induce context switches in any way it pleases. What could possibly go wrong?

Yield can be broken up into a goto_task(currentTask->next) operation with a register specifying the next TCB address, and yield proper which is a special case of goto_task that goes to the next one.

Once again, IF=0 while we are in this context and it will until IRET loads EFLAGS from the TCB.

Now we can make interrupts capable of causing context switches. They have access to the same stack frame that a kernel thread does. An interrupt can even enter a specific process and have that one handle the IRQ for it right away!

The IRQ is NOT able to go to a process and in any way return. That would make no sense.

> So basically, we are implementing a setjmp/longjmp except it has control over the whole CPU :)

> Any function that is non-reentrant can be made safe by disabling interrupts while inside it (not before calling).

## Do it in C?

I was impressed by the code generated for my bit array allocator by GCC and think that C could still be useful for OS/90 in the kernel. As stated previously, we are making a very small kernel with very limited responsibilities.

C has the advantage of being more self-documenting and flat-out being way easier than assembly. But what about the low-level components? How can we implement the scheduling features?

```
```

### New Style

To emphasize the fact that this is a rewrite, maybe I should change the style. I can do anything I want to. My choice to imitate the Win32 style, at least at first, was arbitrary.

I like using the Whatever_Case style, but maybe I should just go back to the snake_case. It looks good with acronyms, which do not need to be capitalized, and is very readable

typedef struct and typdef enum should no longer be used. Always use a simple struct or enum to prevent any kind of confusion as to what that type actually is.

Type names can be snake_cased.

What about integral types? I already know the size of those types, but why should there should be any use of `int` unless the value is genuinely signed? Signs affect the behavior of comparisons. I do not want to write `unsigned` all the time though.

In most cases, the sign will not matter much. size_t can be used whenever dealing with the size of something. Bitwise is unaffected unless a shift is done to perform a multiplication.

The previous types U8, U16, and U32, along with their pointer counterparts, convey more information than needed. For a loop iterator, it really does not matter what type it is.

> Remember to place asserts before loops to ensure no overflow

I can also reduce the tab length to 2 for all C code. It makes it easier to fit everything within 80 characters in a line. It also makes it easier to have files side-by-side. Finally, 2-space tabs give off a certain energy, IDK.

### New Style Decision

We will use integral types with the knowledge of how large they are, but introduce unsigned variants.

```
int   = 32-bit
short = 16-bit
char  = 8-bit
```
```
uint
ushort
uchar
```

## Name of Kernel Image

I picked KERNL386.EXE because it sounded cool and was similar to KRNL386.EXE. It obviously is not a real EXE in any way whatsoever.

I think it should be changed to something else so we do not need the "protective RET" and reflects what it actually is.

Ideas:
- KERNEL.BIN
- KERNEL.IMG
- KERNEL.SYS
- OS90.DAT

OS90.DAT is good. It is opaque and matches OS90.COM, although I am considering writing other bootloaders.

## Other Bootloaders?

- OS90LOW.COM: Load kernel in conventional memory.
- OS90HMA.COM: Load in high memory area
- OS90.COM:    Load in extended memory

Loading in conventional memory is a viable option for machines with little memory. OS/90 cannot use conventional memory for anything after booting.

## Minimum Requirements

OS/90 will have reduced system requirements after the recent changes. On a barebones configuration, 1MB of memory should be enough to run DOS programs.

# Feb 14

Writing in advance. Check the previous comments on scheduler.

## Scheduler

I do not need a "current" process pointer variable. The current process is in the TCB.

I want to avoid the unnecessary stack calculation. The __seg_fs override can be used, but it must be on the left side of the asterisk so that it is "a FS-relative pointer" instead of a pointer at a FS-relative address.

The FS segment base address address represents the current

## Scheduler Notes 2

Is it possible to copy the register dump from the registers to the TCB directly.

In fact, this may be a better solution than the current plan.

Entering contexts is a little more complicated sinc we have to push sregs in case of V86, but it should be perfectly possible to just copy the information directly to the TCB. Note that GS will not point to the process at this point in time, but the TCB can be accessed through a stack segment override.

The only problem is the interraction with interrupts. The IRQ does not know if the kernel or the user was interrupted and therefore does not know where to put this.

Unless you want to make two contexts for user and kernel, this may not work out. Previously, the stack was essentially the previous user context.

I may want to use two contexts.

Back to interrupts. They will have to somehow figure out what they just interrupted. This can actually be kept track of within a global variable. Previously, when OS/90 used a non-preemptible kernel, this was used to detect when the scheduler is okay to switch tasks, since it could not switch from kernel.

The only problem arises with virtual 8086 mode. The stack HAS to be used when returning. There is also the rest of the IRET frame. There would have to be redundant copying in this case, and V86 would have the most redundant copying. Not to mention we would have to keep a variable to remember the last context when that could be avoided.

Therefore:
- The master IRQ handler will push registers onto the stack and create the stdregs structure on the stack.
- System entries will do the same.

## Exceptions and Interrupts Notes

I cannot continue until I establish exactly how interrupts and exceptions will work.

### IRQs

IRQs are simple. The IDT entry points to the master dispatch routine and the proper level 2 handler is called.

There must be an ability to call an ISR that is implemented by userspace. IDT entries can reference ring-3 code segments and there is no architectural limitation preventing an interrupt from vectoring into user mode. The only problem is with setting the CR3 register.

And by the way, we ARE doing multiple addressing spaces. It really is not negotiable at this point. But doing SAS makes it easier to implement ring-3 interrupts. A SAS will need less book-keeping data in memory

I also do not have a clue as to how I would do multiple address spaces. I understand the page directory entries will be stored in the TCB.

## Memory Manager Notes 1

Linked lists wth 64-bit elements will be very important. It should have a master pointer table for each individual one that signals the start of the list. The MPT should keep track of the size and potentially keep a pointer to the last item.

## MM Notes 2: Address space decision

I am still not sure.

Local address spaces will require us to be able to allocate address spaces for each task. If each task has a 64MB address space as is configured now, it will have 16384 pages, which needs 2048 bytes of bit array data to represent.

As stated previously, half of the address space is reserved. The kernel and drivers get about 128MB of space and the RMR is 1GB.

Linux mmap allocates memory off-heap. The only way to extend the address space of the process is to use sbrk.

> The kernel never swaps. Keep that in mind. No kernel these days swaps on its own.

> Bit arrays are not needed for address space allocation. Either use a spare bit or define pages that reference NULL as unused. Probably spare bit.

### Arguments In Favor of MASOS

- Does not sacrifice the idea of virtualization of susbystems. Each machine is more isolated.
- Larger address space in general
- Better stability
- More impressive

### In Favor of SASOS

- Faster context switching
- Simplified memory management design
- IPC and kernel-user communication without mediation
- Its good enough for Terry A. Davis so why not?
- I can focus on more important parts of the OS.

### Final Decision

Single address space.

Practically every OS that could run under a subsystem can execute without making any assumptions about addess spaces.

# Feb 15

## Exception Handling and IRQ Update

I can send IRQs to protected mode userspace either by direct IDT call or by a far call.

An IRQ vector can be owned by a driver or the kernel, which both can do whatever they want with it.

The kernel may reflect the interrupt to DOS or handle it natively. The only interrupt used by the kernel is IRQ#0.

## SV86

SV86 needs to be completed soon.

## Scheduler

## A Realization

It was correct when I separated the event handler from the subsystem. The event handler idea itself is wrong too and cannot possibly allow drivers to be implemented without knowledge of the subsystem.

Subsystem-specific drivers are to be avoided and will usually be for virtualization rather than actual devices.

The way we are actually going to do it is implement a chain of exception handlers.

The only exception that actually matters and will ever be invoked by a user process is #GP.

Oh wait. We may actually need to recieve the other exceptions. Do exception handlers require a DPL to be 3 to be called by the userspace?

So if we have a divide by zero in ring-3, do we need a ring-3 DPL in order to call the handler? I will test Linux then and see if INT can invoke a divide by zero using INT 0.

I just tried it. INT 0 causes linux to segfault the process. This means that the Linux kernel catches the attempt to invoke vector 0 __directly__ and prevents it.

What this means for me is that I can 100% set the DPL of the exception handlers to 0 and in fact, I totally should in order to be able to catch DOS calls like INT 21H.

I guess the DPL defines what is allowed to call it using INT and friends, so it does not apply to exceptions. Otherwise, Linux would have caught a divide by zero.

## Exceptions and Interrupts, For the Last Time

Interrupts are simple. Nothing more than a simple IDT vector that points to a handler of the kernel of the dirver. It is possible to make valid ISRs in pure C using some macros and inline assembly.

```c

void isr(struct stdregs *regs)
{}

ISR_STUB(stub, isr);

```
The compiler can generate local variables and do whatever it wants on the stack. It uses STDCALL and cleans the stack.

There is no level two. The vector is directly accessible.

### SV86 Interrupts

The entry to SV86 process disables interrupts. If an interrupt does occur during SV86, it is possible to service it in real mode. If the SV86 flag is raised, the stack will be accurately emulated.

Note that in SV86, IRET and INT are handled appropriately. (Can we switch out the GP handler when in SV86? That way, we avoid having to put a bunch of if statements)

## Early Boot Information and Input

Add some routines for printing to the text console, waiting and recieving input, and things like that.

The printf implementation I am using should work just fine if I use a global function pointer. Actually, I think it has a variant that takes a putchar function. Yes, it has fctprintf, which takes a function pointer.

These will use BIOS routines through SV86 exclusively.
```
early_printf();
early_wait_enter();
```

# Feb 16

## Random Idea

Can virtual memory be simulated on computers without an MMU using hot patches/fixups combined with a master pointer table of sorts?

For example:
```
int LOCK_SEGMENT
DW selector

```

Can we use virtual handles that map to physical handles? Virtual handles are generated at compile time and are inserted into a table.

Can it be more like paging?

Specify which register to use for address?

## Driver Loading and other Things

I should add support for loading drivers in to conventional memory.

Also, there should not be separate data and code sections. They should be merged to avoid the alignment penalty.

Could we compile the kernel in "low mode" and have it run exclusively in conventional memory?

> NOTE: If .data and .text are merged (bss obviously will not be), variables in .data have no alignment garauntee! I am not sure how necessary this is.

## Task List

If the task list requires preemption to be disabled when accessing, there is no need for a lock. We do not want the scheduler to switch tasks since the current one is being accessed (and should be blocked, which it is automatically).

It is possible to individually lock the tasks. Only the links between them are a shared resource.

If we want to lock one task for accessing, the adjacent tasks should also be locked.



> Note: Giving tasks an ID instead of just using a pointer is kind of... pointless, because tasks use pointers to reference each other. This means there is no chance of them moving anywhere. Go back to using pointers are task IDs.

# Feb 17

So how do we actually recieve exceptions? Userspace obviously cannot call exception handlers, so the same way as we have done it before.

A table that writes a value indicating the exception index. The IDT entries will point to each element of the table.

## Bank Switching DOS Programs

It is possible to allocate a region of DOS memory toward the very end and use it as a DOS multitasking region? This would be done by the subsystem.

The bank switching would be done using a scheduler hook, and we would check the kernel reentrancy lock before changing any page mappings.

The advantage of this is that the code/data segments of DOS programs would not waste any additional memory that what we already reserved.

Reserving is done by finding out the largest allocatable block, subtracting that by number by the size of the multitasking region, and allocating the difference. The next allocatable block will be a segment that represents the load base of all other programs. The filler block is finally deallocated.

Remember that scheduler hooks can chain, and in this case, video memory and address spaces are both virtualized.

This is a brilliant idea. It overcomes the single address space limitation in a way that fits the design of the kernel and is tailored to the specific subsystem.

How much memory should be allocated? I think the command line string of the driver can specify, but 128K should be more than enough for most programs.

If a program does NOT fit in the DMR (DOS multitasking region), it is allocated and executed somewhere else and is memorized as a "non-conformant" task.

## Notes on Accessing MMIO Regions

MMIO regions must be mapped from a virtual address to its physical address. This includes real mode-addressable memory.

## Task List and Task Locking Notes

Tasks only require serialized access within a critical section when modifying the links or the status.

Preemption needs to be disabled so no thread unexpectedly tries to terminate a task or modify its contents.

Reading or writing anything in a task besides the current one AFTER system entry is undefined.

Conclusion: no need for locking individual tasks. Multiple tasks CAN be serviced by the kernel concurrently.

## Synchronization Primitives

I need to add implicit yield and priority reduction behavior to spinlocks. The default primitives are just inline assembly functions that probably incur very high latency. We need high-level callable procedure variants since this would be quite complex.

```
spinlock_acquire()
spinlock_release()
spinlock_try();
```

These will be prefered functions.

Advanced spinlocks can be implemented that have task signalling. When the lock is release, a thread currently sleeping can be awakened.

Maybe introduce waiting lists. A certain fixed number of tasks can be waiting for a lock to be release, and when it is, they can be awakened.

Tasks now can be in sleep mode, where they will not be scheduled unless

## Time Slice Length

How long will a time slice be? The PIT can run at many frequencies. 1MS

## Callable Contexts
```
Could you implement entering a task like a procedure, where IRQ#0 performs a "return" to the caller? Then you could have a sort of execution loop.

The control loop would not run as part of a thread. It is a sort of shadow program that cannot be stopped.
```

The only issue is how exactly I would implement yielding. This will require the notion of a current process. I guess simply change the current process to the next one. Or instead change the pointer to current to the task with IRQs off?

```
// Volatile? Nah.
struct task_desc_blk *current;

// What if preemtion is off? The timer ISR should not return then.
while (1) {
  for (int i = current->timeslice; i >= 0; i--)
    run_task(current);
}
```

### Implementation Notes

run_task is a bit complicated, since it could enter a different context types.

Whatever I have going on with SV86 is applicable with the rest of the scheduler. It is basically a setjmp/longjmp. We may need to rename those V86 TSS regs to something universal. The difference here is that IRQ#0 will deal with it instead.

The procedure to go back to the caller involves simply copying the values of

# Feb 18

## Interrupts Again

Why not use high-level interrupt handlers? I get that I do not want to force abstractions, but forcing each IRQ to have a stub with a bunch of pushes and pops involved seems wasteful. ISRs must conform to a specific design so that we can switch to a task from an ISR in a consistent manner.

This will also allow for proper handling of spurious IRQs by writing the "actual IRQ" into a variable and passing it to the high-level handler.

The whole reason I tried to have individual interrupt handlers without any abstraction was so that IRQs could be directed straight to userspace. This is still possible!

Remeber the DOS semaphore? How exactly do we handle it? Well its really the responsibility of DOS should an entry to DOS be made by SV86. If an IRQ happens while in SV86, what are we supposed to do, especially if it tries to make an INT 21H call? It is unlikely that it would, though.

## Tick Frequency

Consider the speed of a 80386. About 20 MHz. The scheduler tick at 1MS (or 1 KHz) intervals is running at 100MHz frequency. It will not dominate the majority of a clock cycle. I think 1MS resolution is the gold standard here and I should think about that when dealing with time slices.

# Feb 19

## Kernel Call Interface

The kernel is compiled with options to not save ESI and EDI before calling a procedure. This is done intentionally because it has been observed to reduce code size. Push/pop codes are not generated unless absolutely necessary.

The problem is that changing compiler configurations may cause incompatiblity with drivers and that drivers have to push values it wants to save when we can avoid that.

The solution is to have a call interface that saves all registers except for EAX, which is always the return value. Long return values are not supported.

This type of ABI is not natively supported in C and will not yield any code density benefits for drivers if we just call directly. A wrapper is needed.

I want there to be header definitions so that I can see the parameters with names. A macro like API() can be used to ensure that it works.

`__attribute__((no_caller_saved_registers))` should work. EAX is still the return value and can be clobbered. The function defined with this saves all other registers. A function pointer type can also be defined in this way to ensure correct behavior.

The API call table is a different topic, though. Can we define it to be at a certain location? Maybe it is at the start of the kernel image (4K with 1024 entries)?

Now we need a way to call these functions. The way I had previously should work just fine, just with some tweaks.

```
static const struct _systab * const krnl = 0xC0000000;
```
The actual call mechanism is complicated. The current system cannot work anymore since it is not called directly. A call number is possible but somewhat wasteful. It is only one single 32-bit value being pushed among many others, so call number it is.

The top 16 bits will be the driver ID. This ID can be fixed up with dynamic linking.

Dynamic linking works only for the driver name. Calling the kernel is done by simply using the `krnl` structure.

```
struct thedriver_api *otherdriver {0};

otherdriver = get_functions("EXAMPLE");
otherdriver->example_function();
```

# Feb 20

## API Entry Interface

The previous idea written in C does not work. Separate function pointers for each procedure cannot be done. It is not possible to specify default call parameters on a function pointer, unless we happen to pass the index of the entry.

So what is happening here? What is inside the table? Unless you use a wrapper and pass a function pointer, nothing makes sense.

I know that a function typedef is required. I can define a function pointer prototype?

```
void ($set_irq_mask*)(ushort) asm("0x80000003");
```

$ means it is a kernel interface function. GCC allows $ in identifiers so no problem there. A header file will have all of these definitions.

We need to organize all these entries into a table.

But how do we add it to the table AND create the alias?

### Thinking about it more...

Think about it as a procedure. The driver needs to CALL something. Suppose I go with the call number option. How do we know which function was called?

The idea of using the dollar sign thing is not possible if we use the call number. I kind of want to do it that way, though.

Can we do fixups but using a function pointer? By analyzing the contents of the memory call instruction, we can insert a fixup to a relative 32-bit call. It requires writing a padding NOP.

Nice idea, but I am still clueless as to how I am supposed to implement this.

If you want to change the ABI of a function, you NEED a THUNK. There is no other way. How we are supposed to generate this wrapper is a complicated matter. The dollar sign thing can be the thunk.

We can do something like this:
```c
// exportmeplz.c
void exportmeplz(void) { ... }
// exportmeplz.h
#ifdef _APIDEF_
THUNK(exportmeplz, void);
#endif
```
The actual thunk is a mere call to the function with inline assembly. Here is some NASM-style pseudocode.
```
exportmeplz__:
    push ID
```

I could generate the table in real time if needed. It can START as a table with direct pointers and then be adjusted automatically to go through a thunk.

The actual thunk should work something like a hook.

You obviously cannot use multiple functions with a single thunk wrapper. We also cannot know what entry of the call table was used.

Can we use weak aliases? Constant expressions?
Make $ a macro?

### And Some More...

If you really want the whole dollar thing, keep in mind that you can use a macro define for it. This may simply reference the global kernel call table and call the exact function that is needed.

Internally, it should PASS the call table entry to the actual dispatcher, which can be the very first address in the table.


This means we have to list every API call in dollar sign form. We can do it in groups though.

```
struct systab {
    _A(exportmeplz);
    #define $exportmeplz krnl->exportmeplz
};
```
But the THUNK!!!! How? Inline assembly? Something like the way we handle the IDT?

Also note that pushing can be proceduralized by simply jumping to the push point and back. If so, we could generate a thunk for EACH function.

>

### Symbol Tables

Understand how symbol tables are actually used. We will have several relocations point to the symbol. This means that only difficult part is keeping track of every reference within it. Then we have to find the symbol in whatever we are exporting to and its relocations and just copy it.

# Feb 21

patchable_function_entry(n,m) is an attribute that inserts NOP statements. The M parameter is th enumber to insert BEFORE the label of the function! Bingo!

The patch will call the function below it (with a 16-bit offset override) after pushing the registers to the stack. Or, it can push an associated ID.

I should do it the simplest way possible. Just put them in a call table and fixup the call table to be eight bytes or so before the actual function. I can then make simple declarations for each function.

We can use pusha and popa.

Actually, we can NOT. This will restore EAX too.

```
push ebx
push ecx
push edx
push esi
push edi
```
This is only 5 bytes. Same with popping. Expand the patch region to 16B. 10 bytes for push/pops leaves 6 bytes for the rest. Can it be done? Call is 5 and ret is 1. Perfect.

EBP does not need to be restored since the callee does that.

Note that variadic function will automatically use cdecl. The caller cleanup must be done directly after the function call. This means that separate conventions must be used for something like printf.

printf will be the ONLY variadic function in the kernel API. I have no plans for anything else. In such cases where a variadic function could be used, an array pointer is prefered since it is more versatile and less prone to user error.

The next topic is how we actually create the API call entries. I STILL want to do the dollar sign thing.

We can generate the API table as a structure. Then a dollar sign can be used to establish the function pointer typed call interface. It will use a constant pointer to a constant with static visibility so nothing is generated in `.data`.

The API_DECL macro will have to change so that it uses the driver-visible ABI, but it remains essentially the same.
```
API_DECL(void, exportmeplz, void);
```

Creating the entry in the table, using the same ADD macro:
```
struct _systab {
    _A(exportmeplz);
};
```

YKW? I dont want the whole dollar sign thing anymore. I do not want to type "$" and immediately get blasted with all the functions the kernel exports. We will do it Java style with a bunch of subclassifications. Well, probably only two levels of dots.

Here will be the classifications
* sched
    * irq
    * v86
    * taskctl
* mem
    * pgframe
    * vm
    * container
    *
* deb
* drv
* misc

# Feb 23

Technically, I did not totally suspend the project. I am just not going to be actively coding. I will write down ideas to use later.

Putting the NOP statements before the function label is not technically required. If we use a multi-type NOP somehow,

The reason this might be viable is because the function may be aligned by the compiler for performance.

But what is the typical alignment? Also, we are optimizing for code density. If I need a function running fast, I can manually align it. GCC will usually align to 16 bytes, which is exactly the size of the patch point.

Can we use the previous idea of having to "prep" a driver executable? It can work. We need some kind of special program. The problem is that the driver will have to be rebuilt

(IRQ0 updates BIOS time?)

## Regparm

Regparm globally seems to decrease code density. It is not possible to zero extend a narrow immediate value with MOV. However, I am not fully sure. The best way to use x86 is generally to maximize register usage and occasionally push to the stack. I will check my bit array function and see the generated code.

It also does not necessarily improve performance since complicated functions will have to push registers anyway.

Regparm does make trivial functions much faster. The code for the bit array allocator uses less instructions with regparm, as well as a static version of the get bit function. I could just use the inline hint.

If register parameters become universally used, we can avoid having a special driver ABI sicne registers will probably be clobbered anyway.

(mno-accumulate-outgoing-args should be applied just in case btw.)

## Types

C allows for the constant qualifier to be bound to a typedef. I can make all variables constant by default like in Rust.

Pointers are tricky. I can make a typedef for them.
```
mpmInt // Mutable pointer to int
pInt // Immutable pointer to immutable int
mpInt // Mutable pointer to int.
```
This also makes arguments constant by default.

Converting between mutable and immutable types in expressions is possible.

What about structure types? Maybe go back to typedefs. Use a macro to make the definition.
```
typedef struct {}TYPES(NAME);
```
This would generate pNAME, pmpNAME.

Double pointers would be difficult with this, but I avoid double pointers anyway.

## Back to Regparm

Stack calling conventions are already used by my assembly code, so I should add some kind of asm_link attribute.

They also tend to use CDECL.

# Feb 24

## Subsystem-aware Drivers

If I make a mouse driver, this driver will expose an API to set the size of the screen, control mouse sensitivity, and whatnot. It will also have some way of reporting mouse positions relative to the screen.

The difficulty is making this work with the DOS subsystem. The mouse API could be hooked, but special parameters need to be provided in order to control the emulation behavior for tasks. The mouse API has many different settings.

The graphical cursor settings in the mouse API may also need to be supported, which requires making assumptions about the screen. This may require access to VM framebuffers as well.

We may also need to deal with direct hardware access for the mouse, including direct IRQs.

## SV86 Hooks

Why do I even have an array of hooks? Why not just let each hook procedure recieve the index? This is way more dense and probably has little performance penalty issues.

Also, do we need to make sv86 entry function nest-capable with a special control path for nesting?

## Static Analysis

I am thinking of adding static analysis to the build system to catch errors. I am specifically looking to accomplish this by using some kind of attribute system.

For example:
```
_interrupt_ void isr(void *r)
{}
```

Functions that are not marked as \_isr_safe_ are not permitted to be called inside the ISR.

When it comes to output and input parameters, a pointer to an immutable argument automatically insinuates an input, since the function cannot alter it. Typical C programs do not have this, so it makes sense to have annotations to indicate that.

I just figured out that defining an enumeration inside the return value actually defines the constants and the enum itself inside at the file scope.
```
enum Color { RED, GREEN, BLUE } get_random_color(void)
{
    enum Color ret = GREEN;
    return ret;
}

int main()
{
    Color c = RED;
}
```

## Restrict keyword

C supports restrict, which is a qualifier that indicates that during the lifetime of a pointer, nothing else will also be pointing to it.

This allows for some optimizations and compiler warnings when such behavior is violated.


## API Idea

Use function codes lol. The entire interface must be specified with utmost precision. Numerical function codes are the best way to do this.

The regparm ABI must then be reconsidered. Where does the function code actually go? And the problem arises when thinking about how the call parameters are supposed to be copied. Probably not that hard to do, but functions with three parameters will always have to push to the stack.

I can create a giant enumeration for them.

FYI regparm uses the following order: EAX, EDX, ECX. The rest are on the stack. I think EDX comes before ECX because functions that take two parameters might be using a pointer and a size argument, and potentially running loops with ECX. Quite clever.

Regparm is okay for the kernel. The code density of drivers is more important than the code density of the kernel. That is why I tried so hard to have a driver ABI designed around that.

I also do not want to ruin pointer-and-count/whatever-style functions because ECX would have to be used if EAX is the function code.

### EBX is callee saved?

Yes it is. Just tested it. This was probably decided because EBX is often used as an address register and does not need an SIB byte when doing so.

I will disable that.

### Questioning te whole callee save thing

Last time I did code density tests, I had a lot of procedures that were not implemented and simply returned. That may have leed to inaccurate results.

I will do some experiments with REAL programs and see what happens.

### Conclusion

Stack calling conventions for drivers. Register conventions for the kernel wherever possible.

This is because of the function code phenomenon require a register move or stack spill for functions with a pointer and number parameters list that have loops using ECX. It is also more code dense to use stack operations.

## Testing

I picked a the bootprog floppy image generator. It is 500 locs and does a lot of structure manipulation.

### 1 -O2

- Text: 8164
- Data: 368

### 2 -Os -mgeneral-regs-only -mpreferred-stack-boundary=2

- Text: 6050
- Data: 364

It is quite clear that only -O2 is worth testing any further.

### 3 -Os -mrtd -mgeneral-regs-only -mpreferred-stack-boundary=2

- Text: 6018
- Data: 364

### 4 -Os -mrtd -fcall-used-esi -fcall-used-edi -fcall-used-ebx -mgeneral-regs-only -mpreferred-stack-boundary=2

- Text: 6154
- Data: 364

### 5 -Os -mrtd -fcall-used-esi -fcall-used-edi -mgeneral-regs-only -mpreferred-stack-boundary=2

This time, we are not call-clobbering EBX.

- Text: 5982

### 6 -Os -mrtd -fcall-used-esi -fcall-used-edi -fcall-used-ebx -mgeneral-regs-only -mpreferred-stack-boundary=2 -mregparm=3

This is with no register callee saves AND regparm 3.

- Text: 6134

### 7 Same as before but without EBX being call-clobbered

- Text: 5966

### Regpam, the usual options, and EBX, ESI, EDI are SAVED

Same as before? How?

### Disclaimer

I used my PCs gcc, not the cross compiler. This is mostly a RELATIVE benchmark to determine how certain options affect code generation.

### Final Analysis and Conclusion

GCC apparently loves to use EBX. It may even be calibrated internally to do so, who knows. Allowing EBX to be call clobbered causes a massive change in code size. 3 vs 4 shows an increase by 136 bytes with just call clobbering EBX. 5 vs 6 shows a difference of 168 bytes.

OS/90 will have the following internal ABI for maximum code density and hopefully performance:
- Regparm conventions for first three parameters. Passed in order: EAX, EDX, ECX
- EBX is callee-saved
- All other registers are fair game except EBP and ESP
- Stack parameters are pushed in the cdecl/stdcall order, aka in reverse of apparent
- Variadic functions require caller cleanup and do NOT use registers

## API Calling Final Decision

I give up trying to make it more code dense. There is no reason why drivers need to recieve a pointless performance penalty. We will bring back the previous call table and drivers will use the same calling conventions as the kernel.

It is possible to fixup the modrm CALLs and turn them into immediate relative calls by fixup, followed by one NOP byte.

Doing this may require patachable function entries, but less that our idea of appending 16 bytes to every exported function.

The patch code simply pushes the call table index and saved EIP onto the stack and calls a function that performs the necessary work.

If I do it using calls:
```
push dword [esp]
push MyIndex
call dofixup
```
This fits in 10 bytes. If I use a hypothetical special INT call, it is merely 7 bytes.

The findings are clear. The only way to patch indirect callers is to reserve an INT vector for that purpose.

There is the potential problem of trying to get the value of the indirect call address and use it later. Compilers may do this to improve performance and avoid extra memory accesses. This is handled by checking that the opcode is a valid indirect call.

> A few tests on godbolt show that an extern with the prototype extern `const void (*const test)();` does optimize using registers to call. We must make sure that there is a const..

A valid call [address] instruction is formatted as`FF` followed by the bits `xx010xxx` Really we only need to check for 0xFF since an inc dword [address] instruction which encodes to the same 1-byte opcode will not call anything.

Calling a register leaves no patching opportunities since the code is two bytes. It must simply be ignored. Such behavior is unlikely, however, since functions are not typically called multiple times. The exception may be loops that cache the location.

> Note that the kernel should not call the API table. This would be totally useless, but would not malfunction.

An indirect call is much faster, so this is a worthwhile optimization. The only penalty is indirect register calls being slower and 8 bytes for every exported function.

Actually, I can make it even smaller. I am using an INT call. It is possible to change the EIP to continue the function.

This means that the whole thing can be done in under four bytes
```
int Vector
DB Index
nop
```

The INT handler is written in assembly and is unlike any other IRQ or exception handler in that it does not actually save registers unless required by the ABI. It implements the patch operation.

> IRQ range is A0-AF. B0 will be the FIXUP vector

# March 24

I am not currently working on the kernel, but I will write some notes as to what I should do when I go back to it.

## Reorganization

The entire code will have to conform to my previous guidelines (with pascal case) with some changes.

* IA32-related procedures will be prefixed with "i386." Example: i386SetLDTEntry
* Functions exposed to the kernel API will have it in the name now. "Os"
* Subsystem prefixes are back. If a function has no specific subsystem or could be used by multiple, can overlap or the prefix can be omitted.

Prefixes:
- i386: Low-level architectural functions
- Os: Any API call
- Sd: Scheduler
- Isr: ISR handler
- T0: Safe for interrupts off context caller
- T1: Safe for non preemptible context caller
- T2: Safe for preemptible context caller (may still not be thread safe)
- Tx: Safe for 0,1,2
- As: Safe inside ISR or IRQ off section
- Ts: Thread safe (does not imply reentrant)
- Re: Reentrant (implies safe in all cases)
- Um: Uses mutex lock
- Xm: Expects mutex is held
- Xd: Expects preemption disabled
- D{p,i} Disables preemption or interrupts

Os must take precedent over all in the ordering. Scheduling/context/threading characteristics come second and can be merged. Subsystem is the very last.

Now we can self document functions and make wrong code look wrong.

Examples:

OsReDbPrintf("Hello, world!");

But hey, at least you know right away if something is thread safe!

OsT12Sd_Svint

Read this as "Driver API call that is safe in preemptible and non-preemptible context"

Technically, Svint CAN be used in an ISR but that is only an internal feature. I could maybe change this.

## IMPORTANT INFORMATION

In C, adding any integer to a pointer has the implicit affect of multiplying that by the size of the type it points to! Why the heck did they do this?

## Fourth Rewrite?

This is big enough that it might as well be the fourth rewrite. It better be the last.

Here are the things that need to happen:

- IA32 subsystem gets ALL procedures renamed and rewritten if needed to conform to the new ABI.
- Scheduler is complicated. Every function and type must be renamed to comply with the new guidelines.
  - Header files are redone. Scheduler is now merged into ONE giant header file.
- Type.h is fully reworked to reflect types used by the xconio API.
- As much code as possible is to be deleted or relocated into the OLD folder. More code means more maintenance, especially if that code is not being used already. The codebase must grow naturally.
- Segutil needs to be properly implemented.
- All assembly code must be updated to fit new ABI or discarded
- Documentation needs to be thoroughly cleaned up when everything else is done.

I almost want to do the ENTIRE thing from scratch, but there is little reason. IA32.asm is just fine, but it needs a serious makeover.

# April 4

The previous plan is essentially correct. A lot of work to do. Keep in mind each bit of the entire project needs to be fully tested as well.

I think I should read the documentation and make updates. I need to get back in the zone with kernel development, especially with the scheduler.

## Subsystem and Driver Communication

Device drivers need to somehow communicate with the subsystem. I would like to reduce the necessity for subsystem awareness and make drivers capable of handling many different situations.

Drivers may implement a hardware or software interface, or both. If they are implementing a hardware device by emulation or arbitration, some standardization is possible. If it is an interrupt call or something like that, the issue is a bit more complex, but something could be done.

# April 5

Follow the plan. What you want is not what you need.

The assembly code can use the ASM_LINK macro. String functions definetely NEED to use the new calling conventions though for performance and should be written in assembly too.

Straight to ia32.asm then. It's a kludge, but it will be worth it to clean up that stuff.

## Segment Related Stuff

I do not like the "Ia" prefixes and want to remove them. But before that, I need to look at what I have.

- IaGetBaseAddress: Get base address of a descriptor with address

- IaAppendAddressToDescriptor: Set the address, should honestly rename to SetAddress.

- IaAppendLimitToDescriptor: Self-explanitory. Change to set limit. Clobbers EAX,EBX,EDX,ESI

- SetIntVector: Set an interrupt vector. Clobbers ABCD, which is not good.

> I can rewrite everything to conform to the new ABI though. Not that hard to do.

New names:
- i386GetBaseAddress
- i386SetDescriptorAddress
- i386SetDescriptorLimit
- i386SetIntVector

Remember the regparm order. EAX, EDX, ECX.

YKW? I dont event want to bother rewriting any of that crap. Its tested and works.


# April 6

I think I need a comprehensive plan for reorganizing the scheduler.

First of all the header files. I tried a Java-like approach where I made everything importable separately, but this is not typical for C.

STDREGS is a structure used by all parts of the scheduler in some way.

## Scheduler

Does the scheduler NEED to work this way? Why not just give IRQ#0 some random PCB pointer that tells it what to run? The handler can copy the context to its trap frame and return.

Scheduler decisions are made by setting this pointer thing. A post procedure can handle this process.

Dealing with yields is a different story. That requires a very clear way to enter an arbitrary process. I think yielding is a very useful feature, by the way.

I am deciding not to do cooperative multitasking in any way. If I want the ability to just enter a particular context, what do I do? The yield function would do what the scheduler does, but IRQ#0 needs to be informed of what happened.

Or I can do away with the idea of a "current process" entirely. That pointer I was talking about is really just the NEXT process. It needs to be set to point to the next process.

Yielding involves relinquishing the rest of the timeslice and basically acting like IRQ#0 was called.

Well what is it that IRQ#0 does? It just pushes the current context onto the stack and maybe modifies it. Nothing too complicated about that.

I think I need more clarity on what a yield actually is. Yielding is ALWAYS done by the kernel context of a PCB on the behalf of that PCB. This changes the active process.

The answer is already written. Call the IRQ#0 handler but decrement the counter!!!! There is no reason why that cannot work.

> By this I mean calling the IDT entry, not the high level handler. We will have to use `INT 0A0h`. Seems like a bad idea but it really is not.

I keep talking about yielding for a reason. It allows for a massive performance improvement. Letting the system hang for the remainder of a timeslice after a system entry before the kernel thread can be terminated is wasteful. Spinlocks can be significantly enhanced by implicit yield semantics. Otherwise, spinning on a lock would waste massive CPU cycles.

```
// Power of 2 must be used.
ENHLOCK el = { ENH_1_OVER(32) };

EnhLock(&el);
EnhUnlock(&el);
```

Actually, system entries do not work quite like that. I have demonstrated previously that it is perferctly safe to IRET back to the user context from the kernel thread. This does not change the "current process" at all and has no way of interfering with other processes. Just make sure to turn of interrupts when doing the exit.

### Conclusion

The whole context thing is not really necessary, as demonstrated above.

SV86 is a different story, but remember that it has special treatment. I can do it in the same way as I did before.

## Other Notes

I need to establish a practice of working on only one subsystem at a time. The project needs to grow naturally as needed.

Right now, everything is about the scheduler.

## Scheduler Plans Again

I will completely wipe the scheduler code as it is right now. Just did, good.

Now, there will be these files:
- sv86.asm
- task.c
- irq.asm
- main.c (scheduler)
- sync.c

SV86 will be completely done in assembly. I have no reason to use C. Register conventions will be used.

irq.asm is also assembly, and will deal with IRQ dispatching. I will translate any existing code.

main.c will be the basic glue code.

task.c will provide functions for controlling processes. It will implement creating tasks. Working memory manager is not required, and a callback is used to receive a memory page. Yielding and idle wait is also found here

sync.c will implement the enhanced synchronization procedures. I will make this later. This will build on existing primitives.

# Apr 7

## SV86 and V86 IRQs

Reflecting IRQs to DOS is a complicated subject. As of now, SV86 runs in a preemptible but interruptible context, which means that it is possible for SV86 to interrupt SV86.

BUt that is not the only issue. How do we even reflect to real mode?

I mean sure, we could use the whole BIOS restart vector hack to handle IRQs. That is what Windows 3.1 standard mode used to do, and its speed was not actually that bad.

The only issue is that is so painfully slow. But it might just work.

## Going to Real Mode

The i386 can easily switch to real mode by simply turning off the CR0 PE and PG. While using the 286 reset strategy does work, it is not more efficient.

The process is quite complicated.

- Jump to protected mode switch region
- Save IDTR, GDTR, LDTR, into a special region in HMA. CR3 will retain its state since we are NOT resetting.
- Disable paging
- Switch GDT to one with special base addresses and probably using 16-bit selectors
- Disable protected mode
- Do whatever
- Load the original GDT, LDT, IDT
- Enter 16-bit protected mode

This is not exactly a smooth process. The reset thing is not too bad, though I am not sure how good BIOS support is. Well its part of the BDA, so it probably is supported. I can try it on qemu.

Just tried it and it worked. I just had to use the CMOS instead.

As for the i386 solution, it can work without having to load another GDT by simply reserving segments for that purpose. It is also more reliable.

> Not to mention that the IRQ mask may or may not be wiped by the BIOS.

So why am I doing it this way? There really is no problem with it, as long as the driver does not do anything goofy, but that is a risk that always exists. For example, if any DOS ISR enables interrupts, an OS/90 IRQ could be handled by a DOS driver and wreak havok. I am pretty sure this was never done back then.

Speed will suffer, but DOS compatibility is not really about speed. Using some sort of SV86 would not be a whole lot better.

# April 8

Implementing the going to real mode process involves copying the trampoline code into the HMA at a specific location. Doing this also requires the code involved to be base-zero because of the segmented model used.

Because the HMA is being used, the code actually needs to be ORG 16.

Doing this can only be done by precompiling the code or making a linker script section. The former is simplest.

# April 10

Wow, the days are passing by so fast.

## IRQ Mask

The interrupt mask register found on bootup is used to determine the status of interrupts. If an interrupt is masked, it is available for general use by a 32-bit driver. If it is unmasked, the interrupt already has a handler somewhere.

## Load in HMA?

If I load the kernel in HMA (assuming that it fits and it probably will), there are a few advantages.

I will not be able to run the kernel in the lower half.

## The IOPB

Will I actually be using the IOPB? My idea was to allow direct access to a specific device, but this is quite complicated since it would require direct access to the interrupt in many cases.

DOS software could access the ports but not be able to access the IRQ, for example, since ring-3 IRQs are not supported for V86.

Do ring-3 interrupts work though?

IRET is needed to return from an interrupt. Returns to different rings can only be done using task switching.

The conclusion now is that directly handling IRQs in ring-3 is not possible with IRET.

But does it have to be IRET? We could just use an INT vector to emulate IRET. This would make the interrupt handler slower.

I am starting to think this may be a bad idea.

## Back to the IOPB

The IOPB takes up 8192 bytes of memory. It also is not compatible with my bit array functions.

The idea of it was to tightly control which ports programs were allowed to access for security. It does not have to be used though. If there is not one, or the IOPB pointer is set to an invalid offset, only IOPL matters.

The IOPB is only checked if the current ring is less privileged than the IOPL ring. If IOPL is set to 3 for a process, it can directly access anything.

This means that without an IOPB, there can be no mixed configurations with both real and fake devices.

Unless I emulate every IO instruction. I already have code that does this based on reverse engineering the ISA instruction encoding.

Emulating string operations makes disk IO significantly faster, which is likely to be the V86 code that takes up the most CPU time.

It looks like my current code cannot emulate 32-bit operations. This is not good because I have no idea what the BIOS code could do. If the BIOS tries to access PCI, it will fail.

It seems that most assemblers output the 66 override before the instruction and REP at the start.

The IOPB is kind of required because setting IOPL to 3 in V86 causes it to be able to call the protected mode IDT directly.

This is not that bad since each handler will invoke a system entry, but this may not be the desired effect.

> TODO

## Excpetions?

I am not so sure now of using exception handlers to emulate instructions...

It is kind of needed since INT calls should not call exception handlers. Trying to do so will always cause an exception.

I have tried implementing a system entry event concept. The idea was that exceptions and software interrupts are "events" which get an ID.

This made calling the IDT directly more practical. The only issue is that it assumes a DPMI-compatible environment. It also means that SV86 and UV86 need to be handled quite differently.

What if we have an independent IDT for each DOS process? That was the original idea. Fake IRQs need special handling, but such a thing is possible.

The problem with that is handling IRQs, except it isnt really a problem since the IRQ vectors are reserved.

DPMI exceptions can then be detected by checking if the cause of the interrupt in the range was not an INT instruction, in which case we are clear to proceed with handling the exception.

If no handler is set, it can be reflected.

The IDT would have to work in a certain way. Special handlers would need to be used for exceptions that check if an INT caused it and reflect to SV86. This is subsystem-specific.

The local idt will be referred to as the LIDT and not the instruction.

## LIDT

The LIDT will be 2048 bytes long. The information block of the TCB is not defined, but it is certain that a larger TCB size is needed.

We will go back to 8192 byte TCBs. 4096 bytes at the end are the stack. The first 256 are the kernel information block. 1792 bytes are for the subsystem's use and 2048 are for the LIDT.

Actually, we do not need to do it this way. Just use functions for getting certain information in a portable way.

Actually, I can fit the TCB is 4096 bytes with a 2048 byte stack and a 256 byte subsystem block.

But what will the LIDT actually run? It needs a special system entry procedure. It also needs the ability to handle interrupt capturing and whatnot.

Also, the LIDT will vary between active processes and switching the IDT when entering a kernel context seems wasteful.

I think I should just emulate all IO. This means a global SV86 register is unfortunately required. The #GP handler must handle everythign according to the global SV86 flag.

Sadly this is how the 80386 was designed. I have no idea why INT and IRET are IOPL sensitive.

## Excepion Handling and Other Things

I still have not decided on how this is supposed to happen.

Here are the possiblities:
1. Local IDT, IOPL=3 in SV86, no IOPB
2. One IDT, IOPL=0, all INTs cause exceptions, IO emulated
3. One IDT, common system entry for exceptions and INTs, IOPL=3, direct IDT access

Option 3 requires a different ISR in order to memorize which vector was invoked. This is a bit wasteful, as it will take about 2048 bytes in order to do this. It also requires quite a bit of conditional branching since I tried it before.

2 is what I was tring to do after the redesign. Every ISR is ring-0 and each INT will cause a #GP. Then we use hooks.

The way hooks work is that a new handler replaces the previous one and then calls it if there is nothing to handle. It must start with the final handler in the list in order to override previous ones, though it should not matter much.

V86 hooks will be the same.

## Exceptions 3

Exception handlers will go to the system entry procedure, and there will be one. The error code is something that must be mentioned.

OS/90 allows multiple programs to cause exceptions at the same time and handle them concurrently. Exceptions of the same type can also be handled this way.

This means that the error code of some exceptions cannot be saved in a global variable. The index of the exception handler must also be saved.

This means that an exception handling table like the one used for IRQs is needed.

Do we need to worry about the error code though? Yes. It needs to be pushed out. Why could they not just add an error code register?

I can use a lookup table to determine if a pop should occur. Branches are not good though.

# April 12

## V86

I can have one giant chain for all V86, but this would be bad for performance since I expect to create quite a large number of hooks.

A chain for each vector is more sensible, although it does NOT need to implement every vector, but it might as well since there is not much to save by chopping out a few of the 8086 exceptions.

1024 bytes must be used for the initial table, which is quite large.

## Load in HMA

I can in face do IO into the HMA and there is nothing to stop me. OS/90 turns on the A20 gate and leaves it on. The only reason the XMS specification recommends not doing IO there is because DOS typically normalizes pointers passed to it.

This gives me some more idea though. If OS/90 can 100% fit inside 64K, which I am sure it can, I can even put in inside the bootloader.

61440 bytes is actually more precise. Its a hard constraint, but the compiler is good at keeping the code small.

OS90.EXE will perform the whole procedure. It will incbin the kernel binary. I have to make in an EXE because

### An Obstacle

The initialization page tables take up a significant amount of space.

## Extended Memory Free Region

How do I find out where the available extended memory is? I think it was supposed to come after the kernel, but now things are different. I will have to pass the extended memory base address to the kernel when switching to protected mode.

# April 13

## HMA

The problem with this is the initialization page table, which will waste 4096 bytes of memory for nothing. Unless I redesign the whole thing to not use a higher half kernel, which is probably a bad idea, this may still be infeasible.

I will leave this as a possibility, but in the future, I still need to find if it will even fit.

So far the numbers are good, so if the initptab can fit inside the kernel image, then that is fine.

Oh wait, remember how the HMA is not 64K and a page has to be masked from its size to get the real number of bytes? Yes, I have 4K, but I actually need 8K to make the whole thing work. I need a page directory and a table.

# April 18

## Plan

SV86 will not work in T0. I cannot think of a specific reason for this, especially considering that SV86 is not preemptible to begin with, but it might be because some BIOS functions wait for interrupts and there could be a blockage.

It could be because SV86 has to go through the system entry, which is a preemptible region normally and I am not sure if it will behave correctly with interrupts off.

I am just not sure. I do not see why interrupts would be disabled considering that preemption automatically is.

If the call is doing something that requires disabling them, then we should let it happen.

## The Actual Plan

- I need exception handling completely working.
- The scheduler does not need to multitask yet.
- Interrupts must be handled.
- Real mode reflection should be completely working.
- SV86 should be completed and tested, including the exception handler

When SV86 is done, I will test it with everything possible. BIOS INT 10h, file IO, all of them. I need to make sure it works perfectly.

# April 19

## Direct Interrupts

The OSDev wiki says that the interrupt requests ignore the PL bits on the IDT entry. THat could mean that it does not check privilege levels when switching. I am not sure.

I think I decided earlier not to use direct IDT entries due to various complications.

## IDT is Working

The entries are being placed in the IDT. This is good. Now I need to move on to somehow being able to handle interrupts.

IRQ#0 will ALWAYS be handles in protected mode, but the rest are totally reflectable.

So what prevents me from just enabling interrupts?

The level 2 interrupt handlers are already configured btw.

## Is IMR Handled Correctly?

What order does the PIC use? I looked it up and the IMR goes from bit 0 to bit 7 in expected order.

The problem is that the master PIC has a mask of 0x8F or 10001111 in binary. This is impossible because the timer IRQ must be unmasked. I am not sure when this happened.

ITS IN REVERSE!

# April 22

I just read that switching to real mode apparently DOES require changing to 16-bit protected mode first. On a real PC, it will apparently lock up the CPU if the code segment is still 32-bit.

http://www.osdever.net/tutorials/view/protected-mode

Bochs does not really care, but this is actually undefined behavior and may not work on all CPUs.

This means new segments must be added to the GDT. The code and data segment must both be byte granular and have a limit of FFFF.

In my case, they will overlap and have the same base address.

To enter this segment, I will use a far call from the master ISR.

## Memory Manager

I will have to start from scratch since none of my previous writings are very relevant to the current situation, but there are some bits and pieces I want to remember to include.

- Raw memory region allows direct access to all physical memory in a linear region. It is ring-0 only and is used to manage MM structures.
- Page Frame allocation table (PFAT)
- Dynamic lists, std::vector style
- Allocating virtual address spaces
- Page frame chains with IDs
- Holding a chain-local index in each chain block for virtual memory support
- Uncommitted memory allocations, except the memory must grow sequentially (we look backward until a committed page is found and looked up in the PFAT)
- Virtual memory is allocated by reserving a linear region in kernel or user memory space.
- DOS memory allocation features (DAlloc, DFree, DRealloc)
- Allocations will not free themselves after task termination, subsystem decides what to do.
- Managing the dirty bit
- Locking pages with a special bit.
- Swapping, will it be chain-level or page level?
- Remember that the PFAT is very important for virtual memory. The front/back links are very important, as well as the index. We do a delta calculation to figure out how many pages need to be swapped back in. A chain cannot be fully swapped out?
- Eviction function?

Read ## Physical Block Table (PBT) and the bracket comment. Useful?

# April 23

SV86 will be the focus for today.

I will have to design a few new things. The #GP handler needs to be worked on with emulation for IRET, INT, and other things.

This means that there is no way to escape the interrupt counter. Luckily, we do not have to worry about interrupt reentrancy and other stuff.

#GP should never come from SV86 unless it is a sensitive instruction.

IO instructions, as it seems currently, need to be fully emulated. I really want to avoid this. An IO permission bitmap allows for a complete avoidance of the overhead.

Something else to consider: will SV86 support trapping IO, for example to emulate a device but use an existing real mode driver?

I think NOT. It is better to control the entire interface. If a driver is going to emulate a piece of PC hardware, it should probably implement a proper interface for it too. A 32-bit disk driver should control INT 13H, not emulate the entire ATA disk interface.

## SV86 #GP Handler

The general protection fault handler can be inserted in real time since it handles every possible correct situation in SV86.

I will EMULATE IO port access for faster disk IO. I have proven in previous documents that the speedup is significant and documented the format of IO instructions to make this simple.

The problem is that my existing code only decodes the operation, which is fine, but it does not execute them. Maybe i can find some way to make a branch table using the bits and run the instruction, but that would be too many combinations.

Makes me question if all this is worth it. I think I will bring back the IOPB. By default, it will allow all (set to zero).


# April 26

## SV86

SV86 needs to be able to run nested hooked interrupts. Consider this: INT 21H uses INT 13H to access the disk for file IO. If INT 13H cannot be trapped, there cannot be 32-bit IO.

A possibility is to remove the current capture chain entirely and use some kind of stub that goes to protected mode, but this would require switching from T1 to T2.

A better idea would be to retain the capture chains but allow them to be used in each instance of INT. Nested calls to INT are handled by GP# with accurate stack emulation unlike the initial SV86 call which does not use a stack. The INT counter is used for this.

I have to consider how to return to the caller. Perhaps I can guarantee the contents of the kernel stack prior to the V86 entry so that when SV86 causes an exception, we can immediately get the necessary information to go back to the caller. We only need to GO BACK, not perform the whole return process.

I need to change a few things to do this. What need to be restored is EIP and ESP, which are not recoverable anyway. I do think they should be on the stack for cache locality.

# May 7

## Naming Conventions

Are the naming conventions excessive?

First of all, reentrancy can denoted as it is on Unix. Reentrant must imply thread safe. Safety within an interrupt context is a different issue and a reentrant may still not be a function you want to call inside an ISR.

Well, a reentrant function does not access any outside state.

```
OS_SVINT86_t12
```

Reentrant will never be used unless it is actually reentrant.

```
OS_printf_r
```

I think interrupt safe implies reentrant. Actually, I see no reason why any function called in an ISR would be reentrant, unless that function is not shared whatsoever, and for the OS it will be.

# May 8

## Naming Conventions Updated

```
OS_     Any function for the OS API
_r      Reentrant
_t{012} Context type
```

Subsystem tags are pointless and just take up two characters for no reason. They do not help very much either.

Also svint should now be macro-cased. How about OS_SVInt86_t12()?


# May 9

## TODO List for next few days

I have to study for my last AP exam over the weekend, but Thursday and part of Friday will be avaialble.

I need to do the following:
- Change naming conventions. First thing.
- Keep working on SV86 and interrupts.

## Can I make a build program?

I am not a huge fan of makefiles. I would prefer to have a simple program that does the building with parameters only. This should be portable too and work for any C project.

```
make90 --profile "OS90 Kernel" --proj kernel --inc include -build build
```

Maybe this can be a python script.

All I need is to setup the includes

# May 10

## Makefile

Keep the same.

## Use Assembly?

Besides the bit allocator which I cannot seem to be able to write in assembly no matter how hard I try, everything else can be in assembly.

That means the ABI can be whatever is convenient.

For C, we have to actually set registers or the carry flag before calling functions. They can be called using the standard mechanism.

```
_EAX = 0x10;
_EBX = &params;
OS_INTxH();
```
The only issue is compiler warnings for pointers. Is there a way I can ignore them?

No, I need separate void* registers.

The C API is not really that important. I need to establish a proper project structure for actually using assembly.

What I need for assembly:
- Debugging with printf
- Assertions

I will extern functions instead of using prototypes or headers.

I do need a way of making imports and exports tables since the current method does NOT work.

I will have a standard header file with the necessary features.

```
DEB "EAX = ", eax
DEB "EAX = ", [ebx]
```

### Maybe... Not?

Is there a real advantage to this?

Do you want to write this sort of code?

```
char string[] = "Hello, world\n\r$";

VOID Example()
{
        STDREGS r;
        r.EAX = 9;
        r.EDX = string;

        _iEAX = 0x21;
        _EBX = &params;
        OS_INTxH();
}
```

This only needs to apply to assembly functions. In that case, I can write all my other code in C.

Actually, this does basically nothing useful. Improving the performance of functions is all about using better algorithms and reducing the instructions or memory accesses.

## Drivers

I am not so sure about the idea of using the table for calling procedures. Being able to link the functions sounds better, but the catch is that we cannot use ordinals of any sort and must rely on named symbols.

### Executable Format

> I will make it perfectly clear right now: not a single option is going to be easy to do.

But what executable format should I use? EXE has a lot of bloat from Microsoft-specific things. ELF is too complicated for simple drivers, although very well documented.

Anything else requires either a special toolchain that I do not currently have (but can have). I can also roll my own format with the whole ELF simplified subset idea.

I do not need to do any of the manipulations of the relocation or symbol sections. Instead, I can dump the contents of the ELF executable and generate the appropriate content.

ELF has a dynamic section (not a real section but reserved region of file) that specifies every library that must be imported by name as NEEDED.

# May 14

## IO Handling

It looks like my idea of the exokernel design may not be viable any longer.

I have a new concept now. There are three components to the use of a device:
- Subsystem client
- Subsystem link
- Device driver

All three are separate drivers.

Creating a link gives the subsystem but NOT managed tasks exclusive access to the device driver interface.

The device driver should provide a function for locking the device.

In the VxD model, the device driver handles all access to the device by emulating IO or blocking all processes trying to access it. This is still the same. The main difference is that the subsystem is added as an extra layer.

The subsystem must control the driver before it can be used, and the only way to do this is through the link driver.

When a program performs port IO or in any way required the use of a device, the driver will decide how to act, but the link driver must be used to signal it to do so. Because the subsystem is responsible for emulating IO port access, it must decode the operation and send it to the link.

The actual interface does not need to be specified.

## IO Handling: Analysis

Not a bad idea, but the lack of standardization in the interface is not exactly a good thing. I do not want to write the drivers and get annoyed with how often I need to rewrite everything.

I also am not fully sure how bus subordination is supposed to work.

Also, I am not sure why the subsystem has to do even deal with the link driver at all. The point was to abstract the difference in client subsystems and separate things. The link driver seems unnecessary.

The connection idea could still be used in order to avoid having a chain of IO port handlers. A subsystem could have list of drivers that must receive any IO. This does not need to be an automatic process and can be manually dispatched by the subsystem.

It may be better to simply latch the driver onto an individual process. This does not HAVE to be automatic. The user could decide what devices get passthrough or which ones get emulated using parameters, or select which devices are included in the subsystem.

We are still not trying to create a full virtualization system, although it could be possible.

# May 15

## Handling Exceptions

I can simply create a list of kernel mode exception handlers for each task!

That is essentially what happened with DPMI, but for user mode.

Currently, there is a chain of handlers. Why do that when you can just call the subsystem exception handler directly?

After all, only one subsystem can actually react to exception. They cannot be interfered with by other subsystems and passed along.

The excpetions that are handled by the subsystem might be quite limited. Floating point errors 100% need to be abstracted to account for FPU differences.

The coprocessor segment overrun and the general protection fault are called depending on the type of FPU. The FPU error IRQ could also be used here, and it would require abstraction.

I was going to add a driver to manage the FPU, but that would be impossible now. Subsystems should not need to know about the FPU, and only the kernel is supposed to deal with calling the high-level exception handlers.

Page faults need to be exclusively handled by the kernel. There is no other way. It would be cool if the subsystem could have some control over virtual memory, but that will not be possible.

The list of exceptions may be something like this:
```
VE_DIVZ
VE_SEG_NP
VE_STACK_FAULT
VE_ALIGN_CHECK
VE_FPU_ERROR
```

## Subsystems and Drivers



# May 17

## Real-time Scheduling

OS/90 will have a feature where a select few tasks can be scheduled as periodic and real-time.

Actually, real-time may not be the most accurate way to describe it. I will call this "interval-based task scheduling."

This will be for situations where a high-priority event must be handled at a very specific interval.

There will also be support for synchronizing ITS tasks at any point in time to account for any errors.

A use case for this would be vertical sync on generic VGA hardware with no interrupt for it.

ITS tasks are expected to yield immediately after performing a critical task. The task that would normally be run by the scheduler at that point in time without ITS is then given its time slice.

ITS uses modulus to determine intervals. The system uptime is mod'ed with the number of miliseconds at which the interval breaks.

ITS code is usually implemented as a tight loop. It must yield its timeslice to avoid significant  harm to system performance.

### Purpose

To allow for certain harware drivers communicate with low-priority devices like keyboards, or handle situations that are inherently periodic, like vertical blanking.

Having a thread with uncertain scheduling properties is not a good idea by comparison. If there are many processes using a lot of time, there is no garauntee that something will run periodically.

VBLANK could be handled by polling the appropriate VGA register every 8 MS or so.

ITS allows the keyboard and mouse to both be polled in a multitasking environment.

# May 22


## ELKS

ELKS could in theory run as a subsystem of DOS. I am not sure if ELKS drivers will be supported, but it would be nice if that was possible.

Just looked at the ELKS repo and it seems like modules are either neutered or non-existent, so this is not possible.

But I can run ELKS programs. As long as the executable is loaded properly, it is simple enough to trap any INT 80h calls and perform the necessary translation. I wrote some docs on how that works. DOS/DPMI can handle all of this.

From what I understand, ELKS may support memory models but it has a single 64K heap. XMS exists, but that is separate from the actual OS.

This means that executing ELKS programs in protected mode should not be a hassle at all. I think ELKS already has limited support for 16-bit protected mode.

Consequently, I do not need to have a separate subsystem or any at all to run ELKS programs.


## Subsystems?

Do I really need support for subsystems?

I considered the idea of running programs for ELKS directly on OS/90 using a translation layer. The potential problem is related to address spaces.

DOS already has a multitasking region. Unless there is some way they can be shared, perhaps automatically by the kernel, the subsystems would either need two separate regions.

Anyway, I could figure this out. The issue is whether or not the concept of a subsystem does any good.

Why not do what I did previously and make OS/90 a DOS emulator?

This would mean that the RMR, EXE loading, etc will be handled by the kernel. DPMI will also have to be supported. Nothing changes, but INT and exception behavior becomes standardized.

The subsystem idea is not bad for performance since it uses callbacks to handle exceptions. I can benefit from the separation that it provides between the scheduler and other components.

## SV86

Is SV86 really going to be non-preemptible?

My new design for INTxH uses a lock instead of a preempt increment, so now I wonder.

As it is currently, SV86 does NOT actually use a single buffer for register parameters, so that is not a potential issue.

THe only potential problem is the system entry procedure and needing to deal with a supervisory virtual 8086 mode in the scheduler.

The advantage could be enhanced multitasking. I could access the filesystem using DOS while allowing other tasks to run as long as they are not blocked. Each task can independently request SV86, but only one can actually get a service.

Handling services concurrently presents many problems, however, and is not required. It requires separate stacks and potentially special configuration to avoid conflict. BIOS code cannot possibly be trusted to be reentrant and may very well disable interrupts to ensure this.

Without concurrent services, there are advantages to this approach, though there is added complexity.

Tasks must now have a kernel-SV86 state and must also be able to switch to and from that context. The proper handling of IOPL-sensitive instructions must become a task-local operation.

Additionally, SV86 requires the scheduler to be fully working, even before the memory manager is ready. I am not sure about how that will work.

Is it possible to have two versions of SV86 or something? Probably best not to.

### Conclusion

Its probably not worth it to do all this. SV86 is fine the way it is. 32-bit disk and FS drivers will make it all faster.

# May 23

## Error Codes

I did previously talk about the subject of error handling, but I looked at the DOS extended error codes and I think that it is a good idea. It reports the "locus" of the error (block device, memory, etc) and even a suggested action. The action may not be very useful for the kernel since it does not usually ask the user to correct an error.

> I do think there should be a way to receive keyboard input early in the boot process so that drivers can let the user correct any issues. This will have to use the BIOS.

## Keyboard

The keyboard driver needs to emulate the BIOS interface.

Keep in mind the way scan codes are handled with it:
https://www.stanislavs.org/helppc/scan_codes.html

## Thinking About Things

I will write only one operating system in my entire life. Logic would conclude that this operating system that I create should be the best that I can make.

Is the OS as it stands right now exactly that? I wonder.

The OS does not need to be based entirely on the design of the BIOS and DOS just because it uses it internally. Maybe there can be some sort of abstraction layer between the underlying low-level software stack and the userspace or even high-level drivers? Maybe like Windows 98 with its WDM VXD.

But maybe not...

At some time, I should clear my head and envision the perfect operating system.

# The Perfect Operating System

This is not a journal entry, but a short little essay of sorts to collect some ideas.

## Userspace

### Paths

I like the idea behind Java NIO with its Path object and the ability to concatenate paths or make the relative to something, etc.

In the OS, one would create a path object and then create a file using the path as an argument.

```
HPATH hp = CreatePath("docs/readme.md", P_DRIVE_REL | P_FPATH);

HPATH localpath = SwitchPathDrive(hp, 'C');

HFILE hf = OpenFile(localpath, FILE_RD);

DeletePaths(2, localpath, hp);
```

Paths can be drive-relative, folder and drive relative, or asbolute.

The driver letter can easily be changed as it is internally stored as a single byte.

The right and left side of the path can both be extended, and duplicate slashes are automatically removed.

Drive letters are still good and the simplest way to handle partitions.

### General IO

UNIX is known for device files, which are a decent idea because they are accessible from the command line, but I feel like it does not have to be this way. This was done because back then many things were done using scripts. In the early days of UNIX, there were not even shared libraries, so piping output and communicating directly with devices was a desireable thing that made the userspace more versatile.

That was all good when people only used the command line, but UNIX really began to show its age when complex specifications were built on top of it to adapt it form environments outside of mainframes and servers. Just look at Free Desktop and dbus for example.

Now what design is that? I don't know.

### Pipes and Redirection

## Kernel Mode and Drivers

### Translation

Perhaps there can be an altered model of user-kernel communication.

Subsystems currently play the role of a translation layer for the underlying DOS, but a more abstract interface for subsystems could be possible.

They could translate to some sort of kernel language that is then translated to something understandable by DOS.

I think I should try to think of an example.

Maybe there can be standardized driver models. For example, standard console drivers and standard FS drivers, things like that. Userspace does not use files but addresses a driver using some sort of addressing method.

```
Address:

LLLLMMMMMmmmmmFFFFF

L = Locus
M = Driver type major
m = Driver type minor
F = IOCTL function
```

In command line, it would be more like this:
```
cat file.txt > <chardev:reciever:printer:MAIN,write>
```

I think this idea is very old, perhaps older than UNIX, like the time when computers were mostly operated by punching in codes to make them do things.

It is also not objectively better than UNIX.

# May 24

## Decision

I will NOT plan any major modifications to the OS design.

At least not for now.

## Still Thinking About It

SV86 is the core of the OS in a way. I wonder if there is a way I can reduce its importance and NOT have it be some sort of standard interface.

It really should not matter though. The driver model is barebones at the moment. It can have a framework built on top of it.

# May 25

## Device Files

I will reuse an older idea. The whole unix-style device filesystem will be supported using a Q drive letter and is inside the folder SYS.

The filesystem does not work as expected, though. Each item can have child items. There is no real directory.

```
type test.txt Q:\SYS\PNP\DEV\CHARDEV\PRINTER
```

## Filesystem

I need to be able to allocate drive letters so that virtual drives or other filesystems can be implemented by drivers. Otherwise, they would have to do attempt to access each drive to detect existence.


The kernel needs a filesystem API for accessing the DOS services in an abstract way. They will automatically perform translation as well.

To support files larger than 4GB, I will use 64-bit seeking, even though DOS will probably never support this.

## A Different Model?

But instead of trapping DOS, why don't I establish a native kernel interface, and from there drivers for the FS or disk can hook things?

The default behavior will be to reflect to DOS through SV86. The request can be trapped at this stage, or it can be trapped before going to the kernel.

So basically:
```
     Native Interface |=>| Hooks
     SV86 => SV86 handlers
          DOS+BIOS

```

The idea is that I can implement some higher-level operations without having to fully depend on DOS or use the not-so-fun and SV86 interace.

Subsystems translate system calls to the native kernel API rather than DOS itself. This makes subsystems somewhat portable, should I decide to make OS/90 capable of running on ELKS instead of DOS.

The DOS subsystems seems to be quite dependent on DOS though. It needs it to allocate conventional memory and as planned currently sends every INT that is not locally trapped by the process straight to SV86 for DOS to handle it.

It does not HAVE to be this way, though. If I add this abstraction layer, I can run DOS programs directly on ELKS. THe only problem is reduced performance from the abstraction overhead.

The advantage is that drivers do not actually need to know anything about DOS whatsoever. I can take this a step further by really pushing for those device files and use them in kernel mode as well to implement a sort of intramodular procedure call.

## Plug and Play

OS/90 will never support ACPI and I have no problem making it as incompatible with it as possible out of pure spite. With that being said, I still need to handle PnP and power management.

I decoupled PnP from the kernel a while ago, and it was for a good reason. I also got rid of the bus interface and allowed buses to decide how they are to be accessed by subordinate drivers. These were good decisions, but there is the issue of plug and play.

The idea is dealing with docking/undocking, shutting down, sleeping, etc. OS/90 will probably not support hibernation, but it may need to be capable of idling.

Device drivers control all devices of a certain specific type. That means the drivers need to receive certain events that relate to PNP in a proper way without having to know anything about what kind of PNP is being used.

We can have a command set given to the driver, totally optional of course (so I can delay doing this until a later time), which sends events.

For example:
```
PNP_POWEROFF
PNP_SLEEP

PNP_DOCK
PNP_UNDOCK

PNP_LID_OPEN
PNP_LID_CLOSE

PNP_REMOVE_SUBORDINATE
PNP_ADD_SUBORDINATE

PNP_ENABLE_DEVICE
PNP_DISABLE_DEVICE

PNP_UNINSTALL
PNP_INSTALL

PNP_DRIVER_DISABLE
PNP_DRIVER_ENABLE
```

- With that subordinate stuff, I may actually need to add back bus drivers.
- No idea what that lid stuff does. I just put it there.
- Installing means to make that device accessible and recognizing its presence. Enabling simply "enables" the device according to what it considers to be an enabled state.
- Disabling or enabling a driver means to make the device it controls temprorily unavailable or restore it from such a state. On the startup of a driver, it is already enabled.

It does not matter how perfect I want to make it. What matters is what I can make that one person can understand. This will have to be done.

Commands given to PNP drivers can always fail or be terminated due to a device disconnection, hardware failure, software bug, or user action. I think my advanced error handling idea can be used for this. I could even have a table of error action handlers.

## KNI

I just looked at the DOS INT21H listing and the filesystem calls. Do I REALLY want to implement all of that?


## 32-bit Disk Access

There are two controllers which both can have their own states to keep track of the drive selection. It is also possible to send various commands to both, and this can potentially be done in an interruptible context. Read the ATA page on OSDev.

# May 26

## IPC, Drivers, Intermoduler Procedure Calls?

The OS/90 scheduler will support the following features:
- Yielding a task timeslice
- Yielding to a specific task
- Periodic scheduling (will probably not add immediately)

## KNI

I will NOT do the KNI thing.

## Subsystems

There will be a DOS subsystem. Whether or not I need other subsystems is actually debatable.

As stated previously, I could run ELKS within its own subsystem or simply use DPMI/DOS to execute ELKS programs. Doing so makes it possible to have ELKS programs interact with DOS programs seamlessly.

But even if ELKS is its own subsystem, such an interaction could still be possible. The standard IO handles are the same. File handles can totally be shared between ELKS and MS-DOS. Exceptions do need special handling for handles to work, so a separate subsystem is logical.

# May 29

## Interrupts

When I type on the keyboard, it appears that no interrupts are reflected to DOS. I suspect that the Bochs BIOS simply does not use the IRQ and only polls.

I will try QEMU.

There are no problems at all. I believe the interrupt reflection routine is working.

Time will tell if that is really the case.

## Scheduler

My next job will be to get kernel threads to work. To do this, I will need to allocate memory or just reserve it in the BSS section.

A few changes must be made in the scheduler. I am not so sure if I need to fully expell a task block from memory. Sometimes it is acceptable to simply mark it as dead so that it may be reused immediately.

Deallocating the page is not TOO expensive, but I also want the memory allocator quite isolated from the rest of the kernel.

The onyl issue with resident memory is that I do not have a way of garbage collecting any task blocks that are no longer needed. They will just take up space for no reason.

I think deallocation is needed. Freeing a page can be done with one single write to the PBT.

# May 30

## Scheduler

Once the scheduler starts, the KernelMain procedure will never exit. It simply ceases to exist as an execution state.

I will need to create two threads and run them to demostrate that this all works.

## Virtual Address Spaces 2

Being able to map abritrarily is a complicated feature. The whole page table gargage collection idea can work though.

Originally, I had a reserved chain which was used only for page tables. The RMR enables such a thing to work.

The only issue is that this is not very convenient when we want to delete parts of the allocation. For that reason, I may need

## Catch 22

If SV86 depends on the scheduler being ready and the scheduler depends on the memory manager being ready, there is a problem of circular dependencies.

This means that the scheduler must be totally decoupled from the memory manager. There needs not be any "fork" or "exec" command. The subsystem deals with most of that.

SV86 does not actually depend on tasks being able to run as long as interrupts are enabled.

## Wait

There is a difference between a generic interrupt disabled section and an interrupt service routine. Any kernel thread can disable interrupts. I cannot think of many problems that could arise. After all, the SV86 context itself can also disable interrupts.

> THIS IS IMPORTANT. T0 can be deceptive unless properly defined. I may need to respecify some things.

Context types:
```
ISR: Interrupt service routine. Cannot call non-reentrant code or access data that is not IRQ-protected.

T0: Voluntary interrupt disabled section. Only reentrant code can be called.

T1: Voluntary preemption disabled section. Anything that does not hold a lock is safe.

T2: Preemption and interrupts disabled. Most functions are safe.

```

## Invalidation

I need this:
```
PageInvalidate(PVOID addr, LONG count);
```

addr is automatically rounded down to a page bound.


## V86 Issues

I used to just disable preemption every time I entered SV86 to protect the SV86 data in memory. Not I have a lock. Since I know that SV86 needs to be able to run in an interrupts disabled context (NOT ISR), a lock is not possible.

I think what I need to do is re-enable preemption the moment I can an INT handler and disable it again. I want to prevent preemption for reflection and fetching the data.

Just add an inc and dec.

## OS API Naming Concention

I no longer like using the OS_ prefix. I think I can just use underscore and a capital letter. Even though it is a reserved namespace, my names are surely not going to be used by any compiler and there is no standard library either.


# May 32

## Naming Convention Decision

I will NOT do the underscore thing. Not good practice. There will be no special convention for API calls.

## VERY IMPORTANT

> The kernel should be designed around the API and not the other way around.

Each subsystem needs to have its functions outlined before I even work on the kernel at all!

# June 1

## Assembly Code (IMPORTANT)

I NEED to ensure that ECX is not callee clobbered by any assembly routines

# June 2

I need to make sure ECX is saved. Will do that now.

## Notes for the Future

I should make OS/90 into a VERY basic DOS multitasker until the UI can be fully developed. Something like the first version of OS/2.

Maybe use the VGA page flipping for that, idk.

# June 3

## IMPORTANT: Notes on FPU support

IRQ#13 is special because it is NOT ANSYHCRONOUS. This allows me to make some assumptions about the circumstances of an IRQ#13.

It is practically an exception handler and represents nothing more than a simple change from user to kernel. So long as preemption is turned off in the critical region to prevent other tasks from also using the FPU, we get full control.

A driver will handle the FPU. No need to worry right now.

## Page Tables

Can we statically allocate all page tables?

A 1GB address space has 262144 pages. It takes 256 page tables to represent these, which are 1MB in size.

Such memory use is very high. It does not help much that the virtual address space is shared and global. Other operating systems handle constricted address spaces more efficiently.

But keep in mind that a computer with somewhere between 16-64MB of RAM is unlikely to access such a large address space. The size of the swap file is the important determining factor for the size of the address space (which will be configurable).

I would estimate that four times the physical memory is about as large as the virtual address space needs to be. Excessively large swap files

## Swap Partitions

If I add swap support, I should use swap partitions instead of swap files for performance reasons.

# June 5

## Stacks and Contexts (IMPORTANT)

In the current multitasking design, the trap frame goes into the stack at a quite predictable location and is local to each process. But there is another register dump structure.

The way I understand it is that the trap frame is ONLY for going back to user mode when running in kernel mode. I suppose this means that the register dump is only kernel mode.

But actually, it should mean that the contents of the register dump are only the state of user or kernel when the task is not running. If a kernel mode task accessing its own task block, the register dump contains garbage from previous kernel mode register saves.

When the task switch interrupt happens, whatever is currently running, user or kernel, has the context saved in the task block.

But what about ESP0 is the task state segment? When kernel mode returns to user mode within the same task, ESP0 is set to the very top of the stack area to reset everything for the next entry to the kernel. This is done in an interrupts off section.

The process of switching tasks involves simply saving the trap frame of the interrupt to the current process and copying to the next.

The scheduler does not need a concept of a current process as a variable. The current process is defined by the stack currently in use.

The tasks will be organized in a circular array. The first task will reference itself.

Yielding a time slice does not work when preemption is off. It simply calls IRQ#0 directly with an INT instruction to initiate a context switch.

# June 6

## Task Blocks

TBs must use a doubly-linked structure. This is not currently the case and it needs to be updated.

A singly-linked circular array works fine for insertions at the start or end (which is normal for scheduling tasks) but does not when removing tasks from the list arbitrarily.

Task pointers

## Alternative: Task Slots

OS/90 does not need to run many tasks. On my linux system, I have a total of 700 threads running with the browser open. When I close it, I have 500 threads running. It drops to 300 when I close VSCode (what the heck).

Why a code editor needs 200 threads is something I will never understand.

But considering this...

A task block can be reduced to 128 bytes. 300 threads is unrealistic, but with that many I need 38400 bytes.

So why allocate a full page every time I need a new task? Creating new tasks is not inexpensive because memory always has to be allocated. Because stacks must be used, the other approach is not exactly faster either.

The advantage is a larger kernel stack size with very little change in memory usage. I can even make kernel stacks customizable, though most systems have a fixed size for various reasons.

There is one problem and that is performance. Getting the current task takes a subroutine, unless I use a special variable at the end of the stack for that, and even then, memory accesses are required.

The scheduler currently depends on the stack-task binding. Task linking is the only way to efficiently multitask.

Conclusion: We are NOT doing this.

## What Comes Next

A major milestone in my OS will be getting tasks to work. I will implement kernel threads. printf is a complex function that will test all the registers, so I will try to print things in two threads.

## Yield

Can I just switch stacks?

# June 7

## Task Blocks Again

Because task blocks never move and are always 4K in size, there is no reason for task states to even be recorded. Active tasks remain in the list. Inactive tasks are simply detatched from the list to be reattached latter.

## Scheduler Time Slices

I should have a global variable called "time slices granted" so that CPU percentages are possible to calculate.

## Initialization

How do we initialize the scheduler?

## SV86 Call Mechanism

Can I save stack space by using registers directly. Of course I will have to save them, but there will be no need to copy to the memory that many times.

```
_ah = 0xE;
_al = 'A';
INTxH(0x10);
```

The problem I have encountered is that GCC does not support using partial registers.

Actually, it DOES. All I have to do is declare the register as an array. A few defines can handle the rest!

There is the advantage of not having to decide if the registers need to be stack allocated or not.

The problem is not being able to decide the stack, which makes this whole thing infeasible.

Conclusion: keep it.

# June 9

## Scheduler Startup

## Features

I think the OS/90 feature detection idea is really good. It is a bit like in DOS with installation checks, but instead of using a reserved interrupt call, a string or ID could be used.

It can work like a dynamic version of Linux CONFIG macros.

Code:
```

FT_PNP_BIOS pnpb;

VOID InitFeature(VOID)
{
        pnpb = GetFeature("PNP BIOS Kernel Support", &&Found);

        Found:
}
```

## Scheduler Ideas

Linux has a preemption counter for each thread. This makes sense and works. This way, a process with preemption off can yield to another without leaving it off.

Another idea is that disabling interrupts is transparent to the drivers and is done by a procedure. This procedure can simply MASK the interrupts rather than actually disable them! Not only that, it could actually leave IRQ#0 enabled, thereby allowing T0 to become preemptible!

Okay, maybe that was dumb.

Perhaps each task can have its own interrupt mask value, or virtual IF.

Just throwing ideas out there. >>> The preemption counter idea is a certain improvement. I also think that a high-level interrupts disabling mechanism is necessary so that an IF=0 task can switch to another one and have IRQs back on again.

An IRQ does NOT have the ability to switch to a task of course. But why not? Can I add a mechanism where an interrupt can participate in scheduling or something. Maybe native threaded interrupts by default? Maybe some sort of jump or INT instruction call to initiate a switch.

Making IRQs a preemptible context is crazy. It could make sense in a real time OS, but it might be excessive here.

# June 10

## Previous Ideas

Preemptible IRQs are quite unrealistic, though the idea of interrupts being able to switch tasks is something I though look into. I may need some sort of uniform interface for switching a task, perhaps a jump point.

A preempt counter for each task block is a must. It makes sense because yielding from a task currently blocking preemption is necessary.

Adding a HL interrupts disabled thing is not necessary since the IF in the EFLAGS register is thread local anyway. Making the interrupt mask local is an interesting idea however, but I do not see a major benefit. Under what circumstance would it be useful? Perhaps

## Switching Tasks

I could use the IRET instruction to switch the stack and so that the INT 0A0h handler can switch to the next task. It will try to save a context to it, though.

So no.

Another option would be messing with the task links. The operation is basically a linked list entry move operation.

## IRQs

If IRQ#0 can switch tasks, why not let other IRQ handlers do the same? A thread pool can be used to handle IRQs. If that is so, we could make it possible to allocate memory and do other things within interrupt handlers! Of course, most of the actual work on the handle will need interrupts disabled.

A thread pool will be quite large, at leas 64K. I will allocate it in the kernel memory.

## Thinking About it Some More

An IRQ can do a context switch, sure, but there are problems. First of all, it would simply RESUME the thread, not instantiate a new one. Not exactly an advantage. There is only one context for the interrupt, so it is essentially NOT reentrant because of that.

Basically, the whole threaded IRQ idea cannot really happen unless I add interrupt contexts to each task block. This is not something I would like to do.

## Conclusion

Preemption will be localized by adding a counter for each task. This allows for the safe coexistence of preemptible and non-preemptible tasks.

T0 needs to be specified as a yield-safe context, since it kind of already is.

## What to Do

I am a bit bored of this now. I will try to get a Windows virtual machine on my Gentoo desktop so I can play some video games. I have all the necessary RAM and CPU cores for that.

# June 11

## Subsystem for XENIX?

I should look into supporting some sort of UNIX environment.

UNIX support mostly hinges on the system calls.

Things that need to be supported include:
- Access controls
- Long file names with dots or spaces in them

Implementing long file names with no modifications to the 8.3 format requires

# June 12

## LFN and Unix

Some versions of DOS are capable of long file names, but there is no requirement within OS/90 that it be supported by the underlying DOS.

LFN does not need to be supported natively. It is possible to make a file called `__LONG__._$_` or something in each directory and use it to store long file name strings.

This can be done at two levels: the subsystem driver or the filesystem driver. The FS driver has to hook the INT 21H interface, so it is most likely the better option.

## Drivers Planned

- EMSRD.DRV: EMS RAM drive
- VLFN.DRV : Virtual long file names


## Features Again

Can I add the ability to extend features? For example, how do I detect subsystems? Can I check for a feature named "subsystem" and access them? I can add an interface for subsystems since they are an important part. No need.

## Userspace

I have written about this before and will do so again: I need a real OS/90 userspace subsystem.

Okay, I started a specification for the shell and various commands. But I am now thinking: do I want an OS/90

# Jun 13

## Continuing Userspace

I now have a shameless UNIX knockoff shell specification. But what will the userspace API actually be like? Maybe that is not too important right now. I am sure it will involve IPC and asynchronous IO.

The difference between the DOS subsystem and whatever UNIX subsystem I may come up with is the handling of console IO. Currently, the DOS-S sends teletype output directly to a virtual framebuffer.

Well, not quite. AH=9 INT 21H is supposed to output to STDOUT. Attempts to force duplicate any of the standard handles as well as attmepts to read and write them are all caught.

The input to the programs is handled with V_INTxH, which is used to request keystrokes. The keyboard driver implements the BIOS interface.

By simply assuming that the BIOS exists, there is practically no need for a real driver model, but it can exist. A feature could be used to detect the presense of a keyboard input driver and request an input stream.

But there are issues. How do we decide which task gets the keyboard, and how does this work in a desktop environment?

> "serve" input?

Yes. We need to be able to SERVE input. That means we call a function to send it. Using the BIOS interface and a V86 hook for keyboard input is not fully necessary, but in the end it will require the use of a special interface.

A similar thing needs to be done for the mouse. How do we implement clicking in the command line window? It will be essentially the same. The data needs to be "sent" or something.

# June 14

## Keyboard and Mouse

Keyboard input can only be received by one single task. This will usually be the UI. So how is the keystroke sent to the task that requests it?

The BIOS interface is used by the subsystem to request a keystroke for the task. The actual task that should receive keystrokes must be determined by the keyboard driver, and the rest have to wait.

By comparison, UNIX systems usually have a raw keyboard input device of some sort. STDIN and STDOUT handles are special because they have a fixed number and are local to each process, but are inherited by child processes of init. This way, output is directed to the TTY device by default. Input likely is mapped to the keyboard, which in the devfs can be symlinked to the actual keyboard device's text input file. The TTY device can also handle input too.

Cannot confirm all of this, but that is a working stdio model.

## Everything is a ...

I wonder if UNIX is the last operating system, a design that can be perfected no further. I suppose MULTICS is slightly better by making everything a segment, and files are just mmap-ed.

How about... everything is a... pointer?
Everything is a... memory address.

All operations are loads and stores?

Whatever it is, we are STUCK being unable to do better than UNIX because humans just can't do any better than files and directories.

Obviously I cannot MMAP the whole filesystem. But do I even need to use real addresses?

> Regular FS but with random access load/store and string operations?

# June 15

## Interrupts and Task Switching

Can I make interrupts switch to the task that owns it?

Also, with proper synchronization, an IRQ should be able to access the process in some ways, right? As long as it is the current one?

## Subsystem Independent Program

I want to have a text editor that can work in any subsystem. My idea is to allow for a driver to implement a program for multiple subsystems.

## The Native Interface Idea

Think about the benefits! The filesystem can be totally enhanced. Transparent file compression, long file names, file permissions, and other things which FAT does not have already. These can be supported using overlay FS features.

NEI has the advantage of making these features very easily accessible by subsystems.

Ideas:
- Executable-sepcific compression
- Could I have a concept of a program that mediates access to a file?

> Native Executive Interface (NEI/90)

# June 20

NEI does not need to be structured around any existing interface. The point of the subsystem is to translate to it.

## No NEI

I tried it it before and tried it again for some reason. DO NOT ADD ANOTHER INTERFACE. Think small.

# June 21

The OS/90 kernel is not tied to a specific OS or even a driver architecture. It is a paravirtualization interface like Xen, but not as advanced or isolated.

## Kernel Stacks

OS/90 is not a normal operating system, so it does not need an arbitrary limitation on stack space if that impedes on the intended design.

Currently, accessing the task block of the current task is done by performing an AND operation on the stack pointer. This is very fast and requires no memory access.

An alternative is to have the stack come BEFORE the task block. Knowing the stack size would then be a requirement, but the calculation is still free of memory accesses. The approach is to add the size of the stack minus one and and by the bitwise NOT of the stack size minus one.

## Wait

Do interrupts work properly?

Yes they do, silly. The ESP saved onto the stack is the old one before the interrupt.

# June 22

## Idea

What if I have a folder where adding anything to it notifies a program controlling it?
Or a folder that recieves a local path when some arbitrary path is given to it. For example: /dev/net/dns/www/google/com.

# June 23

```
&:\ATA/atactl.exe --identify > identify.bin
&:\subsys\DOS\run.exe prog=C:\COMMAND.COM mem=128
```

New idea! Instead of using IO to interact with device files, we can simple EXECUTE them!

When a drive specification is wrong, COMMAND.COM says the same message for letters and for any other character. This means that DOS programs can interact with this filesystem. I had this idea before, but I will repurpose it. Now it has an improvement.

The kernel interface will support standard IO and typical FS operations, which have to be used in the translation.

## Implementing the KFS

The KFS requires memory allocation to work.

It will simulate a FAT filesystem with short 8.3 names. Extentions are needed because the executable special files need to have the ending EXE.

```
typedef struct PACKED {
        KFSPROC handler;
        PVOID   parent;
        PVOID   next;
        BYTE    _;
        BYTE    name[11];
}KFS_NODE;
```

Use 6.3 RT-11 style names?

RUN.EXE

## Why Subsystems?

DPMI on its own is not totally sufficient for implementing an operating system like Linux or even Elks. It would need some way of extending the DPMI interface to do this.

The main problem with going full DPMI is the fact that I have to integrate it with the rest of the kernel. I can try not to. In fact, the whole DOS subsystem idea can persist.

Remember, the OS/90 kernel does not force a certain type of userspace. If someone wants to build a totally custom OS with my kernel, I would be happy to support that. (More later)

As it is now, the kernel code will be much neater than the previous DPMI-orniented code. All I am suggesting is a reconsideration of the necessity of the DPMI idea. I can reduce it to one subsystem basically.

But why? My current design accomodates subsystems perfectly.

What I have now is there to provide a logical way of linking userspace with the kernel. Don't settle for less.

# June 25

No, I cannot use 6.3 names because drivers use 8 characters. Unless I use the last three as part of it.

## IO

How IO will be performed on OS/90?

I think I should do some kind of minimal NT-style IRP message passing system.

```
//
// A prefix bit deterimes if it is required or optional
//
IODF_REQUIRE()
IODF_OPT_METHOD(IODF_ASYNC, IODF_EXCLUSIVE, IODF_USE_TIMEOUT. IODF_TERMUSR)

KFS_HMEM

typedef struct {
        LONG            kfs_handle_from;
        LONG            kfs_handle_to;
        MUTEX           mutex;
        PVOID           thread_user_caused;
        LONG            ms_timeout;
        LONG            ms_min_delay;

        SHORT           param_com;
        SHORT           param_subcom
        LONG            param_flags1;
        LONG            param_flags2;
        BUFF_BRK_PROC   param_buffbrk;
}IO_DIR;

typedef struct {
        LONG            result;
        LONG            real_flags1;
        LONG            real_flags2;
        PBYTE           drvname_servicing;
}IO_TICKET;
```

```
IO_DIR iod = {
        KFS_MEM,
        0,
        0,
        NULL
        0,
        0,
        IO_FS,
        IO_FS_WRITE_BUFFER,
        IODF_REQ(IOFF_ASYNC)
};

VOID Example(VOID)
{
        EARLY_CONSOLE console = GetFeature("Early Console");

        console->AttachStdio();

        IO_TICKET *iot = RequestIO(&iodir);
}
```

A quick little mock-up.

# June 26

Interesting idea with the IO thing, but it does not seem like it would make IO faster or anything.

The only reason why Windows 95 has that wacky IO driver model is because it is not a fully preemptive OS like mine is. It needs to schedule events for later because of non-reentrancy. OS/90 does not have this problem.

Concurrent IO requests are not a problem at all. I think an IO driver model CAN exist, but it should be more like a very debilitated version of linux cgroups. Basically, make a bunch of groups with different priorities and let the user tune them.

I need to keep in mind the design of the filesystem and block device drivers. I expect that if they implement caches, they will do so separately.

I think the FS driver should be dealing with caches. The disk driver should get buffers sent to it and do the job of ordering the requests correctly and running them.

Disk cache could make sense because then we could optimize based on usage patterns. Things like read-ahead buffering can be done. I can even use some sort of cache locating table. IDK. I could come up with something.

A FS cache however is a very good idea for making certain FILES way faster. It also allows for more options in regards to protecting data by controlling the buffering of FS structures.

Okay, I thought about it a little bit. Both can coexist. Caches provided by the disk driver are exactly that, caches. That means writing to them make the cache dirty. They are to be copied into file buffers which can actually be interacted with by the user withotu having to tell the disk driver at all. Read and write commands can operate transparently on memory buffers which can be later written back.

If the filesystem needs to read sectors, the disk driver could find out that the requested sectors are available in a cache. The cache can be revealed as read-only so that it can be copied or otherwise used.

Idea:
Keep a table of blocks of sectors with pointers to caches and a byte to count how many times the block is requested. Once done using it, the block is decremented.

> Disk cache has another advantage: being able to read the same block at the same time. FS cache can only do this for files and not just any sector.

Once a block or group of blocks get decremented to zero, the number of blocks decremented are removed by searching the (number of blocks*2) blocks starting from the begining of the table if they have the lowest number.

```
1 1 2 2 2 3 3 3 3

0 0 1 1 1 2 2 2 2

First two are deallocated
```

There are some potential issues. The main problem is that in the begining, there will be no caches in use. This will lead to constant allocation and deallocation of the exact same block.

Solution: there is a limit to how many cached blocks there can be. A zero is not an instant eviction. Only if it is zero and has a buffer and there are no more cached blocks left does this need to happen.

The revised algorithm is that when a range of blocks are decremented, caches with the least value of the requested range length times two blocks will have their caches invalidated and freed from memory. This involves at least two loops. The least value method is needed in the case that every single block is being used in some very strange configuration.

This may be good for improving read speeds, but what about writing?

> We need a way of scheduling some sort of IO transaction without immediately starting any operations. For example, reading filesystem structures with known locations. Basically batching the requests and sending them at once. This allows for optimal reordering.
> Doing that may require a more advanced interface, since INT 13H is not really designed for this.

INT13H can be used as the basic request interface so long as the thread making the V86 call is blocked, which is will already be. With a V86 hook it is basically a procedure call.

There is still the problem of not knowing the true sector size of the disk drive. Real mode INT13H only knows about 512 byte sectors and the BIOS has to simulate 512-byte sectors. Some hard disks also support virtual 512-byte sectors.

INT13H AH=1? How do we handle this global error state?

The biggest problem is the 512-byte sector limitation. This can be overcome by merging requests to read sequential sectors.

The major problem is the extended error state. Because it is global, it is hard but NOT impossible to make it work. Each thread can get its own error code and drive byte.

The final issue with INT 13H is that it uses CHS, which is useless for hard drives that use LBA.

## Printf Problem

printf cannot use an async write to stdout call. Since printf is likely to call the write function multiple times, it could lead to garbled output.

This brings up the idea of serializing requests and having some way to do that. An improved async printf would stack up the requests and then send them to be executed in order. This way it can all be async without the garbled output. Printf itself can also be serialized so if there is a pending write the function can simply wait for its turn.

Async printf is not a bad idea at all. I am thinking about using thread pools and maybe having single threads perform multiple sequential jobs. stdout is very low priority and should not need to happen as soon as it is requested.

I like the IO group idea. Let the user decide what needs to be prioritized.

I also need the ability to chain IO requests. When it doubt, use a linked list I guess.

# June 29 and 30

Great stuff with the IO model. I think it has potential. I need to consider one thing though: swapping must bypass disk cache and file cache if using a swap file. Unix has O_SYNC for doing this. The IO manager may need to be adjusted to handle cache bypassing.

## Swapping

Do I really need it?

Loaded question. Let's break it down.

Memory allocators are capable of swapping on their own without virtual memory by paging. Early 16-bit Windows once did this.

The problem is:

The swap file or partition is not treated as a simple extention to the memory that contains arbitrary pages. It instead has a series of allocations fully copied into it. This would lead to fragmentation, although the large size of the swap makes that less of a prolem.

The resulting problems are:
- Higher disk IO traffic
- No random page swap control
- Heavy defragmentation

Regardless of how I handle swapping, there is the problem of disk buffers used by DOS preventing the proper use of swapping. This may not be a major issue since very little conventional memory is used by DOS for swapping anyway. Users should be advise to reduce the disk buffer size.

Anyway, using the memory allocator to handle VM is possible.

The problem with this would be mostly the same as forcing entire page chains to be swapped as one unit, but the problem of fragmentation is more significant.



## Boot Options

This is actually really important and should have been thought of a long time ago. There is a need for boot options.

I can use a command line for OS90.COM but that could get too long. A file is better.

It will be just like CONFIG.SYS. Simple equal sign expressions. Everything excluding the space is part of the value.

I will need to support having data types so that the conversion can be instant in case a configuration value is looked up multiple times (it should not though)

```
STR 8042.90
STR DRV=PCI.90
STR DRV=ATA.90
STR DRV=FAT16.90
STR DRV=DOS.90
STR DRV=KBSWITCH.90

STR EXECLN=SUBSYS/DOS INIT M=128
STR EXECLN=SUBSYS/DOS RUN C:\COMMAND.COM
```

This configuration would simply boot into COMMAND.COM. KBSWITCH can be a driver that uses the keyboard to trap ALT-TAB and switch screens using the DOS subsystem.

Reassignments are a bit complicated. I think I should change it. I can have DRIVERS.CFG which is a simple list of drivers and their command lines.

Then there can be an OPTIONS.CFG file. Perhaps also a KEXEC.CFG.

Parsing the options may be a bit tricky. I think I should make all options have 8-byte string labels for simplicity.

## Initialization Order

This is extremely important. I will have to update my current document.

## Video Drivers and Page Hooking

I need to be able to unmap regions of memory such as framebuffers so that a video driver can capture any writes to it.

The current architecture does not seem to have a concept of "page hooking" or something. It does allow for bank switching, but nothing to capture a page that is not meant to be accessed.

Well, there IS a page fault handler exception for each task. A video driver could hook it. This page fault handler of course will be very abstract like several other ones.

> Do I need to use high-level exception hooking? I can simply have the FPU driver hook the right things and automatically use the i486 interface for the exceptions. If I can hook page faults, that would be a good thing. Page faults will generally be handled and consumed by the memory manager, but if it wants to, the pages can simply be passed on to a driver to deal with.

I can add a PG_HOOK attribute.

# July 1

## Virtual Memory

The swap file is not really a sort of extention to the memory that can be allocated or something. It is a place on the disk where pages go when they are expelled from the RAM.

Non-present swapped out pages are always mapped and used in the page table entries. With that being said, I can use the address field of the PD to locate the missing page totally independently of the PBT or the address layout of the RAM.

Basically, we allocate page slots on the swap using a simple bit array. Then I can write the data there. When a page fault happens, the swapped out page will cause a fault.

> Maybe we should be able to know what processes own what memory. It seems useful for swapping.
> Also, it is important to have OOM killing. I may need to tell subsystems about this.

> Filesystem with alphabetical ordering of directory contents.

# July 7

## Memory Manager Ideas

I am specifying the MM API and I came up with a potential change to the memory block structure.

First of all, I will rename it from MB. The reason is that MB is defined as a macro. I will rename it to a MCB or something, since it will work like that soon.

MCBs are now 64-bit. They have the 16-bit back and front links, as well as a 32-bit handler procedure for collateral and hooked pages. That leaves me with one 16-bit field that is not being used.

My idea is to use the 16-bit field to store the number of contiguous frames after the current one. The rest of the MCBs can be zeroed or all-1'ed out to indicate they are not being used.

This CAN work with my idea of paging, but it would require cutting the blocks. This can be done, but the added complexity may not be worth it.

I need to find something else to do with the 16-bit field left over. Based on my calculations, it will occupy the same number of pages no matter if I keep it or remove it and go with a 48-bit MCB.

I CAN store the intended page attributes of the block. This can totally work, but it would be a bit iffy with virtual memory. The present bit obviously cannot be respected and there will be some other implied semantics. With that being said, it can totally be a good idea since there is persistence with the page attributes even if things happen to the mappings.

This means that mappings only need to be updated with a simply a chain ID, pointer, and count to remap those blocks to the address space.

### Handling of Accessed

I need to remember to keep track of the accessed bit. It could be useful for keeping track of page statistics.

That is actually a decent alternative to the current use of the 16-bit value. Maybe I can squeeze it in with a bit field. I only need 12 bits to represent the page bits,  and even then, I do not need all of them so I can probably reduce this.

The only issue is how that is actually supposed to be handled. Any demand pagable memory must keep this count updated.

There is no hardware mechanism for keeping an access count. The bit must be manually changed if it is used for collecting statistics.

I just looked it up. The accessed bit only indicates if the page tables were used to access it. The dirty bit indicates if it was written to. The dirty bit is intended to be used for demand paging because any pages that are dirty may need to be written back.

In the entire time I have been thinking about the virtual memory design, I never thought about the dirty bit. It is not actually needed. If the system memory is under pressure, it will simply swap some random pages out and leave a record to get it back.

It does have some utility for situations in which something in the memory must be at some point expelled to the disk only if it was written to. For example, a memory mapped file. A good chuck of the file will be in the memory, and some of the pages will actually reference sectors on the disk.

OS/90 can support these by adding even more page modifiers. For example, a modifier that indicates a random sector on the disk, with the size of the sector being basically transparent. For disk cache pages, another modifier can be used.

I only have two page mods left. I can get rid of the swap ones in favor of disk transfer pages.

I also need to consider how the current IO model is supposed to work with all this. The swap file (or partition later on) will be on the boot drive, which is something that must be figured out. DOS keeps track of the current drive so it should be possible to get the one that was used to boot with SV86.

> Maybe use a linked list for IO requests like DOS SYS drivers do. IDK when in doubt, always use a linked list.

### RMR Reconsidered

The reason I added the RMR was:
- to support dynamic arrays without having to use address space
- modify page table structures allocated in memory without using strange hacks

This is no longer needed. All memory control structures are now contained inside the kernel address space. There is no need for the RMR, and the original idea of the linked list array can be done without it.

# June 8

## IO Model

I am starting to think that my KFS idea may have been too complicated. It also appears that thread pooling is impractical because that would take up too much memory for the process blocks. Creating a task is not that slow anyway.

## Swapping

First of all, I do not need a special page mod for disk transfers. This can be implemented by a disk driver, which can use hooked pages and replace the address with the remap function.

## TODO

## Scheduler Ideas

I think I should change the time slice system. A more intuitive way to handle scheduling is to use percentages.

We can keep track of the total number of time slices issued, and the maximum can be 1000 miliseconds for one second. It is not hard to do this.

To give more time for one process, we must take from another. A "time pool" called the system idle process can give up time.

The problem is of course that only 1000 tasks can exist, but I remember having under 500 or so active threads on my Linux system. Should not be a problem.

Anyway, this is a WAY better idea that using time slices alone. Changing the time slice of one task cannot be done correctly if without regard for other tasks.

Each task will get at LEAST 1 MS. Once all time slices are being used, the CPU is at "100% usage"

# July 9

## IO Model

No KFS. Way too complicated and very little benefit. I think having shared memory pages with userspace where configuration options can be exposed is a better idea.

The current IO model is far too complicated. It is designed around the whole asyncrhonous printf example, which is pointless. There is no real improvement to system responsiveness by making printf threaded since printing more than 80 characters with it is usually wrong (unless using a file perhaps, which is soemhting else).

If I want to make something BETTER than other operating systems, I have to do things differently. It is also important to note that OS/90 is for single processor CPUs. The job of scheduling and the IO manager as well is to make sure that the CPU is always doing useful work, basically to keep the "flow" of data running while also improving the "smoothness" of the system.

The question of doing the work now or later or in bits and pieces is one of responsiveness as viewed by the user.

> Use an IO stack? Linked list? Both?

> Another thing about IO: Some IO does not need to be asynchronous simply because it has no chance of starving the system resources and does not depend on any other IO. Printf into a file is a different story, but the data is going to a buffer first. The system will already multithread this with no problems, and the buffer must be generated first before writing.

## TODO

I will start copying and pasting my TODOs.

- Driver global and local event handling (including termination of early boot services, keyboard may need that)
- How do we access task blocks? The RMR is very wasteful, but using a window would not be great either.
- Implement a fully working iterative non-recursive binary search. Ordered arrays can use this. Binary search does not need to look for a single specific value. Binary search can also use comparisons to find the place to insert a value. I may need to expand stack space for it to work and disable preemption.

# July 10

## Crazy Idea

Can I make OS/90 capable of running Win386 VXDs?

Using a compatibility layer could be possible but there is a lot of potential for instability and it could even be impossible to do. Without a full listing of the API, it is not possible to know.

> I can use virtual machines and trap any hardware access.

I think Wine implements some parts of the VxD API. Will check it out.

dosemu also does.

https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/krnl386.exe16/vxd.c

It actually does not look that hard! But I may need to use some sort of device stack to make it stable, combined with a user mode VM... if I need it of course.

I will need to look at some of the ASM headers in the DDK to make this fully work.

Actually, these are only a tiny subset of the actual VxD API. The real referrence is VDAG31.TXT. It is a very long document. Some of the calls are not that bad and mirror what I currently have.

To save time, I could try to find a way to emulate the entire VMM inside a virtual machine. I can use segmentation to control the address space and run it in user mode (since kernel does not use segments).


I will start copying and pasting my TODOs.

- Driver global and local event handling (including termination of early boot services, keyboard may need that)
- How do we access task blocks? The RMR is very wasteful, but using a window would not be great either.

> Note on DMA: The scheduler should prefer CPU-bound tasks rather than IO bound tasks when a DMA transfer is taking place. The kernel does not perform scheduling, so subsystems need to control it instead. How can I make it so they can cooperate?

# July 11

## More Crazy Ideas

A VxD compatibility layer would be quite challenging. While OS/90 has many of the same features, it will have to "fool" the VxD on many occasions.

There are a total of 209 API calls. Some of these are internally used and others could in theory do nothing at all because OS/90 just does not work the same way.

My main concern is how we would translate the memory manager calls. That would be very difficult because OS/90 and VMM32 do TOTALLY different things with memory management. VMM also allows for direct modification of page bits, which obviously cannot be allowed.

I can also use a full virtual machine to run the VxD. i386 does not have nested paging, so the MMU has to be simulated using the host paging.

## Why Single Address Space?

Multiple address spaces would actually cause very few issues for the implementation of mapping functions.

The only issue is that it becomes necessary to allocate page tables very carefully. The non-presence of page directory entries could be used to implement dynamic table allocation.

## 1MB Region Mapping

I can permit interrupt service routines to directly change the page tables with a special function. This is fine as long as some kind of lock is used.

## Switching to Digital Mars

DMC has a advantages over GCC:
- Potentially better code generation for old CPUs
- Better inline assembly that is not a pain to use (beside register clobbers)
- Support for segmentation (if I actually need that)

There are some difficulties with getting it to work with a 32-bit flat model. The default memory model is Windows NT, which is flat and 32-bit.

The memory model seems to determine what type of code is generated. DOSX can be used to compile to an object file. The linker needs special parameters to generate a flat binary.

So it looks like the command for the linker is:
```
optlink /BINARY:80000000 [object]
```

BINARY is intended to generate .SYS files for DOS which do not have a PSP. It gives a bunch of errors when I try to link but the .SYS file is generated. This .SYS file is also about 16K in size, which is strange. Must be because I picked DOSX as the link option.

No, DOSX do not work at all and says that 100h cannot be initialized. NT prints out all the errors and adds bloat to the code.

I just looked into it and apparently DMC was apparently written by one single person, the only example besides HolyC by Terry A. Davis. It makes so much sense for an OS written by one person to use it!

## The Actual Transistion

All of the compiler-specific things are in Type.h. This file has been around since the begining (hence my refusal to lower-case it) and is very gcc-specific.

First of all, the memory fence. I no longer use fences for variable accesses. I use basicatomic fenced move operations, and I can switch over to ASM statements for it too.

Second, the likely and unlikely branch things. The compiler has no way of knowing what branches are more likely and applies very basic heuristics if any to determine which one is the not-taken case and therefore faster. I would like to have this for the more algorithm-intensive sections. For now define it to do nothing special.

Third, the types. These may need to be adjusted since the ABI may be slightly different. Not hard to do; they have a table for size types.

#4, the string op macros. It is actually faster on the i386 and the i486 to use a subroutine for faster string ops. Using stack conventions reduces the register pressure for the compiler and a simple calculation can be done to maximize movsd iterations.

Example code:
```
; In C: void memcpy(void *dest, void *src, unsigned int count)
; OS/90 will use stdcall. Same order as cdecl, but callee clean.
;
    align 64
memcpy:
    push    esi
    push    edi
    cld

    mov     edi,[esp+8+(8)]
    mov     esi,[esp+8+(12)]
    mov     edx,[esp+8+(16)]

    cmp     edx,32
    jbe     .small_block

    mov     ecx,edx
    shr     ecx,2
    rep     movsd

    ; Copy remaining bytes
    mov     ecx,edx
    and     ecx,11b
    rep     movsb
    jmp     .end

.small_block:
    mov     ecx,edx
    rep     movsb
.end:
    pop     edi
    pop     esi
    ret     12
```

It is a win-win despite the seemingly slower initialization. The memory transfers for the string ops take up most of the CPU time and the code will run much faster on many systems.

Calling it can be smaller due to the push byte operation for the size of the operation if it us <=255.

# June 12

FOUND IT. I need to use the `-NL` option so that libraries are not imported. The linker does some strange things however. It still generates an MZ file, although the code is there and is correct. I provided the BINARY option.

Wow, the code size results were very close. 2755 for DMC and 2743 for GCC. And GCC was on `-Os` while DMC was optimizing for speed!

DMC goes down to 2208 when optimizing for size, a difference of 547. I am sure that DMC uses much more aggressive size optimizations, but that makes it totally viable now.

I will optimize for speed when compiling the kernel, but drivers can be size optimized.

## Toolchain Setup

I was able to invoke the linker with the BINARY option and the NL DMC option make the linking possible. The output is in MZ format, but the code seems correctly generated, except for the base address which is still base zero.

Okay, it seems there is no way to do it with the linker. I should generate a symbol list with the compiler instead. Then I can use a python script to perform relocations. Should be foolproof.

Not really. Not all relocations work using symbols only. There are functions that are not named and are local to each module, and they also need to be relocated. Variables must be relocated too.

I guess I should look into the open watcom linker. It is able to set the base address and uses simple command line arguments. I need to pass "-L" to make this work. OWL generates ELF executables too so I can put it through an extra layer of processing if I need to, but it probably should handle flat binaries fine.

This is one example of how it can be done:
```
 name kernel.bin                  # name of resulting binary
 output raw                       # type of binary (ELF, PE, MZ, PharLap, ...)
    offset=0x100000               # skip first meg; base address of binary
 file startup.obj                 # objects to link - this one has to be first
 file fdc.obj
 file gui.obj
 file idt.obj
 # add more obj files here
 order                            # in which order should segments be put into binary
    clname CODE offset=0x100000   # offset for reference fixups
    clname DATA
```

BSS is not included here since it is uninitialized. OS/90 requires it to be zeroed, so I need a way of somehow creating the necessary symbol. Alternatively, I can simply zero out the extended memory, but this would take too much time.

Section alignment is not really that important, but I do need to have a large enough alignment that the ALIGNs in the code are valid.

Page alignment is not a requirement. The only things that need to be page aligned are the page tables and the page directory, which are allocated sbrk-style after boot by the memory manager. Actually, the page directory is already is the HMA and there is only a need for one. We can leave that.

It seems that DMC does not allow for arbitrary alignment. However, it DOES allow for structure packing to be changed and this does mean alignment. If the members are aligned, surely the whole structure also is. I tried to align by 4096 and it did not work, so it must be 1,2,4,8.

Also, I now see the linker producing a .SYS file containing the correct code. Strange. It still has the wrong base address, so the linker is still not usable.

The plan:
- Build watcom linker
- Configure linker options for 1024-byte section align
- Migrate `Type.h`.
- Purge most basicatomic features except for the basic loads and stores. Consider removing those too.
- Add alignment garauntee structure types.
- Change asm statements to DMC format
- Update makefile to use different options and generate+link OMF files
- Update documentation to highlight all changes.

# July 13

I now have the watcom linker executable and the DLL it requires. WSL is able to run it. The location is `/mnt/c/WATCOM/binnt/wlink.exe`.

The linker options will be tough to set up. I think I should just generate an ELF file. The WLINK documentation is extremely long and 95% of it is useless.

Wait, maybe I should get the compiler working first? Until then, I have nothing to actually build. Work on the build system.

## Linker Strategy

- Build objects as OMF
- Merge with WLINK into ELF
- ELF is converted to flat binary

I will investigate the sections generated by Watcom so I know what changes need to be made to the linker script. The system linker should be safe enough since the compiler

If the default ld does anything weird with the final binary, I will surely switch to a proper linker.

## Makefile Changes

There is no need whatsoever to create the entire source tree for the object files. A better solution is to generate a unique number for each file that increments and link everything in one directory. Much simpler.

I no longer have to use the two stage linking process because open watcom's linker works fine. ChatGPT actually knew how to make a linker script.

## Assembler

NASM is giving me strange errors about an unrecognized segment value. Think I should change over to JWASM. There is also UASM.


REMEMBER THAT MASM TREATS SYMBOL REFERENCES AS MEMORY ACCESSES WITHOUT THE OFFSET KEYWORD!

# July 14

## Source Tree and Build System

To simplify the build system I unfoldered everything and added section prefixes to order the files alphabetically. This is actually easier to read on the explorer panel.

The build system will be even simpler now. I no longer need a makefile. A simple build script can do it all. There is no need to create a source tree folder structure in the build directory or to hash the names of the files and use the same directory.

The assembler apparently can also assemble using a wildcard too.

Wildcards did not work, but using a list of things to compile did. I now have a problem where DMC refuses to detect my include path, even if I change the INI. I will give it one more try.

Okay, I think things are working now. I just need to modify the code to get it to work.

Also, I found out something interesting about DMC. It has register variables! They are just variable names with underscore prefixes. This makes it possible to call assembly procedures without having to use a specific ABI. Maybe I can try to make a DOS program that prints hello world using that.

The compiler could not find a library but it did generate the right code.

## Ideas

During a DMA transfer, code that heavily uses tight loops should be prefered as it will allow the memory bus and the cache to be used without interference.

I think I need a notion of global scheduler states that determine what types of threads should be prefered at a given point in time.

## ABI Notes From DM Website

```
For 32-bit memory models
Functions can change the values in the EAX, ECX, EDX, ESI, EDI registers.

Functions must preserve the values in the EBX, ESI, EDI, EBP, ESP, SS, CS, DS registers (plus ES and GS for the NT memory model).

Always set the direction flag to forward.

To maximize speed on 32-bit buses, make sure data aligns along 32-Function return values

    For 16-bit models. If the return value for a function is short (a char, int, or near pointer) store it in the AX register, as in the previous example, expon2. If the return value is long, store the high word in the DX register and the low word in AX. To return a longer value, store the value in memory and return a pointer to the value.
    For 32-bit models. Return near pointers, ints, unsigned ints, chars, shorts, longs and unsigned longs in EAX. 32-bit models return far pointers in EDX, EAX, where EDX contains the segment and EAX contains the offset.
    When C linkage is in effect. Floats are returned in EAX and doubles in EDX, EAX, where EDX contains the most significant 32 bits and EAX the least significant.
    When C++ linkage is in effect. The compiler creates a temporary copy on the stack and returns a pointer to it.
```

## RMCS Fix

MASM and UASM cannot embed binaries. This is a significant drawback. The best way around it is to use the bootloader to do it! It can embed the binary because it is written in NASM and so is the RMCS.

The booloader only needs a few fixes now to add this. I can keep all of the original code.

## Build System

This is getting impossible. I tried a symlink for the path and it still does not work.

# July 15

OS/90 is approaching what could be considered the fourth rewite. I think there were three so far.

I am so lost right now. I need a proper TODO. Maybe a TODO file.

## The RMCA

The RMCA can be incbin'ed. UASM supports it, although it is not documented.

# July 16

I got IA32.ASM to assemble. Now I need SV86.ASM done. This will be a hard one.

# July 17

## Do I Keep SV86 Code?

I will keep it. No need to fully delete it. Just review it, remove anything that seems wrong, and refactor/fix.

## MASM PROC

MASM has a lot of implicit semantics with the PROC statement. It automatically generates the prologue and epilogue of functions under certain conditions.

See here:
https://stackoverflow.com/questions/57377397/calling-a-standard-library-function-in-masm

It does generate the `push ebp; mov ebp,esp` pair, but it also allows for quick restoring of registers that have been clobbered.

For my SV86 code, this is obviously not acceptable. I need full control here.

## Tabs and Spaces

I am officially switching to tabs+spaces. The file size is way too big with 8-byte indents. Right now the source code is 17MB of space, which is far too much. 50MB is the maximum for GIT, and I do not want to reach it.

Well to be fair, the code is only about 136K, and that actually includes all the object files! The manual is 68K.

Okay, I did some investigation. The git-related stuff it what takes up all the space. One .PACK file is 15MB.

I will switch to tab indents now. It makes no sense to use 8-space indents in an OS based on minimalism. Current kernel source size is actually 208K. I will see how low it goes.

# July 18

## Memory Detection

OS/90 does not need to detect memory at all! Just like any other boot environment, DOS has already detected the extended memory. To access more, a different XMS manager is needed.

To detect the exact amount of memory, an XMS block may need to be queried. All that needs to be done is a simple.

## Tabs Conversion

I dropped the file size by changing the license notices to a smaller format and also switching to tabs. The file size is now 118K.

## Write an IDE?

Can I write my own IDE for OS/90? I am getting a bit tired of VSCode. Maybe I can finish ATM/90 and make the IDE with that. I should add windows resize events so that it works properly. That way, I can run it directly within the OS.
(I also may want to port over to SDL3 and add support for other fonts. I do not want to be tied down to 8x8 since it looks bad on this screen.)

Alternatively I could use PDCurses so that it is truly portable.

> To be fair, I kind of hate CTRL/ALT/SHIFT keyboard shortcuts except for control.

> If I make an IDE, I could add some other neat features like a debugger window tree for whatever interface, breakpoints, a UI designer, etc.

# July 19

Why make an IDE if the current one works fine? Also, how do I expect to make Git work? I guess I can use a command script.

I should do a quick check on ATM/90.
Okay, it looks like this is an up-to-date version.

ATM/90 is a much more mindless project. As long as I put consistent effort, I will get closer to completion. It is also a bit more boring. Not much else can be said until I load the repo.

## Idea

I want to be able to have a key combination that allows for sending commands to drivers or drivers asking the user prompts. This can be done with the SysRq key.

In that case, early boot IO is not sufficient. I need a way to handle this situation at any point in time and exit.

This will be the "Kernel Control Panel"

It will look like this:

```
K~ lsdrv
# Name      Size    Cmdline
0 PCI
1 DOS32
2 ATA
3 FLOPPY
4 FAT32
5 EXECTVE.DRV
```

Maybe I should make an interpreted scripting language for this. User and kernel. (push, pop for procedures and jump locations?)

Idea, use numbered aliases! Combined with a push/pop method of function declaration or something, conditional local function definition could be possible. Numbers also help avoid the overhead of string lookup.

So basically these numbers are like registers. Interesting.

What if the program counter is the location in the file.

## DOS Extender?

I found out that DOS/4G runs in ring-0 under DOS and does not even use a TSS! This means I could use it to boot OS/90. The advantage would be no bootloader. The problem would be:
- Where does it even get loaded and mapped?


# July 20

## Complex Error Codes

Argument against:

Are these complex error codes a great idea? DOS is a simpler operating system so it was able to have all these really specific error codes. The value of the error information is significantly lost with the extended complexity of OS/90.

There are lots of errors that either:
- Happen for obvious reasons that are API-defined and can be recovered
- Non-recoverable according to API and require bluescreen
- May return various outputs that are device-specific and cannot be fully covered in Type.h

In general, the API describes the error behavior and advises how to respond. Allowing the behavior of function calls to drastically change between versions is a braindead idea. Consistency is always the most important thing in programming.

There are a few decent changes that could be made. The error code could have a component relevant to the user that is simply displayed but not acted upon. The Kernel Control Panel should make it very easy for the user to keep track of problems. I can add some sort of dmesg-style log buffer to go with it too (cache purging could be used). Regardless, the main purpose of anything that is not very specific to the API and describes a defined API call should be nothing more than a simple additional detail that could be used by a developer to track down where things went wrong.

A different approach would be to flat out use a string pointer for that extra infromation. The issue would be with alignment, since a few bits have to convey the real code.

Alternatively, I can also rely exclusively on logging.

One other problem with my complex error codes is the difficulty of making the APIs fully utilitze it, and potential problems resulting from changing return values by accident.

## Error Codes Decision

There should be two error codes: OK and FAIL. Nothing else.

> DOS made use of advanced error codes because it had no logging features.

# July 22

## Shell Interpreter

It seems like some types of tokens have multiple possibilities. For example, the `.call` command can receive expressions, symbols, values, etc.

Because there is no two stage interpretation with tokenization and lexical analysis, I must be able to determine what a space-delimited token is.

If it starts with a dot, it must be an internal command. Dollar sign means register. I can make `@` the variable indicator. Brackets are used for expressions. Anything else can then be assumed to be a regular command.

Okay, I thought about this for a moment. I do not technically have to do this. If the DOS subsystem is made a mandatory part of the OS, I can simply have an autoexec file run using the default command interpreter.

# July 23

## Remove permanent HMA?

The permanent HMA serves a few different purposes. First of all, it is necessary for the IRQ handler to switch to and from real mode. This requires memory somewhere in low memory to be ever-present so that it can be called.

Unless I use a disgusting memory manager hack to change a memory mapping.

# July 24

## HMA

No, the permanent HMA is not going anywhere, but I may want to find a few hacks to make it possible to fully emulate the real mode addressable memory.

## A20 Line

https://en.wikipedia.org/wiki/A20_line#Affected_programs

A few programs like MS Word Spellcheck, versions of MASM (though not any worth using on a 386), and Small-C depend on the A20 gate. I said at one point that I do not care about the A20 gate. This is still completely true, but after reading some of this information I am starting to reconsider my idea of doing nothing for A20.

I need a fixed mapping to use the physical ID-mapped HMA and a function to set it up and switch back to normal operations. This way, the A20 gate can be simulated. Simulation simply involves mapping the pages in the virtual HMA to the first 64K.

The reason why the address wrap feature was used was to decrease code size since they could use far pointers to the low memory without having to store the high component and could simply load FFFF into the register.

DOS did introduce features that allowed things to be loaded high to reduce conventional memory footprint, and these were in fact standard since DOS 4, and should not have been disabled unless required. Programs were given no garauntee of the HMA and only one could control it, and in fact there was a feature that blocked programs from using the HMA if they requested too few bytes.

Anyway, the software that depends on the A20 gate was outdated by 1989 or so. Real mode-only compilers and assemblers are not that useful on a 32-bit system, and newer version are available now.

> Conclusion: no A20 gate. I honestly do not care about it.

The HMA will contain only ring-0 pages. I will do my best to maximize the use of it.

## HMA Problems

The HMA map is wrong. While the stack is properly configured and the first two paging tables are correct, there are THREE paging tables including the PD.

The actual map inlcudes:
1. Page directory
2. Page table for first 1M (and HMA I guess)
3. Page table for kernel

Also, in the new HMA map, I will need to specify it based on pages. Nothing should be unaligned or non-granular whatsoever. The SV86 stack should also be alot smaller. 20K is WAY too much.

I will create a header file for this for assembly and C. They must be coherent.

## Plans For Source Code

I may want to unify the headers more. A single include will be used for drivers.

## System Call Table Problems

There is only one issue with using the call table, and there is a simple remedy. It must be internally used by the kernel if it is to be hookable. Otherwise, the kernel may bypass the hook when calling itself and make the whole thing pointless.

I could use patchable function entires of course, though I would have to test if they generate correctly first. I want DMC emitting the code BEFORE the epilogue.

Looks like it did work! I can use a multibyte NOP to incur a practically non-existent performance penalty. This allows the kernel to call itself and get hooked. This also allows for symbol exports to be used instead of a table in the first place!

The only issue is that driver sizes increases since symbols have to be named in the file. The symbols are not saved in memory once inserted, however. Symbols also reduce boot time due to string searching, although I did everything I could to reduce this.

So basically...

Table pros:
- Easy to hook
- Fast load time
Table cons:
- Slower call, although global cache can mitigate
- Easier to make mistakes in kernel code
- Annoying to code kernel with

Symbols+patch-hook pros:
- Posthook and prehook by changing prologue
- No mistakes with kernel code
- More readable code and less typing in kernel AND drivers
Cons:
- Slower load
- Kernel binary larger
- How the heck do I export the symbols?!

Symbols are more robust. I know VxD used call numbers to decent effect (which is basically like using a table), but I will do it the OS/2 way for once ever. With symbols, I do not have to worry about keeping structure compatiblity between version.

The only issue with this the actual implementation. How the HECK will that work? How are exports specified? How will the loader even insert the symbols and what structure must it use?

Of course, it has to mimick the internal structure of the executable, which must be fully loaded into a memory buffer.

# July 25

## Hooking Safety

What if a function is hooked while it is running? How do I deal with that?

## Testing Open Watcom

I would like to try the open watcom compiler on the printf implementation to see the code size so I cna see if it can outperform the DMC-generated code in size. Also, I will TIME both samples to see which one is genuinely faster.

The reason for this is that while DMC has WAY better inline assembly, Open Watcom was a much more well-reputed compiler among game developers and demosceners. If it is FASTER and generates smaller code, I will use it.

I will switch to the compiler that generates the fastest code. The test will be to printf a number of things.

```
printf_("Hello, world!\n");
printf_("Example\n %s %i %x %c", "Hello", 8086, 0xDEADBEEF, 'A');
```

Whichever runs this test faster wins. Measure-Command will be used to get the performance.

Both will be compiled for speed optimizations. I may reconsider the preference for speed if they are close and one has a better code size.

Let the games begin!

Okay, I got 9, 10, and 15 MS for the DMC version. This is not precise enough. It can only matter if I run it many times. I will run 100 iterations.

Still giving the same times. I think this is based on scheduler ticks that the program used. Of course, my CPU is so fast that it blazes through the whole program within the same time.

Okay, I will simply run it thousands of times. I will try 4000.

How about, 65536 times!

Results in MS:
- 392
- 408
- 399
- 408
- 411
- 407
- 414

I think I will go higher. Expecting this one to be very close.

Results in MS for DMC:
- 482
- 572
- 532
- 538
- 565
- 566
- 570

Starting to wonder if these statistics are worth anything. I think I should reduce the iterations, take more samples, and use a STOPWATCH for it.

## DMC Statistics

- 18.20 S
- 19.57 S
- 17.77 S

AVG = 18.513

## Open Watcom Statistics

Watcom made the binary for a simple hello world excessively large for some reason, so a code size comparison may not be possible. I will however notice any major differences.

Watcom does not like the source code. It seems to complain about the variadic arguments and apparently this compiler does not support C++ for iterators.

Looks like things are not too good for Watcom. It can't compile it! I need to find away because it looks like a very advanced compiler.

## July 27

What the actual heck! This has to be a compiler bug. It is giving the most nonsensical errors imaginable.

Maybe if I build the compiler from source it will do better?

Guess what? I literally don't care. I will treat the stack arguments like an array like I used to back in the day. I just need to make it compatible with the regular variadic features.

va_arg is just a pre-increment and a cast. va_list just needs a pointer to the first argument and a counter.

I already don't like this compiler, but I REALLY want to try using it. I read that it can also inline IO port instructions with the right options, which is a plus.

I found this:
https://www.youtube.com/watch?v=_7dkppo9VC4

Very interesting. This is playing a DOOM replay file. First comment is interesting. Apparently modern compiler technology really is better `:(`.

I mean, c'mon. I have seen the code GCC generates. Some of the tricks it has I could never think of myself when writing assembly. And when it is not outputting genius code it writes exactly what an assembly programmer would.

## Reminder

Read the TODO!

## Printf

The printf implementation is actually incredibly bloated. It should be be used for regular logging purposes. According to Godbolt, it uses over 224 bytes of stack, which is definetely not an indicator of good performance.

I don't really feel like changing it though.

## Going Back to GCC

I have to do this now. It won't really be that hard, and I can keep the basic idea of my build script.

I am thinking of using DJGPP so that OS/90 can bootstrap. It supports GCC 10, which is VERY recent. The only parts that cannot be done in DOS are the emulation and image writing things, but building the entire OS is not hard.

I need a uniform way of describing components and their install locations though. This will be done is DOS btw.

```
```

The naming conventions will change. C sources will need to use 6.3-style notation because of the prefix.

## Using DOS as Development Environment

This is now totally possible. I only need to find a way to mount a disk to install the files. The built-in IDE RHIDE can be used to host the editor and compiler. It will be complicated to get the journal to work because RHIDE does not seem to word wrap by default.

I probably should not, but I keep getting this delusion that coding like it is the 1990's will motivate me or something.

A named pipe can be used to activate the virtual machine. Mounting the disk that the emulator should use can allow me to install the modules.

# July 27

## OS/90 Directory Structure

Because I am introducing the idea of modules, I should specify where they are installed.

```
OS90/
    SYS/
        PCI.RZM
        ATA.RZM
    SUBSYS/
        386DOS.RZM
        DOS/
            AUTOEXEC.BAT
    KERNEL.BIN // Or something else idk
```

The DOS subsystem is a special one. It has its own AUTOEXEC file that it runs automatically using the COMMAND.COM inside the root directory.
Most importantly, DSS allows direct access to the DOS FS since the interface is 100% compatible.

## Subsystems

I wonder what other subsystems I can add. A Linux subsystem is theoretically possible but would be extremely complicated. Also I don't like the "muh security" idea behind Linux and wonder if I could theoretically get away with not taking file permissions seriously at all.

Win16 is something I may want to consider, but the 32-bit enhanced versions probably make VxD calls that are hard to emulate, if not impossible. I have a full VxD API reference which I could use to make VxD loading possible, but of course that would not be very much fun.

Okay, but consider this. What VxDs are actually remotely useful on OS/90? Any common drivers for things like the mouse, keyboard, graphics, etc. probably mangle with Windows internals and depend on Windows existing, and if not, they are simple enough to be implemented natively and with much better performance.

VxD emulation is only useful as a bragging rights thing. The amount of time that I would have to sink would be atronomical and for very little gain.

## Windows Compatibility Again

Windows 3.1 can run as a simple DOS application, at least theoretically. If DPMI already exists, Windows will use that instead. In the end, Windows can be executed inside OS/90. The only problem is with integrating it in a way that is useful for the user.

Windows as a subsystem would not be that great since I would need
- The entire KERNEL interface imlemented
- NE loader
- A bunch of other things I am forgetting
- A 16-bit compiler that will work for this, or some kind of thunk mechanism

## Here we go Again...

WHY SUBSYSTEMS!!!!

I will probably come to the same exact conclusion, but if I dont, I am fine with that too as long is it leads to the OS being dumbed down even further.

> Note: DSS will need to be able to multithread DOS programs. This is necessary for implementing an OS like ELKS or even Linux under DPMI. To do this, multiple tasks must be owned by a single DOS box.

Consider the potential problems associated with the current subsystem design. Can I redirect the standard output of a DOS program into one for a different OS like lets just say Linux? Not really possible unless I use some kind of common interface.

On he other hand, implementing DOS as the building block permits the same interface to be used for several related subsystems.

Basically:
```
    DOS
-------------------
    |   |       |
 ELKS  WinNT    Just DOS
```

DJGPP is proof that a UNIX-like interface for DOS is perfectly possible. Even DesqView/X proves even further that DOS really is all you need.

The whole standard output thing is something I have spent time before thinking about. In the current subsystem model, DOS machines are separated and standard output just goes to a virtual framebuffer. While that should exist in the old-now-new model, a DOS box does not need to be locked down as much. INT21H AH=9 can be outputted to standard output, which can be processed by the OS.

A subsystem driver is technically needed, but the only purpose is to simply perform the tasks that obviously cannot be done by DOS in real mode.

## Making It Happen

Guess we really have to go all the way back now, since I already thought about how the architecture would be if it was formed around DOS.

My first design showed its poor thinking because it required the PnP section to be part of the kernel. It could not even handle interrupts without "plug-and-play," which worked even without actual PnP BIOS support.

My second-or-so design got closer to DOS and DPMI compatibility, but there were a number of conflicts and difficulties which I need to deal with right now once more.

First of all, what happens when the INT instruction is called? What does the IDT contain? I found that using IOPL=3 to go directly to the IDT was a very difficult thing to use, but I will review it. INT can be made to cause an exception instead and be handled that way.

I will not go by what I already wrote about this topic because last time did not work out.

### Solution One: Exceptions

A general protection fault handles everything. The IDT is filled with NULL ring-0 descriptors that cause a protection error for protected mode, and V86/SV86 both use IOPL=0. Attempts to change IOPL are rejected.

### We Have A Solution

That is literally the only way I can have it now. Anything else would be a tangled mess of nested if statements and I want none of that.

DPMI can be mostly decoupled from the scheduler, except for V86 and SV86 which is barely its business anyway. DPMI will simply hook the second-stage exception handler.

If it detects an INT 10H, it will know not to call the exception handler there but instead attempt to perform the whole DPMI handling chain of events.

# July 28

## Code Organization

I need to FULLY separate DPMI from basically everything else

## DPMI and Subprograms

When a DOS program creates another, it will obviously not multithread, at least not automatically, but the task that started it will be frozen. This is necessary for DPMI programs like text editors that can open a shell. The return code byte is conveyed by writing to the parent task block.

TSRs in practice they have limited functionality, but will work under DOS emulation.

## Improvement to Uncommitted Memory

Instead of looking up tables and wasting time, I can simply indicate the exact chain that an uncommitted page belongs to directly in the page table entry.

## Virtual Address Spaces

I can improve the ability to find free address spaces by using what is basically the same as heap allocation.

A new page modifier is used to indicate the header of a address range. It will have to link with the other ones.

This allows for the best fit algorithm to actually be fast.

Furthermore, things that work for heaps can be done here too. Blocks can be coallesced to reduce lookup times.

The PTEs are initially zeroed to indicate that they are unallocated. Allocating VAS is done by looking for anything in the free list that is available and then generating a zero...

# July 29

## Reconsidering Symbol Exports

Now that I am using GCC, I should be able to generate the patch regions for API calls. The only problem is that GCC seems to use inefficient sequences, using single byte NOPs. I can of course change this at run time.

There are problems with directly changing an API call entry code. If the procedure is already running.

Swapping is supposed to be built on hooks. I need to figure this out.

Symbols are supposed to be faster because near calls are used, though some extra bloat is added to the executable in the form of string information.

Symbols also make the executable format actually useful. Without it, I might as well just have a flat binary with some relocations. I will keep the specification of course.

Actually, what I have said so far is kind of nonsense. Symbols are 100% possible and I would prefer that too, but extra effort is needed. It is still worth it. Not having to keep a compatible call table across versions seems like a good thing.

When a function is being hooked, nothing should call that function and it should not be running already.

> Symbols HAVE to be used, or the kernel would have to be rewritten so that every self-referencing call invokes the hooked function.

## You Know What

Is relying on hooks bad design? I think so. The only reason I even want to use hooks is to implement swapping with minimal effort.

In that case, why not just have a call table for the memory manager? Why wast 4096 bytes (aligned BTW) and add pointless overhead for no reason?

And with symbols and patching, why add some needlessly complex locking mechanism to safely hook things? Just use a call table for the memory manager and that should be good enough.

Even better, I can simply design the memory manager even better and not rely on hooks at all. There are only three or so functions that assist with swapping.

## Virtual Address Spaces

This whole free list thing seems to be quite compicated. It will probably need a third entry to define the actual size.

Also, fragmentation of the virtual address space could be partially alleviated by allocating much larger chunks or forcing a certain minimum size.

For example, allocating 2 pages vs 1 page could simple allocate 4 pages no matter what.

I can also use other heuristic measures to reduce it. Overallocation can be used.

Example:
- We have these chains: 384K and 128K, and 512K
- This is very large, but does illustrate the issue.
- The 128K chain will cause significant fragmentation
- The solution is to give each block approximately the same number of page mappings.
- If we give each 512K or 128 pages, a total 384 page table entries are used.
- The total allocation is 1M.
- Assuming the system has about 2M of extended memory, the x4 rule suggests an 8M address space size or two page tables.
- There are 2048 entries in our page tables.
- 384/2048 = 18.75% of total entries.

> Correction: Mapping real mode memory requires one entire page table. The kernel requires other one. In total, there are actually 4, but two are generated for the extended memory.

I could use a conversion rate so that I can get a non-linear scale. There is another solution.

I can also keep track of the size requested by the last allocation and try to keep it constant.

> I do need to report the actual number of page mappings allocated. Software needs to know about this to take advantage of the overallocation.

I do also need to remember that fragmentation is an issue of timing and ordering. In the ideal situation, fragmentation would never happen. In my previous example, how would I know to allocate 512K of mappings if it came in that order?

Hints could be used as an alternative. Memory that is unlikely to be resized can be placed in one region while smaller things that do resize can simply fragment each other somewhere else.

As a general rule, do not always consider larger blocks an immediate benefit. There needs to be a trick to it, or it will just make fragmentation more costly.

Try this: use a step function to determine the thresholds. These can be configurable with macros.

Another idea: Keep track of the average allocation size and if a new range is drastically higher, do something different. Maybe the average can be used to compute the overcommit.

## For Now

I can do it the dumb way for now, but I do need to support the new way to do it. The size of a page allocation must be reported.

## Limitations

The next pointer of the PFE is a signed 16-bit number. This is NOT good. It NEEDS to be bigger of the OS will not be able to handle more than 128M of RAM. I want the OS to handle over 2GB if possible.

There is currently a page proc inside the PFD. Remember that a page proc is a function called whenever a page is accessed as part of page hooking. Page table entries are not large enough for this, so the idea was to put it in a PFD.

This is terrible. PFDs are now 12 bytes which is not even and is larger than originally intended.

I will simply remove it then. Whether I need it or not, I cannot have it.

16-bit values for next pointers restrict the system to 256M. That is not acceptable either. Windows 98 can do 1GB and I have to beat it.

Oh also, it does not HAVE to be even. Also, even 16-bytes is actually almost nothing, even for a computer with a 1M stick with 393,216 bytes of memory only needs 1536 to represent all 96 pages. In practice, 4K will be allocated by the kernel, but still practically nothing.

Using larger entries, it is possible to use pointers rather than indices, which improves performance be reducing register pressure on the compiler.

```
typedef struct {
	SHORT           page_bits;
	SHORT           rel_index;
	PVOID           next;
	PVOID           prev;
}PFD,*P_PFD;
```

This is what I have now. 12-byte structure and 4-byte aligned.

I changed it to be 16-byte. This is more optimal as it allows the SHL instruction to be used. No LEA, but still better.

## Virtual Memory

I can do swapping a little differently. Instead of swapping pages only when memory is low, I can make swapping a bit more transparent.

Pages in the VA can be hooked to invoke disk transfers. This would involve using a temporary buffer large enough to hold two sectors or two pages, whichever is larger.

The page still remains "out" of memory. It just needs a small buffer to make it accessible.

This is basically demand paging by definition. We only keep enough memory to make the data on the disk available through mapping. With proper synchronization techniques, this can be done with minimal down time.

I will think about this tomorrow a bit more.

Okay, some ideas. The transfer buffer is basically passed around between tasks that are demand paging. When one needs to access swapable memory, it will have the virtual address mapped to the transfer buffer and the disk transfer is done.

I could of course consider the entire memory to be a disk transfer area, but having it preallocated sounds better. I can also allow for multiple swap transfer buffers and use a semaphore.

## ABI

I will bring back the old ABI. It generates much leaner code.

# July 30

Make the UI kit with DJGPP and DOS? It would be a good way to test the performance. I could also make the UI real mode DOS-compatible, although that would have limitted use.

# August 1

## Tasks

How do I modify the registers of a task and how are they actually saved?

When a task enters the kernel, the registers are saved to the stack. If the kernel thread is preempted by another, the state must be maintained by also pushing to the stack, or something. Really?

> Preemption is the important thing here. My decision needs to be based on the scheduler design.

The registers of the user are on the stack. These values are always restored before entering ring-3 again. This means that if the kernel must modify the registers of a task, it can do so by modifying the stack frame.

Switching from the kernel to any other task requires its own save buffer since it is preemptible and cannot share with user mode.

But what happens in the scheduler?

The stack frame of the user entry to ring-0 does not exist at all unless the thread is in kernel mode. If the thread is in user mode, it will, upon being unscheduled, save to the same buffer as the kernel.

The scheduler tick does not interact with the system entry stack frame at all. It simply does not need to. Its only purpose is to return to what will eventually become the saved context of the task.

# August 3

## M_Map Inquiry

Is this function too complicated?
```
STAT M_Map(LONG chain, PVOID baseaddr, LONG start, LONG len, LONG attr_override)
```

The idea of this is to permit dynamic structures. I can remap parts of a chain instead of the whole thing and override the page bits.

This is apparently a very dumb idea. Why bother with this complexity when I will end of HAVING to scan the chain no matter what?

I will explain more. The attribute override thing could be useful, sure. But why do I need to specify what needs to be remapped? How am I supposed to keep track of it in a practical scenario?

Okay, to the point now. If I want to remap a part fo the chain, I will have to scan no more than the whole chain. If so, why must I specify what needs to be remapped if I have to iterate anyway and access memory while doing so?

The alternative is add a flag in the PFD that indicates the page frame has been mapped. To unmap it, I mark the page frame as needing remapping and remap the memory.

I am starting to rethink the whole page bits in the PFD thing. Why do this? It just makes remapping more inefficient unless I change it after mapping.

M_Alloc is supposed to be barebones, not a real heap allocator. A proper malloc should be built on top of it. M_Alloc/M_Map are mmap but only for memory. Dynamic allocation should use a heap that supports compact and page-based allocations.

The changes I will make:
- STAT M_Map(LONG chain, PVOID base)

Proper memory management would include

# August 4

## New Things

To properly change page bits, I will use a 32-bit mask where the high bits are the "bother with" bits and the low 16-bit value is the "set to" bits.

This does not mean anything for the initial mapping, but does no harm.

## Swapping

I wrote some of the specs for memory swapping, and I am wondering how far I can go with it.

What if I made the swap an extension to physical memory that can be allocated using M_Alloc?

Sure I could, but the demand paging thing is still essentially the same. The swap file can be allocated using a bitmap.

No matter what I decide to do, swap cannot be transparently accessed. Unless perhaps I emulate any instruction that accesses it, which actually would not be too bad because swapping is already slow.

Even with emulation, there still needs to be buffering. Pointless idea.

### Algorithm

It could be theoretically possible to swap out pages that are not frequently used. The dirty bit is usable for this. The only issue is that it requires monitoring. Perhaps page faults from not P pages can be used to get a count.

It would slow down the computer to have this running in the background.

It is also possible to do userspace virtual page faults. The OS can let swapping be done by something that knows more about the working set and exposes proper memory managent systems.

### Page Frame Descriptor Changes

The PFD is now this:
```
	PAGE_PROC       proc;
	PVOID           next;
	PVOID           prev;
	SHORT           rel_index;
```

What are these page procs and what do I expect to do with them? Apparently I wanted to have them be used to implement swapping and page hooking. At this point, page procs could be used for uncommitted memory, but that only works for virtual address spaces, not physical. I got rid of page flags in the PFD, so consistency would call for the removal of the page proc. It is also way too large.

```
	PVOID           next;
	PVOID           prev;
	SHORT           rel_index;
```

Finding out the size of a chain currently requires iteration. I am not sure how often this needs to happen. I see no real need to cache it.

### Back To Swapping

Demand paging will of course be used, and I want DPMI clients to be able to take advantage of it. For that to work, the userspace page faults of DPMI 1.0 cannot be supported (which is acceptable and reported by DPMI).

I do not want to implement swapping or really even think about it right now. I just need the interface.

I will shoehorn the swapping into the OS when I decide it is time. For now, I won't even bother.

## DPMI DOS Translation

I can implement a certain level of protected mode translation for DOS services. I could make OS/90 capable of loading `KRNL286.EXE`.

Maybe later

# August 6

## Collateral Pages: Totally Useless?

Why is this in any way whatsoever a good idea?

I need to stop getting clever with the memory manager because it has lead nowhere good. Collateral pages are a perfect example.

Okay, so we are out of memory. My original idea was to not have the allocator fail but do everything to fullfill the allocation. Suppose this happens. Caches will be purged.

This is not fast, but holding the chain ID in the PTE is a good idea and makes it faster to deallocate the data. I suppose the entire chain is collateral.

The problem is that each page will have the procedure run. Collateral pages are not fast and just kludge up the memory manager.

Instead, I can fail the physical memory allocation. Another function can be responsible for allocating pagable memory mapped to an address backed by either RAM or the disk. Virtual and physical memory need to be separated totally.

# August 7

## No "features"

I need to worry less about "features" right now. Every single "thing" I add or minor performance concern I have increases the time to finish this project quadratically but most importantly, leads to zero fun or enjoyment.

Features are so cringe. I need to add less of them. Minimalism versus "doing everything properly" is the difference between getting the OS to kind of compile in the next few months or the next year.

Do not optimize anything that is not speed critical. Specify the interface before writing any code. Make all code reusable because everything is subject to change.

## Uncommitted Memory

This is very important to support, although not required for DPMI 0.9 compliance. If I do not add it right away, I definetely want it to be possible.

I am going to fully separate physical memory from virtual memory. That means PFDs must not contain anything related to memory.

The current design is good. The 20-bit value will be a chain ID.

## M_Map

The reason why the old version allowed for mapping only parts of a chain was for dynamic data structures. This is actually a bad idea. Dynamic data structures do not need to use one chain. OS/90 can allocate as many chains as there are page frames on the memory. Chains can be mapped side-by-side. Growing and expanding can be kept track of easily as it is essentially like implementing `sbrk`.

Allocating may not be cheap, but neither is resizing.

## Uncommitted Memory Changes

UCM will now allocate a new chain every time it is accessed. Resizing is not better because it is just as slow.

## New Function for MM: M_VmCopy

This will allow for copying allocated virtual address spaces.

## Allocation Speedup Idea

I can keep track of contiguous ranges.

## Different Structure?

I could consider a faster algorithm for page table allocation. A tree-like structure could be used, but I am not yet sure how.

Maybe the whole memory can be represented as one single tree entry. Allocating is done by halving it until reaching the necessary block size or something.

Suppose we have 4M or extended memory and we allocate 4K.

```
4096 = 4,194,304(1/2)^x
```

According to the Desmos, it will take ten entries to represent the allocation. This gets higher as the memory size goes up.

It is unreasonable to preallocate enough memory to control the entire physical address space, so it would have to be reduced, which means that failures can happen at any time. Also, uncommitted memory cannot simply make a new chain because chains become a scarce resource.

This will punish small allocations and reward larger ones.

I could alter the design to not halve but instead divide by 4.

Okay, interesting idea, but how does this actually manage non-contiguous blocks of memory? Maybe the tree can side-link.

The tree itself is only used to manage fragments of memory. Side links can produce the chain.

But then why bother? Maybe for malloc/free it may work, but if I introduce a chain it makes the whole thing pointless.

Not really. The speed of allocation is increased for large chunks somewhat since a large number of iterations can be avoided. The problem is allocating the 4K page fragments. Exponential increase in page fragments based on total memory sounds bad. It's not linear, so the increase would eventually clog the memory.

# August 9

## Userspace Interrupts Revisited

Passing control to a ring-3 code segment from an ISR is a bad idea for reasons I have already explained months ago. Another way to do this kind of thing is to use a far call or something.

> Probably not going to do this.

## Virtual Memory

I have written some information about demand paging, but the whole procedure is not fully clarified. How do I allocate swap space, for example?

I think I need to make changes to the memory manager. It needs to be a zone-based allocator. This way, it can allocate chains for physical memory and swap pages. Zones could also allow the allocation of DMA buffers in the 15M region. These changes are not major.

My goal is to bridge the gap as much as possible between RAM and the disk. Demand paging is one of those ways, but having a global buffer for it does not sound like a good idea.

I could design a way to encapsulate paged memory allocations and set certain quotas, such as forcing a certain amount to be present and serve as a buffer.

Probably not, but just as an idea.

Basically, I will have multiple chain types. Mapping them can be done using physical or virtual memory mapping.

Typical allocations for software would involve multiple chains.

There is no longer a need for chain-local indices because we are now using multiple chains. Eviction to the disk can involve splitting the chain, which is not impossible and actually makes sense. The operation can even return the new chain IDs.

Remeber, chains are meant to simply keep track of groups of non-contiguous data that are free in an array of memory units. Anything can be kept track of using a chain. It is like the FAT filesystem.

Actual swapping is complicated. I cannot remove a page at any point in time because there is a risk of a deadlock due to a lack of a transfer buffer if the system is low on memory.

While evicting a page, that page cannot be accessed anymore. Anything swapping related will surely have to disable preemption or there is a great risk of a deadlock due to no transfer buffers or something else. I could stall every task that tries to access memory, but that has a chance of freezing the whole system.


There is also another thing to note. If memory is swapped out, how do I know when to swap it back in? All this talk about merging the disk and the RAM into one is nice, but is it actually practical?

The point of swapping is not to put data on the disk. It is to save memory. The intention is to put that data back in the memory.

While the idea of being able to allocate and map swap space like regular RAM sounds interesting, it is actually more complicated.

Like I said previously, swapping is intended to temporarily remove data from memory. I can do the whole mapping swap into memory like RAM, but swapping is a different thing altogether.

If pages are permanently "out" and disk mapped, it just means heavy IO occurs every time it is accessed.

Well, not really. A buffer exists for the transfer that makes it faster, but the buffer is limited and several programs may want a chunk. That is simply a problem with demand paging itself.

Also, random note on uncommitted memory. Iterating from the bottom of the PFT is not necessary. Because only one page frame is being allocated, it is much better to scan from the center and check both halves, with the top half being more worthwhile to check first.

## VM Ideas

I can keep track of the number of page faults a task has caused. Swapping out pages of a task that is already page faulting constantly would only make things worse.

It will be measured in page faults per second

## What I Have to Do

I will rewrite the memory document and fully describe everything. I also think I should change the names of some data structures. PFD is not very memorable.

MPAT: Memory page allocation table (0 and 1)
SPAT: Swap page allocation table

The chain allocation functions will become zone-based allocators with structures maintaining their states.

There will be a zone for 1M+64K to 15M and 17M-whatever.

I also think uncommitted memory can be rethought. Allowing individual pages to be uncommitted may not be good for performance. Memory is usually allocated sequentially by programs too.

## Zone Allocator

# August 11

## Zone Allocator

I need to have a method of knowing what process allocated the memory. Causing a memory leak when forcefully termminating a program is unacceptable, especially for conventional memory.

I can maintain a fixed array of allocation chains along with a zone tag. This can work, but is limited. There is another way.

Another way is to have side links on the zone entries. This is better than using a a table because there is no built-in limit. The first entry should have the side link.

The side links form a singly-linked list. When deallocating, encoutering a NULL reference means that it must end right there.

This would require keeping track of the last chain ID for processes. I would like this to be fully automatic and unrelated to processes.

### Alternative #1

Chains no longer have a more weakly defined end and size. Any part of a chain can be treated as a separate chain. A random block in the chain can be resized into the main one or beyond.

The problem is that it does not actually permit instant dealloation.

### Create a Zone For Each Process

Crazy idea. Technically possible. A subzone is doable.

In this model, program most likely get contiguous memory and it is non-resizable.

The advantage is that the zone can be fully deleted at once.

But how? I do not have a measure for deleting a zone.

Bad idea.

### Fixed List

The process contains chains and a zone table tag. For example, 32 chains and 8 32-bit packed zone table indices.

Most DOS programs with DPMI support allocate a giant chunk and use their own allocator. DPMI servers often had fixed limits.

### Quick Note

Virtual memory can be greatly enhanced by grouping allocations. I can swap PROGRAMS rather than random pages. This can permit more intelligent swapping.

As stated before, a page fault count is quite useful.

### Ring-3 IRQs

By using a ring-0 alias descriptor, I can make it perfectly possible to call protected mode interrupt service routines directly. Doing this in real mode is a bit more of a challenge.

OS/90 sends IRQs to actual real mode. This makes protected mode callbacks difficult or impossible.

> I need to think about this more.

If an actual IRQ is received, it cannot literally go to a real mode program. Even if it is non-DPMI, this is a bad idea that makes it too convoluded.

Fake IRQs are better. Just schedule the event within TI to be handled in T2 later. This would normally lead to massive latency, but there are solutions like exclusive tasking.

### Exclusive Tasking

True single tasking is impossible because mutex locks implicitly yield, and non-preemptible threads are permitted to acquire locks.

Scheduler modifications can be made to disable yielding entirely, although this raises the risk of deadlocks.

This means that fake interrupt handling latency will probably be bad, unless I switch to a task that needs to handle one.

# August 12

## Zone Allocator

The zone allocator is basically finished. I suppose DPMI memory will use fixed allocations.

Should it though? Reconsider.

If I use a linked list, there will be only one link. It will be a circular list so we do not need to know where it starts.

Only the first entry in the chain will use the auxillary pointer.

Entries can only be added to a group. They cannot be removed. The point is to deallocate at once.

Wait, I cannot have a circular array with no double links, especially if I do not know where it started.

# August 13

Most programs will implement their own memory allocation systems. The NASM source code has `nasm_malloc` and `nasm_free`. DOOM has its own heap manager.

The page granularity of the allocation (something warned about on the DPMI spec) is what makes it ineffective for general purpose use.

I can have 24 or 32 allocations available. They cannot be physical memory allocations, but virtual memory regions with the possibility of swapping.

## More Swap Ideas

Since demand paging will be used, I have some ideas that would make it useful.

The page file is allocatable, so I will make it so that data can be copied to it.

I also should have a VirtualAlloc function call to allocate virtual memory. This will be very similar to the Windows NT API call, but with some differences.

## Documentation

The doc for virtual memory and swapping should be cleaned up and have some parts separated, as it contains implementation details.

## Swapping

Making swapping perform well is more important than maintaining an advanced orthogonal address space idea.

When something is removed from memory to the swap, there needs to be a way to put it back in. It cannot stay there forever.

I can do something with timing. Swapped-out regions can have some sort of priority-based scheduling that swaps out pages that are percieved as more important. The problem is that if this is not done right, it could actually cause more disk IO and not improve performance.

Process "importance" or "priority" is a better way to deal with swapping. Paging out memory used by a less important process has a much more noticable impact to the user.

## Loading Executable Example

If an executable is larger than the memory, it is necessary to allocate the largest block of physical memory possible and then swap space. I may need a function to allocate the largest chain possible.

There are alternative methods. A smaller buffer can be used to load to both RAM and the disk. The executable data can go into another chain that grows in size.

## OS Page Flags

I would like to have a page fault counter built into the page table entries. This would be possible if some PTE numbers are removed.

- Collateral pages will be removed.
- Page hooks are removed

Virtual memory will work differently now and the attributes may need to change. The use of allocation zones will also change how this works.

Chains cannot be represented in pages because the zone cannot be identified and virtual memory will require multiple chains to work properly now.

# August 18

Last weekend of the summer and I am going to college. Not a bad thing at all. I have always gotten more done while going to school.

Overall, I did not really accomplish a whole lot, and it appears that I went backward if anything. I will come back out of this stronger though.

## Boot Configuration

I can use `CONFIG.SYS`. Unrecognized directives do not cause boot errors and are actually used by OS/2 to set certain options.

Here is a list of potential options and example inputs:
```
OS90_VAS_SIZE=
OS90_Z0_GRAN
OS90_Z1_GRAN
OS90_Z2_GRAN

OS90_DEVICE=C:\OS90\DRV\PCI.RSX
OS90_ONSTART=C:\OS90\ATMDESK.EXE /S=80,50
```

It seems like DOS may give warning messages when they are not recognized. I could use the environment instead.

# August 19

The environment segment by default is very small. It needs to be enlarged in the configuration for most users.

Otherwise, it is good enough for configuration options.

## Driver Names

They are now RSX for Resident System Extension

## DOS Emulation Handling

The command.com that executed the bootloader along with the bootloader itself need to be freed from memory. There are actually methods of finding the allocation of the initial COMMAND.COM. The PSP will already point to the bootloader on startup.

This document explains how to locate `COMMAND.COM`.
```
http://www.piclist.com/techref/dos/psps.htm
```

The method is follow the parent PSP until the value of the last PSP is equal to that of the current program.

This allows for getting the environment of command.com. This is dynamically allocated and not part of the executable image.

The first segment entry in the PSP can be used to locate the executable data, which may then be freed.

The current PSP is then inaccurate because no program is actually running. When DOS needs to be entered, the PSP must be set appropriately within a preemption off section. DOS requires the PSP to be correct and has an internal variable for it. Local data such as the JFT are part of the PSP.

### INT

INTxH is the generic interface used by drivers or the kernel to call real mode services. It is not enough on its own to allow for full emulation of all DOS services.

DOS programs will sometimes use services that only INTxH can access, but there are a few things that need to be added to make it work.

Calls to the DOS API require:
- Preemption is disabled before doing anything else
- Program segment prefix is set to currently active process
- All hooks made by the client are processed
- All hooks made by server software are also processed.
- INTxH can be called.

Preemption really HAS to be disabled while in SV86. There will be direct hardware access and other things that cannot be tolerated in a multitasking environment. It is not a big deal. If multitasking needs to be improved, simply use 32-bit drivers and avoid that.

Also, if I want to recycle the existing SV86 code (do I still have it?) I can adjust the calling conventions.

### General Ideas

Do not worry about DPMI yet. It is a simple extension of an exisitng interface. I will have structure fields for it, but no support until I can actually run DOS programs, which is not too far out actually.

## Build System

I have flattened all of the header files and source files. This is actually very common in many projects since multiple directory levels are not necessary for headers and code directories.

## My Next Laptop

If I get a new laptop, I may need to get an x86-based one. Open watcom does not run on ARM-based computers.

Windows for ARM does have emulation for x86 programs. From what I have heard the performance is decent enough, so it could work, but how would I actually install the toolchain? I also need to make sure it runs 32-bit apps, since watcom might not be 64-bit.

32-bit programs are actually supported.

# August 20

## Next Laptop

The OS is now moving to a Windows/DOS-based toolchain, although UNIX can obviously be supported.

## Moving To Windows

It's so over for my Linux install. Should back up the data if I want to format that drive.

For now, I need to convert a few scripts to Windows.

The build system is simple enough. All source trees are flat and can be compiled with a single command. That is the correct way to do it.


# August 21

## Name

I am not going to make a full rebrand totally out of the question. There is no issue since the project is not done yet. The main problem will be updating the documentation.

First of all, it needs to be very clear that OS/90 is in fact an operating system. I like having OS in it, or something that implies an operating system.

I can take a jab at OS/2 and name it OS/3, but that is just a little too aggressive. OS/2 deserves it though. The moment I found out that OS/2 uses ring 1 and 2 and has a FREAKING 16-BIT USB DRIVER I stopped caring about the thing. Like are you serious? 16-bit USB driver? ArcaOS had to get rid of that.

The whole thing is a bunch of layered model garbage. I don't care if anyone tries to debunk my claims here, I will stand by my belief that OS/2 is a trash operating system architecturally.

OS/3 gives OS/2 exactly what it deserves, but it also implies that OS/90 is a successor, which it is not. ArcaOS is.

PC-MOS is already taken, and it's a really good name.

I think I should stick to OS/90.

# August 23

## Printf implementation

The current printf implementation is horrendously bloated. I am 100% I can make a better one that uses less stack, no offense.

I have some code that works for converting integers to strings. There is code for hex too, which is a lot simpler.

Hex conversion is essentially a shift, and, table lookup. The buffer can be fixed size and always output 8 characters.

```
emitf("$hh", 0xDEADBEEF); // 0DEADBEEFh
```

## Segments

I hate segmentation and honestly do not want to deal with it much at all. I genuinely do not care how much "performance" there is, as if it is speed critical at all. I want segments to be handled in the dumbest and most impossible to mess up manner possible.

```
VOID L_CreateSegment(
    SHORT   selector,
    LONG    base_addr,
    LONG    limit,
    BYTE    dpl,
    BYTE    type,
    LONG    granularity,
);
```

When I create the system descriptors and all the others, I will do it using this braindead method.

There are not that many:
- Null
- Kernel code
- Kernel data
- LDT descriptor
- TSS descriptor
- Real mode swap CS
- Real mode swap DS (same base and limit as CS)

L_CreateSegment cannot be the only one. For example, setting specific parts.

For each part, the name is self-explanitory:
- PBYTE L_SegmentAccessPtrGet(SHORT sel)
- PLONG L_SegmentExtAccessPtrGet(SHORT sel)
- LONG L_SegmentLimit(PLONG newlim)

Using these may need to be serialized depending on the situation. In the case of DPMI, they are allocated before use, so no worries.

# August 24

## Protected Mode Translation

First of all, translation for protected mode when calling real mode functions is a layer that exists on top of the PM hooking interface. The reflection handler will perform translations on the registers if supported before passing it down to real mode handling.

I should avoid having to copy buffers. One way is to use mapping instead, but there could be issues with that if interrupts are depended on.

The idea can be to have a 64K map region that must be acquired and released if and only if translation is being performed. This can allow for direct communication.

It may not be totally necessary because V86 hooks are already in protected mode.

### Mapping Instead of Copying

The only issue with this is that DOS normalizes all pointers passed to it. This means it cannot for example read file data into the HMA or something like that.

A solution is to use an old trick I found and allocate a region of memory at the very end of the conventional memory and use it as a transfer page buffer. This can use mapping instead of copying, which is way faster.

This is still quite complicated. If we do a hello world with a 32-bit address and want to avoid copying, this is not so easy. The alignment of the data will make it difficult to properly map the data. This could lead to bugs if the full 64K need to be used at an unaligned boundary, and the only way to avoid this is to actually allocate 128K, which is quite a hit for conventional memory.

This buffer is not intended to be shared by all programs. No, it will be local and I can establish a protocol for it working that way.

128K is a lot even for extended memory. Bad idea in general, honestly.

### Strategy

Each DOS API call must be translated if it uses memory operands.

Most use DS:DX for addressing. Since this is essentially the rule, the system should be structured around handling the exceptions to the rule.

I can keep a lookup table that is large enough to represent the whole API (0x6C is the highest). If the entry is zero, no translation is needed since the API call is register-based.

If the value is one, it can be handled using DS:(E)DX translation. The approach for that is a PM reflection handler copies the context, changes the mode to V86 and modifies the segment registers and EDX, and finally the request can be passed to the local real mode vectors, and if there is no hook, it can go to global real mode reflection or SV86. It is quite complex and not very fast, but would make OS/90 a very nice platform for DOS developers.

### ACTUAL STRATEGY (COPY TO JOURNAL)

No, it will NOT work quite like that.

I want the ability to implement entire API calls using the direct context of the protected mode call. For example, a call to INT 21H AH=9 can be totally handled in PM using the segments as expected, while real mode can use a different tactic.

To do this, PM reflection and RM reflection will NOT handle PM extensions. Instead, it will be an SV86 global hook. The real mode reflection handler will pass down a PM context and the SV86 handler must capture this.

Suppose INT 21H AH=9.

- INT is received by exception handler
- Looks for local PM handler
- Finds none, passing to real mode
- No local real mode handler
- SV86 captures and finds out this is a PM context.
- If within the jurisdiction of extensions, it will be translated.
- If NOT, the call is carried out noramlly by the SV86 handler assigned

INT 21H will have an SV86 handler that is global. INT 13H will have one too since the API does support extensions.

If protected mode drivers want to hook something like INT 13H, it can work just fine, but it requires acknowledgement of extensions. The SV86 handler could be passed a protected mode context, and the only way to know is by checking the VM bit. If it was protected mode, it has to handle it differently.

The default SV86 handler for the INTs will copy the context to another one, and will do so directly using the registers of the calling task.

This makes writing drivers that use SV86 hooks kind of annoying. INTxH will automatically set the VM bit, so it is not that bad. Register-based APIs do not have to worry about handling PM.

Suppose I create an unrealistic AH=9 hook:
```
V86_HOOK INT21H_AH_9(PSTDREGS r)
{
    if (!(r->EFLAGS & FLAGS_VM)) {
        // In protected mode
        char __far *msg = MK_FP32(r->ds, r->edx);
        int i;
        for (i = 0; msg[i] != '$'; i++)
            putchar(msg[i]);
    }
}
```

This is obviously not how it should be done but illustrates the idea. Nobody would ever call INT 21H AH=9 directly in normal DPMI, and if they did, it would not work properly. On OS/90, it would. The PM call in both instances has the entire contexted passed to a real mode reflection callee. In our case, an extra layer exists that makes it work when it normally would not.

Note that this code is completely impractical. It is meant to highlight that the protected mode context can be received and detected for extensions.

## GUI Paradigm

OS/90 does not need to rip off Windows because Windows already does what it does better than I can with the time I have. If I want the ultimate DOS multitasker, the GUI will have to be different.

Instead of windowing DOS programs, I can have one run like a background image. Why minimize it?

I can make it like Borland Sidekick but much more powerful. For example, I can have a caulculator open while editing code. Another idea is having a popup window with a list of active programs that can be minimized or moved around.

# August 26

## Note on GUI

Add a button to a window for going back to the main command line.

## ASSIGN and SUBST

NOTE: How do I do things like aliasing the drives? (ASSIGN and SUBST)

I cannot seem to find a system call that does this.


It seems like DOS uses a system-dependent drive parameter structure for this.
There is not INT 21h call.

Might as well leave it to DOS. It should be a global context.

Hooks on the FS API revolve around performing some of the FS-related accesses in protected mode.
The FS driver does not need to implement substitution. DOS will be left to handle locating
a file and properly opening it, or listing the right dir contents.

Okay, that does not really make sense, but if DOS does open a file (we sent the request to it after monitoring the input), it will resolve the real path using its internal structures. Then it will attempt to find it and ensure it exists before presumably preloading some information about it that is none of my concern. INT 13H handling can take care of that.

So basically, remapping drive can work and in fact, it can carry over after boot! There is no need to reimplement it either.

Protected mode will also keep track of the recently opened file.

## Disk Cache Ideas

Can I use some sort of "page table" concept to avoid using the ordered list/binary tree idea?

Also, can the disk driver be portable between OS/90 and also DOS?

## PSP and Local Operations

I can make INTxH calls from drivers a bit more streamlined by having some sort of initial VM. I think Win386 did something like that. Any requests will be safe as long as the VM is not in kernel mode.

> The correct way to wait for a task to exit kernel mode is to turn off preemption and yield in a loop after checking the status. Because the operation is not atomic, this must be done.

# August 27

## Calling Interface

I still have not technically decided the interface by which I will call the kernel API.

To get the best code density, something similar to VxD can be used, such as using a function call-style interface. This interface helps the code density of drivers. Using register conventions allows reused values to be moved around as well.

This makes me wonder if I should just write the whole OS in assembly, or at least a large part of it.

This already seems to be the case. Whatever scheduler code I have written is already ASM. The only exception is the zone allocation, bit arrays, and executable loading.

Doing those in assembly does not seem very rewarding. The zone allocator CAN be converted though, since it is a rather simple algorithm. Other features can be thunked for it to work.

The executable loader does not sound like much fun to write in assembly, but the process is quite straightforward.

Even if I increase the use of assembly, I will miss out on certain C features most of the time such as placing functions and data in separate segments for optimal space use. I am quite good at packing the data, so it may not be a problem.

## SFT and JFT

The program segment prefix has a JFT built into it with a total of 20 handles. It also has a segment and offset in the PSP so it can have a different location.

The first 5 entries are standard IO. These require special handling. Output of characters to the screen will be done using callbacks.

Input must be "served" to the program. The UI or whatever else is running sends input to it. This becomes complicated because the keyboard driver could choose to emulate the whole 8042 interface.

The actual console IO model is not fully established at the moment. I will eventually need a real keyboard driver.

Special handles are not global like the rest. Things like redirection will require hooking the interface. There is no other way.

# August 28

## Call Interface

I need to think long term. Making the kernel easy to develop is fine, but using standard C calling conventions is not exactly the best idea for the drivers.

The best code density is achieved by using register arguments. This allows for exchanging, saving to the stack, reusing values, etc. which is good for reducing code but is not really that great for performance. The disadvantage is that 32-bit arguments will require a full 32-bit write, which can be mitigated on i486 and older CPUs that do not mind a partial access.

The call interface can be separated from the implementation, which internally can use whatever conventions I need it to.

One way to do this is have some kind of enumeration or a set of enumeration with defines to set their starting points. The enums will define call numbers.

```
enum {
    M_PhysAlloc = MEM_MAJOR,
    ...
};

PZENT _M_PhysAlloc(LONG bytes);
```

The interface can use registers. A thunk then takes place. Each function takes a certain number of arguments, which can be copied

In assembly, it could be used like this:
```
OSCall<M_PhysAlloc, 5>
```

Or in C:
```
OSCall(M_PhysAlloc, 4);
```
The ABI can require a return value in EAX, so this can work just fine.

This is all based on the theory that the kernel will have to use all of the registers. That might work, but there are procedures that do not work like that, such as the memcpy routines.

I think they can be statically linked. If FAT uses 8K or 16K clusters, there is no meaningful loss to statically link it. Just copy the C file and the header to be done with it.

## System Entry

The idea is to make everything an exception. Every vector will be ring-0, which will cause a general protection fault.

#GP can be sent for a variety of reasons.

## Calling Idea

I can place the push instructions in the call target! Everything else can work as expected, though some adjustments may be necessary. Maybe ask ChatGPT for some ideas.

I need to push, EAX,EBX,ECX,EDX,ESI,EDI,EBP, or 7 registers. That can be done with 7 bytes.

I can also use the jump to procedure right before return optimization, which requires a jump instruction. The 32-bit jump uses the E9 code.

The format now is:
```
7 pushes
E9 xx xx xx xx
7 pops
ret
```

20 bytes total. There is no specific need to fit this in exactly one page, but I really should. This can fit 204 calls. Linux has somewhere around that many system calls. Should be good enough.

The ABI of the table entry

# August 29

# August 30

## Segment Alignment in Kernel

I may not really need this. The linker sorts out the alignments because every variable or function gets a separate segment.

Finding the size of the kernel can be done in various ways. Looking at the page tables works. The bootloader will have to zero things out or something, but it should work. I can also check if the mappings are sequential. When they stop, the end is reached.

I do have to ensure a page multiple size. THe file must be extended.

The only issue is alignemnt directives in the assembler code. I cannot depend on an align directive at all since the segments can be rearranged however the linker wants to.

The only way to do this is to have some kind of assembly version of TYPE.H.

## MASM Is Annoying

I cannot get segment alignment to work. It is the only way I can ensure alignment of a segment. NASM on the other hand, works perfectly.

I will need segments to align all things. This will make assembly code a bit annoying.

Macros will be needed to simulate the MASM functionality.

I will create a macro pack like this and preinclude it. All code will be converted to NASM.

```
LONG x 50
LONG y 30
LONG a 1,2,3
RES LONG 8

STR st "Hello"

KERNEL_CSEG
main:
    xor eax,eax
    ret

END_KERNEL_CSEG

```

## Win32 Console Apps?

HDPMI has a layer for Win32 applications, and it is build on DPMI woth extensions. I can port some of its source code to run Win32 console apps.

An example would be the Digital Mars C compiler toolchain, which is good for 16-bit DOS programs.

Open Watcom already runs under DOS.

I think HDPMI has a loader which automatically configures things

## More On Segments

I do not need a data segment because it adds a 4K alignment requirement. A possibility is to merge code and data.

This is not very realistic since there is a class-based filtering.

I will separate code and data then. Perhaps if I want to make the kernel code read only.

## GCC VS Watcom Again

I based my decision to prefer Watcom over the much more modern GCC on the generation of a variadic sum function that would be better inlined.

I need to test actual algorithms. The bit array allocator actually matters, for example.

### Bit Array Results

Its so over for Watcom. I rolled back some of my changes to the code and used the original, hoping that both compilers would be more familiar with the more typical C style.

It was not totally fair. The get bit fucntion used an inline bit operation while watcom did not, but it hardly matters. GCC was able to inline the code when using static declarations. It all fit in 66 lines of assembly output.

Open watcom lost so badly it's not even funny. It generated procedures even though they were static and did not bother inlining. Max optimizations were enabled BTW.

The version of GCC was 9.5, which is supported by DJGPP if I am not mistaken.

Conclusion: Watcom is a compiler I respect for its historic quality and use in some very good games in the 90's, but it really cannot keep up with GCC. I will leave Watcom for userspace programming since it is very easy for that.

GCC and Watcom are close when generating the more common code sequences, but GCC is insanely good at inlining. The inline assembler is also much more powerful with automatic operands and flag outputs.

The GNU linker is also not a total pain to work with and generates flat binaries with a single invocation.

WAIT A MOMENT. It looks like the code generated by GCC is a bit wrong. Wont change the results drastically, but still matters. I see a BT CL,CL sequence, which is totally wrong.

Now it is a lot more complicated. Still generating wrong code.

Anyway, GCC still does better.

# August 31

## More On GCC

- GCC has the __builtin_expect function for branch optimization.
- __builtin_assume_aligned

I am switching to GCC. This will require separated sections for code and data, but I think it is worth it.

## Bit Allocator

I fixed the bit test. I have to start the input operands with %1.

It does not seem to work properly. Maybe check the new version? GCC might be better with that.

I switched to the new version. It has not been changed in a long time and I tested it once. The generated code is nice but I could probably outdo it in assembly if I had the time.

## What Should I Do?

I am getting quite bored with the OS project.

# September 1

The transition to GCC is fully in affect.

The bit array allocator may or may not work. I do not remember clearly. It is not necessary right now. Given that everything that it does can be done using a faster method (e.g. linked lists). Practically anything is faster than that junk. The only thing bit arrays are truly good for is low-frequency types of allocations, maybe pools. In such a case, some sort of cache can be used to quickly provide them and allocations can be larger than requested.

Into the reserve.

It is getting really large now. I will delete E_BOOTIO.C because it is not updated and will probably not be needed in a while.

X_LDR.C is kind of useless because the code is a bit of a mess. The executable format isn't even specified. I will not even be using it. A call table will be used instead.

L_SEGMNT is not complete. I am not sure what I will do. I think those routines will be used to initialize because I honestly don't care anymore. IA32 could be rewritten in C, but the inline assembly will make it look bad like it did the last time.

I think IA32.ASM is a total mess. This happens every time I have to write assembly. The whole code always turns into garbage as soon as it breaks the 100 lines barrier. I am happy to make startup less than 0.001% slower if it means cleaner code. It is loading drivers that is slower because of disk IO. I am in no position to code golf when my OS basically exists only in part within the docs folder.

## IA32: Getting It Done Plan

The IDT will have no real vectors except IRQs and exceptions. Every INT causes a GP.

- IRQ base is at 0xA0
- First 32 are exception handlers but ring-0 so they cannot be called by userspace directly.
- Exceptions end up going to system entry and will store an exceptions index along with an error code on the stack for total reentrancy
- Other vectors are zeroed out and will always GP if called. Ring-0 should NEVER call anything with INT by the way.

A quite simple layout. The system entry routine will have some logic to route the handling, but will hand it over to DPMI/DOS if it has nothing to do with SV86 or the kernel causing an exception (which can happen and is not always fatal).

SV86 is defined by the SV86 global INT counter or something. Branchless code recommended.

Kernel mode causing an exception handles it essentially the same as userspace if it is a segmentation fault, page fault etc. The task is simply terminated, and so is the DOS instance containing it. If any special handling needs to exist for the kernel, that is fine. This means the kernel can recover if it accesses an invalid memory region.

Basically:
- Is this SV86? If so, handle that way.
- Otherwise, just send it to a high-level exception handler. They will be hookable.
- Thats it.

I should merge some components together so that the compiler can inline. The system entry can also be done in assembly because it is quite trivial.

The code file can be called `CENTCOM.ASM`. It will be central command.

### Okay But Actual IA32

IA32 does the following:
- Set segment registers
- Set TSS and LDT descriptors
- Move the IRQ base
- Fill IDT with entries
- Copy the RMCS region to the HMA

Some ASM with raw strings will be needed, but it can be done in C. The GDT should not be defined in L_SEGMNT.C because that module is a library only.

RMCS is a bit difficult. C does not really have a concept of INCBIN. GNU AS does though and it can be used.

The IRQ code can be rewritten. It works perfectly but should have IO delays and can easily be translated to C.

The IDT is simple enough. It will be in BSS and therefore automatically initialize to zero.

Still seems better to use ASM. But then again, I am very bad at organizing assembly. Better to write AsCembly than just ASM ATP. Some parts can be translated into it from what I already have.

# September 2

## Switching to GCC

GCC requires a UNIX-like environment. I can get a cross compiler running on windows directly, but it will take some effort.

I would normally prefer DJGPP, but I already know how much of a disaster that was.

But DJGPP would be better in a lot of ways, if only I could set it up correctly. Running it in DOSBox is really slow and Virtualbox is not better.

The correct solution is to use a GCC 9.3 (latest for DJGPP) and keep the door open for self-hosting. I still need a real GCC.

Or I can transfer my files back to WSL and work there. I already have a cross compiler there, although it is way too new.

## Ideas

- VGA arbitration per process
- Virtual KVM switch driver?

I like the idea of being able to run Win16, Breadbox Ensemble, and the OS/90 native interface (ATM/90) all at the same time. Because each of these require access to the display adapter, arbitration is needed.

When a DOS VM does any action that changes the video mode, a new arbitration context is created and it takes control of the display. Each VM has its own local video mode.

The video can be switched using a KVM (keyboard, video, mouse) switch driver.

This is not technically required, but Windows 95 certainly did it this way. In fact, it was very intuitive back then and a simple ALT-TAB could switch programs.

# September 3

## Toolchain

I will switch to WSL. Commit changes.

Okay, switched over. I need a cross compiler toolchain now.

I have a copy of UASM. Could put that to use. I should start by porting the string routines to it. They will be needed for startup.

# September 4

## Problem with chains?

The issue with chains is that an ID cannot be truly verified because it could be reused. This may be an issue if we want to implement virtual address spaces.

## Virtual Address Spaces

I need a sort of VirtualAlloc function. I will need to allocate from several pools and map multiple chains into the same region.

Finding out the chain that a page belongs to is not too simple because it could be in multiple pools, but the process is still similar.

Remember the rule: NO FEATURES. Make it really simple. I really like the multiple pools idea, but also I do not see the reason to have more than three. One for before the memory hole and one after. That should be good enough. Of course there is swap.

The granularity will create some problems too. If I need 4K of memory to map in a region, how am I supposed to allocate it?

Page granularity must either be 4K at all times (with transparent hugepages perhaps) or it must be global. If the system has 64M, allocating in chucks of 64K is viable because there are 1024 blocks to allocate.

It should be up to the user to decide the allocation granularity.

The granularity must be with respect to the ISA memory hole, which we will assume to always be there. The largest logical chuck of allocation is maybe 1M, or something 15 is divisible by, but there is practically no reason to do this since 1M is way too much to swap out at once, even for modern computers.

### Resolution

Allocation granularity options are:
- 4K
- 8K
- 16K
- 32K
- 64K

No need for other options. 64K is used by Windows NT to this day.

I will have to make a few changes to the zone allocator:
- Allocate in granular chunks, not bytes

Why not have a giant table for everything, including swap? That way, swap is orthogonal and is actually not even swap anymore.

The issue is that I cannot allocate swap directly. I cannot copy to it. Swap is no longer swap at all then. It becomes the 100,000x slower version of RAM.

Swapping is generally bad, but for some background tasks, it is okay to disk-back some pages. Ideally this should be done automtatically. One example would be the table used to store the PCI/PnP configurations or something. There is little reason to use it for it to be used except during the loading of drivers.

Making the address space orthogonal and use the same table is fine, and I can allocate specifically too by specifying the base entry of the allocation. If more memory is needed, it will go straight to demand paging right away.

# Septempber 5

## Swapping or Disk Mapping

I like the idea of disk mapping. I can map a total of 1 million items using the 20-bit page index. Disk mapping can be used to implement swap.

My intent with swap is that INT 13H is called directly. The disk driver will only do the job of completing the IO request that way. INT 21H will be called to flush buffers, and this call must be hooked to remove any caches.

Caching does not really get involved here.

I see little reason for disk cache to exist independently.

## Virtual Address Spaces

I can have a special page table entry that terminates an allocation. That way, a complex address range can be freed iteratively.

## The Plan

This whole project is in shambles. No idea what to do anymore.

I do now. Focus on documentation and journal entries. Write the specs or you are wasting time.

I need to flatten the docs. No folders whatsoever. Implementation details should not be in the manual.

# September 7

## SV86 Redesign

The current previous design I intended to use would cause SV86 to be a special context that is incapable for yielding or holding locks. This is a problem because INT could cause a nested call to a trapped function and that needs to run in a proper T1 or T2 context.

It would make allocating memory in an SV86 handler impossible. Just consider that! INT 13H could not possibly work.

The solution is to make SV86 program-local. The user context switches to V86 mode but with elevated privilege implied.

The SV86 flag is then made local to each task (not process). There is only a need for one, as well as an INT/IRET counter.

Disabling preemption for SV86 is a bad idea because it is not actually necessary and makes legacy code fit poorly in a 32-bit system. Some 16-bit real mode code is perfectly fine to preempt as long as it never gets reentered.

SV86 will hold a lock. Preemption-offing is a weak guarantee that cannot work.

The program will have its state changed. But how is that supposed to work? If the kernel mode thread needs to call SV86 and do something with it, how do we get back to kernel mode?

To make this work without making the kernel too complicated, SV86 needs to be preemptible in the sense that its context can be restored at a later point. That means a ring-3 context will be saved to the task registers.

This means the kernel will switch to virtual 8086 mode. Upon an IRQ#0, all the data gets dumped in the current state block.

But the problem is that the stack pointer is important in determining the current task. I am not really sure how well this would all work out for the system entry process.

It goes back to the weak guarantee of no preemption. I like yielding in locks, but it is actually a problem if preemption is made local.

On the other hand, a global preemption context makes SV86 totally safe and separate from scheduling.

A global context seems to have been what Win386 used. Linux does local, but Linux is not the same thing.

Interrupts should be the same. The whole context theory is FAR mroe complicated when the contexts are local. There is T0 acting like a TI if CLI/STI are used. Disabling preemption having almost no guarantee for most kernel functions. The list goes on, and there is hardly any benefit.

But how are we supposed to get a T2 preemptible context from SV86? The answer:
- If an INT is called, exit SV86 and let T2 call it manually. That way T2 can be entered or SV86 can be entered, all from T2, regardless of if handlers are set.

The system will be structured like this:
```
BYTE RUNxH(BYTE vector, PSTDREGS param);

VOID INTxH(BYTE vector, PSTDREGS param);
```

RUNxH returns the vector of the INT operation that must be done because another INT was found. It is implemented in assembly. RUNxH will simply run the vector.

INTxH is the abstract interface. It will handle the chaining. RUNxH will not require duplicating the context and operates with no penalty to stack use.

Other instructions beside the INT family are emulated inside of #GP.

## Take A Break?

Maybe I should try to make a boot sector game or something. Maybe some kind of space invaders or Brick Breaker and get back into the 512 bytes demoscene. I don't know. Not exactly the most chill experience to cram as much conent into 512 bytes.

I keep saying today is not the day but I cannot count how many days I have said that. What is happening to me? Why do I spend hours gaming every day but I cannot sit down and work on this project?

## TODO List

- Get a cross compiler
- Set it up
- Write a bash build script
- Install bochs for WSL (should have the debugger, right?)

The files will be transfered to the disk using mtools. Done with the current nonsense about mounting a local directory and all that. It could be useful later though.

Unfortunately mtools is extremely annoying to use. Like really.

Do I even have a boot disk? Yes, I have a VDI image which Bochs can accept. 500MB large and full of software to test.

mtools cannot operate on a VDI though, so I must convert it to an IMG.

Just did that. Now install mtools and try it out.

And it works! I need a configuration file for it, but other than that, no problems. mtools will be used from now on to perform.

## Compiler

GCC 9.3 is the highest that should be supported for features because I doubt there will be a newer DJGPP and I want the door to be open for that.

I will build the cross compiler from source. I will use a ramdisk to make the compilation faster too. Build target will be i686.

# September 8

## Compiler Build

Compiler is set up now and works.

## Build System

If we can even call it that. Just a build script that compiles all the object files. DJGPP is supposed to come with some old version of bash. I was not able to set that up, but I do need some standard way to compile things.

The process is essentially:
```
i686-elf-gcc $ARGS -c *.c
i686-elf-ld $LDARGS -o KERNEL.BIN
```

Inserting this into the file will be done using mtools, which is the proper tool to use.

Under DJGPP, it will be necessary to copy to a boot disk or something. I am not that worried about that though.

## Posix/UNIX Support?

I may decide at a later time to implement a Posix-compatible interface. For this to be possible, I have to ensure that certain things are supported:

- Memory mapped files

I think introducing a page hooking feature should solve that problem.

## Getting Bochs

I may need to build bochs. The Windows version has all the right features, but I cannot easily use it from WSL.

It may be a good idea to make a few changes to the bochs source code. The port E9 hack needs to direct the output to a proper log file. I do not want to have to set up a printer or COM port.

The logfile can be anything I want. Just shoehorn it in there. Clear it on startup or something.

## E9 Hack

I found the code that deals with E9. It is a simple putchar and the output buffer is flushed. Not sure if bochs uses stderr, but I could use that instead.

The syntax for filtering stderr in very nasty and I do not want to deal with that. Better to output to a file. Just a startup variable for cleaning the file and the rest is simple.

It's complaining about X libraries missing. I think I will just use BOCHS.EXE since it is already configured.

## Next Steps

- Write the build scripts in bash
- Write the main run script
- Basically that is it

I need to change the location of the kernel to 0xC0000000. I am abandoning the raw memory region idea and have no need for it. The bootloader will need some tweaks.

> Note, the bit array allocator may not work on 64-bit.

## Assembler Choice

I prefer NASM, although it does not have some of the nice macros that UASM or MASM have.

NASM is used by FreeDOS, so I should have no issues with that. Will port existing code.


# September 9

## Userspace File Caching?

File caching can be highly independent from the actual filesystem. There is no need to cache directory entries or the FAT. A portable and equally performing solution is to have a high-level API that does this.

I can implement the standard open/close/read/write/seek calls but using direct IO that UNIX provides or the FS functions of the FAT driver. The getdents call can also be done.

Userspace caching can be faster too since there is no need to switch to the kernel if the cache is hit.

Maybe I can find a benchmark and replace the file calls.

The userspace controlling performance-enhancing features sounds like a good idea. System processes should not be bloating the memory on a proper OS. Why not swap out a dormant process that is right on the taskbar?

Actually, this might be a bad idea.

## Plans

I am not far from being able to boot. I need my IO operations back to output in the console.

## OS Name

Seriously consider renaming it.

# Septempber 11

## SV86 Solution

Why not just schedule a reflected IRQ and call it with INTxH? It will be slow anyway, so why not?

## The Linker Script

Something is wrong. Every time I use something in the BSS section, it uses address zero.

Maybe the loader could be at fault?

# September 12

## SV86 Idea

No, we are not doing that. Very bad latency and no real reason for it either.

## Environment Block

DOS stores the environment block in this format:
```
VARNAME=Thevariable\0
```

It is essentially just an array of bytes. The names of veriables are capitalized.

I should provide a method of reading the boot environment block.

https://jeffpar.github.io/kbarchive/kb/078/Q78542/

## The Linker Script

There is no reason to separate the sections in the script. I had an older version that did not do this. Need to retieve that. I had it working at one point.

Unless something is wrong with the loader, and I doubt it (in which case I can fix by rolling back an older version), the linker script should be the sole cause of any issues right now.

## It Works

Don't know how, but it works. Now I need to try the BSS thing.

Aaaaaaaand it' not. Compiler is not to blame. Only the linker can be at fault.

It seems to work now that I adapted a linker script from OSDev.org. Turns out I need to specify its exact location.


# September 13

## Central Command

One file will handle task switching, system entry, interrupt handling, and the basic scheduler operations. It will be written in assembly.

Exceptions are the only method of system entry. The IDT will be NULL except for the exception handlers, which will differentiate and dispatch to a handler.

> Use separate tables for kernel and user?

Yes I will. Anything to reduce conditional branching will be faster. Kernel-mode exceptions require very different handling (it is usually a critical error).

For example, the kernel probably should not trust the memory addresses provided by the user and terminate a program because the kernel accessed an invalid location. It is the kernel that is at fault.

If the kernel detects anything that is problematic, it will need to get the requesting program to know this by reporting the exception through simulation. This cannot be done if the kernel decides to do it.

If it the end of the handler chain the exception is not caught:
- Kernel-initiated will cause critical system error
- User-initiated will terminate process.

The requesting program can still be terminated under the kernel though. It is obviously fatal for the program.

### The Layout

- Handlers for IRQs essentially the same as before
- Each exception will pop an error code to a memory location while interrupts are OFF
-

## NULL Pointers

NULL pointers not being instant errors is concerning. I believe the page fault should ensure that the entire IVT is protected as well.

This can be added later.

## Memory Address Translation

I will need services for getting linear addresses using segment registers and offsets. This will be validated too.

```
// Returns NULL if the segment is invalid, offset is out of bounds
// or the expected offset is.
//
// If expected is zero, the check is essentially skipped.
//
PVOID T_SegToLinear(SHORT seg, LONG off, LONG expect_max_off);

BOOL T_SafePush(PSTDREGS r, LONG count, ...);

```

## NULL Pointers Again

A page fault would be generated if there were an access to the BDA in the timer routine.

## Interrupt Service Routines

I will make interrupt service routines assembly-only unless thunks are used. They should NOT use stack conventions. Does it matter though?

Should I?

# September 14

## Exception Dispatching

Is there a more efficient method of dispatching exceptions?

I need the error code to be removed from the stack no matter what. I also need to differentiate between the exceptions. Very few options exist.

There are 32 exceptions possible. OS/90 implements only 20 (maybe support the rest?). I am thinking about a 512-byte table for dispatching, with each table entry being in 16-byte blocks.

I can afford to use 512 for this purpose, but it is a lot and not totally necessary.

Normally branch targets should be aligned at a high boundary so that the CPU fetches as much executable code as possible in the icache.

# September 15

## Next Steps

I need to enter T2 somehow and have scheduling working. This requires interrupts to be recieved.

For now, I can permit only IRQ#0. I do not need the other interrupts. Reflection to real mode will be very complicated and is not needed now.

# Septempber 16

## Getting to T2

The startup procedure involves the creation of a boot thread to prevent a special case in the scheduler.

To get to this point:
- IRQ#0 must be ready to be recieved
- I can really just ignore the rest
- The IMR decides which interrupts have a real mode handler

It is not that far away from now, although getting it to properly function will be a challenge.

For now, I will use fixed task blocks since the MM is not ready.

Getting into T2 continued:
- TSS.SS0:ESP0 is irrelevant right now because we are not using ring-3.
- Entering the initial task requires a special process

To get into the first task, the stack must be switched. I cannot just let IRQ#0 fire away.

As an alternative, I can also declare the initial task block in the BSS.

## Problem? BSS!

How does BSS actually work? The bootloader is contingent on the size of the executable.

Yes, it allocates all of the memory. But it does NOT map it in.

Strange. My executable seems to handle BSS just fine. It may be getting encoded into the executable.

Does not appear to be the case. This is actually bad. BSS by that logic should NOT be working at all.

Did I really not think of this? The only reason BSS is not totally failing is probably the page size round off. Otherwise, it will not work. Just a simple characteristic of flat binaries. BSS is 100% program-managed.

Maybe check the boot code.

It is a total mystery as to why this worked in the slightest.

According to the Bochs debugger, a range the size of 0x100000 or 1M in size is used. Could it be because I have an array that is 32768 ints long or 128K in size?

1M is ridiculous for the kernel, but it is what is happening. Now I will try without that.

Once again, same thing.

What is happening is that XMS is used to allocate all available memory. 1M is mapped based on the location of the kernel.

Since we know the physical location of the end of the kernel image including the BSS section, there is no problem? Yes. All is good.

## Scheduler

I need to enable interrupts. To do that, I need ISRs. There is some existing code for this.

# September 17

If I turn on IRQs, nothing bad will happen. I currently have an IRQ#0 handler.

## Potential Improvements

IRQ#0 can be handled directly. This is essentially a must have.

By doing so, I can have other means of context switching with less stack operations.

For example, I can limit the number of registers actually pushed by using less.

Also, I can use moves instead of pushes. The registers of the interrupted thread can be saved directly to the task block. The new context can be loaded by copying the registers and and running IRET.

The problem with this is V86. I would need to know if the segments should be pushed or simply copied.

For that reason, I will keep the design mostly the same, but IRQ#0 should be 100% branchless and should also not require calling anything. The IRQ dispatch logic is quite branchy and is only okay for low latency.

IRQ#0 handler:
- No branches at all (try it)
- Aligned by 64
- Should run in under 100 clocks on a i386.

I need to make the preemption counter check somehow branchless. I think I could change the IDT entry to skip over code, but that would make the preemption counter disable interrupts, which I would like to avoid.

Okay, just one branch. It will be expected that the counter is likely zero.

# September 18

## Scheduler Working?

I ran the kernel under QEMU and Bochs. Results seem good. No critical errors. The initial thread appears to preempt correctly.

The next steps:
- Get a working printf implementation (or make one)
- Clean up the code
- Make multiple threads run

# September 20

## Task Switching

Whatever I had previously was somehow rewriting the same exact context, hence why the other "thread" did not run.

I changed a few things and will try to test it.

# September 23

## The Reason Why Scheudler Is Not Working

The stacks are not actually switched. Ring-0 does not change stacks or even push the current SS:ESP.

What happens is that the stack is not changed upon returning. This means that the task is unable to switch properly. That is why A prints out and then it gets stuck on B.

The problem with this is that x86 is really not designed to handle this sort of thing. I cannot force it to have a IRET frame.

IRET is also the only way to properly change the stack, flags, and the rest at once.

It is impossible to use LSS because it would not be possible to return.

So unless I do some kind of really disgusting hack, it is quite difficult to actually change the kernel stack the way I want to.

## Kernel Stacks

I need to leave a 4-byte gap so that the calculation works correctly I think.

Maybe it is better to have variable-length stacks. The current plan does not really work with the idea of maximum reentrancy.

## New Approach

First of all, I will change the design to permit the use of variable length and variable location stacks.

This means a single variable will contain the address of the current task.

This somewhat simplifies things, but there is still the issue of how I switch stack.

It is possible. Under a multitasking real mode environment like desqview it can be done, so it should be basically the same here.

```
<Save current context>
<Switch to next task>
<Enter the context ....>
```

I cannot use the whole use move instructions optimization because I need a register to address the task block.

The sequence of events:
- Load context into all registers but EBX
- IRET to set flags
- Restore EBX
- Go to process, but we cannot!

If I chang the stack pointer, I cannot get EBX back.

Yes, but if I save EBX somewhere else, there is no problem.

Except there is because without EBX there is no way to enter the process. Maybe I can use a push/ret.

Yes, do that. Push/ret and use a memory location to store the previous value. This prevents having to do another 20 memory operations. 40 is better than 60. OFC the cache density is good, but why waste the clock cycles?

## TODO

- Delete everything that exists right now first.
- Do not reuse code. Do everything with a separate branch.

## Stacks Again

Linux uses PAGE_SIZE*2 to determine the size of a kernel stack. Linux is a much more complicated operating system, so it needs more stack space, especially on 64-bit.

Assuming the need is half for 32-bit, 4096 bytes is enough for most situations.

3K can be reserved for the stack.

# September 27

## SV86

I am getting bored of the scheduler. Closer to getting it working, but still bored, honestly.

I should try SV86. It seems more fun.

### Doing It

First, I need to get a monitor working.

I have no need for a hook or something like that. The monitor is really the only thing that needs to run when in V86. It handles #GP.

Because SV86 is performance-sensitive, it makes sense to change the interrupt descriptor entirely.

### Emulation of instructions

IO codes will not be emulated at all. IOPL=3 will be used and all INT calls will automatically trap because of ring-0 IDT entries.

The INT instruction is handled by saving a value and pushing values to the stack. The monitor exits so that the caller can decide to go in again.

IRET is a monitor exit as well. It will write a different value to a global variable indicating that the last SV86 entry terminated normally and is not requesting further emulation.

INTO should probably never be called.

### API

- LONG V86xH(BYTE, PSTDREGS)
- VOID INTxH(BYTE, PSTDREGS)

V86xH enters V86 using the supplied context. It proceeds to execute until it hits an INT. The monitor writes the vector to a variable and tat variable is returned by this function.

INTxH automatically re-invokes V86xH. Using the return address on the stack, the monitor handles the job of switching control flow in the context before it exits.

The algorithm is like this:
```
    Call V86xH
    If it encountered an INT
        Run it too
            Do this in a loop
```
This can be done recursively and that may not be the worst idea.

I should mention that INT will set up the stack btw. If the INT must be done it real mode, it will be.

# September 28

## SV86

I should write this in assembly.

## Doing It

# October 1

## The Plan

Go back to the scheduler. I need to follow the plan.

### Scheduler?

I technically do not need multithreading at all to work on other components. I will still have to do it anyway, so might as well finish up.

# October 9

So I wrote some SV86 code last night. Will test it later.

Anyway I would like to introduce a crazy idea.

Putting it in another document.

# October 11

## Changes to Strategy

The scheduler has shown to be extremely hard for some reason. I cannot seem to get it to work. Maybe SV86 is what I should focus on. Plenty of things do not need the scheduler.

## Project Status

For academic reasons, this project will be indefinetely suspended until further notice.

In the meantime, I will carefully evaluate what projects I want to have on my portfolio and decide what to work on next.

## All Projects

- I __NEED__ to contribute to an open source project. I am thinking of ELKS.
    - Try to patch the kernel and make it fully preemptible?
    - Add shared libraries and a new executable format
    - Port pthreads (do this first). Try to do it efficiently or something.
        Maybe hook IRQ#0.

- OS/90 of course (the magnum opus)
    - Make videos demostrating things that work
    - Post on YouTube, make it link-private

- ATM/90, the text mode UI library of OS/90
    - This is quite easy in general
    - Make videos for it
    - If you REALLY want to code, just do this
    - Code cleanup seriously needed

- The 80x50 text mode 3D renderer
    - This 100% needs video demonstrations

- Some kind of game that uses the renderer

- DSL
    - This is a VERY difficult project, perhaps harder than the OS actually, at least time-wise
    - Do not think of this much


# October 19

## Debugging Features

I should be able to output a boot long for the user. There should also be a kernel developer log level that can be a build option.

For now, I think I will be fine.

# October 22

## DSL

I do sort of wonder how necessary this is, or to what extent it needs to be developed.

Will I do a full Linux VM? It is a massive undertaking. It is actually a better idea to implement the Linux layer as part of OS/90, either in a driver or the kernel.

This would make most features way more functional. mmap can actually work efficiently. Multitasking does not need to interfere with the rest of the system.

That requires a working OS/90 system. That is far from happening right now, but I really need to start working harder on it.

## SV86

I need to set up the #GP handler.

## Buffer overflow Page

When I map memory regions, I will add a sentinel page that reports an error if accessed. It can just be a "null" mapping but I can use something else to indicate that it is not.

This can apply to userspace too and terminate a process when an error occurs.

# October 23

## Notes on Memory

- Uncommitted memory has no reason to be executable or read-only.
- Reserving a range has no real purpose because it will be filled with RAM anyway.

I will do away with reserved memory and have a simple alloc/free interface, where alloc can also force a location.

# October 24

## Userspace

I am using DJGPP to develop the UI toolkit.

This is not exactly a permanent arrangement for a number of reasons. First of all, DJGPP uses a segmented flat model which is not ABI compatible. I could try to extend its memory, which may involve a massive memory copy, but it is theoretically possble.

OS/90 userspace programs cannot run in a segmented model because DJGPP does not support it, but they cannot do certain incorrect things because there is segmentation to prevent that.

# October 26

## Documentation

I do not like markdown that much. I will keep it for this journal, but I will adapt the documentation to a new style that is easier to read.

```
================================================================================
    Major Heading
================================================================================

--------------------------------------------------------------------------------
    Minor Heading
--------------------------------------------------------------------------------
```

Function descriptions will follow this style.
```
    VOID ExampleFunction(VOID);
................................

   BRIEF
~~~~~~~~~~

   NOTES
~~~~~~~~~~
This function does nothing.

 WARNINGS
~~~~~~~~~~
This is not a real function

 SEE ALSO
~~~~~~~~~~

```

# October 29

## DSL/90 Ideas

A full Linux VM is a cool idea, but requires a lot of work. The main thing I struggle to understand is some of the rules regarding creating new processes and threads.

I think that UNIX operates under the principle of each thread is really just a process.

The context of a process is abstractly defined as containing:
- The register dump (ofc)
- Parent and child pointers
- mmap regions
- shared libraries in the address space
- special handles (stdin/out/err)
- argv and environment block, deep copied too
- signal handler list

DSL is simpler because shared libraries and mmap regions are 100% shared in the same address space.

mmap in particular cannot at all be local to the process. We will just keep a global list of mappings.

Shared libraries are interesting. Normally they are to run in totally separate address spaces. Some components like the .text section are sharable, but only considering some restrictions.

In ELF-i386, there is the GOT and PLT, which are needed for properly addressing sharable data that could change location but not contents. I am not sure how much I need to deal with that, but I expect that the ELF format will describe some of these details.

### The ELF Format

ELF is actually not the most complicated format ever. It has a lot of components, but it makes logical sense.

### Signal Handlers

Most signal handlers will terminate the program if a handler is not already set.

Some handlers are called until the program finally terminates itself. For example, catching segfault and not terminating will repeatedly cause it to be called again.

Segmentation faults can occur for a number of reasons. Accessing a buffer break allocated by the OS for example is a special error that only OS/90 can catch. SIGSEGV can also be caused by a null dereference which must always be an error, or accessing memory outside the Linux memory area.

### Terminal Handling

For ncurses programs to work and most other apps, it is necessary to emulate the terminals correctly.

This means supporting raw/cooked mode (may use IOCTL), termcap, ANSII codes, etc.

# October 30

## Components to Unit Test

- ATM/90
- FAT and IDE drivers
- What else?

### PCI and PnP

A DOS environment is far more preferable for things like this. The only issue with using DJGPP for it is the fact that API calls from the kernel do not exist and need to be handled manually.

API calls normally have to address the call table, but we will not do this and just write it in.

## General Structure of the PCI Bus Driver

The PCI bus uses an 8-bit bus selector, a 5-bit device selector, a 3-bit function selector, and finally an 8-bit offset that is always 4-byte aligned.

The point of the PCI driver is to enumerate all the devices, configure their IRQ lines, and leave the rest mostly alone.

The BIOS is required to perform the configuration process so we mostly just trust it, but if an IRQ is not already taken by real mode, it may be kept track of as reclaimable by the PCI driver.

After enumeration, all devices and their currently used IRQ lines are listed, and any lines that are still masked at this point (they are at startup) are also taken. This can be reconfigured to allow other buses to exist.

Under PCI, it is necessary to share interrupts because there can be many devices installed. This is reduced by using the line number field.

### Legacy Interrupt Vectors

Bus and device drivers have to account for real mode drivers potentially existing. Any attempt to handle an interrupt in protected mode requires a 32-bit driver of course.

The bus may never reconfigure an interrupt line until it checks if a real mode handler exists and loads a driver before enabling the device in any way.

Some legacy devices can be reconfigured, but this is not at all mandated as it is with PCI and is overall too convoluded.

The PnP BIOS is capable of detecting legacy devices and making it clear what resources are off-limits for devices. In the case of ISA PnP, I can NEVER trust its present configuration.

I think some kind of PnP root driver will be needed to actually enumerate resources. This may go to the kernel.

## STDREGS

Why even have all those fields? I seem to be getting warnings and errors from it.

Just use a macro.

```
regs.eax = R16(0xE, 'C');
```
This is actually more terse.

The only issue is with fetching values, so I guess not.

## PDCurses

The problem with curses is that I cannot really make it a global UI toolkit. It relies on some global information.

First of all, any compatibility with existing curses applications is out of the window. It would be nice but it is NOT feasible at all. There can only be one instance actually running and the color pairs information makes it essentially impossible to port existing programs.

They will have to run under a DOS box.

The advantage here is that
1: The ATM API can be build on top of an already existing interface
2: The exsiting PDCurses library can be linked straight into the program.

With -Os I get a total library size of 40700 bytes.

## Using Curses

Curses is not a pleasant API in general, but it is complete and working.

It is not really suitable for making complex GUIs, but a layer can be built on top of it to solve that problem.

It will be essentially the same as ATM is planned in some ways, but the drawing parts are mostly automated.

Something like this:
```
SCRLPLANE *scrlplane = CreatePlane(
    GetParentWindow(this_win),
    SP_BOTH,
    100, 100
);
HBOX * hbox = CreateHBox(plane);
// Width 0 means as long as needed
BUTTON *buttons[4] = {
    CreateButton(hbox, BT_STYLE_0, 1,0, "Test", NULL),
    CreateButton(hbox, BT_STYLE_1, 1,0, "Test", NULL),
    CreateButton(hbox, BT_STYLE_2, 1,0, "Test", NULL),
    CreateButton(hbox, BT_STYLE_3, 1,0, "Test", NULL)
};
```


# October 31

## Why Curses Actually?

Making my own API sounds cooler actually.

Currently, I am trying to get the mouse/keyboard driver to work properly. I am not that far from that.

## No Curses

It is better to write a separate API.

## ATM Note, Use Getchar?

I am not sure what kind of black magic is done underneath to make getchar non-blocking, but I do think that DJGPP supports that.

# November 1

## Problem With Getchar

This does not allow all the F-keys to be used and some other ones are not supported either.

I have to make a KB driver. No idea why it stopped working.

## For the OS

I am trying to get SV86 to work. The register dump does not appear to be loaded correctly.

Why does my code just magically rot and stop working?

I can confidently say that stdregs is NOT correct.

The handling of high and low registers is wrong. If I use EAX, it loads properly.

Fixed it. I just put high and low in a packed structure and made a union with the 16-bit register.

Looks like I got the 'A' character to print. That was the first thing I did when I started OSDev'ing.

# November 2

## SV86

> The plan went to the other doc, will copy

EnterSV86 will unconditionally enter SV86. The return value is a pointer to the opcode that caused a fault.

### Potential Issues

Not being to simply return is actually slower. I have to call EnterSV86 every time an instruction occurs that needs emulation.

The thing is that going back from the #GP handler is more efficient that returning to the caller (not that slow however) and then performing another expensive ring-3 switch.

Things like pushf or popf are faster if I do not return to the caller and far less overhead is incurred.

I think I should keep the existing design.

Wait really? The exception handler IS switching rings.

It can be:
```
Enter -> #GP trap -> Ring 3
```

Or
```
Enter -> #GP trap and return Enter -> Enter ring3
```

The total number of ring switches is essentially the same. The advantage of the first approach is not having to finish the whole function to emulate a single opcode or call procedures to do it.

So we are keeping it all the same.

## SV86 Hook Chains

There are two ways:
- Linked list

VOID Handler(PSTDREGS r)
{
    ;
}

- Return-linked call-down

```
PVOID Handler(PSDREGS r)
{
    // ...
    return (PVOID)prev_handler;
}
```
In effect, the kernel runs the chain sequentially and no excessive stack space is used.

- Compulsory call-down

```
LONG Handler(PSTDREGS r)
{
    return PrevHandler(r);
}
```

This has no additional slowdown because the argument will need to be pushed anyway.

As usual, the most recent hook is the one that runs.

Calling the previous handler requires a jump instruction with optimized C because the arguments are the same. This is 6 bytes for the memory reference one.

Basically, the argument is still on the stack. As long as everything else is cleaned, a jump is fine. This may reqire an epilogue or a three-code sequence to clean the stack arbitrarily.

### Decision

Number 3 does not incurr any exessive stack use and is simpler.

There is no ATE return value. Calling the next one is a calldown operation.

# November 3

Actually, not having a return value may be incorrect. After running the hook chain, we must determine if a reflection is needed.

They will return chain then.

## Some Notes about V86

Userspace programs can contact SV86 and hooks if they do not set their own handler. Some APIs can be implemented in protected mode as extensions however and may need either an INIT hook to change vectors or a separate chain list.



# November 4

## Idea for Task Switching

I can eliminate the need for branching in the task switch subprogram by using a pointer to a jump label in the task block.

# Novemeber 6

## The task switching idea

Its great actually. I need to set the procedures on events like system entries and exits however. This is for the exception handlers of course.

This would not be a major problem for SV86.

There are a few different types of switches:
- R0 -> R0
- R3 -> R3
- R3 -> R0
- R0 -> R3

The regster transfers required for each vary.

## Next Things To Do

Get Interrupts to Work

## Interrupts

I do not need to read the ISR all the time. It is a total waste of clock cycles and is done inefficiently right now anyway.

The only time it matters is if the ISR is zero or not, indicating a spurious IRQ. Otherwise, we dont need to care.

### At all?

Having an ISR for each IRQ is not a good idea. I need to branch them out somehow.

Here is a layout:
```
[IRQ#xx]


```

# November 7

## IRQ Solution

If I want to fully eliminate the IRQ problem with minimal branches and avoid accessing the PIC so I cannot make any mistakes.

One solution is to create an ISR for each. Since STDREGS does not follow the pusha structure and I don't want to use that to begin with (its kind of slow) I have to place a number of instructions for pushing and popping.

Because the values of GS and FS are undefined in ring-0, they do not need to be saved by any ISR.

The push/pop set is:
```
        push    DS
        push    ES

        push    EBP
        push    EDI
        push    ESI

        push    EDX
        push    ECX
        push    EBX
        push    EAX

        ; {...}

        pop     EAX
        pop     EBX
        pop     ECX
        pop     EDX

        pop     ESI
        pop     EDI
        pop     EBP

        pop     ES
        pop     DS
```
This is 18 bytes long.

A solution could be to create separate IDT entries for each IRQ. Since there is no temporal locality, cache is not harmed.

The only problem is wasted bytes for no particular reason.

A better idea would be to do the same thing as exceptions.

## Faster Scheduler

Pushing to the stack is a waste. It is theoretically better to copy the registers directly and temporarily use one for addressing.

> Enhance the trackpoint

# November 8 (LATER)

I am trying to get interrupts to work.

## Memory API

Why not have isolated address spaces?

Immediately, I run into some challenges:
- Accessing the independent virtual regions
- Drivers have to be mapped to a special region of the VAS
- Allocatng VAS may need to be done semi-properly
- OS will be slightly slower
- Less bug-prone.

A lot of static limits could be a viable solution for reducing some of the complexity.

I also wont literally allocate a full page directory for each process even if some pages alias. I would prefer to copy a full page directory entry for 4M or mapping per copy, which is very large and can be done on ring-3 task switches.

Memory management structures will be static and unchanging. The RMR is not really needed. I will have to implement some level of mapping ranges allocation.

Actually, page tables can be dynamically allocated now.

### Decision

The PCI bus starts its memory mapped region at 0xC000_0000 which leaves about 3 GB of memory. The problem is this is not even totally predictable and outside the capability of anything other than E820 to detect.

I do not need to worry because XMS already allocates the largest possible block of memory. Any XMS manager that tries to use PAE will certainly not work.

PAE will not make sense for OS/90. Memory exceeding the amount of addressing space is illogical unless a HIMEM-style system is used. I know linux has something similar to this but with isolated address spaces it can map some things to high memory, it just struggles to access that data across threads so it has a special API for that.

I can have an API that enabled it to work with some sort of bank switching or lock/unlock.


# Novemeber 9, 10

## Memory

No changes.

## Sitrep on interrupts

Currently not working, but the switch to real mode works and the interrupt is correctly serviced.

The only problem relates to the access of memory in protected mode.

## Problems With Reflection

For some reason I am getting a floating point error message that never ends in Bochs and qemu reports a divide by zero.

Exceptions should never occur in BIOS code or interrupt handlers. I have a feeling I am running bogus code or something.

Actually the divide by zero message was because I actually called divide by zero. I though zeroing out the registers would aleviate any potential addressing problem.

It appears that qemu only works because the interrupts are not actually being recieved at all. The counter in the debug screen does not change at all.

So what is wrong? It appears to be partially running total nonsense code. The keyboard handler is at 0070:0016. Does this even actually run?

Floating point should never be used by the BIOS. This may be a massive assumption, but it really should not occur, at least not without manually saving the FPU registers as no interrupt handler really should be using the FPU anyway and a BIOS API will never change them.

So I am confident that nonsense code is being executed. I do not think the OS is capable of exiting real mode either because of the loop.

Actually, it seems qemu does get the interrupts but it stops after the first few or so.

I may want the interrupt mask register to reflect what already was masked when the OS started. Although none of the other interrupts should be occuring at all.

## Still Not Working

An invalid opcode message is printed by the BIOS and bochs complains about some kind of bound check, which I managed to capture and debug. It is caused by a jump operation!

There is no segment bound problem either, and there never really should be. Addresses are supposed to wrap around in real mode. The jump is also relative yet somehow causes an error.

## IRQ Mask

If an IRQ has a handler assigned to it, what happens when another is reflected to real mode?

In general, interrupts should never be enabled by an ISR, but they could be. Perhaps only the current interrupt should be serviced at a time with the rest disabled.

Also, the mask could be destroyed too.


## Yes That Is the Answer

# November 11

It looks like masking the timer interrupt caused everything to work. Perhaps the keyboard driver tries to implement some king of timeout or something else related to time. Whatever it does obviously cannot work because the timer frequency is way too high or something like that.

Results are better with SeaBIOS and no major error messages occur. The only problem is the display not initializing.

## Got It Working?

I added the interrupt mask feature and the keyboard IO works fine without errors on the default bochs BIOS.

Interrupts also remain enabled and operational in the main loop of the kernel. No unusual behavior so far. Reflection seems to work perfectly.

## Regression or Bootloader Problem?

I get an error when trying to reference the IDT.

This did not happen before.

I think this has more to do with my attempt at calling INTxH. Perhaps the page tables got overwritten.

Yes I am quite confident that page tables were overwritten in the HMA. There are only two mapping ranges when it runs properly, one for real mode and one for the kernel.

Actually I called V86xH to print a string.

Yes it seems like a regression now. I had it working once. V86xH is not working.

I should change the permissions of the page to ring-0 to catch the error.

Some details: the segment appears to be wrong. It just calls C000:0000.

# November 12

Hello world is now working. The register structure is fixed. Interrupt do not work with the printf though.

All my page tables get totally trashed for some reason and I execute at invalid addresses.

I will see if interrupts do this.

- Tables are good after interrupts only
- Interrupts on after printf is good

Not getting any errors now. Strange. No idea if interrupts will work while in v86. Can only hope.

Until then, I need to work on finalizing the INTxH interface until it can call INT 21H.


# November 13

I enabled ring 3 write protection and it is clear that a stack over flow is the source of my problems.

This has nothing to do with the RMCS, but it is overwritten at some point which causes interrupts to fail too.

The V86 interface is somehow broken and is causing this overflow.

# November 14

## Potential Problems

Each execution of V86xH will clean up a certain amount of stack space.

The TSS has ESP set to the current value. Interrupts that occur while inside the V86 context will use this value to set ESP.

If I use this to clean the stack in any way, problems can occur.

# November 15

## The Problem

After the 1st call to V86xH, the system fails.

Results look different let me rerun.

A page fault happens when the system reaches a protected address and the handler fails because the address is wrong.

This may have something to do with my return handler.

I will make the default handlers FFFFFFFF so that I can catch the instruction that x3faulted.

That does not really help.

Anyway, I think that the return mechanism is faulty.

It looks like an IO instruction may have had something to do with it. Now that I fixed the problem with the TSS IOPB, things should be better now.

Although this is a full on crash and not a monitor hang. Or it was interpreted as another fault somehow.

Still not working, but I corrected something at least.

## Got Done Today

- Fixed TSS problem
- Human readable integer printf capability

Thats enough now. Time for physics homework.

## Debugging Functionality

```
dlog() - Driver debug log

assert_equal(msg)

assert_nnull(msg)

assert_null(msg)

```

Examples:

```
LONG Add(PLONG i, PLONG j)
{
    assert_nnull(i);
    assert_nnull(j);

    return *i + *j;
}

```

The debugging library will be designed to not actually modify the original code significantly. It does not insert inline conditionals and instead uses function calls. Each test function is wrapped in a macro with memory fences to ensure correct ordering.

assert_nnull()
```
push dword [esp+8]
call _assert_nnull
```

The calling conventions are cdecl and the attribute no_caller_saved_registers so that nothing has to be pushed and all arguments are cleaned automatically.

# November 16

## STDCALL

I will use STDCALL for all API functions. It is way more code dense. Existing code does not need to change much since most API calls are written in C.

# November 17

STDCALL is objectively better. It works for variadics by fallback to caller cleanup.

For the kernel, it matters a lot less.

# November 18

## Success Wth V86 and Interrupt Reflection

The next thing to work on is INTxH.

It will need the ability to automatically grant a stack if one is not already provided.

The only issue is that Another instance of INTxH may try to use the SV86 stack.

If we are actually entering SV86, preemtption needs to be off already.

> Consider the reentrancy here. Also the stack only needs to be explicitly set when entering SV86 and not capturing.

There is no real reason to have a reserved stack for each process for when servicing real mode requests. At the same time, I also do not want to take away the freedom.

Having a stack in the HMA works.

Don't think there is a problem actually.

## Is the recursion correct?

After I run a certain INT, I need to continue executing.

V86xH will break out once it encounters and INT, but it needs to continue running until IRET is encountered.

So the current recursive model is wrong.

The correct way to do that is:
V86xH should not actually emulate an INT instruction at all. That should be done by the high-level monitor.

V86xH should be renamed to EnterV86.

It will only enter V86. No stack emulation whatsoever.

The monitor will remain mostly the same however.

## EnterV86

An INT will be emulated at the stack level and so will an IRET, but a monitor exit is also executed.


# November 20

## ALERT: STDREGS IS BUSTED!

It is not loading correectly. Seems slanted by one dword.

Yup still not working. It should be. Most likely a problem with the ASM code if anything.


# November 21

## No

It works now for some reason. I added 4 to the base address and it worked. No idea why this worked. Need to keep that on sight in the future.

It looks like INT/IRET emulation is no longer being done using V86 assembly code. I will adjust for that. IRET should simply exit without emulating anything.

## IOPL

Setting IOPL to 3 makes most BIOS code significantly faster since CLI/STI are very common. It also allows IRET to run in actual real mode too.

If no handler exists for the INT requested, I could just change the IOPL.

There are some difficulties with this though. I could use task gates and a separate IDT for SV86.

How would that work? If IRET is executed directly I would have to find another way to handle exiting.

Also, This would destroy the registers most likely.

Also using a TSS makes no sense because it takes 104 bytes.

## Dumb Ideas

Not doing that.

Also the handlers should be checked for each subsequent INT.

## Userspace

I will not use DJGPP or anything like that for the userspace. In fact, I plan to write a real userspace API. I will probably port newlib and use that as the standard library.

DJGPP is far too bloated and cannot be integrated well with the rest of the system. It does have dynamic libraries.

# November 22

## V86 Algorithm

# November 25

V86 does not actually fail right now.

I will rename STDREGS to REGS.

# November 26

## Joined the Mac Cult

I need a cross compiler now.

So it seems that GCC requires something called libgcc to operate correctly. This is OS-independent and generates calls to code that would be inefficient to repeatedly duplicate inline.

For example, 64-bit multiplication and division on 32-bit systems.

I can compile a kernel without any of its features, and this has worked so far, but this should be done for long-term reliability.

To do it I just need to link the kernel using gcc instead of ld directly and pass -lgcc to get the library.

I am going to use a proper compiler, not the built in gcc which is actually just a symlink to clang.

I will just download it.

There was a brew package for it, so that is good. GCC 14 btw.

Now I need bochs for apple silicon.

Looks like there is bochs for Mac on brew too. Newest version too and works on Apple Silicon. That is convenient.

I also need mtools. There is also a configuration file on my other PC.

Got that one too. Just need to copy the configuration file also.

# November 28

/*
	XMS is supposed to allocate all available memory for the kernel and
	everything else. This linear block is safe to use by OS/90.
	XMS is never supposed to allocate over the ISA memory hole because then
	we lose a bunch of memory in between the HMA and the hole due to
	allocating a massive chunk that cannot fit in there.

	As a result, memory must be "correctly" detected using the BIOS
	interface.

	The following interfaces exist:
	- INT 15h,AH=88
		This returns the number of 1K blocks after the real mode memory.
		The only problem with this is that the size reported may not
		actually reflect the true amount. Some BIOSes do not correctly
		report the actual size and are capped to 15M.

		Also recieves data from the CMOS ram, which means it can all be
		done without calling anything.
	- INT 15h,AH=0C7h
		This returns a structure which describes a bunch of memory
		information.
		Windows 95 does not support this function even if available.
	- INT 15h,AX=0E801h
	https://fd.lod.bz/rbil/interrup/bios_vendor/15e801.html#sect-1776
		- Tells the amount of 64K blocks above the hole
		- Used by most OSes if E820 is not supported.
	- INT 15h,AX=0E820h
		- Every ACPI BIOS has this. Phoenix BIOS v4 did it first and is
		used on some 486 boards.
		- One way to do this is to pick out the top-2 largest memory
		regions and go with those: the pre-memory hold region and the
		above-hole region.
*/

E801 is able to get everything that I need and can handle any amount of memory. I see no real reason to use E820 except for "doing it the right way" that OSDev insists on so much.

There is the PCI memory hole. It is normally located at 0xC0000000 but there is no specific requirement. Only the BIOS knows for sure. This leaves 3GB of addressing space on average.

I will just trust the amount of memory reported by the BIOS that is linear and simple enough to keep track of.

I am not concerned about losing any memory from this since OS/90 is exclusively 32-bit and really does not need that much memory anyway.

I need to continue getting the OS to emulate on the Mac now.

## It Does Not Work

Bochs does not seem to compile properly. I don't think the MacOS UI code has been ported to Cocoa and that may be the cause of the problem.

I may need to switch to qemu, which is a better emulator in general.

qemu can be built from source to support the GDB stub. This will have me way behind schedule but should be worth it. qemu also has the port E9 hack. Most OSDevs use qemu because it is mostly better.

Other options exist, such as DOSBox-x. It has accurate hardware emulation and a built-in debugger, although not a very good one.

## I Still Want Bochs

I got it to work now. This is good news. All I had to do was select a display library and SDL works.

## CHS Problems

There is a discrepancy between:
- MBR CHS information
- BIOS INT 13H CHS translation
- CHS properties reported by bximage

One solution could be to just use SeaBIOS since that works.

# November 29

I am not going through this CHS nonsense again. I will go back to the FreeDOS image.

# December 1

## Problems With V86

The monitor has the following issues:
- IRET discards the results of execution and just exits out, which is wrong behavior.
- INT/IRET emulation code still exisits in the monitor.

# December 2

## V86 Report

I made a number of changes and also removed the old stuff not needed anymore. I also rewrote the algorithm with gotos.

- There is no infinite loop.
- The code runs a segment override prefix for SS and faults on it. The instruction is movaps or something like that. It makes no sense at all and the code that ran was inside the BIOS rom.

This means that somehow the code that is being executed is wrong.

> Something to do with copying the iret frame information back?

# December 3

## It Works!

Fully working it seems. I was able to print a hello world string. Will save at this point to github.

## Where to go from here

The INT interface is low-level and does not handle the stack and interrupt flag automatically, but it does work.

Another detail is the program segment prefix, which has to be set correctly when calling to DOS. We will hook the whole interface.

Is there an invar for this? I cannot seem to find a current PSP variable.

It is not that important actually. The SFT is global on OS/90. Nothing should be called from the kernel that is specific to a process, but in that case I need a special API for that.

## Things to Write

I need an MZ executable loader. It will also initialize a context.

I can actually use the real interface to load programs. It looks unintuitive though and I would prefer to avoid that.

# December 4

## Stacks for V86

There will be a uniform call mechanism to access V86 services.

First off, I want the stack to be automatically selected for the most part.

But how?

The V86 call mechanism seems to access the stack automatically and does not account for hooks just yet.

I will figure things out. I should not use a stack unless SV86 entry is needed.

But if I am within an SV86 capture, can I call SV86 again? It is supposed to be reentrant and I do not want to impose an unnecessary restriction.

If only one stack is used, it would not be reentrant because it is dependent on a global resource in a context that cannot be restarted to continue. This is fine if I do not want a V86 handler to be able to call itself, but this definitively makes V86 calls 100% non-reentrant. I cannot call from different threads unless I use a lock.

A lock is not so bad. While the stack is in use nothing else will actually run because preemption will be off, although I wonder if I need a lock at all at that point.

EnterV86 already disables preemption. I can do it before entering it.

I can give each process a separate stack for this purpose, but it does seem wasteful.

I am not sure how often I would want to run INTxH in a V86 capture handler.

Right now, the INTxH obligatorily accesses the stack. It is possible that some INT calls also require a stack to be used. EnterV86 could be called at any time so we really do need a stack.

I can have a limited number of stacks subdivided from the total size in the HMA, or I can allocate more (boot option?).

Also the stacks do not need to be big because DOS switches stacks anyway and also because captured INTs don't even need much at all.

## Reentrancy Is Needed Actually

A filesystem-related INT 21H handler needs to be able to call INT 13H whether it is captured or not.

If I allocate the stacks automatically, it can be reentrant as long as the allocation part is thread safe.

I can use a bit scan instruction to allocate them by having a limited number of stacks which possibly makes it atomic.

Also, for proper system throughput, I will need a much higher number of stacks unless I try to go for lazy stack allocation and only get one for EnterV86 calls, but I'm not sure.

## Reentrancy of INTxH

INTxH is actually the implementation of RUNxH or something like that, which does not worry about hooks at all and just runs using the IVT.

Adapting it is a challenge.

- Almost all APIs that use INT use registers for parameters and not the stack, meaning that a stack only needs to be there as a necessity of entry/exit for EnterV86.

- INTxH needs to be callable from a hook handler. There is no logical alternative.

- INTxH therefore needs to be reentrant, that is, it should never need to wait for a resource to be free, hold any locks, or things like that.

- We need to preallocate all stacks because INTxH may never fail under any circumstance.

I can have each thread recieve a real mode stack. It does not need to be big because of reasons I already know.

The stack can be thread local or DOS process local.

Process-local makes a bit more sense. I can force a DPMI program to allocate memory of a particular size to handle all these calls.

The size of the stack is debatable because stack size varies. An AHCI/SATA INT 13h would probably use a lot of stack space. I think 4K is a good amount, but giving it to every thread is a bad idea.

Only userspace threads should get real mode stacks except when a kernel thread also uses one.

I can also have some method of deallocating them.

## Bored?

I can do a few other things.

# December 5

## SV86 Stacks

Stacks can be of adjustible sizes. They all have to be allocated using the DOS API. They should be allocated individually.

DOS will be configured to use a best fit algorithm for all allocations.

By default stacks should be about 512 bytes but it should be adjustible. They should only be allocated if needed.

I really want to use the bit scan to allocate though.

## Hijack the DOS Allocator?

The DOS memory allocator can be enhanced.

## Other Projects

ATM/90 is still on my other machine and not synced yet. I will have to do that later.

## Bootloader Rewrite?

I objectively do not need to rewrite the bootloader since much of that stuff already is good enough the way it is and a lot is assembly-based anyway so it would just add a lot of extra stuff to it.

However, C code would be more readable and easier to modify.

Hmmm. I consider my current bootloader to a masterpiece. It literally just works.

The only reason to redo everything is if I include the bootloader and kernel image into one single executable.

This is a good idea and also a fun thing to occupy myself with I guess.

The loader/kernel program can then be compressed with lzexe or something else with a good compression ratio.

All I have to do is copy the kernel image into the file. Unless the kernel is less than 64K (it probably will not remain that way) I will have to use a memory model with large data segment support or use a special override for the image and have the rest be tiny/small. The linker may complain if the executable is too large.

This is easier to do in C and it may be worth it.

## Can Get It To Work

ia16-gcc and ia16-binutils dont seem to compile at all. There are no MacOS packages.

Might as well use Turbo C under DOS. Digital mars also exists, though it may not run well or at all under mac.

DMC can run using HDMI. Turbo C++ is probably a better option.

I will use Turbo C++ then.

I can include a binary file using an assembler statement. I can also put it in the executable as an extension.

# December 6

## Loader Issues

XMS is supposed to be limited to before the ISA hole so that the kernel does not ignore all the memory before it.

OS/90 will do advanced detection to find how much there actually is.

I can only use memory that is within the allocated region. This requires knowing which XMS handle was used.

## Bad Idea?

It violates cross-module isolation which I should have and also does not have any major advantages either.

I really just want to code something at this point and not the kernel.

## Yes, Bad Idea

I can adapt the existing loader and use a simple incbin. The part that does the copying can be easily modified.

The only issue is that a COM file is used which may not load if it is too large. This means I have to somehow convert to MZ, which I do not know if it can be done.

A com executable could technically be larger, but I do not know if DOS will react normally to that.

## 100% Bad Idea

WIN.COM loaded the VMM. While compression was used for it, I will surely not need that and there will be two files anyway.

## Next Steps

I will get the scheduler to work. There is no need for 32-bit software support. V86 can be the primary userspace supported by the scheduler, as long as the system is extensible.

Because switching modes requires calls to DPMI, I can keep track of the mode of each thread and dispatch to a proper switch routine.

I will need INTxH to work for things to be done with this.

The DOS emulation layer will be barebones and will lack the DPMI functionality for now.

## Scheduler and Testing

I am able to allocate memory and using that I can try to load programs. I may keep a table of task blocks.

A jump location will be specified in the TCB. It will handle the full switch given the scenario with minimal memory accesses.

For example, to switch from r0 to v86, we have to save only the important segment registers load all the registers and also contruct an IRET frame with the new segments.

These are the scenarios:
r0 to r0
r0 to r3
r0 to vm

r3 to r0
r3 to vm
r3 to r3

vm to r0
vm to r3
vm to vm

This is 9 scenarios total, which is a heavy amount of boilerplate but should theoretically be faster.

r0 to r3 could be merged together and use the same procedure to switch between them, though pointless loads and stores of the FS/GS registers would occur, which arre underfined under the kernel.


# December 11

## Notes On ia16-gcc

I do not need it to run natively. The whole toolchain can run under DJGPP using a DOS emulator like DOSBox or a qemu OS emulator.

# December 12

I don't need to do anything with the bootloader.

# December 13

The OS has obviously stangnated because of stuff I have to do for college. I am not sure where I will take it from here.

With V86 being partially complete and working, I think I can continue with the scheduler.

Userspace cannot work without the V86 interface fully operational.

## Link-time optimization?

LTO can reduce the size of the binary. It is used by SeaBIOS for that purpose. Also it can make the code faster.

No.

## Testing Scheduler

I do not need extended memory allocation for the process control blocks. I can just statically allocate them in the kernel for the time being.

Without a full V86 interface I am unable to run ring-3 V86 programs since I do not have the infrastructure for that yet and there is no V86 call with hooks at the moment.

I will test ring-0 threads first.

```
VOID CreateTestThread(PTASK t, VOID (*proc)(PVOID));
```

The threads are not supposed to return because they have nothing to return to.

I will run procedures that are complex enough to check if the switches are being done properly.

As for DOS stuff, I cannot multitask more than one DOS program and it would be quite difficult too unless I use a .COM file and load that. Directly calling V86 may work, but as stated, I cannot run more than one without serious problems.

Also, a callback can be used for the MZ loader or even a generic COM loader so that a filesystem driver can hook it and optimize program execution separately.

For now, only ring 0 threads need to be tested. Also, there will be an initialization thread.

The test will be to print out characters and eventually strings to indicate which is running.

# December 15

## Task Switch

The current approach has lower cache locality annd requires additional operations to get the correct handler.

The advantage is that I do not have to first push the registers, then copy them, and then pop another set.

That is two read/write sets versus three. Also, V86b requires the use of a branch, which is 100% unpredictable and will be very slow on Pentium processors.

I will have to be able to set the TSS correctly though. I can adapt it to the current design.

## More on That

Is this really worth it?

At this point I could just use hardware multitasking as it does everything automatically. The only problem is not being able to use REGS anymore, which is a problem although a rather minor one.

I do not know if hardware task switching is known to be very fast.

The only potential problem is the need for a task gate or I have to put the gates in the GDT.

The full switch is somewhere between 210-301 clocks. There is support for task linking althout it is obviously garbage.

Although a mostly atomic switch operation to change tasks seems like something useful.

Task gates are slower that jumping to a TSS descriptor.

# December 16

## Task Switching

An interrupt will

# December 18

## C Library

I will eventually write a fully featured C library. It will be called jlibcq.

## Naming Conventions Changes

I no longer like the Quake convention. Can I do this instead?

```
mXAlloc
mXFree
sYield
```

## Current State

The project is in shambles. I sense myself losing interest in the OS.

## Bootloader

I REALLY should not even try anymore to change the bootloader. But changes do have to take place.

## Task Switching

There are many problems with hardware task switching. Interrupts will NOT save the registers, and there is no indication anywhere that they do either.

I hear that Win386 may have made use of the TSS. Perhaps it copied in the context and then did the switch.

The other problem is that the kernel binary becomes bigger because a large GDT is required.

I will stick to the current solution.

## Scheduler

The kernel will not contain an actual scheduler. That has to be done by manually changing the task time slice property.

## Bootup

I think I should have a configuration file that deals with the memory configuration.

There are problems though. Programs that already use XMS will disrupt the process.

The bootloader in its current state actually needs to be changed to work for systems with more memory. A rewrite in C is possible now that I have watcom installed.

## Open Watcom

The compiler is now fully configured. It can compile DOS programs and uses a special run command to set the environment.

The inline assembler seems to support basically all the features I would need, even for kernel-level code (which is default for 16-bit).

## Plan

There will be two regions usable by OS/90, the extended memory before the ISA memory hole and the region after it.

The location of the region after the hole does not matter, we will limit the size to about 2.5G and that is all.

The region before the hole must be allocated via XMS and its size and location must be reported.

The bootloader must:
- Allocate the largest possible block. This is the second one if the next succeeds
- Allocate another if there is more
- If the last allocation failed, we have under 15M, that is what we report for the before region and the after does not exist
- If it succeeded, we got both of them, report both.

I will use an INT call or maybe an interrupt vector (not to be called) to report both blocks, kind of like ^C or critical error handlers.

B0 - Base of primary memory block
B1 - Size of primary block in full 4K pages

B2 - Base of secondary block
B3 - Size

The size is zero to indicate it does not exist, regardless of base

The kernel binary will be included so that it can be compressed with the rest of the code.

# December 19

## Bootloader Rewite

It is sad that I have to let my old bootloader go, but there are some significant changes that must be made.

Maybe C is not totally necessary. I could just use the incbin approach.

Anyway, I discovered a good compression tool called apack.exe that runs under DOS. It is able to shrink the file by about 50% so far, which is a good result and proves that the embedded binary idea is viable.

When I add DPMI, there will be a lot of redundant code.

## Bit Array Allocation Revisited

There is a hardware accelerated method of allocating bit arrays using bsf, which can work on a 32-bit value too.

The most slow part is the array item per-bit iteration. BSF will find the first

It may be faster, not really sure.

The only problem is if we have a situation where the allocation failed.

gcc generates good enough code anyway.

I also cannot find an actually good use for this anymore. There is port allocation for plug and play though.

# December 22

## Bootloader

I would prefer to use FASM over watcom assembly.

# December 24

## Using C For Bootloader

The XMS API can be called using modified calling conventions. Pragmas exist for that.

Some function calls have mupltiple returns, but they can be split up into separate calls or be turned to a procedure.

I can leave the operating system to do its own job at detecting memory above the ISA memory hole.

Or I can do it all in the bootloader and not clog up my kernel code with INTxH garbage that is not needed.

The compiler does not let me use 32-bit registers, so I cannot handle more than 64M of memory at a time. I could of course allocate in a loop.

Or I can configure the XMS driver to not allocate more than that amount of memory and detect the rest manually. There is a BIOS call for that.

Just like before.

Now I wonder why I bothered doing this. It appears the only thing I have to gain is the compression and integration of the executables.

Aside from that, the bootloader is perfectly working.

So I can basically reuse the existing code. The only potential problem is too much memory being used before the ISA hole and there not being enough space for the full kernel and BSS section. This is very unlikely, but I may want to print an error for that, and do a proper check in the kernel.

Nobody should use that much memory, but I can see real mode disk cachers potentially needing that much and I need to report such errors.

Honestly, if you need that much disk cache, just use a 32-bit driver. It will be way faster. 32-bit disk access will be one of the first things I will do once drivers and programs can load.

Using the existing code, I can add a check to ensure that there is space to load the kernel and that the XMS manager reports the correct amount of memory (along with an appropriate message.)

There is ONE thing to add: report the size of the memory

# December 26

## Changing Direction

I am thinking about writing for the 512-byte demoscene. I tried the CHIP-8 interpreter and it was not possible.

Some ideas:
- The snake game
- Space invaders

# December 28

## Back to OS/90

The bootloader needs to be fixed.

Objectives for the bootloader:
- Report the size of the allocated region
- Maybe add some sanity checks to ensure the kernel will actually fit. This requires checking the file size.


I find myself poorly motivated right now. Maybe I should wait to go back home before coding anything.

# December 29

## Plan For Now

- Update and clean up documentation.
- Really am not in the mood for coding right now, but my next order of business is to make the bootloader report the size of the allocated region. THe rest is manually detected using the correct function from the BIOS.


# December 30

## C Library

The C library can target a DOS environment and expect the existence of DOS calls. There is no need for it to be portable.

I will have to test it on DOS, probably with DOSBox-X.

### Compiler

I have a copy of DJGPP for MacOS. If I can link my own library separately, maybe I can use DJGPP.

I just need to provide the necessary options for a freestanding environment.

crt0 also needs to be properly configured. Things like malloc and the default streams will have global variables local to the program.

crt0 is always statically linked. It is not really an archive though, but a single object file.

Not really. There is no linking process that would make crt0 capable of that. The library just needs to manage the context of the calling thread. In DOS, this will obviously not exist.

Also, I have to keep in mind that DJGPP is not the best tool for creating a shared library file, unless I use the .o format and strip it, and therefore restrict myself to its unique COFF format, which is fine.

### Linking and Shared Libraries

DJGPP can generate a .a file from the objects. The most optimal way to link the library is to:
- Separate almost every function into a different source file and therefore object file
- Strip the final result to make sure there are no non-exported symbols

Symbols not marked as visible explicitly can be removed using the strip `--all --discard-all` command. DJGPP supports this. Visibility attributes override this behavior.

See: https://blog.fesnel.com/blog/2009/08/19/hiding-whats-exposed-in-a-shared-library/

### Userspace Program Linkage

I suppose a special section needs to exist for linking with libraries containing a list of libraries to import.

Compiling programs will be rather difficult. Executables will be converted to single object files as well which have to be stripped.

### Implementation Technique

Names will have to be obscured in some way, either with underscores or something else. GCC makes this easy using the alias feature. I can make a macro like this:
```
int __jc(printf)(/*---*/);
```

The main technique I will use will be unit testing. I will run some functions in complex ways to test expected behavior, as well as run comprehensive tests to ensure proper behavior.

### Calling Convention

Programs designed for a different theoretical libc need to be runtime interoperable with my library. This makes very little sense in my environment, but the idea is that if I had some other C library (this is possible), I want the program to be able to switch between them if requested.

Now that I think about it, I cannot really come up with anything in particular that requires me to use common calling conventions. jlibcq is not a drop in replacement. If a program or module uses a GNU or newlib header file, it must be linked against that.

So there are no mandatory calling conventions, but cdecl is actually superior and conventional for C. It does not repeatedly change the stack pointer upon returns and allows successive calls to be cleaned with one addition.

### Motivations

The glibc port that DJGPP provides is probably too bloated for DOS. The Open Watcom one is probably good, but I just want to have the experience of writing a C library. It would look good on a resume.

It is a lot of work though.

An advantage may be that the static linking can be made much more efficient for programs that require it (native DOS).

### Notes on Compilation

A different C library requires separate compilation. The compiler and the library have to interact in the correct way to generate executables that work with one specific library.

The naming of functions internally is totally my decision. I can do what newlib does and add a trailing underscore and then use a define.

I am not sure if it would be a good idea to define the function calls barebones, even in a freestanding environment.

### Can This Be Done?

crt0.o is probably a required part of the program if I compile with DJGPP. It includes a stub too and the mode switching code.

I am not sure how I am supposed to write my own version and test it.

# January 1

## Build System

Can I update the build system to python?

I don't really care much to writing OS/90 in a way that would have been feasible in the 90's. But either way, Python CAN run under DOS, although it is difficult. The latest version possible seems to be 2.4.2 which is very old.

# January 18

# January 19

## What To Do Now

First off, I do not need a perfect build system. Just one that is good enough. My soruce trees are flat.

Once a build system is finished, I can continue working on booting the kernel.

The old bootloader has significant problems, as specified earlier. It is incapable of reporting the true amount of memory on a system or sectioning off the contigous blocks usable by the OS.

Aside from that, there is really no problem. The less I change the better.

From what I can tell, there is no need to change the whole thing.

Actually I remember some things now. We can force the user to limit the size reported and do part of the detection in the kernel but only for post-ISA hole memory. This means we ONLY neeed the size of the first allocated chunk and its location.

# January 21

I am really bored of the printf thing. Currently, I have a printf-style function available and it actually does not need to be compatible even if I give it more features.

## Build System

I should keep it very simple. The purpose is mainly to make the build system more modular and easier to understand.

### Components

- Tools: a class that is used by its name and not instantiated that abstracts a shell command or other program. They must be built before use if necessary.
    - Operator overloading allows for C++-style redirection of outputs.
        - Example: x = Grep("-n '//'") << Cat("main.c");
    - The call operator is overloaded.
    - The command string is generated and then executed in the shell.
    - () is not required if there are no arguments.
    - Tools can easily be reimplemented in Python if they do not already exist in order to avoid executing a real subprogram.
    - Arguments can be arrays, strings, or maps.
    - Builtin tools: LineCount, LnPfx, LnSfx

- Module: A folder containing a file called `1.py` and `__init__.py`.
    - There is currently no support for dependency checking
    - A module is not a package. That is what a package manager is for.
    - Modules can be build, cleaned, configured, tested, and ran
    - Testing modules requires all other modules to be built.
    - Only one module can be tested at once.
    - Modules are controlled by the Project Executive. It provides builtin Tools for installing built files into the correct paths and sets proper options related to testing.
    - Modules can set local module variables and inherit global project variables

- Project Executive
    - This is a script executed by the user combined with some library features mostly used internally
    - Distribution generation handled here
    - Provides features used for installation of built files

- Module Installation
    - The system has a defined filesystem structure.

> "Solution"?

## Filesystem Structure

```
OS90/
    DRV/
        0/
        1/
        ...
        MY.DRV
    CINC/
        MYLIB.H
        STDIO.H
        ...
    LIB/
        LIBC90.DLL
        MYLIB.DLL
    HOME/

    KERNEL.BIN
    OS90.COM
```

# January 23

## Build System

Not having a proper build system is a major issue and is making it impossible to develop the OS.

I will return to makefiles.

Installation will involve mcopy but will work in a way that is compatible with DJGPP. That way a user could recompile their own kernel or something like that.

Makefiles can also include others. This can be used for configuration.

# January 24

## Makefiles

The current makefile w

# January 28

## C Library Testing

I may not need to use the other computer for the C library at all. It is more convenient to do it here. DJGPP allows me to run everything on the intended architecture and reduces the build complications.

DJGPP is required in general. Userspace must compile with it. Libraries will be generated as object files with names stripped and hidden default visibility.

DJGPP can also do freestanding compilations so that is not a problem either.

## Internals

The testing architecture will be based on constructors. These work under DJGPP with no problems.

A custom assert will be used which prints out a message with more detailed information on failure.

### Function Declarations

I will imitate the newlib approach by adding a newline to the end. A macro can then be defined. This means it will work even if I were to write this:
```c
int printf(const char *, ...);

int main()
{
    printf("Hello, world!\n");
    return 0;
}
```

`printf` will expand to its correct meaning.

The only problem with this is that assembly code may reference the wrong names. The thing is most assembly code will probably not even work on OS/90 if it was written for UNIX or anything else. It is unlikely to be portable.

GCC allows function names to be decided using asm(). I can have two implementations of some sort of NAME macro. One will give it the stdlib name. The other will give it the underscore one. In C however, underscores will always used for library functions.

This only matters for the declaration, not the definition.

Example:
```
void *(const char *__s);
```

I prefer preceding underscore.

## CLIB Build System

All code will be in a single directory level. Each file will be compiled individually.

This will be done 100% according to good practice.

The following targets will be supported:
- lib               Build everything and link as stripped object file
- install-dos       Install for DOS compilation environment
- install-test      Install in the test directory (used by DOSBox-X)
- uninstall-dos
- test              test-exec
- clean

# January 30

## Memory Manager Overhaul

I cannot think of a reasonable malloc design without multiple address spaces. brk would not be feasible without the potential for memory copying or changing address locations.

## Ideas

### Accessing process memory

A special region in the kernel address space will be used to map 4M granular chunks of process address spaces to be accessible by the kernel.

Each process can be given access to some fixed amount of memory. I can copy page directory entries to the assigned region for easy access.

### The HMA

The HMA is completely changed. It cannot be used as a communication region or contain page tables after boot.

Well actually it can. As long as we save and restore the real mode pages that is all fine.

Page tables obviously cannot reside in it since their addresses will change.

### Allocating Page Tables

Allocating page tables is the biggest problem because they cannot be accessed directly.

It is not that bad if I use the process page directory entries to find their locations. Only one needs to be accessed at a time so it is easy to map a dedicated region.

## Maybe Not?

It will make the OS slower. Also, malloc does not benefit from it since allocating pages is not actually that slow. The only advantage is DOS programs being able to access more memory.

## LIBC Name


# February 3

## printf

I should simplify the code and make printf no longer portable:
- Assume 32-bit SysV ABI, IE long is 32-bit
- This means size_t, ssize_t, and long are 32-bit.

I can therefore reduce the overall complexity.


## COFF Loader

I can load COFF files built for DJGPP faster by detecting that it is a stubbed COFF file and perform the loading process entirely in the kernel. This avoids having to perform slower DPMI calls.

# February 4

## Support for Wide Character Functions

printf can be made to work with wide characters by including the whole implementation as a header file and defining a macro or two so that it compiles a different version. Two separate files can be used for this as well.

wprintf is not supported by the DJGPP library so I have no ways to test it.

# February 5

## Type layout

Reconsider:
```
LONG CPC    =>  LONG const * const
LONG PTR    =>  LONG *
LONG CPCDR  =>  LONG const * const __restrict
BYTE CPCDR  =>  BYTE const * const __restrict
```
Shorter than writing const and restrict.

Also consider changing the names to the old ones:
```
BYTE    S_BYTE
WORD    S_WORD
DWORD   S_DWORD
QWORD   S_QWORD
```

# February 6

Currently, there is no way to build the OS and the codebase is currently undergoing some changes.

I will not continue with the kernel until the following conditions are met:
- printf is done and tested.

malloc can wait although it may not take that much time.

Currently printf is going through deportification to make it 100% optimized for i386. I have a sript that executes it in parallel with

## Notes About malloc

A custom malloc will probably not comply with pointer aliasing rules.

It can return a restricted pointer but that would have to cast to an incompatible type. Void pointers have this problem in general, but malloc could easily collide with a dangling pointer.

For that it requires a special attribute for GCC.

The real thing is that pointers to the allocated block will alias and can be of any type. There is no way to predict this.


Functions that return non-aliasing pointers can be marked with the malloc attribute. Even if it returns NULL, it is underfined to ever dereference a NULL anyway.


## Notes About memcpy

MMX is the most common SIMD ISA of the 90's. It is capable of 64-bit memory accesses.

According to Intel optimization guides, 8-byte transfers are faster even if the bus is 32-bit, such as a PCI video card.

String ops are still very fast though.

## Kernel Formatted Printing and Strings

Strings will be implemented as `BYTE PTR`. This is legal in C and I used this early in the project.

Because the kernel is meant to be clearly distinguished from the rest of the environment, we need a proper version of each feature that standard C has.

printf is very complex to be integrated into the kernel. I like the function in general but I think that it may not be the most optimal.

The most significant problem is the use of C integral types which I would normally like to avoid, but I already know what they alias to anyway.

Maybe I can change NULL to OSNULL and it should work.

## Kernel String Operations

These will be rewritten in the appropriate style.

## printf Again

The buffer commit procedure is different between printf and snprintf. printf prints the number of correctly generated characters, while snprintf prints the number that was supposed to be printed as the size of the output is already constrained and nul is inserted to make the size clear.

For the C library implementation, it will be necessary to use different commit options for both. sprintf is deprecated but many old programs use it and it has a much simpler procedure.

Well that was pointless because printf is never used to commit to a real buffer. The actual C library will probably call fwrite or fputc.

Buffer commit will influence the return value. This should be made clear.

# February 7

## printf disaster

I made the snprintf commit function work correctly but now absolutely nothing works.

Aside from the buffer commit function, everything is insanely broken. Attempting to call the core function causes it to fail. This happened on Linux too.

## Solution

I did not add any major features to the unportable one, so I will go back to the portable one.

# February 8

## Memory Operations for OS/90 Kernel

I prefer pascal strings and think they should be used. The string operations will be designed for pascal strings instead. There will be compatibility for C strings with the null terminator.

There is no standard format for a pascal string, but mine will use a 16-bit size specifier. It will have some overhead but I consider it worth it.

I also will implement some of my own byte operations. Can there be special optimizations?

## GCC Builtins and Optimization

I have heard that both glibc and builtins are not always the fastest options.

So I have a heuristic plan for a fast set of memory operations.

memcpy can use a ternary operator and check some compile-time constant values to determine the appropriate action.

If the block is a multiple of 4, we can inline a full rep movsd.

This is not entirely bad.
```
push c
push s
push d
call memcpy
add esp,12
```

This will be at best 17 bytes if the count is <= 255. It also depends on the stack pointer heavily (both inside the call and outside of it).

If we know how many double words there are to copy, there is no reason to do this. GCC does have string op support.

I should also research how good GCC is at copying structures.

Also, memcpy returns the destination. I would prefer a function that returns void because it is more efficient and does not need to return anything.

I cannot reference memcpy at all by symbol name because drivers cannot use it.

So I will need to create my own inline method of doing byte string ops. Many of these operations are highly inline-able.

## New Trick

It is possible to use LEA to multiply by odd numbers in a chain.

```
lea eax,[ebx+ebx*2] ; x3
```

This is faster than using IMUL and does the same thing.

# February 9

## memcpy misalignment recovery

The calulcations:
```
- Align both pointers
- Calculate the extraneous byte size
- Copy count - uabytes / 4 DWORDS
- Copy extraneous
```

Here is the idea:
```
0123456789ABCDEF
_NNNNNNNNNNNN___
_BBB________B
____DDDDDDDD____
```

The offset of the pointer within a DWORD block does not tell the number of extraneous bytes at the head, but 4-x gives that.

This process is done to both pointers to get the number of head unaligned bytes.

The next step is to calculate this exact same thing BUT for the pointers plus the copy size >> 2.

In this case we DO need the other pointers.

# February 10

## COFF Loader

The kernel will operate like this:
- Drivers are COFF object files, not executables, like dynamic libraries.
- Kernel symbols are exported by name, no longer using a call table.
- Drivers can include others as dependencies
- The method of listing the import library list is manually implemented for flexibility.

To access something like the PCI API, the PCI driver must be present and included as a dependency.

```
#include <drvlib/pci.h>
#include <krnl/drivers.h>

DRIVER_IMPORT_LIST("PCI");

void Main(void);

STAT GeneralDispatch(DWORD code, DWORD arg)
{
    switch (code)
    {
        case GE_INIT:
            Main();
        break;

        default:
        return 0;
    }
}

void Main(void)
{
    WORD n = PCI_CountDevices(PCIDEV_IDE | PCIVEN_ANY);
    if (n == 0) {
        Report("IDE drive not found\n\r");
    }
    HPCIDEV PTR p_devs = malloc(n*sizeof(HPCIDEV));

    PCI_OpenDevices(PCIDEV_IDE | PCIVEN_ANY, p_devs, n);

    // ....????
}

```

Just some random code I threw together. Probably not going to look like that in practice.

It may be necessary to load libraries at run time. This can be solved by making undefined symbols not an error and if they are called, the driver can be forced to deactivate.

## Managed Memory?

I can garbage collect driver allocated memory to make memory leaks slightly easier to prevent.

```
DrvAlloc(DRV_NAME, 100);
```

Internally, a small header can be used to keep track of the driver, just 8 bytes to identify it.

A fast way to lookup driver names is to load the first 4 bytes of the name (its unique identifier) and find the similarity before checking the next 4 bytes.

This can fail if the second double word is the differentiator unless we scan that too on a second pass. Two DWORD compares cant be that slow.

## Alignment Guarantees

I would like to make a guarantee that types are naturally aligned regardless of compiler settings. This allows for some simplifications.

For example, an inline movs can assume the alignment of pointers.

This can be problematic for referencing data that does not have a definite alignment.

It is very helpful to know the alignments because it is possible to use the correct inline assembly operation.

Not really though. The size matters more. But it cannot be taken advantage of if the alignment is not defined.

Really? I think alignment guarantees are fine, but it can make structure packing impossible without special types. The improved optimization when size optimizing is obvious though, but I can enable it manually too.

Leave it to the compiler.

As for an auto movs operation only the count matters and what it is divisible by. A compound expression can sort it out. If the count is not a compile-time constant, we just do movsb.

Strings, however, may need alignment. Zero-terminated strings probably do not since they are already slow, but pascal strings can benefit. If a 32-bit size is used, it will already be aligned in practice.

## The Loader

Loading an executable is a process with individual steps. My first loader code will not load and execute it, but demonstrate that it works correctly.

The loader must be able to parse all parts of the file for access using an interface.

This loader is needed because we want to be able to run DJGPP executables without any of the bloated init code. We also need to be able to load drivers.

There are stages:
- Header and section parsing to convert to internal representation.

```
COFF_Open
COFF_Close

COFF_ForEachSection
```

Symbols get special fixups and are dealt with once other things are handled.

Sections are parts of the file. They are read directly from it into memory using a specified address which is also the fixup address. The loader is designed for single address spaces.

# February 11

## Integral Types

If I know exactly what BYTE, DWORD, and WORD are, what is the point in even using them? Most of the time, functions that take integers will take a 32-bit one.

I will test many components outside of the OS to verify correct behavior before final integration. It is not convenient to use the integral types combined with the kernel types. It makes little sense because I already know what they are, so it is all pointless.

Also, pressing the shift key all the time is avoidable.

Basically, the exceptions to the type rules make the rules pointless.

Also the pointer idea needs to go because it looks unconventional and not very C-like.

I should make the code look familiar to the average C programmer.

## Changes to Style

Structures are capitalized and typedef'ed as usual, but integral types will replace the OS/90 ones.

The main reason I even used the all caps integer types was because I wanted to immitate the Win32 style, which only works the way that it does because long ago, there were different pointer types (far, near, etc.) and that made things easier.

Anyway, I will remove the typedefs permanently.

Rules will include using unsigned whenever a signed is not explicitly needed.

The integration of C-like function calls makes it totally pointless to even bother with the current typedefs.

## Attribute Malloc

This attribute only applies the unique addressing knowledge if the pointer is not null. The thing is, null pointers are not 0, so it cannot really do this.

However, dereferencing an INVALID object is illegal in the same way dereferencing NULL is. If the object is invalid, it will cause an error at runtime unless caught by a conditional check.

So malloc is safe to use even if NULL is not 0. In fact, the not_null attribute can be used to indicate that the pointer will not be zero.

malloc cannot return a pointer to something that is already allocated. If something is freed and allocated over, any pointers that existed to it previously are considered invalid, or "dangling pointers." Same goes for any pointers stored inside the block or references thereof.

The attribute would be: ` __attribute__((malloc (free), returns_nonnull))`

# February 14

## Disk Cache and FS Cache

Instead of using a cache coordination protocol, both can hook each INT 13H. If the filesystem driver is loaded, it can apply its own caching and if it decides not to, it can deffer to the disk driver by refusing to handle INT 13H.

The caching the FS driver would do relates mostly to directory contents and FAT structures. The protocol for caching those is significantly different and not much code could be reused either.

## Malloc

Wasting 128 bytes for each small allocation frame seems like a bad idea.

I should use 64-bit masks. It is slightly slower but much more efficient.

Also bit scan is not arbitrarily fast. It costs 3 clocks per leading or trailing zero. For a 32-bit integer, the worst case is 10+31*3=103 clocks which is VERY slow.

It is actually faster to perform branches and avoid this.

On the 486, it is just as slow. No improvement at all. The pentium roughly halves the clocks and also there are branch prediction benefits.

The number of leading bits to scan can be reduced by alternating between forward and backward scans.

On modern CPU's, bit scanning has not been data dependent for a while.

I asked ChatGPT about this and it says that alternating reduces the worst case complexity.

## Driver Symbol Exports

For comparison, the PE and ELF formats use sections for import and export tables.

I may have to do this because the symbol table may not be very god at differentiating.

There seems to be no way to do...

# February 15

I do not intend to write my own linker, so the loader can be optimized for loading executables and libraries.

I have confirmation that COFF does not support symbol visibility. The compiler says so when I try to change it to hidden.

## Return To Call Tables?

I can try to use symbols, but there are cases in which using symbols exclusively is not desirable. If I want to access the services of a driver it is best to do this dynamically and not import the driver like a DLL.

I can define a macro for each export function that call the table automatically.

This is actually a very good idea.

With one thing: the call table must be considered costant, along with all members. That way the compiler can optimize it better.

This means that we do not have to deal with symbols at all ATM.

Macros just alias the table entry. It could even work within the kernel.
```
#define M_Alloc ((SVC_TABLE*)(0xFFFFF000))->M_Alloc
```

For drivers, this makes the code more readable. For the kernel, it is totally disabled by a compiler define.

The only issue with this is the overhead of accessing the memory.

In assembly, I could write:
```
kall    M_Alloc, 8, PO_RW
```

kall being a portmanteau of kernel and call.

Member dereferencing is a high-precedent operation, along with calling.

So if I add a number to the return value of one of these macro calls, it should work.

A more robust way could be to define a function-like macro. This is necessary for some functions to be efficient, such as memset of memcpy.

## memset

memset only works at the byte level. It can be optimized by broadcasting the value of this byte to a full 32-bit register, which allows for SIMD-style operation with the stosd code.

## Making String Ops Faster?

Null-terminated strings are known for their slowness.

In general, strings are not usually that large. There are also optimizations that can be expanded from macros to make some of them simpler.

strlen on a constant string can return its size minus 1.
strcmp can expand into an ASM snippet that does a fast comparison either using base register indexing or a string op.

chtype functions can be totally inlined. There is basically no reason to call anything since they are about two or three comparisons anyway. Lookup tables are not even worth it for these.

There is a difference between the function macro and the name. They cannot redefine.

## Linking Userspace with Kernel

printf.c will simply include the printf.c of the library. Same with malloc. Defines can be used before to control parameters.

## malloc

64-bit allocation flags will be used for the smallest frame. This means 60 bytes per allocation.

## Making Memory Allocation Faster

I have discussed this before, but it needs to be done again.

Physical memory allocation is currently VERY slow. The main problem with it is that allocations in higher regions of memory require potentially thousands of iterations, which is VERY slow and requires accessing very non-temporal regions of the memory table.

I can use tree structures.

There are two potential tree ideas:
- Binary tree where the physical memory total is halved by 2
- Large allocations get a large block with no children.

The requirement is that allocating these structures must be done in a fixed region of memory and should be fast.

# February 17

## Allocating Memory

I will not make any major changes to the basic algorithm. It will use linear searches.

But I will use a few new tricks to speed up allocation.

- Parallel arrays for determining free memory

This allows 16K aligned and granular chunks to be allocated, which is 4x faster.

Large allocations will allocate in 16K chunks __using a memcpy-like optimization__ so that it uses as many 16K blocks as possible.

I can use a lookup table to speed up the counting process and fall back to a regular calculation for chunks that are too large.

Idea: size goes into byte-based lookup table.

Memory waste is not permitted.

- Alternation

Alternating from the bottom and top of the tables halves the worst case complexity

- Fast single page allocation

The kernel malloc will heavily use single page allocations.

This can be optimized by scanning 32-bit free table entries and checking if there are any free.

Checking the individual entry can be done branchlessly using test/set and a table for the index, but it is best done with branches to avoid memory access.

The last entries are the least likely to be allocated.

Single pages can be allocated in an alternating pattern from the center of the largest memory block.

### Summary

This requires no changes to the algorithm but significantly improves the speed of allocation, especially of large blocks.

Allocating page granular chunks is always possible, but it is just slower and done as a fallback. No memory is wasted.

## Avoiding Zero Extends

The zero extend operation is usually slower than zeroing the register first when accessing a memory operand.

The ABI mandates that there is no extension done upon pushing to the stack except for char to an int, which is the C standard.

There are reasons for this. The compiler may push a 32-bit register and not bother doing the sign extension itself, which may be good for code size.

However, it is slow to do it in the first place for data that is constant (e.g. an interrupt vector) and movzx is slower that xor/mov on some CPU's, despite GCC generating it on 386 tuning.

Even if a function genuinely takes a char, it should always promote to a full integer.

This may be useful for printf.

## Aliasing in the Kernel

The memory manager will probably make extensive use of unsave pointer operations and casting. I would like to avoid writing the MM in assembly.

Mostly for the memory allocation stuff, this is a possibility. Bytes may need to be accessed as a word.

I can always use a union. This is cleaner most of the time.

```
#pragma pack(4)
typedef union {
    unsigned char   b[4]
    unsigned short  w[2];
    unsigned int    d;
}MFE;
#pragma pack()
```

The performance is basically the same, since we are dealing with offsets anyway.

## MM Terminology

Tables of Frames:           fft
Virtual address space:      vas
Buffer break indicator:     bbk

## Opaque Types

I can do something like this to add compile-time diagnostics to misuse of buffers.

```
typedef struct IN;
typedef struct OUT;


void ReadData(IN *buff, unsigned size);
```

For calling:
```
ReadData((IN*)buff, 1024);
```

It is almost impossible to get wrong. This is also good if the function takes more pointers, some with different properties.

This is for functions that do not have a definite type. Instead of void, we use an opaque pointer that requires an explicit cast, which makes it clear when reading.

If the data type is known, this is NOT done.

Other examples of this include unsanitized strings of buffers from userspace in the kernel.

My example may not be perfect, but this is considered good practice these days.

It does not violate pointer aliasing. I can create as many unsafely casted pointers as I want. They are not invalid unless accessed incorrectly when more than one is still in scope and get used.

Opaque types never get dereferenced.

I can also make pointer types that act as containers, with 4-byte packing of course so they can be passed. Restrict variants can exist too.

And a define can generate them for custom types.

void ReadData(in_char buff, unsigned size)
{
    for (int i = 0; i < size; i++)
        buff.p[i] = GetDataOrSomething();
}

ReadData((in_char)my_buffer, 1024);

With proper naming, this may not be necessary.

Actaully NO, we cannot cast a pointer to a structure. But we can use a compound initializer.

ReadData((inbuf_char){my_buffer}, 1024);

The purpose is to make sure that one type is not mismatched with another when the underlying data type is irrelevant or intentionally hidden.

Void pointers are generally unsafe and the C compiler will only generate a warning. This is not a problem with C++ and I can of course just prohibit it with the appropriate option.

So really it is more of a style thing and a way of proving that one knows what is being done. The purpose is self-documentation.

I should not spam it too much though.

But this would improve the self-documentation of the APIs and the code.

# February 18

## Pointers and Buffers

void PrintStringConst(in_const_char_r str)
{
    //...
}


void S_Terminate(inp_TASK task);

S_Terminate(my_task);
S_Terminate(my_task);

### Maybe Not

This is only useful for opaque types of void pointers, which do not actually have a definite type.

This is good for ensuring that the compiler has an error on all possible settings.

Consider the unix read.

ssize_t read(int fd, void *buf, size_t count);

size_t and int are integer types that could be theoretically confused. Same goes for the pointer.

An alternative could be:

FILDE* open( /* ... */);

ssize_t read((FILDE*)fd, void *buf, size_t count);

Or whatever. I will keep this in mind for later.

## Scheduler

The time slicing can be replaced by the idle thread deciding which task to switch to in a loop.

That completely throws out my percent of the CPU idea. It was not that great to begin with, but now I must think of a different plan.

Timing becomes more precise though, especially for asynchronous signals.

This is not actually very good at all. Totally inflexible. It changes at most one single time slice granted to a program.

Very bad idea.

## The Improvement of the Switch Action Idea

We completely remove the need to perform useless memory copies which ammount to about 20 DWORD copies, or 40 load/stores. The only problem is the mediocre entrance mechanism.

Maybe I can do parts of the switch process in the entry point before passing control over. That way I can save instructions and improve cache density.

16-bit offsets will be used by the table, improving the density further.

## Time Slices

This is absolutely critical. I may chose to omit this temporarily and use round robin but a usable operating system will need proper scheduling.

# February 20

## Return to Bit Maps

Using bit maps allows for fast garbage collection of the entire frame.

I can safely special-case the first entry or slip in a bitmap into it and still get access.

4 bytes is enough to protect the entry. The size class is essentially a unique identifier, so if we catch an invalid value as we are allocating, even if it is free, then the heap is fouled.

The first two entries NEVER change.

I will also add pointers to the next and previous frames.

# February 23

## Why C99?

I understand limiting the standard library to C99 to reduce the amount of work I have to do, since most software for DOS is written for even older dialects and C99 is what made C what it is today.

There are however some features I really like that cannot be provided by non-standard C++ extensions. One of them is constexpr, which is supported by by C23 for variables only.

C++11 provides the complete version of constexpr.

There are LOTS of things that are greatly simplified when using C++. For example, the printf implementation can be made more portable by determining the number of digits in numeric characters at compile time.

So why should I care about C99? Even GCC 9 supports the new versions of C and CPP. Why do I want to simulate oldness?

If I wanted to simulate oldness, I could have compiled the OS in Turbo C or Open Watcom, something actually historically used in the 90's.

This would effect the performance characteristics, and that is not what I want.

But at the same time, people DID write bloated C programs with poor optimizations. They wrote and ran these programs on their own systems. It was used for GUI programs mainly because they mostly call functions.

I have made no efforts at portability. So what will I do?

Why is it OS/90? It is not '90s themed at all. I hate that decade. I only like the technology.

I do not live in the 90's. Maybe it is more impressive if I limit myself to 90's technology.

NASM was introduced in 1996 although it did not reach full functionality until later. Open watcom was very popular in the 90's and was cheaper than other options.

Many features that I have used so far are C99. It introduced:
- long long
- inline
- snprintf
- variadic macros
- many more

C99 is the premier C standard. Anything older is just deliberately creating more work for myself.

I think strict C99 compliance is a good compromise. C89 is far too archaic.

## malloc Code Ideas

Because of the high amount of generic code I wonder if I should just compile it in C++. It does not depend whatsoever on anything OS/90 related anyway.

Or I can include the implementation several times with macros as parameters.

I think C++ is better. For integration, this is not a major problem and I can easily add C++ compilation to the build system.

I wonder how much C++ I can even use though. printf could be enhanced with C++ compilation if it did not depend as much on buffers on the stack.

C++ also can use variable length arrays on the GNU dialect.

No I will NOT do this. I can recycle the same code however, using an include file.

## malloc Again

Using a lookup table is a good idea when the number of bits is 4.

Instead of (ebx is the input):
```
    mov ebx,[mask]
    bsf eax,ebx
```

I can write:
```
    mov ebx,[mask]
    mov eax,[ebx+table]
```

The top takes 10+3*3=19,+4=22 for the bit scan in the worst

The bottom takes 4 for the first and 4 for the next, but there is a 1 clock penalty for the immediate (on the 486?) and another for the address generation interlock.

So 10 in total. The only issue is the decreased cache density.

Bit scanning can be done directly. On old CPU's, this is not actually slow and its seems the speed is consistent.

To make the code simplified I should not use structures for frames whatsoever. Instead, I will use a stride parameter to use for moving through this list.

BTW when it comes to memory latency on modern CPUs it is way slower but the CPU is also faster, and on an old CPU the memory is very often clocked the same or similarily to the CPU.

## Rewite malloc

It is really bad and disorganized. I will end up deleting the code. Probably dont need to delete all of it.

# February 28

## printf and Checking Argument Sizes

Many of the conversion handlers check the buffer length.

### Iterate Faster Backward?

If the buffer is written to in reverse, there is an advantage that involves keeping the result of the division needed to get the digits.

The modulus of 10, which we also can divide by, is already computed in x86 as part of the operation.

So we can remove the multiplication totally and do one single division operation.

The buffer passed will be the tail byte. Getting the outputted character count should be returned for fast buffer copying.

This is WAY faster. One expensive operation instead of two or three. It even beats the lookup table AND it is portable. It is also simpler!

The only issue is that I have to rewrite some of the code that uses it.

## Why It Is Better and other Improvements

The code is shorter first of all.

Also, this is way faster than a portable three expensive operations style because only one division happens per iteration and no multiplication at all.

Even the lookup table is slower because it is less cache dense (10*4=40 bytes) and requires memory access with a ModRM/SIB operand and gets a one clock penalty on the 486 for the IMM.

There is only one potential bottleneck and it is the fact that the iteration count is always the number of characters in the largest number.

Also the pointer arithmetic should be done in the procedure so

## Optimizations

The compiler knows how to use LEA tricks to speed up multiplications. The early out that I already implemented gives it the information to know that it is safe.

        mul     esi
        mov     eax, edx
        shr     eax, 3
        lea     edx, [eax+48]
        mov     BYTE PTR [ecx], dl
        dec     ecx
        cmp     ebx, ecx

Actually, the compiler generates this as part of the loop ONLY for constant propagation. constprop symbols are used locally to calculate things in the static context with inputs under control of one translation unit.

If the scope is public or an unknown input is passed, this does not happen.

But the compiler is able to do that. It can completely fold functions by predetermining the results requested.

The code is much more bloated that I had hoped. I can write this in assembly if I wanted.

Yes, I could really write my own better implementation. It generates many pipelines hostile codes that I could beat myself.

# March 1, 2025

The compiler outputs extremely bad code for the conversion. In fact, it is so bad that I may need to provide an alternative i386 implementation in 100% inline assembly to make up for how bad it is.

Here it is in GNU AS, but the pointer is AFTER the buffer, equal to its base plus size.

```
/*
    This code beats anything that even the latest GCC outputs. If compiling for
    i386, this is used instead of the generic version. Could be changed to fit
    64-bit too.

    "Real Programmers Don't Use Pascal" vindicated.
*/
asm_v:
    push %edi

    movl 12(%esp),%ecx  ; ECX = iterations
    movl 8(%esp),%edx   ; EDI = buffer, we need it for later
    movl 4(%esp),%eax   ; EAX = value

    ; If the number is under 65535, reduce iterations to 5 for ~-200 clocks
    cmpl $0xFFFF,%eax
    jae 0f
    shl $1,%ecx

    .align  16
0:
    subl $1,%edi        ; Decrement down
    subl $1,%ecx        ; Decrement loop counter
    xorl %edx,%edx      ; Clear EDX for division
    divl %ecx           ; EDX now equal to the digit
    add $'0',%dl        ; Convert to character
    movb %dl,(%edi)     ; Copy to the buffer
    jecxz 1f            ; If zero, leave
    jmp 0b
    .align 16
1:

    mov $'0',%eax
    repe scasb
    mov %edi,%eax

    subl %edi,8(%esp)
    mov %edi,%eax

    pop %edi
    ret
```

I added a fast exit for a value in the 16-bit range which reduces iterations to 5.

This is worth it since the inner loop is about 46 on a 386. This saves 230 clocks for small numbers as it will unconditionally generate zeroes.

But the timings would still be essentially the same.

This is so much better than the compiled version that I would have to conditionally include it into the portable version. Even with the calling conventions followed, it is way smaller than the compiler's code.

Backward copying is way faster than any calculation I might need to do so I can correctly copy it.

Finding the number of leading zeroes can be done using `repe scasb` with AL set to '0'.

## Other Ideas

I can pass the value of the integer as a pointer so that the call can be dispatched with a table.

One table can grab the buffer size, and another can dispatch. This may be somewhat expensive for cache locality.

I can also set the iteration count using a parameter and completely avoid this. The cost is minimal and reduces the lookup table sizes.

Not quite. Size counts are just a performance optimization. I still need to dispatch the right function call.

The only problem my procedure has is that it takes many iterations. Buffer length parameters reduce this problem, but a lot of code uses the default size. It is better to have a good average or if possible, reduce iterations in general.

A lookup table can be used that also is compatible with buffer lengths (can be the length itself) and the entries should be 8-bit for density.

I don't think this would help much. The automatic 16-bit optimization is good enough.

## Conversions Again

Technically I do not need a special converter for each type, and although it can be used, it will not make the regular size faster.

Anyway, 250 clocks is not actually that critical. It takes 300 to make a system call and 300 to go back.

## Improvement Idea

I can use the -1/not trick to get the number and constrain the iterations as with strlen. There is no need for a rep prefix. The only issue is relying on a separate branch

Maybe not, but the scasb instruction can be sped up by setting EDI to a more reasonable offset.

# March 2, 2025

## Shell Completion and other enhancements

I want OS/90 to have excellant tab completion.

Some changes to the filesystem hierarchy are needed. I will add a directory for command line completions.

Another enhancement I would like to do is make the shell prompt fixed to the bottom of the window. This can be done by making the prompt variable equal to an empty string and not echoing characters until the command is passed.

Internally we will buffer the input before sending it.
Also we are talking about the DOS prompt program, not the shell itself.

The current working directory will be shown on the top of the window.

So the idea is this:

```
--------------------------------------------------------------------------------
[^] C:\OS90\HOME
--------------------------------------------------------------------------------





















--------------------------------------------------------------------------------
>>> dir /P
```

When enter is pressed, the command will go through and the DOS process of the shell will be stalled until the subprocess exists. Somehow I am supposed to know this and disable all the other controls.

Changing the CWD of anything but the shell is a very bad idea, so it is turned off along with the command input.

I should experiment with this concept on UNIX.

Also, the up arrow to see the history can show each command string by stacking them on top of each other so that the user can see them more easily.

# March 3, 2025

I should also add horizontal separators if possible to separate the output of different commands.

To test the concept, I can work on a terminal wrapper program for unix only using ncurses.

## ia16-gcc compiler is now working

I am able to run this compiler under Docker. No need to start the container either, I can execute it independently and build programs.

## Idea For New Project

A new shell?

So I would have to implement all of the features available to the DOS shell.

I have some ideas for features. First of all, the shell can be somehow reentrant, so that it can execute multiple shells using the same code.

This would be possible in OS/90 but I have no way of testing it now. It could be implemented as a DLL perhaps.

I can also use my 16-bit compiler to write it for DOS. This gives a lot more flexibility.

The code would have to be resident but the entire context can be placed in extended memory using XMS.

The problem is doing this globally. How would this actually coordinate properly in a multitasking system?

Probably don't need to think about this.

Motivation has been quite low lately. I don't really know what to work on.

Tomorrow I have 4 hours to myself. Maybe I can record a video.

I need actual ideas.

## Video Ideas

I am really starved of video ideas for my youtube channel.

I need to do this as an extracurricular activity. I will write some ideas down.

## Build System

Header files can completely generate the code outputted by the compiler even if only headers are changed. Altering a static inline can completely change the code outputted.

This means that C sources should depend on their headers.

It also means that header files that are globally used should be used carefully.

Right now my build system will just delete things. If a module has no changes then it will not.

make works using targets. If the target is older than the source, then it has to run the job for it.

I can depend on the header files per file somehow and it can work like that. At least the C library will need something like that as it will be composed of many up to a hundred files with the current layout.

## DOS Control Blocks

There is a structure used for controlling a DOS program that may be sharable with ring-3.

Specifically, I would like there to be access to the framebuffer of the DOS program.

Also, there is a need for cooked and raw IO so I can avoid allocating framebuffers.

Not quite. A framebuffer has to be allocated, and it cannot be compatible with the ATM format unless it is copied.

That makes me wonder if the current memory management system for ATM is actually a good idea.

Or I can use a stacking window manager with no implicit composition.

## DOS CTL Interface

Making this ring-3 accessible can be very powerful.

# March 4

## printf Redesign

Converting signed numbers is changed now. As long as the buffer is large enough, it is very easy to append a negative sign to the end.

```
if (v < 0) {
    char * a = utoa_bw(blentab_i[f->lenmod], -v);
    a[-1] = '-';
}
```

The conversion that I have now already works as intended. We only need two converters, one for int and one for long long.

## C Library Notes

OS/90 is supposed to have optimized file IO at the kernel level, where there is more control.

I wonder if I should strive for simplicity by making IO completely unbuffered.

The problem with this is seeking would be extremely slow because of the ring switch overhead.

The method of buffering is implementation-defined but the userspace can allocate them at will.

## Idea: ring-0 userspace?

This would allow direct access to ring-0 functionality but compromise security somewhat.

A solution is to run programs inside segments, but the problem is that allocating memory becomes a challenge.


# March 6

## printf Reorganization

Buffers should be allocated at the same size. This is not expensive and reduces the complexity of the stack allocation. The compiler does not need to remember how much it allocated as well.

The calculations for the character count will remain the same.

Calling the conversion function will take place with a helper.

It will convert either a signed magnitude or an unsigned integer and put it to the buffer, all automatically and using the appropriate converter.

The buffer will be allocated based on the number of characters for a uintmax_t, just to be safe I suppose.

### Handling Signs

First of all, I want to make it clear that I do not care much about how well it performs in ia16, that is not the intended target. The newlib printf is far better if size matters, and it does for 16-bit.

But it is intended to work on basically anything.

So putting the full value into a long long or intmax can work and it did work too.

The problem is that the conversion requires divisions. If done on a 64-bit number this would be very slow.

And fetching the argument must be done portably anyway.

### Solution

A universal auto-converter helper function will generate everything, including the sign.

### Zero padding?

Zero padding integers will actually require handling the signs. We NEED to somehow communicate the sign after a conversion.


# March 7, 2025

## Universal Callbacks for printf

Buffer copying for zero padding is slowed down by however many characters are needed. Space padding also requires an individual buffer commit for each character.

These are both useful features that are significantly slowed down.

Not to mention, there is no benefit to intermediary buffering while doing the conversions because they will end up going into another buffer anyway.

printf uses fwrite, which is faster, or fputc which is slower. Either way, there is slowness inherent to how we pass the data to be printed.

So there is no reason to not use buffer commits for conversions. This also saves a lot of stack space.

There will also be a character duplication call.

This will classify as an partial rewrite. The dispatch parts are fine.

## New Features

I will define `ctype` to the character type being used.

## Ideas For conversions

With character duplication padding spaces or zeroes can easily be calculated. Padding will not have a prefix if not a signed value, so there can be a separate procedure for that.

Buffer copying will not be necessary at all.

Integer conversion helpers will deal with zero and space padding on their own. The difficulty is with the fact that there is no intermediary buffering.

Here is a solution:
- Converter takes printfctl
- The 0 option excludes padding and precision, so we check for it first.
- We check the sign if applicable and store the appropriate first char to output
- During conversion loop, skip digits out of bounds
- Decrement the padding chars for each leading zero found

Maybe I should NOT. The buffer will not be that big. The conversion can do buffering itself.

Not really. We will output zeroes anyway. So just subtract the max digits from the zero pad area width or precision, whichever is applicable.

With that being said, iterating backward is the fastest way to convert.

Also, the integer conversion buffer, if used, must be inside the printfctl structure to avoid needlessly allocating it per-conversion. We only need one.

## More Ideas

Add more options:
- Disable long long
- Disable zero as padding (0 flag)

These are not very useful for the kernel and require bloated calls to libgcc that could otherwise be removed.

# March 11

## printf

The current integer formatter is ridiculously complicated. It stretches C syntax to the maximum and is impossible for anyone else to follow. It partially works, but honestly could be far better.

Using dup and cmt is good for density and reducing stack usage. I can simply handle every possible case after performing a proper conversion of the number.

The sign, if relevant, should be passed when converting signed integers.

## DOS libraries

There are several non-standard DOS APIs.

For example:
- dos.h
- i86.h
- direct.h
- conio.h

These provide the convenience of the C language to DOS.

But I have an idea.

Most of these functions essentially go directly to DOS with INT 21H or something related. I can easily write most of the calls as wrappers in inline ASM and let the compiler do the rest.

GCC is able to do some of the things that Watcom is, but only for static inline assembly procedures.

For example, nominating a return register is doable. (IS IT REALLY?)


# March 11

## Build System

Header files in OS/90 have the potential to change code. Anything that depends on a header file must be recompiled if that header is changed.

Tracking these sorts of dependencies is difficult.

Basically, if a module has a header file of its own, it can be considered its own unit. Other files are unique because they require immediate recompilation because they may be included by header files themselves and more than one unit.

But really changing any header file has the potential to completely change everything that includes it.

Unless...

I analyze every header and source file. Some build systems actually work like this.

The issue with this is that it has to apply to header files too. Drivers also will use one sigle unified header so this does not benefit much, and they also will be tied to a certain version of the headers. Or something like that?

No, drivers use a defined interface. If this interface changes, the drivers have to be recompiled to match.

Same goes for anything else.

Headers that generate static inlines or anything that directly modifies code under certain circumstances is supposed to also work this way. The problem is that the results are not immediate unless everything is cleaned.

So there is one simple solution. A change to any header file recompiles everything.

For a makefile, this requires depending on the headers somehow.

I will have to figure this out for the C library. The kernel will just clean after each build. It is not that big.

## What is Next

printf can print signed and unsigned numbers. It now need hexadecimal support.

The code I had previously should still work with some minor changes.

Also, I need %s too.

# March 13

## News About printf

My current implementation compiles to a significantly smaller binary compared to the old printf implementation I was using earlier.

I need to test without float though.

I disabled float and it seems that mine is very close to what is generated by the other printf implementation.

I have a few ideas for making it smaller:
- Check for valid format chars without the ~512 byte table
- Disable excessive inlining on some functions

The no prefix printing function should definetely not be inlined.

Actually, the original comparison was unfair! When using the latest GCC, my code compiles to about 2900 bytes according to godbolt.

So this beats the github printf so far.

I am kind of skeptical of the information from godbolt compared to the djgpp size command. I have to use only one source.

But my implementation actually won so far in terms of code size. Stack usage appears to be a bit smaller, but nothing crazy.

Starting with: 2985

After some testing, it appears the compiler generates better code when the compound expression for the conversion prefix is expanded to a full table.

Doing so achieves the size: 2937, or 48 bytes less.

Also, it appears the compiler is not doing a good job with optimizing the bit fields. They reduce the code size so I will keep them. Even changing the container type from char to an int appears to be slow.

The function is inlined within the printf_core function.

This can in theory be optimized away by a single write.

# March 14, 2025

## Floating Point Conversions

x86 has an instruction called fprem which performs a modulus. This is potentially useful for digit extraction. It can be the inline implementation of fmod.

The number of digits can be computed by looking at the exponent part.

Some interesting instructions (C equivalent that can inline?)
- FRNDINT
- FXTRACT

## Next Things to Do

I will not do any of that now. I need printf for the kernel. Float conversions will be done later.

Currently, the # modifier does not fully work when space padding.

## printf seems to be working

One more thing to do. Print strings with %s. Fairly easy and I already have code for it to reference.

That is done now. I can move on to the kernel.

## String Instructions

The whole set needs to be supported. I do wonder how they will be added to drivers.

The best solution may be to have some sort of support library. The compiler may call an actual function and that needs to be linked in.

This library can contain a bunch of object files for relevant string instructions linked together, or anything that stdlib provides which is needed in the kernel and may be inlinable.

This makes the library a build prerequisite to anything that uses it.

The size increase is basically negligible.

# March 15

## Why Not Watcom?

Watcom has limited C99 support. Maybe if I really want to simulate running the OS on old hardware or allow old toolchains to build it, then using Watcom or something like that is the way to go.

Of course a modern compiler will beat whatever existed back then.

Watcom had support for 32-bit mode since 1989. MSVC was 32-bit since about 1993. Borland C++ supported 32-bit code since 1993 too.

## Make It Portable to All Compilers?

I like the idea of being able to boostrap OS/90 on any old computer with any compiler. Even a modern one like GCC could work too.

NASM was first introduced in 1995 and is currently a hard dependency. It was not fully capable until later, but still 90s-era.

While the idea of being able to fully compile OS/90 on an old computer from as early as 1989 is very interesting, I wonder if that is really what I should do.

The vibe of compiling the OS from DOS, with the black and grey text flowing, and then running everything like that, it is something I like to think about.

But I am writing this OS from a 2024 MacBook, and I have new tools that don't do any new magic, but provide useful features that I can use to get more performance and efficiency.

First I must understand the plausibility:
- printf would have to be mostly rewritten, with some parts reused, and retarget C89 (which would be a simplification)
- Using the restrict keyword comes with challenges, as not all compilers supported it before C99. Watcom is able to, but with underscores.
- The C library may miss opportunities to be more efficient

There are advantages. I can avoid adding the newer features. There is no long long type at all in C89. printf is also simplified although with most of the basic features.

## Features Currently Used

- VLA's, compound expression, type-safe min/max macros, and a number of other features are currently used.
- Inline keyword
- Slash comments
- stdint.h

stdint actually does not exist in C89 whatsoever. It may be supported as an extension. That explains why Linux uses __u8, __u16, __u32 and other types like that.

This makes a portable printf impossible, although my existing one is already very non-portable.

## The Idea

The main difficulty is:
- Testing to make sure all compilers work
- Inline assembly code
- Linking the final executables
- Build scripts
- Executable formats for the target
- File names must be DOS compatible

DJGPP inherently is incapable of creating OBJ files. I suppose OBJ is to different from ELF and COFF. It has to do the linking and the final build.

This makes COFF difficult to use as an executable format or DLL. DJGPP cannot link for anything else either.

Watcom can handle COFF and I believe most win32-capable compilers handle COFF too. The 32-bit OMF format was mainly used for DOS.

MSVC also may use COFF as well.

The idea of supporting old toolchains is strange because I do not have access to any of them right now. The nuances of versions and supported features are difficult to ascertain. How far back do I have to go? What needs to be supported?

Consider using a configuration script. But also I need some automatic portability.

Using some preprocessing to make everything work is all fine, but when it comes to actually linking executable programs, there is no consistency.

A tool to convert to/from coff and omf DOES exist and is part of the DMC toolchain and there are other tools for the same purpose.

I do not know if such a tool existed back then.

I need a real format. A DOS 32-bit executable will not have the necessary relocations.

It does not really matter of an OBJ/COFF converter every existed. I could write my own if I cared that much. OS/90 never existed in the 90's.

coff2omf is available from the DMC toolchain. I can include it for distributions if it is open source.

The problem is that Microsoft COFF is not the same as DJGPP COFF.

## Note About Environment Block

The environment block is local to each program. This means that when COMMAND.COM is reclaimed by the kernel, its environment block should be copyied to extended memory and reclaimed.

DOS has other internal structures that are best left alone.

I have a document bookmarked that details the contents of the environment.

## The Decision

Compiling the kernel for different toolchains can be supported. I can add that in at any time with a bit more effort later if I don't do it now.

So I must use only one single compiler over all others.

However, I think that my previous characterization of Open Watcom was unfair, as I was not able to compile printf.

My newer Watcom compiler build should be capable of C99. This means I can compile my own printf, but...

It needs a proper build system, perhaps a program that outputs the number of digits as macros or constants. Macro is probably better for watcom.

First I should judge the binary size. Then I should consider the performance.

But DJGPP is known for its slow IO. So the only way I can be sure of performance is to manually call INT 10h or INT 21h.

DOSBox is not the best emulator for accuracy. Size optimization usually improves the performance in ways it would not in a real CPU.

The only way to benchmark the two legitimately is to use a cycle-accurate emulator. I have 86Box installed and it provides this.

## Methodology

> Next entry.

# March 16

A DOOM benchmark I saw using DOSBox showed DJGPP as being ahead of other compilers. But DOSBox treats each instruction as a single cycle.

I will test printf with several calls and complex format options. This will be done in a separate module to inhibit optimizations that could effect results.

Measuring time in DOS is difficult.

## The Actual Watcom Advantage

Watcom supports complex register calling conventions. It allows for return value nomination and selecting the exact register for each argument, all using the #pragma aux feature.

This is very powerful, but obviously sacrifices portability.

For example, memcpy, memcmp, etc. can all be implemented to use the proper registers with no pushing whatsoever.

It is also possible to declare calling conventions and define attributes to be added to functions.

The problem is that GCC is what I am familliar with and I do not really care anymore.

## C89 or C99

If I want to compile the OS with an old toolchain, this requires adherence to C89 instead of C99, or at least I can use C99 as an extension and not make it required.

The C library can support both since C99 is just an extension. Things like restrict and C99 library calls can simply be omitted when C89 compatibility is required.

The kernel would have to be written to comply with C89 but support restrict since it is a major performance enhancer.

## Decision

I still do not want to start over again. It would not be a total restart, but is still quite a major slowdown.

I want to play a bit with Watcom and see what kind of code that it generates.

C89 however is a very limited standard. If I want it at all, and there are benefits, mainly avoiding implementing some library features, Watcom is more reasonable.

The final decision is between Watcom and GCC, not some version of a standard. I will not worry about boostrapping the OS on old toolchains because that is just for a flex nobody cares about. Getting a working operating system out is more important, since OS/90 is not the only thing I want to do, as much as I like it.

Advantages of GCC:
- More actively developed
- Optimization
- I am familiar with it
- Powerful builtins and inline assembler
- Allows the use of IDE autocomplete
- Superior dead code and branch elimination for portable code
- Dont have to redo anything

Advantages of Watcom:
- Control over register parameters and return for functions
- Inline assembler is different but also quite powerful
- Optimization is tuned for older CPU's
- Far pointer support
- Forces C89, which makes the stdlib simpler
- Generates small binaries even with speed optimization

## Investigation: Phase 1

I just realized that Watcom is able to optimize jumps by reusing parts of other procedures. It does this when there is an empty main to instantly return without encoding it separately.

GCC actually does not do this. Quite suprising. This reduces the complexity significantly.

Also, the compiler appears to use register parameters by default. The memcmp function I tested does not reference the stack at all.

With control over register parameters, I could make every function optimized. Something to think about.

# March 17, 2025

I got this off the internet. The compiler will usually inline it. In fact, there is no reason why it should not do so.

```
__attribute__((regparm(3))) /* For GCC to match watcom's auto-regparm*/
int memcmp_(const void* buf1,
           const void* buf2,
           unsigned count)
{
    if(!count)
        return(0);

    while(--count && *(char*)buf1 == *(char*)buf2 ) {
        buf1 = (char*)buf1 + 1;
        buf2 = (char*)buf2 + 1;
    }

    return(*((unsigned char*)buf1) - *((unsigned char*)buf2));
}
```

I am using DJGPP hosted under Mac OS. It is version 12.2.

For some reason it generates inferior code compared to a Linux hosted compiler. No idea why. The compiler core should be the same. DJGPP is mainly just a library, hence the PP=Porting platform.

Despite that, the code generated by the Linux-based compiler is still larger. I don't know if it is faster. Some of the sequences are actually quite similar

The main improvement is the avoidance of slow movzx instructions. It costs 6 clocks on a 386. On Pentium and 486 its only 3, so not a big deal.

I will find another kernel to test.

## Convert Integer With Backward Generation



Watcom does this:
```
Open Watcom build environment ( version=0)
Module: main
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: _TEXT PARA USE32 00000058 bytes
0000				convert_:
0000  53				        push	ebx
0001  51				        push	ecx
0002  56				        push	esi
0003  57				        push	edi
0004  55				        push	ebp
0005  89 C3				        mov     ebx,eax
0007  85 D2				        test	edx,edx
0009  7C 47				        jl		L$3
000B  89 D1				        mov		ecx,edx
000D				L$1:
000D  85 D2				        test		edx,edx
000F  0F 9C C2				    setl		dl
0012  BD 0A 00 00 00			mov		ebp,0x0000000a
0017  89 D7				        mov		edi,edx
0019  31 F6				        xor		esi,esi
001B  81 E7 FF 00 00 00			and		edi,0x000000ff
0021  8D 80 00 00 00 00			lea		eax,[eax]
0027  8D 92 00 00 00 00			lea		edx,[edx]
002D  8D 40 00				    lea		eax,[eax]
0030				L$2:
0030  89 C8				        mov		eax,ecx
0032  31 D2				        xor		edx,edx
0034  F7 F5				        div		ebp
0036  83 C2 30				    add		edx,0x00000030
0039  89 C8				        mov		eax,ecx
003B  88 53 FF				    mov		-0x1[ebx],dl
003E  31 D2				        xor		edx,edx
0040  F7 F5				        div		ebp
0042  4B				        dec		ebx
0043  46				        inc		esi
0044  89 C1				        mov		ecx,eax
0046  39 EE				        cmp		esi,ebp
0048  72 E6				        jb		L$2
004A  89 F8				        mov		eax,edi
004C  5D				        pop		ebp
004D  5F				        pop		edi
004E  5E				        pop		esi
004F  59				        pop		ecx
0050  5B				        pop		ebx
0051  C3				        ret
0052				L$3:
0052  89 D1				        mov		ecx,edx
0054  F7 D9				        neg		ecx
0056  EB B5				        jmp		L$1

Routine Size: 88 bytes,    Routine Base: _TEXT + 0000
```

And GCC:
```
convert:
        push    ebp
        push    edi
        push    esi
        push    ebx
        push    ecx
        mov     ebx, DWORD PTR [esp+24]
        mov     eax, DWORD PTR [esp+28]
        mov     ecx, eax
        test    eax, eax
        jns     .L2
        neg     ecx
.L2:
        shr     eax, 31
        mov     DWORD PTR [esp], eax
        lea     ebp, [ebx-10]
        mov     esi, 10
        mov     edi, -858993459
.L3:
        dec     ebx
        mov     eax, ecx
        xor     edx, edx
        div     esi
        add     edx, 48
        mov     BYTE PTR [ebx], dl
        mov     eax, ecx
        mul     edi
        mov     ecx, edx
        shr     ecx, 3
        cmp     ebx, ebp
        jne     .L3
        movzx   eax, BYTE PTR [esp]
        pop     edx
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
; 74 bytes
```

Watcom and GCC both had a hard time, but Watcom tried to align a branch even though it had little to gain from doing so.

Both have an inner loop:

Watcom's loop
```
L$2:
    mov		eax,ecx
    xor		edx,edx
    div		ebp
    add		edx,0x00000030
    mov		eax,ecx
    mov		-0x1[ebx],dl
    xor		edx,edx
    div		ebp
    dec		ebx
    inc		esi
    mov		ecx,eax
    cmp		esi,ebp
    jb		L$2
```

It looks like a level of unrolling took place here because it is running DIV twice. The number of iterations is even so that is probably why.

Now GCC's loop:
```
.L3:
        dec     ebx
        mov     eax, ecx
        xor     edx, edx
        div     esi
        add     edx, 48
        mov     BYTE PTR [ebx], dl
        mov     eax, ecx
        mul     edi
        mov     ecx, edx
        shr     ecx, 3
        cmp     ebx, ebp
        jne     .L3
```

For some reason, it is adding a MUL instruction. Note that I used -O2.

On -O3, the compiler totally unrolls the loop.

```
convert:
        push    ebp
        push    edi
        push    esi
        push    ebx
        mov     ecx, DWORD PTR [esp+20]
        mov     ebx, DWORD PTR [esp+24]
        mov     esi, ebx
        test    ebx, ebx
        jns     .L2
        neg     esi
.L2:
        mov     edi, 10
        mov     eax, esi
        xor     edx, edx
        div     edi
        add     edx, 48
        mov     BYTE PTR [ecx-1], dl
        mov     ebp, -858993459
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-2], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-3], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-4], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-5], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-6], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-7], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-8], dl
        mov     eax, esi
        mul     ebp
        mov     esi, edx
        shr     esi, 3
        mov     eax, esi
        cdq
        idiv    edi
        add     edx, 48
        mov     BYTE PTR [ecx-9], dl
        mov     eax, esi
        mul     ebp
        shr     edx, 3
        add     edx, 48
        mov     BYTE PTR [ecx-10], dl
        mov     eax, ebx
        shr     eax, 31
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
```

Yikes. It would run much faster than what I previously generated, but this is 224 bytes. An old CPU would suffer from major cache polution, especially with the fact that many did not have separate icache.

If I change the tuning to 486, I get this:
```
convert:
        push    ebp
        push    edi
        push    esi
        push    ebx
        mov     ebx, DWORD PTR [esp+20]
        mov     edi, DWORD PTR [esp+24]
        mov     esi, edi
        test    edi, edi
        jns     .L2
        neg     esi
.L2:
        mov     ecx, -858993459
        mov     eax, esi
        mul     ecx
        shr     edx, 3
        mov     ebp, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     esi, eax
        lea     eax, [esi+48]
        mov     BYTE PTR [ebx-1], al
        mov     eax, edx
        mul     ecx
        shr     edx, 3
        mov     esi, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     ebp, eax
        lea     eax, [ebp+48]
        mov     BYTE PTR [ebx-2], al
        mov     eax, edx
        mul     ecx
        shr     edx, 3
        mov     ebp, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     esi, eax
        lea     eax, [esi+48]
        mov     BYTE PTR [ebx-3], al
        mov     eax, edx
        mul     ecx
        shr     edx, 3
        mov     esi, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     ebp, eax
        lea     eax, [ebp+48]
        mov     BYTE PTR [ebx-4], al
        mov     eax, edx
        mul     ecx
        shr     edx, 3
        mov     ebp, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     esi, eax
        lea     eax, [esi+48]
        mov     BYTE PTR [ebx-5], al
        mov     eax, edx
        mul     ecx
        mov     esi, edx
        shr     esi, 3
        lea     eax, [esi+esi*4]
        add     eax, eax
        sub     ebp, eax
        lea     eax, [ebp+48]
        mov     BYTE PTR [ebx-6], al
        mov     eax, esi
        mul     ecx
        shr     edx, 3
        mov     ebp, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     esi, eax
        lea     eax, [esi+48]
        mov     BYTE PTR [ebx-7], al
        mov     eax, edx
        mul     ecx
        mov     esi, edx
        shr     esi, 3
        lea     eax, [esi+esi*4]
        add     eax, eax
        sub     ebp, eax
        lea     eax, [ebp+48]
        mov     BYTE PTR [ebx-8], al
        mov     eax, esi
        mul     ecx
        mov     ecx, edx
        shr     ecx, 3
        lea     eax, [ecx+ecx*4]
        add     eax, eax
        sub     esi, eax
        lea     eax, [esi+48]
        mov     BYTE PTR [ebx-9], al
        add     ecx, 48
        mov     BYTE PTR [ebx-10], cl
        mov     eax, edi
        shr     eax, 31
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
```

This becomes the new loop:
```
        shr     edx, 3
        mov     ebp, edx
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     esi, eax
        lea     eax, [esi+48]
        mov     BYTE PTR [ebx-1], al
        mov     eax, edx
        mul     ecx
```

Not exactly super fast, but maybe a few clocks less. MUL has the potential to be faster given certain inputs. The code still looks very pipeline-hostile.

With fastest code on Watcom (what I will globally use anyway) and O2 on GCC with mtune=i486, GCC generates the smaller code.

## Testing GCC Again

I passed some other options to GCC now. I added "--param l1-cache-size=8192 --param l1-cache-line-size=16" which fits the average i486.

```
convert:
        push    ebp
        push    edi
        push    esi
        push    ebx
        mov     ebx, DWORD PTR [esp+20]
        mov     ebp, DWORD PTR [esp+24]
        mov     ecx, ebp
        test    ebp, ebp
        jns     .L2
        neg     ecx
.L2:
        shr     ebp, 31
        lea     edi, [ebx-10]
        mov     esi, -858993459
.L3:
        dec     ebx
        mov     eax, ecx
        mul     esi
        shr     edx, 3
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     ecx, eax
        add     ecx, 48
        mov     BYTE PTR [ebx], cl
        mov     ecx, edx
        cmp     ebx, edi
        jne     .L3
        mov     eax, ebp
        and     eax, 255
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
```

45 bytes and way smaller that Watcom's 88 bytes. Watcom lost 15 because of the branch alignment, and if it was not there, Watcom would havee 73.

The alignment takes place at the loop, which is good, but maybe excessive, which is what GCC decided.

The inner loop became this:
```
.L3:
        dec     ebx
        mov     eax, ecx
        mul     esi
        shr     edx, 3
        lea     eax, [edx+edx*4]
        add     eax, eax
        sub     ecx, eax
        add     ecx, 48
        mov     BYTE PTR [ebx], cl
        mov     ecx, edx
        cmp     ebx, edi
        jne     .L3
```

So far, GCC absolutely obliterated Watcom in size and the code generated looks fast enough.

I added some other options and it basically did nothing.

Another attempt, this time using ox instead of a bunch of other options.

I got results ranging from 58 bytes to 72.

I will try to play with the Watcom pragma aux and see how simple I can make it. So far though, GCC is winning by a lot.

## The Code was Wrong

I made a mistake with the assignment of the negative flag.

Still no changes. GCC completely wins at size.

The main issue is that Watcom is not doing a good job with optimizing the absolute value.

## Note

GCC has better inline assembly. It allows arbitrary registers to be used as inputs, although return values are a challenge by comparison.

As for the procedure I was testing, that can be written in 100% assembly that beats both, although GCC does a nice job.

For some reason, DJGPP generates very poor code with the current build I am using. I have a better cross compiler already in my PATH which I will use for the kernel.

For the userspace I have no choice but to use DJGPP because it supports the proper executable format.

The worse inline assembly options, especially when not using the intrinsics is a major issue.

Something like invlpg would require two operations, and an _asm block would require a spill to the stack.

Controlling the input and output registers is a bonus for Watcom, but not allocating them automatically is a problem.

GCC can also specify return registers by returning a variable in a static function that is assigned by an asm block.

## Other Note

I used this code for the testing. Maybe use for an itoa call.
// int convert(char *pb, int v)
// {
//     unsigned i;
// 		unsigned a = _abs(v);
//         unsigned N =
// 		N = v < 0;

// 		for (i = 0; i < 10; i++) {
// 			pb--;
// 			*pb = (a % 10U) + '0';
// 			a /= 10U;
// 		}
//         return N;
// }

## Decision

GCC wins. I still like Watcom, but I cannot use it for many reasons.

## String Library

Things like memcpy/memcmp/etc can easily be inlined. In fact, I don't think they should ever NOT be inlined. The price is minimal in most cases.

memcpy should be called only if the count is unknown, and the same goes for memcmp. The compiler knows how to inline memcmp when it does not use the sign of the return value for example (i.e. checks zero and thats it) or something like that.

I may need to do some experiments.

memchr can also be inlined using the scas instruction. strlen also can be, although the compiler usually refuses to.

Yes, GCC is really bad it seems. I tried the builtin and the library call, even on O3, and it does not inline.

This is what strlen actually is:
```
__attribute__((cdecl, regparm(0)))
__SIZE_TYPE__ strlen(const char *__s)
{
	__SIZE_TYPE__ s;
	__asm__ volatile (
		"mov %1,%%edi\n\t"
		"mov    $0xFFFFFFFF,%%ecx\n\t"
		"xor     %%eax,%%eax\n\t"
		"repnz   scasb\n\t"

		"leal    1(%%ecx),%0\n\t"
		"not     %0\n\t"
		:"=rm"(s)
		:"edi"(__s)
		:"memory","edi","ecx"
	);
	return s;
}
```

I could use regparm for the inline version, as it can recieve the argument directly in the register when it inlines.

But this is essentialy the code. 22 bytes total for stack conventions. The same code can be used for the inline header version and it is 15 bytes.

By comparison, calling is 5+5+2=12 bytes.

# March 19, 2025

## Quick Update To File Hierearchy

```
OS90\
    DRV
    EXE
        USER.EXE
        TERMINAL.EXE
        EDIT.EXE
    DLL\
        OS90.DLL
        LIBC.DLL
        LIBM.DLL
        ATM.DLL
    KERNEL.BIN
    OS90.COM
```

## User Environment

OS/90 will need some concept of a system call. Loading libraries and things like that have to be done this way.

I do not yet know what the system calls need to be and I want the interface to be minimal and complement DOS and DPMI.

The programs for userspace should run in an isolated environment. They will use the DOS interface for file IO and other things, but the context will be shared.

Other programs must run inside separate instances. Managing virtual machines will require a kernel interface. I can do something like NTDLL and make a library for it.

## Executing DOS Programs In Userspace

```
int CreateSpecialRedirHandle(int for, REDIR_RECV rdp, REDIR_SEND rdp);
```

Creates a handle that is used to redirect standard IO. A callback is used to send and recieve buffers.

The terminal program creates one of these for each DOS window. Without this, the DOS program will discard or cancel all IO to standard handles.

```
int ExecDos
(
    const char *path,
    const char *env,
    const char *cmdline,
    ...
);
```
This will exist in the kernel and userspace library.

To make the interface extensible and simple to use, it uses a list of arguments.

Concept:
```
int vm_id;

vm_id = ExecDos(
    "C:\COMMAND.COM",
    OSNULL,
    EXTENDED_PERCENT_MAX(100),
    CONVENTIONAL_KB(128),
    SWAP_PERCENT_MAX(20),
    MAX_XMS_HANDLES(32),
    SET_DPMI(1),
    STDIO_HANDLES(in, out, err, prn),
    STDIO_ENABLE_COOKED_IFNOMODESET,
    USERSPACE_RM_HOOKS(...),
    USERSPACE_PM_HOOKS(...),
    FRAMEBUFFER_SHARE_PTR(&fbptr),
    INITIAL_VIDEO_MODE(3),
    ALLOW_80x50_MODE(1),
    END_OF_EXEC_REQUEST
);
```
This will also use a sentinel warning attribute.

The parameters can be written to a file in binary. Maybe I should convert them to something else though. Perhaps use an array instead and cast values to integers internally.

## Executable Format Changes and Abandon DJGPP?

My DJGPP cross compiler generates truly inferior code compared to its corresponding Linux hosted version.

I may have to use the cross compiler that I have installed. This means the executable format must change.

## ESS Returns?

The relocations, symbols, and other things are all stored inside sections, so it is possible to use linker script hacks to make it all work.

I can give sections headers in the script, although it may be problematic for file alignment. The linker allows the insertion of data items. I can use this to put in the appropriate information per-section.

Sections are important to have because I can allow read only data and code.

## Freestanding disables memcpy inlining

The idea is to decouple the compiled code from any C library. This reduces the performance because memcpy or any other function is called with no optimization.

It appears that builtins do just fine.

Technically I can disable freestanding for userspace because there will be a standard library.

When using i386 tuning, GCC generates quite poor code for inline memcpy. When I use i486 tuning, it generates much more optimal code and uses rep movsd instead of rep movsb. It even unrolls part of the loop for tail copies.

I think I will use march=i386 and mtune=i486 by default.

Either way, I will be able to use builtin for the memory operations. They will work in user and kernel.

# March 20

## ESS In Detail

Some sections must be merged. They may have data with different alignments and things like that but aside from that for constant strings, rodata will merge all subsections.

Section headers will exist. The name of the section is omitted and the section is described numerically.

For executables, the format has one header and several section, and relocations.

Relocations are placed in rel.X and rela.X sections for each section that needs them.

Emitting the relocations like any other section is difficult because it is a special section. They must also match each section because they do not have a section marker.

The first stage is to flatten the entire file into one section. Alignments are applied at this stage into the binary data. The relocations are automatically merged.

A new section with the header is created that contains the offsets and sizes of the sections, because they still exist.

The types of sections are limited to:
- text
- data
- rodata
- bss

For executables, there are no symbols but there ARE relocations. They are required for loading programs into any location.

The symbol table of ELF is like other executable formats. The entries have section markers and reference the string table.

Wait, why no just emit the relocations as they are? Merging sections is problematic. The relocations must be updated the match the changes.

So a two-pass linkage system is needed. Headers can be added last.

## ATM Ideas

Clip rectangle can be stored per window. It is rare that it needs to change that often. Usually we clip to the bounds of the window anyway.

int W90_SetClip(HAWIN w, RECT* r);
int W90_ClrClip(HAWIN w, RECT* r);

>>> Capsule control idea for VF

# March 21

## Kernel

Development on the kernel was slowed down by the fact that I did not yet implement the string library and printf.

## printf as a library?

I can make printf a static library. The only problem is that it expects certain definitions.

printf is a shared module that depends on another. I can safely include the string library.

Or just include the C code?

## typeof on a function type

GCC allows __typeof__ to be done on a function, and this can be used for a declaration.

This means I can do the API declaration more easily.

Instead of the full macro, I can use an export instead.

```
void API MyFunction()
{}

EXP(MyFunction)
```

This can additionally use attributes already applied to the function using the copy attribute.

```
#define EXP(NAME) typedef NAME##_type __attribute__((copy(NAME)));
```

Drivers should be able to call the function like in C.

Unfortunately, this is not capable of that.

I did some testing and apparently it is possible to declare function with a function typedef. Calling the "variable" calls the function by name.

## Memory Management

The idea of using a tree makes sense. The worst case can be be something logarithmic rather than linear. This makes it much more feasible for large memory systems.

The problem is that the physical memory is not represented by contiguous chunks and should not be.

The point is to somehow find out which regions are free quickly.

# March 22

The idea of using trees is based on NOT keeping track of what is actually in use, necessarily, but sectioning the memory off.

If I have just one zone for simplicity, the first allocation would split the block representing the whole zone into two chunks, but not all of it has to be allocated of course.

# March 24

## Linker Options

Currently the kernel does not execute properly. It starts with nonsense code and does not seem to put the entry point first.

Anyway, I added data and function sections to the command line options and I managed to reduce the kernel binary size by 448. Because of page alignment, this is a mostly useless improvement that probably makes the kernel slower too due to cache locality.

The 386 cache controller chip uses 4-byte cache lines so it is already bad for sequential and adjacent access, so this option does no harm. It also has barely any benefit, so I will avoid it.

## Getting the Kernel To Work

Whatever OS90.COM has is correct enough to boot the kernel. The kernel image is broken somehow because the linker does not put the correct instructions at the start.

## Note

LD has an overlay feature. This allows a certain section to share address spaces when they normally would not be allowed to.

This allows support for dual-segmented address spaces. More importantly, this allows for overlay support!

There are things that can be overlaid. Code that does initialization, which includes ALL init function calls, can be deleted after use?

Not quite. Overlays are used and discarded. Plus init functions are not that big anyway.

## Interesting Results

I just added /DISCARD/ for all sections and the code size was dramatically decreased, just over 4K in size now.

I am not sure how safe this is but I did do it at one point.

## Actually

Things are totally generated correctly. Even the XCHG BX,BX at the start is there.

## Breakpoints Do Not Work

I am not sure why. Maybe I have to build bochs myself. I would rather not.

But anyway, breakpoints are not working and I tested it myself with a DOS program assembled with NASM.

It is possible that an update ruined everything. Anyway, I will fix this.

## Local binary installation location?

If I add a single variable to my path that points to a local directory in my home, I can install all the software I need locally without having to install it directly.

I am saying this because I will be building bochs to get around this current issue.

Usually it is ~/opt/ or something like that. It can be anything really.

I have a software directory already so I will use that.

I will take a break and work on something else for now.

# March 28

## Bochs Working?

I build bochs on macports with the debugger. Should be working now.

I apparently have to add the macports binary paths to fish though.

## Working

A bit buggy but works.

## About malloc

The look up table idea was good. Performing the bit scan with the table improves performance, but can I use it for data larger than a byte?

I can simply perform the lookup on each byte, with proper condition checks of course. This makes the code much simpler.

# March 29

## Problem Found?

I think there is a problem with the linker script. The BSS section does not appear to be zeroed correctly, and that is why all the IDT and GDT entries are invalid.

## Fixed

The issue was improper handling of cold and hot functions which init uses.

## Interrupts Not Working

> It may be the gc sections flags btw

It appears that interrupts are not working. They did previously. Only the keyboard and the timer could cause an interrupt.

I put a breakpoint on the timer interrupt so it cannot be the problem.

I will check for IDT correctness.

It looks like my assembler code is not actually saving the changes. Makefile may be malformed.

## Timer IRQ

I am receiving the timer IRQ as expected, but the code obviously does not work. I will have to restructure the handling there.

I was previously able to catch it and let the bios handle the IRQ, and for the keyboard.

If I recall correctly, I was also able to call services that used keyboard input.

# March 30

## malloc

I want to completely avoid branching in the initial decision. Doing chained if statements is far too slow and adds more overhead than I want.

Right now I have a consistent 16-byte header and divisions of 4080 are the basis of the arena chunk sizes.

If I artificially extend it to 32 bytes, I can get 127-byte entries, which obviously cannot be done due to alignment.

If I use 254-byte chunks under this system I only have 4-byte alignment.

Generally this is NOT a good solution.

I would prefer to force powers of two even if I lose memory from rounding. Alignment of about 16 also matters a lot too.

Maybe there is a misunderstanding. Powers of two CAN be used. With a 16-byte header I can use 64-byte chunks and have 63 entries, which fits in the header. 128 bytes allows 31. 256 permits 15. 512 to 7, and 1024 byte chunks permit 3.

Making it branchless is difficult. The input is the raw size in bytes.

We will first check if page granular allocation is needed and do that. Then a different method may be used.

The smallest size is 128 bytes per allocation with the sentinel word. In practice, this means 124 bytes is the maximum size.

If the bytes are added by 4 and shifted by 7, the range of options is reduced to 5, but there needs to be a number of useless table entries.

I think chaining if statements and putting the more likely ones is better.

# April 2

## Directory Entry Caching

Reading about Linux dcache inspired me to consider directory caching more.

Apparently there are more FAT file names in the 13 character maximum than there are 32-bit numbers. This means hashing the file names cannot be done with a 32-bit number, even if only alphanumeric codes are used.

This is fine, as long as collisions are properly detected.

dcache is a good idea, but handling the complexity of the directory tree may not be extremely important. In the DOS days directory levels were not that deep and were restricted

Reference counting and moving a cache entry up can be used to avoid collisions, as they will probably have the same count, right?

Also paths are limited in DOS to 64 bytes minus one for a null terminator. The CWD system call does not allow any more, period.

This is MOSTLY a hard restriction, but there are methods such as renaming a directory into another drive, which is done at the shell level, or so I think.

Well I am not even sure if that would work either. And DOS has no concept of symlinks either.

## Windows 95 Extensions

I should use RBIL instead to get the up-to-date information.

The SUBST command does have a call for that purpose. And the real path limit is system-defined and can be anything, although it is usually in the ballpark of 260 bytes.

SUBST is implemented in Windows 95 using a special call. It in theory should be able to overcome the path length limitation.

## More On SUBST

SUBST and JOIN are actually not built into DOS whatsoever. They are implemented by hooking the DOS interface.

Of course aside from the Windows 95 implementation. They should work per process, but virtual drives cannot be shared between processes.

Oh and if SUBST or JOIN are used before loading OS/90, they will probably not work if a 32-bit IO driver is loaded.

Unless I implement the Windows 95 interface which I may not have to.

## Filesystem Support In General

I will write the filesystem driver using DJGPP and I will run it under DOSBox or something. For performance testing, I will run it on 86Box for accurate performance testing using RDTSC.

The problem with implementing FAT support in DJGPP is the default file caching that makes writing dangerous unless I hook the filesystem. I can also use HDPMI to get the extended interface, although the INT 13H extensions do not support LBA unfortunately. This makes it impossible to handle drives larger than 8 GB without doing it manually.

Or instead, I can embed the filesystem in a file and skip all that stuff. I could even test this under DOS with no problems, but performance would not be that accurate.

I mean the default caching cant be a major problem if you just dont use stdio on files. And read support will obviously come first.

## SUBST Again

This may read the drive parameter table, though I doubt it changes it. Wonder if it will work at all or if it needs replacement.

# April 7

## Random Note On Disk IO

On Windows 95, a program must use an IOCTL command to lock a device before performing any INT 13H operations.

This is a good idea since the interface is already defined and used by some programs, and allows the disk to be used after oeprations are done.

FDISK is an example.

Also, the IOCTL command requires cache flushing before doing something like formatting the disk, which requires the data to go to the disk immediately.

## Userspace Interrupts And Win386

Win386 originally had a strange way of handling interrupts. It would send them to ring-3 by default. It took some time before an API was added to send them to a ring-0 driver using bimodal interrupts.

This was necessary because a driver was not always present to allow a device to be accessed by a DOS program.

# May 3

## I Return!

Got bored of the other project, but I jotted a few ideas down that may be applicable here too.

OS/90 is a project that has a finished goal in mind, but there are components that are not strictly required, such as the 32-bit filesystem and disk access.

ATA disk access can be done with very high certainty but floppy disks are extremely unreliable and parts of the interface cannot be emulated.

The core of the kernel is sort of a one-shot thing, but there are steps I can take to change that.

I need an IRQ#0 handler. I need to be able to create processes.

## Sitrep

I have made genuine progress, but the core of the project remains incomplete. I have written optimized string operations (most of them) and printf, interrupt reflection code that works enough to recieve keystrokes, and the SV86 code can actually enter it and make calls.

## Scheduler

I will do round robin scheduling first, but there is one thing. I need to properly dispatch the whole switch table. The idea is interesting, and may have costs and benefits. Inherently it accesses memory less, but it also makes memory more sparse because of how it works, and it also enlarges the kernel binary when alignment is factored.

It is kind of important to make task switching fast. It happens every 1 MS, which is very frequent for an old CPU. That means 1000 times.

I will use what I have, but make the necessary changes so that it works. Debugging it will not be too hard, I hope.

## SV86 For the User

For DPMI to work we allow the program to change vectors. This interface is NOT meant for general purposes.

Some calls to SV86 are initiated by userspace. There are some considerations:
- How does a driver know if the call came from r3? If it did, there may be security implications and it may be an illegal operation.
- How does the stack work?

SV86 can ALWAYS end up causing an actual mode switch which requires a stack. For ring-3, we already have a stack.

For ring-0, one is still needed. But the problem is that SV86 may then not be available without the scheduler.

It is possible to implicitly allocate space per process for the real mode stack. That is fine.

So most likely, the scheduler is the very first subsystem that must be ready.

A single process is created so things work during initialization.

## Why no process list

Using a process list actually removes any need for memory allocation, plus it also negates any wasted space from process control blocks. Not to mention, it completely removes stack limitations.

The stack part is another story.

But anyway, there is no longer a "linked list" anymore.

The process list needs some good theory behind it though.

## Process List In Detail

I should map it to a certain location in the address space.

Also, how many entries will it need? I cannot think of it needing more than 128 TBH.

Anyway, this changes a lot of things. Getting the current PID is done by accessing a variable and cannot be inlined for drivers. Unless if course I place the value somewhere global.

Also to compensate for the performance loss, I can use FS or GS to be the sort of global `this` pointer that functions can reference if they operate on the current process.

That means scheduler functions need a "this" variant for several of the calls.

## No

For the last time, I am NOT doing that.

The first task will be allocated in the kernel memory, and it does not matter because it can always be reused.

# May 5

## Scheduler

I make the guarantee that the first task does not need the memory manager, or I create a function that initializes a process inside a page.

Also to prevent constant reallocation, we should provide the option to reuse an exist task block. This could be good for thread pools too.

This is actually 100% what I should do. Allocation should never be required.

In fact, for modularity, I might as well exclude allocation from the entire API. It will very rarely be used anyway, since another interface will handle actual process creation.

Adding a process is actually so simple at its basic level that I could even inline the whole thing. It is just a circular linked list where every insertion is basically instant.

## Task Hooks

These are vestigial remnants of an older design. It was intended to implement per-process conventional memory. This is something to genuinely consider, since it increases the amount of memory available to DOS programs.

A posthook makes little sense. I added it because I was writing a Payday 2 mod and the API had posthooks and prehooks.

The first 4M can be remapped with a simple write to the page directory. The problem is that I do not want to allocate page tables for each process because I find it wasteful. This brings overhead to 12K per process.

4 DOS programs could run and 36K would be used.

But that may be the necessary cost.

# May 9

## Concept of a Task

Because I will have a working malloc in the kernel, I can save a lot of space inside whatever is used to represent the DOS program and maybe merge it with the task block. This would make the system way faster and possibly allow separate conventional memory to be viable.

I will probably not end up doing that though.

Actually this 100% cannot be done. I need room for the stack. The IDT requires a huge amount of memory.

As for conventional memory, this is a decision that needs to be made early on because everything needs to account for it.

Honestly I am fine with reducing the amount of available memory if it makes the OS faster. Using 32-bit drivers combined with some memory reclaimation tactics should provide plenty of space.

I have considered allocating a region of memory for multitasking purposes though. It is referred to as the DMR or DOS multitasking region.

The DMR idea was that we bank switch a region of the address space, prefereably at the end of the conventional memory, and programs that don't fit go outside.

But if they DO fit, such programs have their main segment reside in the DMR and not waste the memory of others by simply executing.

The trick to making it work involves coaxing the memory allocator of DOS into allocating the very end of the DOS memory.

The DMR size is a fixed constant that is either built-in or decided at boot time.

I think a size of 180K is good. The only issue is having to copy 45 page table entries. In fact, this is a major problem.

Programs that are not in real mode can still access data in the DMR at will ,so it always needs to be switched.

OR, I can use a lazy approach and have the memory manager bank switch ONLY when the memory is accessed. It can do the whole thing because there is little benefit to smaller granularity.

I am concerned that some programs are WAY to big to fit in non-conformant memory. The DMR size cannot possibly be changed at run time.

However, such programs should know how to use XMS memory anyway.

The problem is that if programs theoretically need to share data, it will definetely fail because the pointers are invalidated.

However, this is very unlikely because DOS programs don't communicate with each other anyway. It is a single-tasking OS after all.

Subprocesses may share memory though. A subprocess is currently a full program.

To make compatibility, a program can simply be executed in non-conformant mode explicitly. Conformancy can be inherited then.

The benefit to this is that I can run as many DOS programs as I want.

Also, not all of the DMR must be used. It is just a region, not actual memory.

# May 12, 2025

## Object System

I find GObject and COM interesting. I think OS/90 should have some kind of global object system to represent devices, UI controls, etc.

Maybe it can enable high extensibility and flexibility.

The idea is that files and folders can all be objects with a hierarchy.

Files can be Readable and Writable. Directories can be EnumerableToStringList.

Also interfaces could even by dynamically attachable. A file can be extended at runtime to automatically store metadata. The cosntructor can be overriden to track filesystem access, etc.

Directories could also implement a sortable list interface, or something like that.

I should probably write this in userspace first. It could have applications outside of OS/90 if it is good enough. If it is space efficient, I could apply this to embedded systems that benefit from code reuse.

## Returning to OS/90?

I am getting really bored of the other project again. Time to go back to OS/90.

## Does the kernel compile?

It does, actually.

## Change in Tabs

It appears that the tab size is changed now. I set it to 4 when I was writing printf because it was too large to read some of the code and fit in within 80 characters per line.

## Kernel

It does nothing right now. The IRQ#0 handler is currently incorrect.

I still wonder if the current approach to task switching is a good idea.

# May 13, 2025

## Kernel Task Switching

I will keep what I have actually. It does actually work, and is extensible enough to allow proper task scheduling later on.

I just need to fix the broken entry mechanism.

## Error Codes

Errors need to be conveyed by functions. I will just use `int` and make anything below 0 an error.

# May 14, 2025

## Memory Allocation

I can use a tree structure for allocation. I have mentioned this before.

Allocations can be kept track of by having the nodes point to each other in a singly-linked list.

Nodes no longer need to represent one single allocation unit, but instead a variable number of pages that can be contiguous and not require linked-list traversal.

This may increase the amount of memory use in general because storing the structures takes more space.

### The Idea

A straight range of pages, turned to an even number as is allowed, is represented by one single tree structure. They are sort of sbrk'ed by the kernel into its address space like the current design.

Allocation is based on dividing the address space by 2 and creating new nodes (or they can be pre-allocated) until creating a list of nodes to bridge with each other to form the allocation.

So if we allocate 24K in a 1MB address space we split the size until we get 16384 which preferably should get one node (solution later) and then we take that away from 24K and get 8K to be allocated, and we try to find a free node somehow that has 8K or at least 2 4K.

Needs more refinement, but I think it could work.

# May 15

## Binary Tree Allocation in Detail

I need to think a bit more before writing the white paper.

The tree is much larger than any array at its highest possible granularity. When we have 15M, the number of pages available is not divisible by 2 to the point of reaching one page, which is fine, but rounding will mean 8K allocations are the minimum.

Also, it takes like 11 levels to reach this granularity. Representing each level is extremely expensive and pre-allocation pay not really be an option.

Also the tree might need to be sorted so that we can find the smallest possible blocks.

The idea is that allocation searches the tree for a physical range the satisfies it and adds it to the list of blocks to count as part of the allocation. Past that point, the allocation is a linked list with each node having a single link.

Before that can be done, the number of pages to allocate must be allocated by finding a chunk on the tree that satisfies the whole request of just part of it with enough for another extra chunk.

This is not really a sorted tree. It represents physical ranges.

The benefit is faster reuse of allocated regions with some wasted memory.

Keep in mind that the tree is not balanced and I have not yet considered how coallescing should work.

// For malloc, why not just allocate an uncommitted heap at program startup like the Java VM and instantiate malloc for the whole thing? That way, we don't have to allocate anything.

// There needs to be a way of trimming the heap though.

## malloc with a fixed-size heap?

OS/90 must have uncommitted memory. This is aside from the actual kernel page frame allocator.

Instead of getting pages, I can allocate a heap and just use that. If it runs out, the program fails.

So long as UCM is implemented efficiently, there are basically no problems with this. Java does this exact thing with the heap size, and most of the allocated memory is not automatically used because the OS is smart enough.

Programs can even allocate as much memory as they want to since they will not use all of it at once. Or if I do not want to make it universal, I don't have to.

A brkoid memory region can be implemented with the basic template of:
- Header page table entr(y/ies) with basic information
- Hard break entry to indicate end

The allocation also can be fixed in size permanently if I want it to, although DOS of course requires this to not be the case.

DPMI allocation however is slow and most programs allocate one block based on available memory and use their own malloc. Some DPMI clients have severe limitations on how many allocations are allowed.

The issue is that virtual address space is limited and I am not sure how it is calculated either. One megabyte for example cannot just be handed out to any program. One page table has 4M to represent.

Page table are currently just laid out flat and cannot be extended any further.

Allocating brkoid heaps is only for userspace C/ASM programs that use the native interface, which is obviously DOS but extended. They will likely not need much, but malloc must have a heap available.

Anyway, this simplifies malloc a lot. The heap is thread local, and it does not need to be reallocated and is not sparse either. The kernel handles allocation automatically upon access.

Also, I would like to experiment with different data structures for malloc too.

Previously I had the idea of using pool-style allocation for different sizes.

# May 16, 2025

I have the driving test tomorrow I think, so I will have to study the road signs and some other things.

It seems I am finding every excuse not to work on the kernel directly.

So long as I make progress, it is all fine. But the scheduler really needs to get done.

We are KEEPING the current structure and correcting any possible errors.

## Object System

An idea for an interface is `Consumer` or an object which can "eat" other objects and destroy them but recieve their data and efficiently store it internally as a child object.

Or instead override the child-related methods to hide the details if ordering is needed.

# May 29

## Idea for MM

To improve the average MM allocation performance, I can maintain an ordered list of free regions of memory.

The reason for ordering is because I can keep an average and if an allocation is bigger than the average it is better to iterate from the top rather than the bottom.

Ordering means we have to use a proper std::set style structure, and it should be iterable.

> Can averages be used for tree structures?

# June 1

## Possible Problem With ctype

Signed chars are a bit of an issue. ctype functions take integers and the value can either be a promoted signed char (sign extended) or an unsigned char.

This means a NEGATIVE index is possible.

So far I have faced no issues, but this is important if I ever want to actually use the full extended ascii range.

7-bit ascii really is as bad as Terry Davis said because it can be signed and convey the same characters either way.

Basically, a few changes must be made to the lookup table and indices to it.

128 more entries to handle -128 for signed char, which is just a reverse of the first 0,128.

This 100% must be done for standards compliance and in case I break any programs like this.

Also, indices will have to be offsetted by 128 after adding the new entries. 768 bytes is a lot to use up but in needs to be done.

## printf problems

I did some reviewing and found that my printf has a LOT of problems. Marked everything with "FLAG:"

## TASK

I HAVE to fix printf and make it more standards-compliant. Also, there should be a proper testing system in place.

A few things I need to accomplish:
- Unify argument-fetching totally.
    - Because non-integer conversions do not apply to float, this can be done
- Take absolute value of a signed char before using it as an index of any kind.
    - Only if char is signed, which it probably is.
- Use signed integer arguments and avoid pointless casting
    - int is known to be at least 16-bit on compliant implementations.
    - For most operations int will always be good enough
- Replace some size_t's with int or short if there is no risk of overflow anyway.

Note: there are actually many platforms with unsigned chars. It is good actually, but sysv does not do this.

This will be tough but has to be done.

### Revamped Argument Fetch

We will output to a union and return the number of characters, preferably as an int.

```
//This is the naming style I want to do more of, since it describes the return value.

union xintmax {
    uintmax_t u;
    intmax_t  i;
};

int b2represent_from_fetch_iarg(union xintmax *oval);
```

We cast whatever into uintmax_t and perform a unified conversion.

It is admittedly not good for performance. GCC seems to optimize it well if we use 64-bit though, but it is a lot of instructions. On i386, we get 64-bit conversion speed for all integers.

Eh, we can profile it later. I know how to do that.

If I want to have some quick performance hack, I can do it in assembly and sneak it in instead of whatever I have now with repeating code.

Also, this returns the minimum characters that must be in the buffer.

My code uses buffer commit and char commit, by the way. I do not need to even deal with buffers.

## Report on printf

I added a lot more flags and I am simplifying the conversion code. It should have roughly the same speed as before without any extra condition checks.

Will take a while but I am making progress.

# June 3

## Notes:

- ctype may still have problems
- still working on printf

## Ideas

Instead of macros, I can use inline functions instead! Or const variables.

But const variables are only folded if their addresses are not taken, so there is a potential risk.

I did read the JSF++ rules and macros for constants are prohibited. I should probably do something like that here.

# June 4

See previous entry.

## static const instead of macros

A modern enough compiler will inline a const variable. In fact, GCC will perform constant folding even with optimization off because the standard actually mandates it to do so.

It is quite critical for a compiler to be able to for example, eliminate an if statement because it is known to be true, and it is a flaw if it generates some totally redundant check.

Anyway, static const is the way to go. Even if I was theoretically using a very old compiler it would not matter because the cost is still minimal.

## printf test suite

I can write the test suite in such a way that compares the characters as they are printed.

Or I can get snprintf to work and do memory compares. Buffer handling should be tested too.

# June 5

## The next things to do

I think I will take a break and work on the other project.

Actually, I am back. I got inspired to work on printf again.

# June 7

## Codepage 437 and printf

I need 437 support. This could mean I have to use unsigned chars.

Or not. At the end of the day, char is just 8 bits.

So negatives are fine because it is treated later as an unsigned value (it has to be). Even printf converts chars to an unsigned value.

CP437 however is meant for drawing more than actual console IO. snprintf and sprintf are used to generate text perhaps before outputting it, and it requires no additional processing for that.

So char stays signed. It really does not matter that much, and I do not want to override the sysv abi.

## Make printf unportable? (No)

printf so far has been a portable implementation and I like the work I put into making this work. But the applications are quite limited and there are at least 100 other printf implementations for different platforms that all do their jobs just fine.

So there really is no point in making it portable. It is more of a proof of concept. If it can run natively using generic code, it can run anywhere.

However, I know the size of everything in the sysv ABI and printf can be rewritten to just assume how large things and how many digits numbers have.

But like I said, it is a PoC. If the same code runs under multiple platforms, it shows that it is correct to an especially high degree.

Plus plently of people in the OSDev community can use it and have it up and running in a few minutes.

In the end, the performance lost is minimal or non-existent with the right optimization.

## Macro Problems

I am getting a lot of warnings and errors because of macros, so I am looking for a way to reduce all of that, maybe with a set of easily foldable pure functions.

I can just use a lookup table. It will not even result in any extra code.

Consider:
```
int digits = digits_to_represent(UINT_MAX);
```

The implementation can be like this:
```
__attribute__((const))
static
int digits_to_represent(uintmax_t v)
{
    int rt[] = {
        []
    }
}
```

No, I was thinking of a table using the size of the value.

__attribute__((const))
static
int digits_to_represent_sigint(size_t size)
{
    const int rt[] = {
        [2] = 5,
        [4] = 10,
        [8] = 19,
        [16]= 39,
        [32]= 76
    }
}

ChatGPT gave me this:
// Decimal digit count for maximum unsigned integer values by byte size
const unsigned char unsigned_decimal_digits[17] = {
    [1]  = 3,   // 255
    [2]  = 5,   // 65535
    [3]  = 8,   // 16777215
    [4]  = 10,  // 4294967295
    [5]  = 12,  // 1099511627775
    [6]  = 15,  // 281474976710655
    [7]  = 17,  // 72057594037927935
    [8]  = 20,  // 18446744073709551615
    [9]  = 23,  // 4722366482869645213695
    [10] = 25,  // 1208925819614629174706175
    [11] = 28,  // 309485009821345068724781055
    [12] = 30,  // 79228162514264337593543950335
    [13] = 33,  // 20282409603651670423947251286015
    [14] = 35,  // 5192296858534827628530496329220095
    [15] = 38,  // 1329227995784915872903807060280344575
    [16] = 39,  // 340282366920938463463374607431768211455
};

// Decimal digit count for maximum signed integer values by byte size (excluding '-' sign)
const unsigned char signed_decimal_digits[17] = {
    [1]  = 3,   // 127
    [2]  = 5,   // 32767
    [3]  = 7,   // 8388607
    [4]  = 10,  // 2147483647
    [5]  = 12,  // 549755813887
    [6]  = 15,  // 140737488355327
    [7]  = 17,  // 36028797018963967
    [8]  = 19,  // 9223372036854775807
    [9]  = 22,  // 2361183241434822606847
    [10] = 25,  // 604462909807314587353087
    [11] = 27,  // 154742504910672534362390527
    [12] = 30,  // 39614081257132168796771975167
    [13] = 32,  // 10141204801825835211973625643007
    [14] = 35,  // 2596148429267413814265248164610047
    [15] = 37,  // 664613997892457936451903530140172287
    [16] = 39,  // 170141183460469231731687303715884105727
};

Should do the trick. But I should also consider the signedness and the other representations.

Well actually those are based on bits which makes it way easier. They can be separate functions.

Conversions now can be separated such that we have one method of getting the value and another method of finding the right conversion without any extra data.

All this does actually is simplify the problem of how many characters represent the largest of a certain type. That is all.

We still need to handle the conversion.

But yeah, we will do this.

The function can be `int digits_to_repr(uintmax_t value)` and I actually do need the signs, unless I make it so one being zero uses the other, which is non-obvious.

The code can be condensed to keep the meaning clear.

## TODO for printf

Add the new implementation for the number of figures.

Also, if I REALLY need it to be a compile-time constant I can actually index the array with no problems, since the compiler knows what is inside it.

Currently, XFIGS and all the other ones are used in the ndc etc stuff, and I do not see a way to avoid all that yet.

Getting arguments from the stack currently uses insane macros which obscure the meaning.

# June 8

## printf

I have that function which returns a struct with all the information.

## Notes on character promotion

Positional arguments do not do this.

Promotion means the value of the character is made to fit an int. The sign of char actually does not matter and MUST not matter because it can be anything.

Sign extending simply makes no sense because we do not actually know if char is signed.

Again, it is just made to fit an int, so it is basically zero extended.

Codepage 437 works fine.

## What to change

The part responsible for getting the value is acting on the format flag, and so is the part that returns the information. That is why there is one function.

I will only change what currently exists. The point is to remove the macros.

Also, XFIG was a very bad idea that existed only for the purpose of consistency. It is totally unnecessary and the calculation can be done with the type alone. It will be henceforth removed.

OFIG is more complicated because it requires a cieling and a division.

It can of course also be made into an array.

By the way, XFIG is literally just sizeof(T)*2 assuming 8-bit bytes. For other sizes: `sizeof(T)*(CHAR_BIT/4)`

Also, OFIG using only the type is done with `(sizeof(T)*CHAR_BIT+2)/3`

## Reading the C standard

The standard is probably a better reference. It says in 7.19.6.1.9 that if the format flags are malformed, the behavior is undefined.

Also, GNU C apparently cannot handle something like:
```
"%10#x"
```

The standard however says flags can occur in any order, so this is valid in the C standard.

Does this mean GCC is not compliant?

I tried it in Turbo C and it did not work.

But clang gives a warning yet it gives the same output as "%#10x"

Although it may not matter much, I should aim to be more compliant.

It is interesting to see that libstdc is more compliant, despite the false warning. I should actually report this as a bug for clang because of the false positive, and maybe report it to glibc for the non-compliant behavior.

Even something like this could in theory work:
```
10l#x
```

Under clang, this does work.

I tried this under clang:
```
20l#lx
```

This actually works. I suppose 'l' check if it was already specified and using long long if needed.

That would be very extreme and it actually is more than what the standard requires. It describes ll as being a flag on its own.

So technically what I have is partially incorrect. It does not handle any ordinary number correctly if it occurs after another flag which does not require its own number.

I will move the code to the default case. It might not matter much though.

## Different way of handling type flags?

I can make the flags work by incrementing a number.

Consider this:
```
hh = -2
h  = -1
.  = 0
l  = 1
ll = 1
```
L can be made to have the same meaning, since C99 accepts it for float. I suppose it was a common source of mistakes.

This means I can have a compliant implementation fo the flags system.

Not really necessary. But it avoids the additional condition check.

GCC optimizes negative switch cases in a table. It mostly just offsets the number based on the most negative in series.

This eliminates one extra branch, leaving only the loop. It might be faster.

Also, the enum values can be optimized to make the lookup tables smaller. We do not actually need to encode any entries for size_t or ptrdiff_t because those are just aliases for some other type.

Actually, this is not a great idea, aside from aliasing some of the indices. Incrementing does NOT save any branches. We check for another l or h and skip an iteration, so it balances.

The switch/case table can actually be made smaller in theory, if the compiler decides to emit a condition check.

The number characters are handled in the default case and the asterick (soon the hash) are beneath the alphabetical characters.

I think I should do aliasing but keep the switch labels. Let the compiler handle that. It knows how to reduce the ranges and make an optimal table.

This also means I have to REMOVE the l_z, l_L and other things. They no longer mean anything.

It appears there is no assume in GCC. It can be simulated with this:
```
#define __assume(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)
```

This does work too. This reduces the code size by omitting extra checks for conditions that are never true (such as integers for sizes being zero).

# June 9, 2025

I am not doing any of that btw.

## Forward declared arguments in GCC

It is possible to do this:

```
int A(int c; int array[c], int c)
```

It is also okay to specify the size of the array.


## printf code generation

It appears that the code generated from the redesigned printf is EXTREMELY bloated. The condition checking and other things is not a major problem.

The switch/case for getting the arguments is HORRIBLE and compiles to up to 8 instructions!

This is massively inefficient for IA-32.

Things like variadic arguments can be simplified in the sysv ABI.

In the end, it is just a pointer. It can point to 32-bit elements or 64-bit ones represented as pairs on the stack. Although very unsafe, it may be faster to do it that way, although most code in practice will not do such a thing.

I will test that first.

Also, consider that intmax_t is the same as long long, so there is no point in all that extra excess code. If I make all the assumptions, the code can be simpler and faster for what I intend it for.

I will test the va_args idea.

Okay, I tried it and this seems to work:
```
#include <stdio.h>

__attribute__((noinline, cdecl, regparm(0)))
int add_ints(int n, ...)
{
    int *v = &n + 1;
    int sum = 0;

    // printf("n = %i", n);

    for (int i = 0; i < n; i++) {
        printf("Adding %x\n", *v);
        sum += *v;
        v++;
    }
    return sum;
}

int main()
{
    printf("%i", add_ints(4, 5,5, 2,1));
}
```

Not sure how, but I'll take it.

Also, since portability is no longer a concern, I can apply proper optimizations, which may include inline assembly.

It will still be in C. I do not want to worry about inlining and simplifying expressions that way.

printf2 will be the new one and it will be a rewrite with some shared code. I will test it in DJGPP.

I am NOT deleting the old one, only switching to a much more efficient version. That is all.

## va_args solution

All I need is a `char*` that is incremented as needed. `char*` is an exception to pointer aliasing.

## Rewrite

Consuming arguments from the stack is simple enough. It is just an inline function that can take the printf context, or it can take a double pointer.

The compiler will probably do a better job with direct access to the member. It does not need to write results immediately.

# June 10

## Changes to be made

The most notable change is fetching the argument using memcpy, which is correct in C99 and overrides pointer aliasing problems.

Also, because of how endianness works, unions are a viable way to do this.

A union like this:
```
union {
    long long i64;
    unsigned long long u64;
    unsigned u32;
    int i32;
    short i16;
    unsigned short u16;
    unsigned char u8;
    signed char i8;
};
```

It is possible to copy into the union using memcpy too, as long as the amount is correct.

Conversions will either operate on 32-bit values of 64-bit ones. I will also still be doing the backward conversion because it is theoretically faster.

I have this for integer conversions with backward generation:
```
asm_v:
    push %edi

    movl 12(%esp),%ecx  ; ECX = iterations
    movl 8(%esp),%edx   ; EDI = buffer, we need it for later
    movl 4(%esp),%eax   ; EAX = value

    ; If the number is under 65535, reduce iterations to 5 for ~-200 clocks
    cmpl $0xFFFF,%eax
    jae 0f
    shl $1,%ecx

    .align  16
0:
    subl $1,%edi        ; Decrement down
    subl $1,%ecx        ; Decrement loop counter
    xorl %edx,%edx      ; Clear EDX for division
    divl %ecx           ; EDX now equal to the digit
    add $'0',%dl        ; Convert to character
    movb %dl,(%edi)     ; Copy to the buffer
    jecxz 1f            ; If zero, leave
    jmp 0b
    .align 16
1:

    mov $'0',%eax
    repe scasb
    mov %edi,%eax

    subl %edi,8(%esp)
    mov %edi,%eax

    pop %edi
    ret
```

The issue is with things like padding, which I think are already handled fine in existing code.

What I have basically does the exact same thing. Also, the above code is NOT working. I have to change that. It should use extended assembly.

## Fixed

The 32-bit conversion code now works.

I need to think about how arguments are fetched.

# June 11

## Progress on printf

```
__attribute__((cdecl, regparm(0), noinline))
int sum(int c, ...)
{
    int r = 0;
    int *v = &c+1;
    for (int i = 0; i < c; i++)
    {
        int param = 0;
        memcpy(&param, v, sizeof(int));
        r += param;
        v++;
    }
    return r;
}

int main()
{
    printf("%i\n", sum(4, 12,32,10,9));
}
```

This works as expected.

So progress is good so far.

BTW 64-bit values are passed by pushing two values. But in what order?

I checked the push order and the data is stored in little endian 64-bit format SIMD can be available on 32-bit processors and 64-bit arithmetic requires the correct format, so the ABI does this.

So yes, 64-bit values are fetched as expected.

## Argument Fetching based on length mods

First of all, every argument goes to the stack, especially for variadics which are integral types. long double also is pushed using three writes to the stack.

This means all accesses to arguments are either 32-bit, 64-bit, or 80-bit.

%lf is a valid long double conversion in C99, so it needs to be taken into account, although the code path for %Lf can be simplified such that it does not need to perform any additional checks to read 80 bits.

The code is simplified to the point that arguments can be fetched as we decode flags, which makes everything easier.

The arguments will probably go in a union. We then act on the value that matches the size conversion.

Technically this is not correct. %n takes only a pointer to a different integer type. This is probably so that the number of characters outputted can be obtained without relying on int.

So no, we cannot just access the stack based on a length mod.

But before dispatching to a converter, we should read the value. This makes everything more orthogonal and not dependent on a "fetch" function which we would have to call or inline.

The length mods can be converted to a size of bytes.

Also, length mods do not always follow the type, although GNU printf certainly requires this.

I intend to follow the standards though.

## %lf is actualy double

According to the standard, 'l' has no effect of %f. So l can be surmized to represent a 32-bit value because long is 32-bit.

Incidentally, a wide character in SysV/UNIX is actually 32-bit, so it really works, although I have no plans to support wide characters.

I probably can though, with not much difficulty since they are 32-bit, but it would be of very little use. djgpp has no such functionality.

So with the exception of the %n flag, all flags represent bits pulled from the stack. L always represents 80-bit because it is only for float.

ptrdiff_t, size_t, and the theoretical ssize_t are all 32-bit too.

Altogether, we can easily get values from the stack in exact byte quantities with no problems with only the exception of the %n modifier.

## Decode process

Specifying a length mod abrogates the default size. This means if we have just %i or %u we need to keep state to represent the actual length mod.

Plus, when I go to convert the integer, there will be no knowing of its actual size because convert_int is universal.

Unless I choose to decouple conversion from the buffer handling, or output code.

This may be a good idea. Have two: convert_int and format_int.

## Faster Conversion

With an oversized lookup table of 512 bytes, it is possible to convert two characters at a time.

Instead of dividing by 10, we divide by 100 and take the remainder of that.

This theoretically reduces iterations by half, but bloats the binary substantially and may not be that fast sometimes.

It however does not waste as much time with the division, so it may be worth it.

On a modern system, I would not even think about doing this.

Anyway, I can write a proof of concept in C.

## The Exact Workflow

### _printf_core

Print characters. If a format is found, call helper.

### void handle_format(context *p, const char *__restrict f, unsigned index)

Combined into one function now. It also calls the format handler to avoid having to relay the data through the structure.

Index is no longer part of the context. Analysis of the old code shows it was only used in _printf_core and the old set_fmt_params, so modular separation is improved.

The string itself is also passed because decode is 100% self-contained and should never be accessed again. fmt_char is still okay, although it can be turned into a boolean because only capitalization matters for g/G or x/X, and only take one bit of space.

> The value fetched from the stack is copied into a union.
> The size of the

## Proof-of-concept converter

```
#include <stdio.h>
#include <string.h>

char twochars_tab[100][2];

void converter_poc(unsigned v)
{
    for (int i = 0; i < 10; i++) {
        printf("%.2s", twochars_tab[v % 100]);
        v /= 100;
    }
}

int main()
{
    for (int i = 0; i < 100; i++) {
        char b[3];
        snprintf(b, 3, "%i", i);
        memcpy(twochars_tab[i], b, 2);
    }

    converter_poc(1234);

    return 0;
}
```

This does NOT work correctly, but it outputs the right generated characters. If written in assembly, it will take up significant space, BUT it will be able to theoretically generate characters at half the cost, at least on older systems.

BTW we should try to keep the table as close to the executable code as possible so that it stays in the TLB entry for one page.

The table is actually 200 bytes, which is not bad at all. It is worth the cost if it speeds up conversions.

All in all, the worst case complexity is cut in half. Instead of 10 iterations, it is five.

Also the compiler has a clever way of performing the mod 100. GCC optimizes MUL and DIV using shifts and LEA. I should look into that although the purpose was originally to make division less expensive.

32-bit division is not the fastest thing though. It will be 38 clocks on a 386 and 40 on a 486.

But there will be only 5 total divisions for a full 32-bit number. That is 200 clocks to convert one number MINIMUM.

So it is still not fast or anything.

I just looked at the 80386 manual and it appears that the multiplication by a factor of 1374389535 which is required as part of the compiler hack is actually really bad. It requires 31 clocks to complete, which is actually slower.

Actually it is 37.

Combined with the other instructions, this is significantly slower than one division, which is 38.

I am not sure why the compiler does this even with older CPU tuning. On a newer CPU, multiplication is way faster and less data-dependent, so it makes sense.

It is still much faster to do a 16-bit division. In such an event, it is better to defer to a 16-bit integer converter, which can be unrolled since it only needs 5 iterations, and no early exit.

A 16-bit converter will only need 110 clocks rather than 190 assuming iterations are reduced.

So I will have to make some modifications.

Also, the idea of using the lookup table is GOOD and I will end up doing it. It works for the 16-bit conversion, although it can be fully unrolled since only 3 operations need to be done, with no constraint on iterations needed.

# June 12, 2025

There is a compiler option called -fno-tree-divide which avoids what it called "strength reduction." This is only fast on modern CPUs.


## Changes to Converter

First of all, one less division is actually needed because EAX will have the final digit.

Also, the number of iterations will be constant now.

There is unfortunately no fast way to get a reasonable iteration count from the number itself and dispatch it fast.

I asked ChatGPT and it does not have many good ideas given the constraints.

But there are not 5 iterations!

It is 4 to get 8 digits and the divident works for the rest.

The only part I dislike is that we have to use several clocks to find out how many leading zeroes exist.

However, this can be sped up. I already know that there is pair-generation. We can just scan against "00" sequences and check if the non-match has a single leading zero in it.

So the strchr we have to do will amount to very little. The condition check must happen anyway as part of scasw.

scas is 7 clocks, so better to not waste them on bytes instead of words.

Remember, 4 iterations. That can be unrolled and avoid any extra arithmetic. Also, I am removing the iteration count.

I really do not want to spend any more time on this.

## Octal conversion

While shifting and offsetting works fine, octal can be optimized with a LUT too.

Two characters can be converted by shift/and'ing 6 bits. The problem is that the LUT that we have is not quite sufficient.

Octal, however, is not frequently used, so it is justified to be slower. It also can be implemented using 64-bit operations.

## TODO for converter

- Remove the iterations argument.
- Unroll all loops
- Avoid extra iteration by using the divisor at final

I just found out something interesting. 8-bit conversions actually divide a 16-bit value by an 8-bit operand and store the result in AH:AL.

So things are slighly wrong here, or not optimal.

16-bit division divides DX:AX with a 16-bit value, but the operand is 32-bit.

So the 43-clock operation is totally avoidable. I will do some more calculations, but at least 38 clocks are used for a 16-bit conversion and 108 for a 32-bit conversion.

The problem with 8-bit conversions is that the actual result is not 16-bit and destroys the original value.

So unfortunately, I cannot do this optimization.

Well partially, I can, by mixing them together. Unrolling allows this.

After doing enough divisions, we can be sure that the accumulator of the operation is representable in a smaller type, and them move to that.

It is easy to transition from 32-bit division to 16-bit because the same registers are used.

> The nasm source code has a good CRC-32 and CRC64 implementation. It also has a red-black tree. Keep in mind the copyright notice. Just in case I need to implement hash tables or ordered sets in the OS.
> TBH I can probably use a different hash algorithm for strings.

Another idea: if I know I will get a 16-bit result, I can perform a 16-bit division on an even larger number because it takes a 32-bit input.

6291456 is one such number. The result is 16-bit and equal to 62914. So it is perfectly reasonable to use that as the 16-bit division threshold. The only problem is that it requires 6 iterations instead of 3, and despite the common case being much lower.

> Even bigger LUT? Three chars is difficult, and 4 for alignment means 4000 bytes! Conversion should be very fast though, but lookup tables should really be used more sparingly.

Encoding 000-999 is not very efficient and takes up 4000 bytes. However, it makes %hh conversions O(1) and %u need only two actual divisions, and we use the quotient once we are done.

This does make printf extremely large though. The code will be 6-8K. I think size optimization is more practical, since the speed critical part will be written in ASM.

Also, printf will use plenty of lookup tables too.

## Plan for integer conversion

For maximum speed, the highest divisor allowed for 16-bit conversion should be doable in one actual division with one finishing operation. That is a true 16-bit number.

The rest can use the 32-bit one, but remember it will be unrolled to use faster operations once we know the result will be 16-bit.

32-bit only needs two divisions by 100 and the rest can be used to finalize.

The only difference is that we use the faster operation if it causes no data loss.

# June 13, 2025

I made a slight mistake.

The modulus operation is only capable of getting 3 digits at a time.

Unless I use a 10,000 byte lookup table, which is not happening.

The table goes up to 999. This means things are a little different.

int has 10 digits and requires 3 divisions. It definatively needs three before the quotient can be used. Also, the quotient will be no greater than 4, which means we can just '0' to it instead of using the table.

unsigned short has 5 digits. 65535 can be divided by 1000 to get 65, which can be looked up after, so only one division is needed.

Not much changed, but the assumption that we can use 32-bit access and get 4 characters out of it is wrong now.

32-bit access can be used (given there is a padding byte) for 16-bit conversions because we can just overwrite the next 2 bytes over the extraneous one.

For unsigned int, this is unfortunately not possible. It is necessary to copy a word and a byte.

## integer conversion

I got the short path to work now. I need to work on the 32-bit conversion and start working on the actual printf.

# July 14, 2025

## integer converter

64-bit conversion is not done yet. It will use very similar code and MAYBE it will also chain to conversions and can even be done in C. It does not need to be fast because it is rarely used on 32-bit systems.

So far, conversion seems to work flawlessly. I just need to implement the strchr mechanism at the end of it.

The highest number of leading zeroes can be determined. Because 32-bit conversions are only for numbers greater than 999999, I can be sure that there will only be a certain number at most.

Specifically: 0001000000

There can be no more than three leading zeroes.

I may come up with some kind of trick.

I can use setCC instructions and a final LEA to capture the exact number.

```
cmp byte [edi+0],'0'
sete al
cmp byte [edi+1],'0'
sete bl
cmp byte [edi+2],'0'
sete cl
lea eax,[ebx+ecx]
```

Not super fast, but on a 386, scasb takes 29 clocks in the worst case. This is less, so it's better.

Technically this is not correct. It checks THREE zeroes, not regarding if one is a trapped zero.

I can insert branches. Or I can have a condition check for TWO zeroes.

Because CMP is just a subtraction, I can do it in a packed manner even when I know there can only be 3 digits. Nah, probably not.

```
cmp word [edi+0],'00'

```

Additive methods don't really work because of trapped zeroes. There also is not common case here either.

I may be able to compress the values into a lookup table or use some kind of boolean logic.

The lea trick is interesting. I think, given a b c are the three characters in question, b can be given priority by multiplying the truthy value so that trapped zeroes are eliminated somehow.

Or some xor or and operation can take place.

b xor c means if b is LZ and b is also LZ, the result is not LZ, so no.

I think AND should be the correct method.

Okay, so this is a one-way function from a b c to 0 1 2 3.

But {abc} is just a number, right? We have no way of converting it to one though.

Better to think of this as bits.

Then a smaller lookup table can be used, perhaps no larger than 8 bytes.

What I had before can have a shifts involved, which lea can do.

```
cmp byte [edi+0],'0'
sete al
cmp byte [edi+1],'0'
sete bl
cmp byte [edi+2],'0'
sete cl
lea eax,[ebx*2]
lea eax,[eax+ecx*4]
movzx eax,[table2+eax+edi]
```

Maybe used EBX instead to accumulate and save an extra byte, or use mov instead of movzx.

Also note that this only applies to full 32-bit conversions.

# June 16

> \# notification when a keyword matches a newly added \#?

## Moving printf to PC

I am going to move printf to my PC to work on it. Because WSL supports the SysV ABI I can test my printf against GCC.

I have greater confidence in the compliance of glib printf than DJGPP.

## printf structure

Fetching the data based on the flags can be done, but the "n" flag does not make it easy. %n cannot take any other flags.

I really need argument fetching to be unified, but %n completely ruins it.

However, unification is still possible, although not in the preferred manner.

Anyway, argument fetching is supposed to happen at decode stage (makes sense given the FDE analogy), and it saves space that could be wasted with branch tables and other things if done per conversion.

A switch table can do the argument copying by the way.

It actually does not need to be copied. A pointer is enough.

The converter needs to know what size to operate on.

This is where simplifications can take place because of the architecture. The types are basically i{8,16,32,64} or unsigned version of that.

The %n handler will probably jump out to something else that handles it.

But fetching it means copying it somewhere, and we still need to know the size of it before converting.

Everything goes into memory anyway, but I am talking about removing the intermediate step.

Getting the correct data type is the requirement of each converter. It has to cast it if the type is narrow too.

Just use va_args anyway. No need to think about all this.

Also, it is possible to cram everything into a switch/case, or at least partially. It still gives less control over the storage used for table though.

It is possible to avoid switch/case by the way. Computer goto is used a lot by emulators.

It probably won't be an issue.

## Hex conversion

This is another "compute kernel", so I need to write this one too.

A lookup table is kind of unthinkable at this point. 4000 bytes is already insane for integers.

The current hex converter is backward generating too, I think.

I think I will reuse that. Same with octal.

Also, I will not be using my PC actually. That screen is not good for my eyes.

# June 17

## Printf buffers

In most cases I will want to write characters to a buffer in memory and then transfer it somewhere. Making system calls constantly is very slow and is something that should be avoided.

It appears that stderr does not output each character individually

Something like:
```
    fprintf(stderr, ">Hello World %n", NULL);
```

This is not safe for standard C but some libraries accept it. Changing NULL to NULL+1 (also undefined) ensures it fails.

In that case, nothing seems to be outputted. Same results when I change shell.

So it appears that having a segfault even when printing to stderr does not mean the generated characters are outputting right away.

This makes sense because fwrite is what actually writes to the buffer, not printf. Formatted printing is the method of generation.

What this means is that printf should be designed only to generate the characters. There is no need to use fwrite or fputc in the actual implementation of each printf function. This would be way slower.

So vsnprintf is the core function used in all formatted printing. Something more fundamental is not required, and there is nothing that the existing "printf core" can do.

So only one buffer commit is required. The only thing to consider is that the final output buffer may be quite large and printf is required to output at least 4095 bytes by the standard.

# June 18

## printf changes

I will implement vsnprintf basically. The printf context will be created inside it.

## Note about PnP

I may need a 286 TSS to make PnP calls! Think about why though, I just read that sentence off OSDev.

# June 19

## printf updates

I know the standard requires 4095 bytes to be printable by printf, but the kernel environment is a very unique situation. Nobody should be printing that much anyway, and a buffer will be allocated for sprintf already.

### vsnprintf

This is the core of printf. It will create the printf context inside its scope.

Buffer generation is changed such that we can actually write directly to it, but as long as it is not overrun.

There is no need for callbacks at all.

Each format will potentially use a conversion buffer and also put characters into the output.

write and dup become independent functions now.

There are two ways to represent buffers:
- start/end/current pointers
- start/size/index

Checking if we reached the end or if incoming output will overrun based on an offset can be done with `curr <= end`

The second uses `index+num_to_write < size`.

I will use pointers. Also the end pointer is the last byte, not the byte after.

However, the calculations, are different when the buffer is overflowed and needs to be truncated.

If the condition above is reached, the caulculation of the number of bytes to actually write is:
* start + size - index
* end - current

Using pointers is simpler.

### snprintf with NULL destination.

This is in fact safe, but the size is required to be zero since there is no real need to truncate if we just want to know how long something will be.

If the count of bytes to output is always zero, there is no problem.

But we do have to keep track of the number of bytes printed total, which is related more to the printf family than the buffer, but it works that way too.

Using only pointer arithmetic is not really reliable to get the number of characters that could have been printed, only the number that actually were.

### Data fetching pipeline

Basically there is a "fetch" or "eat" operation that gets the next value.

The descrepancy between %lf and %l{i,u,d,x,X...} makes it impossible to know the size right away. I think the value should be fetched as it is needed.

We have optimizations for integers to print small numbers fast, namely 32-bit ones. 64-bit numbers can be concatenated, although it is not that fast.

The number of iterations output is no longer needed anymore since conversions have a heuristic way of avoiding iterations in such cases.

Each conversion type should interface with the converter in the most optimal way, so decentralizing integral type reads makes sense somewhat.

In effect though, we are either reading a 32-bit value or a 64-bit one. This can be optimized using assembly, where operations can be inside the table itself with full alignment, so no table is needed.

Maybe use inline asm.

### L and l and not the same for float

Actually only %Lf is valid for 80-bit float. %lf is still double, which is 64-bit and the same as no length flag.

So float and int are different.

Anyway, I can have a function that outputs to a void pointer. convert_int will either grab a 32-bit value or a 64-bit value, and that is a condition is has to check for.

But in effect, all it really does is copy the value to the destination.

### Copy va_list?

va_list is basically a pointer type on i386. It should be safe to copy it around.

https://wiki.sei.cmu.edu/confluence/display/c/MSC39-C.+Do+not+call+va_arg%28%29+on+a+va_list+that+has+an+indeterminate+value

This link has some guideline saying to avoid passing va_list because it is easy to make the mistake of putting in va_end or something like that.

Actually no. While the standard library does do this, the va_list may have an "indeterminate value" or in other words, the arguments may be theoretically exhausted. The va_list may not actually be reusable.

Normally one would have to va_copy.

On the SysV ABI, things bay be different because va_args is just a pointer, which can be passed around.

But consider the fact that it must be MODIFIED as it is used, so that the position is correct.

When we use va_arg, it WILL increment the value, but an important thing to note is that it will only update if the value is in the structure, so it ends up being passed by reference but indirectly.

Also, in my test code only pass by reference seems to work anyway.

Point is, do not copy the va_list unless it is intended to be fully used by the callee.

### Dispatching the conversions

I will use a computed goto this time. It is presumably denser than function pointers and requires one less call/return. The struct will contain the va_list. There is no reason to pass to a function what it can get from one of its other arguments. Just makes no sense. Plus, va_list can be all the way at the top for minimal performance impact from immediate offsets, although putting the buffer there may be a good idea.

# June 20

## Components to work on

(Copy this somewhere else)
OS/90 has the opportunity to recieve widespread recognition and be a platform to build my reputation and future career, even if I am not making money from any of this. So I really need to put in more work. The other project can wait.

I am trying to delinearize my approach to OS/90 by switching to different components every few days. So far I have been wasting huge amounts of time overperfecting printf.

I need something else to work on.

My OS will need:
- An INI parsing library with support for destructive operations
- A parser for ISA plug and play resource descriptors
    - And a way to allocate the resources with respect to bit decode as well.
- An executable format and loader

## For today

An INI parser is easy enough.

There is no fixed standard, and the method of access introduces many possibilities of access.

The functions it should have are:

- int I_OpenINI(const char* path);
- void I_CloseINI(int handle);

- bool I_HasSection(const char* section);
- bool I_HasKey(const char* section, const char* key);

- int I_ReadKey(int h, const char* section, const char* types, ...)
- int I_WriteKey(int h, const char* section, const char* types, ...)

The data types supported:
- Integer (32-bit signed, hex, binary, or decimal)
- String (codepage 437 in theory)
- Array (combination)

The type of a key is defined as an integer if the string complies with the grammar of an integer representation (negative works as a unary operator).

If it is wrapepd in quotes (single or double) it is always a string (must be whole thing until end of line for single value).

If a comma is found in the key and no string quotes are found, the value it interpreted as an array.

I_ReadKey and I_WriteKey operate with pointers and values respectively, except that write uses character pointers to write strings but otherwise uses integers to write content.

```
int i;
I_ReadKey("[MY_SECTION]", "NUMBER_KEY", &i);
```

Structures cannot embed structures.

The entire file is sanitized before any reads or writes can occur when it is opened. The file is also inspected to list lines with keys and clean the data.

The following checks are done:
- All characters are printable
- CRLF is used.
- All keys have no space between the equal sign
- Keys are alphanumeric with no number at the start.

It is better to sanitize text data before parsing it.

The INI parser should be portable.

# June 21

## PnP ISA

Plug and play ISA defined the resource format used by the PnP BIOS to describe motherborard devices.

I can write a program using Open Watcom and access the PnP BIOS using either QEMU, DOSBox-X, or 86Box and get raw data for testing purposes.

In pursuit of delinearization, there are many other things I can do without any dependency.

Examples:
- FAT driver
- ATA disk driver (I should probably wait until I have a PnP architecture)
- Keyboard and mouse driver

I have written some code to use the keyboard and mouse with limitted success.

dosbox-x has ISA PnP on it. I think working on it first is the best idea. I am not sure if the ISA hardware is PnP though. Probably is.

BTW FS and disk drivers are low-priority because software exists that already improves the performance of IO in real mode.

I can come up with some clever ideas, but now is not the time.

PnP ISA tags are a very simple format. They indicate the size in the header most of the time and do not seem to have any hierarchy.

A structure is sufficient enough, with a fallback to dynamic memory, to store all teh requirements of the device.

Tags more or less just say what the device is willing to accept rather that strictly define it.

If it needs a memory range, it can say it takes any 24-bit memory range to indicate full PnP. If it requires a static range, perhaps for a VGA controller, it will request the typical range for that and specify that it needs all of it.

For each IRQ it needs, it will specify several possibilities. Same with DMA too.

This data is read only too. It is what the device requries to function.

## ISA Limitations

ISA has the tag format, but the actual control registers used similarly to PCI BAR registers is limited in such a way that the actual object representation of an ISA device is simplified.

# June 22

## Details about DPMI

The Microsoft Confidential document says that the ^C handler must return and cannot be used to simply jump somewhere. Same with 24h. I think I already wrote this stuff down.

## DMR

The DOS Multitask Region is probably what I should do for efficient multitasking.

It has one disadvantage and it is that many page table entries have to be copied to properly bank switch unless I create a page table for the first 4M.

However, it only needs to match how many pages there actually are. Using page faults to manually replace the pages makes no sense BTW because it already necessitates changing the page tables.

If a program needs 128K to run, 32 changes must be made to the page table.

Also there is no way to avoid this even in protected mode, unless the program is explicitly prohibited from entering real mode at all, which makes no sense.

At this point, full DOS memory virtualization does not sound too bad, aside from adding a 4K overhead for a total of 12K per process.

This is not too bad because most programs will run as threads of a single process.

## 32-bit Services with INTxH?

With the register dump structure used by INTxH there is a way to find out if the context is in protected mode, which sometimes needs to be handled separately, such as when extending an interface.

VM=1 in EFLAGS confirms the context is PM. All information is self-contained. The CS selector is checked for its bitness.

INTxH however has a dillema of how stacks are to be used.

## INTxH

Each TASK, not the VM, will probably need some kind of stack for INTxH.

I tested the V86xH code using the top of the HMA as the stack base.

The stack is 100% needed. We simulate INT/IRET accurately except when exiting.

But if the interrupt is hooked, things are very different.

If a hook is found, there is still the possibility it will go to real mode, but within a call to a hook handler, there is not really a need for a stack. It can be zeroed out to indicate it is not used, and allocated only when needed.

Only one thread can enter SV86 at a time, so there really should be one stack and no more.

In the rare event that an interface requires the stack but also uses a PM handler, we won't even be using the real mode stack to begin with.

And preemption is off only when going into SV86 or maybe while doing certain things with the capture list.

## DOS Memory Multitasking

On average, I doubt most people will run more than 4 or so DOS programs at a time.

So multitasking the DOS memory is quite practical and a great idea overall.

It just adds some complexity to INTxH and V86xH since those two require access to the real DOS address space.

Just as with VMM32, one page table manages the entire first 4M of the address space.

BUT, the HMA is NOT permitted to be accessed. We still have many structures there. Programs are required to operate normally if the HMA cannot be obtained. Only OS/90 actually demands it.

Windows 2.0 actually also needed the entire HMA on 386 enhanced mode. I suppose it is an acceptable practice then.

The HMA is currently used to contain the startup page tables, which are reclaimable and later used by the kernel instead of finding memory for it.

Honestly, I don't think the HMA is that critical for most programs. I need it for the kernel's purposes.

It will also contain an array of far call redirector codes, and it must supply enough for DPMI compliance.

### Implementation

There will be one new page table for the first 4M of the physical address space that is identity mapped. The PD must be changed to this when doing a V86 call.

### Obstacles

The BIOS data area is not at a page aligned boundary and some drivers need to access it.

But there is a global BDA which some drivers may use for some reason, and a local one for each process.

The interrupt vector table works differently. In this situation, things are a bit more orthogonal because using INT in real mode (or reflecting) jumps to an interrupt vector in an absolute location, and the memory for it is sort of implicit with no actual base address.

Also, the whole DOS context can be put in the memory used by the program and reside in a predictable location. This means we can access it with zero pointer offsetting, kind of like how Java maps the heap to address zero.

The BDA ends at 0x500. This is where the DOS context can reside.

Getting to the DOS context is done by switching the page directory entry, which is done by the scheduler.

There are a lot of challanges. The PSPs are no longer uniquely identifying unless created by DOS, and most system calls that require the PSP will not work unless they are hooked so that the PSP is set.

Or, PSP get and set are hooked themselves and we hope the DOS kernel does not just directly access the current process. Windows surely hooked this call, so I don't think there is a problem.

There is also no INVAR for the current PSP.


I looked and cannot seem to find it. I am starting to think DOS just uses the special call for it. The fact it is already an internal function makes it appear to be the case. I will read the MS extensions doc for info.

Get/set PSP is something that is virtualized for clients but the OS does have the use the real one.
There is no way to get the current PSP otherwise.

It can be hooked of course, but some features in DOS need to know what the current task is. At boot, it will be the bootloader.

Hooking it in SV86 mode... interesting idea. But it is not a solution because DOS has its own representation of the current PSP. We do not know where it is.

> How do we know if it is SV86 or a 16-bit DOS program???? <<<<

# June 23

https://gitlab.com/FreeDOS/base/kernel/-/blob/master/SOURCE/KERNEL/hdr/lol.h

It appears there is a most recently executed PSP entry in the List of Lists.

RBIL says it is not used for that if DOS is loaded low, which is will be in this case.

I trust the FreeDOS source code more than RBIL honestly. Both locations line up.

Also, the RBIL listing says DOS 5.0 or greater is required for it.

Anyway, the implication of changing this value directly is that we no longer have to trust get/set PSP and how the kernel gets the current program.

The PSP contains file mappings (file handles to global file records).

I will write a program to confirm the correct behavior.

## Multitasking

Windows creates PSPs for each program it runs. We do not have to do what windows does, although there are reasons why Windows chose this.

Because the API is implemented on top of DOS, it made sense to have a PSP contain the DTA and all the other information. It was convenient enough in this case.

> BTW the kernel needs the current PSP to add a name string into memory blocks.

For the kernel, we can create a real program segment prefix for it so that drivers and the kernel can open files safely and allocate memory, which both require a valid current PSP.

The INIT will require destroying the bootloader's PSP, leaving the current one no longer valid.

As for programs, the 256 file limitation of DOS is quite severe and ultimately unavoidable. In my approximation of Windows' KRNL386 operations, I will have to NOT create PSPs for each program and only have them run as threads of the same program. There is no need for "native" OS/90 programs to require any interaction with DOS programs except to execute them in separate VMs and monitor them.

Also the PSP change function must be simulated in some way that permits Windows to work. Quarterdeck did it, and so can I, as long as the rules are followed.

The current PSP can be virtualized, or the list of lists can be virtualized. GET/SET should be enough. The process will maintain a current PSP variable which is local, and creating a PSP is always hooked.

### Thread-local data?

Some things can be transfered to a thread context to enable enhanced multithreading. In UNIX for example, a thread is sort of a clone that may share some things but still maintains its own context.

Linux has some settings to decide this, but the capability exists to allow threads to copy their current directory.

The idea is that some data is shared and other data is copied.

Because each thread represents a part or a whole of a program, some of the state must be moved to a single thread.

I may need to change the concept of a task so that the DOS stuff is kept there.

### Describing the DOS Context

Part of the DOS context is thread local and the other part is process-local. The process-local part goes in the mapped conventional memory.

Wondering, if we use an IVT, does that mean a program can call the BIOS directly? No, not quite. It is just slightly more accurate emulation for the IVT which is directly accessible in DOS.

If the handler does not exist, then the OS gets involved and applies hooking. Plus, BIOS is not reentrant.

### Memory

To access the actual real mode memory and address space swap is required.

## printf buffering

How exactly should something like fprintf work? This can write any number of bytes to a file.

It is actually quite complicated. Currently, the only way to know how many characters are needed in a buffer is to preprocess the format string and allocate enough memory.

This can be done using snprintf with NULL/0 arguments, but is more efficient if a separate preprocess scanner exists, which can even be used to implement that special case of snprintf.

This really does need to exist. vsnprintf needs a buffer and a size, and we have to know the size before calling it or the characters will be be correctly printed.

Preprocessing does not require accessing any arguments, only the format. A %i or %u will add 10 characters to the buffer for example.

A basic printf will also require this.

## Was it a good idea to change buffering?

My idea was that fprintf and vsnprintf could implement their own systems for buffering, and that it could be somehow optimized for the particular use case.

fwrite and fputc could be used for fprintf within the buffer commit calls. The data could stay on the FILE buffer and extend it if needed.

But the FILE representation of a buffer is very different.

I am not going back though.

## printf preprocess

The size generated does not need to be exact.

There is a problem though. Strings are impossible to predict. This may require actually reading the arguments in that case.

The arguments CAN be read. On i386, it should be safe to just assign a va_list with equals, as it is just a pointer. va_copy should be avoided because it must duplicate the arguments.

ChatGPT also says va_copy does not actually copy them. It makes very little sense why it would do this anyway.

Reading arguments adds no major overhead. Counting the number of characters in certain format options is not necessary and is low-cost IMO.

# June 24

## printf 4095 bytes limitation

A very interesting thing in the standard is the requirement that 4095 bytes must be printable by fprintf, and all other variants are defined in terms of it, so this is required by the standard.

The strange thing is that trying to print more than 4095 characters is not explicitly called undefined behavior, but is tantamount to that because an implementation is permitted to fail the output if it wants to.

Or rather, it is defined as "it might work or might not work".

I think 4095 was selected because it can print an extremely large float value with most of the characters.

Anyway, a program cannot simply assume that printing more than 4095 characters will work. Programs probably don't check though, but there would be implicit truncation and the library is permitted to do so and be compliant.

The thing is, printing even more than 2K characters is possibly infeasible on OS/90 without heap memory allocation. I do not think 4095 is a serious limit.

The preprocess will unfortunately require an strlen call for each string argument. This may not be too bad though. %s is less common than the other ones.

> What does the 4095 limit do for the design.

## printf limits

4095 will probably be the limit for userspace. There is no real reason to print more than that.

As for the kernel, I do not want to waste any stack space, so I will have a static buffer that is locked by a mutex.

The only issue is not being able to log within an interrupt handler, but that can be solved by using a special log buffer for the ISR.

ISRs should be as simple as possible. It is a very contrained environment that should just set some condition variable instead of doing any major processing.

Or it may interact with devices...

But then again, we can handle that already with a separate log buffer.

Plus, we have to write the log results somewhere.

## PnP Topics

Can ISA devices be represented in one single C structure?

Yes, they can actually. A similar thing can be done with PCI, but regardless, each bus can have its own representation.

I am not doing any kind of Windows Device Manager things. Each bus should section off system resources for their subordinate devices.

Resource allocation is not something that happens often. It happens when a device is inserted, perhaps from docking or just regular insertion, and we have ways of getting events like that.

PnP BIOS is sort of its own bus with special on-board devices, although it will share code with ISA PnP. Even if no ISA devices are used, the PnP board devices are basically ISA devices.

### Resource allocation

Port decode must be handled. There is no way around it.

Memory ranges should not matter since the bus is probably smart enough to deal with that if the PC has more than 16M of memory.

My old idea was to allocate bits in a bit array, perhaps with respect to alignment (or just auto-align), and give ranges of ports to devices.

In practice, I am not so sure this is needed. I can probably give 16 ports at a time for a total of 4096 port ranges in the best case.

I don't think even ATA uses that many ports. I checked OSDev and it seems it only needs 8 bytes.

Bit arrays can be used for compactness, but each bit can represent more than one port. This is not a major issue. A total of 512 bytes can be used to do it.

The way that decode width is handled is that every possible combination of the higher bits must be set in the array as needed. Specifically, this must occur 64 times. It is really slow and should be avoided, especially with slow bit arrays.

Instead, I can keep track of 10-bit or unfortunately, 12-bit (thanks to the ISA PnP READ_DATA port) allocations and check them for collisions first.

That goes into the alternative, which is holding the ranges in some array or other structure and checking for collisions when adding a new one.

I may want to have some method of sorting it for faster lookup. Of course, not with trees.

### Memory Ranges

To dynamically reserve a memory hole, it is possible although not advisable to copy physical pages to another part of memory which has to be allocated, and retain the virtual memory mappings. This manipulates the page tables and block-copies the physical RAM, maybe with cache disabled to avoid cache polution.

This does reduce the amount of physical memory since that is all ISA understands.

int M_PokeHole(unsigned page_frame, unsigned npages);

# June 25, 2025

## INTxH

> I might want to make a new API call for this purpose.
> Also, some operations are restricted for the userspace. Consider direct disk access. INTxH as described should not run like this

> Another fun fact: the register parameters can be defined inside the thread context!

I wrote this in the docs. Time to think about it.

INT 13H may be called by the user and require different handling.

I think the idea I was supposed to go with is actually provide a different service for userspace hooking.

The only issue is that I want userspace to also be able to hook DOS to implement services needed for UI integration.

This is done within the DOS services subsystem. INTxH is for supervisors.

Then how does hooking actually work? Can we just have a way of knowing where the request came from?

How about: ring-0 PM code/stack segment.

It is only symbolic though.

Maybe there can be two layers where userspace hooks act as filters.

No, not quite. It is better to check what requested the INT. This can be a parameter too. Maybe the vector can have a bit for userspace.

INTxH(INTUSER | 0x21, &regdump);

Also, using a thread-local register dump is actually REALLY interesting, but it may require some interface changes.

The idea is I can write code like this:
```
_ah = 0xE;
_al = 'A';
_bx = 0;
INTxH(INT_USER|0x10);
```

It makes sense anyway. The user program, or the thread, is making the request here. Even with the kernel doing supervisor calls, there is nothing wrong with this because the data is thread-local.

_ah and _al in this context are all in the TASK structure BTW. Common subexpression elimination and maybe some attributes on the current task inline function make this optimal.

I should REALLY do this. It makes code look much nicer.

# June 26

## Ideas

### The Other Project

Consider the use of forks. This is possible on macOS and Windows, and Linux could support it with a clever kernel hack or different FS.

### Dynamic Code Compression

To make use of lower amounts of memory, I could use my code compression idea and use it to compress at run time instead.

It needs some decision tree to figure out the most optimal start location.

> Maybe use an opcode matrix to plan the opcodes for the compressor

If it can compress to something smaller or small to the point of being worth compressing, maybe it is a good idea.

To be exact, it probably needs to compress to half a page or less, such that it can be packed into one page with another block.

Also, a common address table is not feasible, unless I use some kind of hash table to list them and to a preprocess for it too.

It is a decent alternative to demand paging although not mutually exclusive. If it can cut memory use by code almost in half, paging can be avoided for code.

This requires marks in a page table: Compression Feasible and Is Code.

If it cannot compress to 50% of the original size, compression is not feasible.

I am not sure how the memory will be condensed into a page though.

## printf

Floating point formats are not REALLY that hard. I just have to represent the parts of the number as scientific notation (base 10). Then I can do the same thing I would with integers.

The conversions would be really slow though. FDIV is about 90 clocks and FPREM is over 110 on the 387.

In the worst case, a conversion can take up to a million cycles, which is completely infeasible.

Is there a faster way to divide by 10?

Or is there a way to cut off at a certain point and just generate zeroes?

Interestingly, djgpp libc crashing when I print DBL_MAX. Most likely it printed too many characters. Actually no, it can print lots of characters. Not about 4095 on djgpp yet though. But DBL_MAX is not too large.

DJGPP actually has no limits on printable characters. In theory, we dont need to either, if we do the preprocess thing and allocate a large enough buffer.

# June 27

## char becomes unsigned?

I do not want to deal with the possibility that char is signed because of the.....

DJGPP is able to output signed chars. They can be encoded easily.

The only issue is when passing them to %c.

Actually, there is no issue with that either.

It is all about interpretation. The character may be promoted, but it is then narrowed back to a char. I would like to see the compiler do it though.

It really does not matter for arguments.

Well, it does.

Passing an explicit char argument has no promotion (keep that in mind), so if I try to return it or copy it, the compiler sign extends. Otherwise, it zero extends.

The only way to safely convert a char or a promoted int to the real character is to convert it to unsigend char.

The printf spec requires that the promoted character is casted to unsigned char...

Okay, but how do you think UTF-8 would work? Signedness is just a compiler construct. The CPU knows nothing of it unless multiplying or dividing. The assembler, most arithmetic, and addressing also do not have such a concept.

With the right casting, the compiler will do the right thing. But in theory, codepage 437 needs all 8 bits.

Regardless, djgpp is able to print those characters because they are just bytes and it is the console driver that cares.

The impetus for unsigned char as required is more theoretical. The standard also promotion to convery chars that are in the UCHAR range, such as getc, which on DOS can be used to fetch a full codepage 437 character. It relies on overriding char's signedness when it is signed.

And Terry A. Davis said he didn't like "7-bit signed ASCII" so there is another reason.

Also, unsigned char means I do not have to write unsigned and char is just a byte.

I am working on printf and I think signedness of char should be handled there because that is what djgpp does.

I wonder if DJGPP does this with a hack though. Is putchar(-1) actually correct?

It probably must be because of unicode.

GCC actually FAILS! When I try to print -1 and also 0x80, either signed or signed, it does not print the euro signed.

It looks like extended ascii is not supported. I assume unicode probably works.

It does of course.

Really it does not matter though. All IO functions treat the extended int as an unsigned char. If I want consistency, I can always make char unsigned, but it is not needed and violates SysV.

`putchar(0x80000000)` prints nothing because it prints a null character. That basically explains it.

A char literal is different than an int literal. The promotion still preserved the full 8-bit unsigned range.

It is just necessary to make it unsigned to avoid a sign extension and zero extend instead.

Anyway, I moved printf so I can run some test suites later. I am going to work on that.

# June 29

## Markdown?

I tried to use this DOS format, but I think I need to return to markdown. Nobody is going to have the same editor setup and the text can look strange on other computers.

I can write a tool that does this in python.

# July 1

## PSP

I have to implement the PSP so that Windows can get/set it, and DOS has to be able to access the PSP. This is a challenge.

How about the actual function call for getting the PSP when executed by the kernel is hooked and setting is just an illegal operation.

Then it returns the same exact PSP location which is just swapped out by processes.

What?

Okay, MAYBE I can do something like that, but the thing is, programs need to have a PSP that can be recognized by DOS. Maybe it can be copied to the same predictable location with 64 DWORD copies. Not really practical, but this would work. Not all bytes have to be copied though. Some are not important and the command tail has a size byte. Best case scenario: around 128 bytes are actually copied which is 32 copies or more. Keep in mind the args are terminated with '\r'. Some fields in the PSP are redundant and outdated, such as the FCBs which were considered deprecated even in the mid 80's.

That takes away 36 bytes, so a best case of 23 copies with 25-30 being realistic.

Or I can do some strange partitioning of memory. DOS does have internal MCBs and ways of detecting things that could potentially be reclaimed. I could detect the memory used by it, but this seems unlikely.

Honestly, there is no reason to punch holes for DOS whatsoever. The program should have access to all the conventional memory it can get. Programs should run BETTER under OS/90, not just "as good" as DOS.

> DOS also saves the SS:SP onto the PSP.

## Job file table?

The is a structure which exists in the PSP by default and maps local file handles to the SFT entry.

The entry for Set Handle Count does not specify where the new JFT goes.

Actually, there is a pointer to the JFT. It can be moved within the PSP.

## What is the solution?

The JFT being potentially separate from the PSP and other things make multitasking a bit problematic.

How about the process context, which is part of the DOS address space, contains the PSP as part of the swappable address space and the address is the same?

The "current PSP" basically always stays the same except when SV86 needs to call something on behalf of the kernel. Doing this has complications when IO and memory allocations are involved, but the kernel DOES create a PSP for that purpose. Resizing the JFT can also be done and is probably a good idea too.

## Semaphore

I though I would give up this idea, but if I hook INT 21h to wait on the DOS semaphore, it could be possible to make DOS multitask and each program could execute DOS kernel code.

This may simplify the process of reflection to a simple emulation of the INT instruction, without the need to call EnterV86 or even disable preemption while in real mode. As long as DOS is not called again, we can do anything else in the background.

This is actually a very good idea!

The only difficulty is finding out what parts of the address space need to be hole-punched by DOS and it requires collecting ranges of memory that DOS is already using.

> Also, copying the DOS kernel in some insane way to every VM is crazy but could actually work. As long as the BIOS is reentry-guarded, it is fine.
> There are still many things that could go wrong.

There is a way to get the DOS kernel segment, but it is not standard. I am not really talking about duplicating the kernel. This is just wasteful and the goal is to REDUCE the need for DOS and enhance it, not whatever that it.

But there is a way to get the segment of the DOS kernel, I believe. There are also internal memory control blocks for drivers and private kernel data. This is not really standardized though.

The best way to this is probably to allocate the largest possible block and do something like the DMR idea but with no fixed size.

This means conventional memory allocations are not guaranteed to work. It makes it harder to allocate DMA buffers, but not impossible.

Extended DOS interfaces become more complicated too, but not imposible.

## Extended DOS Interfaces

Normally buffers have to be allocated for each extended interface and copying must be done, but OS/90 does not need to copy, but simply split the transfer to 64K blocks for operations that need it.

# July 7

## DOS Multitasking

First of all, the DOS semaphore does not need to be used. Only TSRs and other strange programs may use it.

There are reentrancy issues with the BIOS too.

So basically, we only execute real mode code iff the OS/90 internal semaphore permits that.

This also prevents a theoretical deadlock if each vector has a semaphore because they could back-call each other. INT 21h could call INT21h or something that also does, and there is nothing wrong with doing that.

As long as only one thread is in real mode software, nothing is wrong.

Essential services like memory size determination or mode setting are more complicated though.

How exactly is the kernel supposed to request services?

It can run an INT on behalf of a process! I think I already discussed this, but this is necessary. There will be an initial task which is finalized at the end of initialization. The file handles and other resources of the INIT process are what the kernel uses to make calls.

> Remember to implement yielding when doing this.

Calling services in physical real mode must be added. What I have now only works for IRQs, so I need some other code block to deal with general purpose INT calls.

Or it can be something based on V86.

> BTW the idea for security is that the INIT program has elevated IOPL embedded in the flags register. This is essentially the kernel worker daemon. It may also implement userspace, but I am not so sure.

Switching to actual real mode has some advantages, actually. There is no MMU overhead een though we still wipe the TLB. It allows the BIOS to switch to unreal mode or PM to run a service that accesses 32-bit memory ranges.

Although I think PCI BIOS support probably moves the MMIO range to something RM addressible. Either way, 32-bit addressing may be used by the BIOS in a way outside our control. So making certain request really SHOULD be done in physical DOS/BIOS.

Interrupts are something to consider here though. We have to go back to the IMR that was used at startup so we do not get an interrupt that cannot be handled. PnP software could change the IMR after detecting legacy devices properly.

Other than that, interrupt will have to work as expected because the BIOS software does sometimes require them, such as for IO delays or getting data from devices.

There are lots of problems with letting the BIOS touch a device while PM is already controlling it. The only reason I need to enter the BIOS/DOS in real mode is to run services before the scheduler and DOS subsystem are fully ready.

I may need a general event for entering the physical BIOS, but caution should be used anyway.

Also elevated IOPL is not the concept I need. Any program should be able to enter the BIOS or DOS and elevated IOPL is needed at that exact instance.

## Is this necessary?

Preventing freezes in the user experience is a benefit, but what happens is that we reduce the working memory that any program has to about 400K. Programs will run about the same as on DOS.

Ideally we should eliminate all legacy drivers, but that obviously cannot happen. Some memory will be lost due to drivers for PMR.

Allowing DOS programs to access over 600K means the software will run BETTER than it would on regular DOS because multitasking with extra memory is possible.

The goal is also to eliminate the parts of DOS that are slow and probably 50% of what it does will be taken over by the OS in a full setup.

Also I already have SV86 code that WORKS and was able to perform IO.

So I will NOT do concurrent SV86.

## Problem with multitasking

> Windows 95 does allocate memory for the kernel under the name MSDOS. I am not sure if FreeDOS does the same, but it does show how much memory is used by the OS. OS/90 will have to do the same thing

So we have to use only a window to perform multitasking.

## Decision

# July 8

## The multitasking model

The goal is to replace DOS for better multithreading and performance. I think I will keep the existing SV86 design.

I have some things to figure out for that BTW.

## How Does SV86 Work?

I should have some terminology:

- INTxH - The calling mechanism used by any protected mode/VM INT call
- SV86  - Elevated V86 that runs in PMR
- PRM   - Physical real mode

One vexing issue is that of stacks. If we capture the INT, do we have to emulate the stack? No, because there is no need.

But some function calls such as PnP require the stack to pass arguments. And if SV86 is used at all, the stack is accurately emulated.

The PnP BIOS will probably be executed in real mode because doing that is much easier TBH.

The point is that a stack may be required even if it is not handled in SV86. If SV86, we do not actually care which stack to use, only one can be used at a time anyway.

Otherwise, there should probably be an option for it so that the stack can actually be used. Using PnP BIOS will probably require a LOT of stack space. I think 64K is required.

Not true. 1024 bytes is the minimum for the OS to allocate or make available for PnP BIOS. It means that we will have a difficulty getting memory in PRM, but is better than 64K.

There should probably be a flag for this:
```
INTxH(STK|KRNL|INT_FCALL, cs_ip);
```

Might work but not sufficient.

This is better:
```
void GetPrmStkForThread(void);
void GetPrmStkForRegDump(REGS*);
```

These will also always succeed and wait until something is available.

The number and size of real mode stacks is configurable.

Also, on SV86 we can just use the global one since it is not concurrent anyway.

Did we sort out the PSP issue?

## The PSP

# July 10

I will put the other project aside and work only on OS/90 until completion.

Are you sure though? Should I make it public and generate attention from the public?

Getting YouTubers to review it? Promoting it on forums? I am not sure all that is necessary. Plus I don't want to associate my OSDev screen name with my real name (unless I change it).

I would prefer to share it with software professionals who can understand it better, and MAYBE promote it in certain DOS and retro forums.

It will be a public project, but I am not sure I want it to go viral. People in real life that know who I am personally could start saying stuff if it goes super viral, and I don't want that.

Plus it is just an OSDev project of hundreds. OS/90 is more of an oversized demoscene project just like any other hobby OS.

So the purpose is not really fame, but to get the attention (hopefully) of people that know a thing or two about systems programming and software development in general.

## Advanced Text Mode

Remember that one game that had a really nice text mode exit screen? Try and immitate that somewhat for ATM.

BTW ATM does not need proper keystroke handling in the early stages of development. Just set up the console to not output on the screen.

Then I can use the keyboard to execute certain commands or step through operations.

I got the exit screen of Raptor: Call of the Shadows. Looks like a good way to do popups and maybe buttons.

## Note About Memory Operations

When writing byte string ops, it is possible to list every address accessed and confirm they are exactly the same compared to the basic per-byte one.

# July 11

> Remember to implement disk drive letter allocation and detection.

## Disk Drive Mapping Allocation

Disk drives can be introduced by a driver upon recieving a newly inserted removable device that is not a floppy or a blockdev driver loads and detects new devices.

This is what is problematic with even using drive letters. They are convenient, but changing the order in which partitions and drives are recognized changes the letters too.

Some OSes will reserve some number of ATA/SATA drives starting from C and then put any disk drives after. Plus there is A and B.

OS/90 has to be legacy compatible. There is a DOS driver for literally every device and entire websites that host them.

DOS has an internal structure for this. I need to find a proper reference. It is the drive base table or something.

It does exist in the drive parameter block, and it has a service that can be called.

The DPB has a pointer to the driver header which has a structure I can write in C, among several other structures.

# July 13

## Kernel Logger

The only purpose for zero/space padding and many other things printf has is pretty-printing for command line programs. Consider DOS MEM.EXE for example.

This is way too large for the kernel, especially with the insane 4000 bytes lookup table.

I can have a logger that uses a lot of macros and relies on concatenation, perhaps with implicit spaces.

```
LOG("var=" L_I32(var) L_CHR('\n'));
```

Or this:
```
LOG("$", 1000);  => "1000"
LOG("#w, 0xABCD) => "0ABDCh"
LOG("#w, 0x1BCD) => "1BDCh"
LOG("#w, 0x00CD) => "00DCh"

```

Use the printf style.

Hex literals will be translated to the MASM style, with zeroes added based on the size requested. Capital letters are default.

^ is to print an integer

~ is to print a C string.

Numbers can be printed as b, w, or d.

Placing two in a row prints the raw character.

Also, newlines will automatically be inserted.

## More on logging

There should be a log buffer somewhere. Upon being overflowed, it should write to a log file.

# July 14

## Logging during interrupts

This is just not really possible, unless the log buffer truncates upon running out. This is kind of bad, because it will inevitable overflow if it holds user information.

It is better to have a log buffer for the interrupt and copy the data later.

For that purpose, there can be a sprintf-style function call.

This is actually not a great idea. The data has to be generated in the ISR and then printed at a better time, after which all kinds of hardware or software errors could occur and potentially render the system unusable.

Which makes it pointless to snprintf. Might as well write to a variable and check it outside the ISR.

ISRs do very little anyway, and are usually written in assembly. OS/90 does not even have a C callback for ISRs defined, IIRC. Logging is probably too much for an ISR.

The T it runs in is determined by the log target.

# July 17

## Extensions

I will require all INT hooks to implement address space extensions for the services they control, and support ring-0 callers.

This means an extensions API is needed to make this easier.

Also, there should be some way of checking if a service is controlled by PM. This can be used to detect the presence of 32-bit disk access in case we need to allocate buffers in real mode.

INT 21H will be fully extended to the level required by the leaked Microsoft docs.

There is also the possibility that I can use a GSE to ask if a certain feature is extended. It should support limited information, such as "the entire vector is extended" in the case where that is actually true of course, or "indeterminate". Maybe.

Probably best to make it exact, but each service can use any registers it wants to.

GE_INST_CHCK_ASE can be added.

# July 19

## SV86

I am still not sure if I should make SV86 multitasking.

Consider for example how INTxH would work. It would force a program to transfer to that context. More likely, it would use a daemon or init process and yield to it for immediate execution.

The advantage is a marginal improvement in concurrency for services that are completely replaceable by PM software.

This would be good if I had SMP, but that won't happen.

## Disk backed pages

I want to make swapping and memory-mapped files possible. This is difficult to do at the kernel level because I have to call some kind of procedure. INT 13H does not really cut it because some block devices do not even have INT 13H support and extending the drive mappings of INT 13H is not really feasible.

I think a better idea is to store the MS-DOS 16-bit file handle. This would be executed with the kernel PSP, so no issue there.

28 bits exist that can be accessed in a non-present page that happens to use the extened info. This leaves a 12-bit index to blocks of 4K, which is not really enough.

Actually, the SFT cannot be more than 256 entries long, so only 8 are needed. This leaves a 20-bit value, which allows for full access to a 4G file, which is the FAT limit.

## SV86 Continued

I will use the simpler SV86 method.

There will of course be the changes I wrote down earlier.

Consider it a recursive algorithm. It will probably not recurse more than 3 times, and there is no need to replicate the register dump. It is now part of the thread block.

Once again, my editor is getting painfully slow. Do I have to switch IDE now? I would rather not.

I am tempted to try and roll my own IDE and that would not be too hard if I had an actual IDE to write it with without this annoying issue.

It seems restarting the app clears the problem for a time.

## Memory Manager

Right now I think of memory as being three zones:
- DMA addressable
- Extra memory (up to ~1GB)
- Page file

File-backed storage is used for the page file. OS/90 does not use literal swapping.

The way it was originally supposed to work is that there is a demand paging buffer that is continually mapped to where recent accesses have taken place. We obviously cannot access the disk directly.

Can I use a different storage format for the pagefile? Some OS'es have a map for accessing regions inside it to avoid making it too large.

A swap file being too large is bad because it makes accesses slower. We use the filesystem API and each access is non-temporal. The FAT has to be used to locate offsets in the file.

It is probably not that big of a deal.

My oldest concept of swapping, which is correctly called that, was to have a system where we copy some number of pages and store some information about where to locate it. There were MCBs that had indices so that entire entries could be removed but replaced with newly allocated blocks after eviction.

Is there some way I can completely avoid virtual memory and do it later?

Not really. It needs to be a consideration.

> DPMI has demand paging support and also the ability to destroy an individual page, which is tantamount to deallocating it. Whatever DPMI does, I have to do.

## GUI Discussion

I am quite certain about designing a GUI. Other operating systems have done it, and it is not actually that much more complicated than a TUI.

There is just difficulty with the planar projection used by the VGA controller.

## New Approach To Development

I need to get back to the kernel and use a linear approach now. Very little has been done for two whole months.

## Logging

- Logging will now use a mask so that there can be multiple targets.
- Logging will use more user friendly format sequences similar to printf but with a different fmt char.
- Logging will be line-buffered and entries are stored in a truncated format.

The log buffer needs to be able to rotate as things are printed in case of overflow. That or dump it to the disk.

There can be several logs.

- Boot log: small log buffer that stays in memory and is entry-trunctated. Deallocated after boot and written to a file. We check if the file is sufficiently sized before trying to resize it. "Boot" is considered the process where all drivers and the kernel are loaded.
    - The boot log it written to the screen
    - Lines are 80-characters long
    - Goes to BOOT_LOG.TXT in root directory

- Debug log: written to emulator-specific output port, and dumped to a file when it runs out. Ignored on production builds.

The way I can do it is:
- Several distinct log levels
- Combinable log target mask options

The options can be:
- Boot log and text display (not available after boot is finished)
- Emulation debug log (the typical port E9)
- Runtime log

This is for the kernel only of course. Userspace will have its own logging based on fprintf.

I can also make it simpler by making the logger write entries to a fixed buffer and have a callback for commit to disk if needed.

The boot console can be detatched after boot.

> Consider using 80x50 mode for maximum information.

There should be actual log levels though. One would be user advice, or a suggested action the user should take, pehaps based on a heuristic configuration check.

## Keyboard Driver

I found a perfectly listed scan code set which also describes HID too. It comes from Microsoft so it should be reliable.

Also, scan codes are NOT different when modifier keys are used under scan code set 2, which is the more common one.

# June 22, 2025

## COM and LPT may be used by real mode

There is the possibility that a real mode driver is controlling the COM or LPT port. It can provide whatever API it wants. COM could be used by a modem driver that implements the packet driver interface.

This is rare for COM but very common for LPT. Printer drivers exist for almost anything on DOS.

This is basically impossible to detect. I may be able to know if the LPT or COM ports are connected but that doesn't mean the user wants to use them in real mode.

## Change Naming Conventions?

I think I can keep pascal case.

Or I can try Capitalized_snake_case. This is used for classes in JSF++.


I like being able to distinguish between API calls and local/inline functions. When I look at pascal-cased functions, I always think of some popular API like for Windows.

I also think Hungarian notation but for logical rather than actual types may be a good idea and could make avoiding bugs easier.

Let's try some examples:
```
Segment_set_limit
Set_irq_mask
```

Snake case is harder to write but way easier to read. Capitalization of the first letter does aid in distinguishing it as a function.

I think each major element of the code should have its own style.

Honestly, this is just a sideshow that slows down progress for no real reason.

I do think local functions should be snake_cased because it distinguishes them from public and API functions.

The OS will have very few public and non-API functions, so I am not worried about confusing them with API functions.

## Brackets and pointer symbol

Pointers in structs and multitline argument lists should have it directly after the type because that is how arguments actually work, and structs need to have tab alignment and easily-readable types. I also do one declaration per line too.

Variables will be written the normal way because that is what I am used to.

### Brackets

Brackets on the same line is by far the most readable. It just consumes many lines on long procedures. I will gradually convert my code to that.

I have already been using brackets on the same line on loops for a while now.

## Agenda for today

I will work on the logger.

BTW using the lookup table is not a bad idea. Other things will take much more space. Plus, it increases boot speed. Printing integers is very common.

> I am doing this

## Note about context switching.

The time slice length is 1MS, which means it runs 1000 times every second.

Suppose it takes 1000 clocks to perform a switch. That means the CPU will spend 1 million clocks doing zero useful work.

So my attempts at speeding up switches is warranted.

# July 24, 2025

> Use a process table and not the current approach? It may actually simplify context switching!
> OS/90 TODO: GE for configuration of devices ended or something, or GE_LEGACY_CFG_END. Or maybe a GE for DOS memory management activated, in case a driver wants a DMA buffer.

> Also, how will translations access memory? I want to avoid wasteful copying.
> By that I mean real mode calling a PM service. And no, copying IS required.

## Context switching

1MS is a very small time slice and will be hard for the 386. The 486 should be able to handle it. I think time slices were usually in 5MS granularity on Windows, maybe more.

It is hard to count how long a context switch takes, but an INT from ring-3 takes 99 to enter ring-0 and 82 to return. I used to think it took 300 to switch rings, but that is not true.

Context switching happens 1000 times every second.


## Order of Calling Hooks

Implementing extensions is difficult if it is the first thing in the chain. It means that extending INT 13H before sending it to a 32-bit disk driver is not possible.

By extending, what do I mean? Data can come in using any addressing mechanism. Converting to real mode format to run SV86 is something that only happens as a last resort.

Hooks can just implement or use a feature that checks the context of the process and uses the correct method of converting the data into a pointer.

So the order of hooks should actually be in order of most recently added, so that additional drivers can extend or take over hooks made by other drivers.

There may be a need for checking if a hook exists though, for rubustness. This can also be hacked in using GE.

## Bus driver configuration system

Bus drivers must somehow be loaded first, before any subordinate driver. The busdev drivers can even be loaded automatically, but I would prefer to avoid something like this.

ISA, PCI, and other buses are allowed to have static resources that cannot be reconfigured on any of their devices.

If there is a conflict, this is an error that the user must be informed about. One of the devices must also be uninstalled.

Some resources are reserved at the boot level. ESCD is not used for this purpose. Any collision with one of those means an immediate removal of a PnP device from the bus. This is supported by all buses.

ISA PnP and PCI both allow the removal of devices so that they do not respond to any transfers.

> I should really make a separate driver for PnP. Or I can allow drivers to be linked into the kernel at build time.

The problem is changes in the configuration, which must be detected by bus drivers. The PnP BIOS is a bus in itself, and it is capable of changes directed by the BIOS, and so is every other bus on the system.

The bus drivers must detect that the bus has been changed. If a deviation exist between the current configuration and the one saved to the disk, this requires every bus driver to know a total reconfiguration is needed.

I can also have the kernel deal with it automatically by managing the entire configuration. All allocations of resources can be compared against the kernel.

## Not a good idea

I am quite sure I need to do bus scans every time. Reading from the disk to get configurations is not fast.


## Logging

I do not think it is necessary to use width fields for the logger formats. int conversions are already very fast and hexadecimal can be made to work only with 32-bit values.

64-bit can be hacked in either way.

# July 26

## Virtual Memory Inquiry

I really need to think about how virtual memory will work in OS/90.

Hard disks in the 80's and 90's were incredibly slow. They also require seeking and rewinding, which adds additional delay, and programmed IO is used most of the time, which partially blocks the transfer.

Apparently drives back then required up to 29MS to seek to a different track.

This means the disk scheduler can actually be aware of physical disk geometry and schedule operations to occur on one single track and avoid the physical seeking. This would have to be user-configured.

This is more of a disk scheduling thing though.

The transfer rate of the disk inself, for the IDE disk I am looking at, is 1MB per second, which may or may not be PIO/DMA. I am not sure.

To be exact, I am looking at the CONNER: CP-342 42MB, which was actually made in 1989. so this is a good benchmark.

Disk speeds have actually not changed as much as storage size. The biggest jump was between HDD to SSD.

The Maxtor 7040A 40 MB had a transfer speed of even less, at about 0.8 MB/S.

So back then, storage was slow but RAM also was not that fast, so the gap was actually a bit smaller.

The memory bus of the Compaq Deskpro 386 was 8 MHz and synchronized with the CPU that ran at 16 MHz and needed 2 clocks to access memory. It was also 32-bit, which is important to note because ISA is also 8 MHz and much slower.

So the RAM is 8 times faster than the disk, and even faster when considering the very long seek time between tracks.

There are mid-90s disk drives that were way faster though, some reaching 10 MB per second using UDMA, but on such systems, the memory is also faster.

There was a certain point when RAM did not follow the CPU clock anymore due to the hardware challenges, but it is still massively faster than a magnetic drive regardless.

But the safe assumption is that the page file/swap space or anything else will be 8-16 times slower than the RAM on an old computer and the gap is much wider on newer ones.

> Pin this for later, it is good info.

## FreeCOM XMS Swapping

This is standard and they do not even distribute non-XMS versions. They all check for availability and try to swap parts of FreeCOM and even inactive programs onto extended memory to save CM.

This is VERY BAD for OS/90 because it has the tendency to overallocate.

Even the nonstandard interface for getting the XMS handle table does not report which program allocated it. Conventional memory can easily be reclaimed however, because MCB chains have PSPs and even process names. It is possible to delete anything with COMMAND.COM.

The only thing to consider is that the first COMMAND.COM program, which does not have a valid PSP as its parent program, is where we extract the environment from.

The kernel can use the environment as a safe and simple way to set options, instead of INI. I have to think about that later.

Anyway, it is impossible to free up the XMS swap used by FreeCOM.

With 12,288K a total of 157K is used presumably by FreeCOM.

This is totally unacceptable. 157K is a lot of memory that FreeCOM should not need.

Even the /low option does not work. I really need a way of blacklisting FreeCOM from using any XMS memory.

That means I need a driver loaded after XMS that checks the PSP, and if it finds COMMAND or COMMAND.COM, or any other indication that it is FreeCOM, it reports that no XMS exists.

It is there just to hook the appropriate interrupt and that is all.

> XMSBLIST.SYS

## How do we unhook vectors?

Hooking is done by taking the current handler and calling it when passing, and replacing it withthe new one.

Uninstalling a vector... IS IT EVEN POSSIBLE!!!!!

It actually is not possible to uninstall a V86 hook.

Anyway, I thought about this because DOS software uses it all the time.

But DOS drivers cannot be easily unloaded.

The current callback return method works for stacking the hooks, but there is no practical way to fully remove.

> Besides switching to a linked list, I can also add identifiers to the hooks to know what driver is using it.

## XMS Blacklist

> Can I use VME? OS/2 did so.

## Changes to hooking

A linked list is required to make removals possible.

It only needs to be singly-linked because removal in place is less common. So a next pointer and a callback.

## Can VME be used?

VME is introduced on later 486 and some 586 processors. This was not officially documented, but OS/2 3.0 supposedly used it in 1994 to avoid freezing whenever programs disabled interrupts, because it actually masked all of them instead of virtualizing it.

VME allowed OS/2 to simulate CLI/STI using the VIF instead.

It also allows INT calls to be directed straight to the IVT! This allows unhooked calls to be passed to DOS with zero ring-switch overhead if I decide to change SV86 to be thread-local.

The problem is that the IVT is not actually localized per process and the intent is to avoid using it entirely.

The IVT and the BDA are less than 4K in size. The load address of the kernel and other modules is undefined and unknown, and other modules could be using memory between.

Copying 1024 bytes for each process (not task) is inefficient, even if 256 DWORD values.

This means we actually need shims somewhere in the real mode addressing range in order to make this feasible AND also somehow assert the real mode lock, since we would be doing it that way.

I would have to allow real mode multitasking, with thread safety of course. Only one thread can be in elevated real mode, and others must wait

Elevated real mode has other complexities, such as somehow changing the current IOPL to ring-3 so that real mode code works correctly.

> BTW, it is true that VME is used by OS/2. Some people report errors on certain CPUs that implement it wrong.

VME should serve as a simple enhancement to an existing process. The OS has the ability to simulate VME or have it physically, pretty much, but at the interface level.

## Elevated real mode

When a program calls INT or similar opcodes, it causes a fault and goes to a dispatcher.

At this level, there are several checks that must be done which have caused performance and complexity concerns. First, we check if there is a handler available for the mode it is currently running in, based on the rules of DPMI.

I actually considered using hot patches into the running program to enter the PM kernel in the fastest way possible based on the current mode.

This actually is not impossible, but may require a fallback mechanism.

Might be another diversion, but if it works, it could improve performance. I think VMM did this too.

> Remember to add creation of mutexes.
> Also, some threads should not be allowed to access the first 1M so null pointers are invalid. That would be userspace.

# July 28

I actually did consider live patching.

A better idea for doing it is to handle only the first 128 vectors (enough for anybody) and have some backup plan for the rest.

Then I use the top bit to transparently encode real mode/protected mode.

This is problematic if the program needs to run code that is for 16-bit protected mode and then switch to real mode, maybe using the same segments.

It seems unlikely. Very few programs even used 16-bit PM in DPMI.

I highly doubt that a program ever needed to run the same exact code in different modes. Even if it used dual-mode code, it could easily decide the right mode to use.

The problem is that this is actually a destructive operation and that it might actually be trying to call that interrupt in question rather than some fast entry mechanism.

I could instead map a range of common vectors to some set that I reserve and prevent programs from using, after careful analysis of RBIL.

Then if INT 21H is called, the program gets live patched at that location so that it calls an exceptionless ring transfer. 21H and 2FH are the most common, along with some of the BIOS ones.

The improvement is that an exception is not triggered, and we can tag what mode we are in to avoid additional checks.

DPMI mandates that when in real mode, all IRQs and INTs go to real mode, which can be done with a live-patched vector that implies the mode it is in.

## Subprograms

Subprograms are a complicated problem. What I can do is create new threads for them and stall the parent, and also create a full DOS program context.

A subprogram requires a PSP and aside from that is just some memory with code.

The issue with making it have a separate address space is that while it greatly increases available CM, it also uses a lot of XM, often much more than is needed. UCM could help with this, if I can have it work for under 1M.

The other problem is that some programs used INT vectors to communicate information, which would cause catastrophic failure for any program on OS/90 if this is done.

The idea however is not entirely bad, and can be improved, but it does make it impossible for subprograms to communicate with the parent program.

So it should at least be an option. Consider a compiler being executed from an IDE as an example. Then it would be useful because the compiler does not talk to the IDE.

Programs did use space optimizations to move as much memory as possible into XMS so that subprograms could run. I know that's what FreeCOM does.

For truly valid behavior, a still have to properly simulate subprograms. That means there previous pointer should be valid or null.

There is also the JFT which has to either be copied from the parent.

It can be done though, and memory has to be allocated for it.

The JFT is supposed to be shared by all programs to avoid wasting memory. It only needs 256 bytes or less depending on how many handles are supported, but a multitasking OS will need many more than the default.

So that is not really an issue.

> This is very important to do. RBIL is filled with vectors hooked by regular programs. These may be shared with subprograms, so it really is something to think about.

# July 29

## XMS Blacklist

This has to be a DOS driver and it must load after XMS.

It will detect the presence of command.com and report no XMS.

The way it will work is that it will store the PSP and the handle as one record, allowing

> When the program call something that has a single internal state, should there be exclusive access?
> Also when drawing, it is enough to add to pitch down in a loop

## Swapping

Swapping is quite complicated and there are lots of ways it can be optimized and implemented.

On Win386, it is not even part of the kernel. Microkernels also do this, but using userspace IPC, which is not actually tha

Consider the DPMI function that marks a page as a demand paging candidate. There is also the discard one, which can be used in many places.


> Programs can enter exclusive tasking and the IRQ#0 handler can actually be swapped out for a different one.
> Wait can I actually do this in general? Change the IRQ handler dynamically?

## Global State in the DOS Kernel is a problem!

Things like extended errors, the list of lists, and various other things make program malfunctions and catastrophic failure much more likely.

People on stack overflow have reported the higher chance of glitches and errors when multitasking DOS programs in Windows 3.x and 9x.

The extended error is impossible unless I do the extremely slow and hacky solution of hooking every INT 21h call that returns an error code and calling the get extended error state so I can preserve it in a thread-local context.

Consider that there is an undocumented function that gets the DS of the kernel.

FreeDOS only needs 46,256 for the kernel, and

> I should add a GE for when it is safe to allocate a real mode DMA buffer. It can take drivers some time to know how much they need.
> The physical conventional memory can be identity mapped for one program to make maximum use of the RAM.

I can also go for a more OS/2 approach and build DOS VMs as a real virtualization interface. I can decide which DOS calls are supported and translate them to a safe call to physical DOS.

It may not even cost that much in RAM and storage, but adds major robustness.

In general, if we do not control an interface in a safe way, anything could happen due to potential internal state, which the whole replicated DOSSEG thing aims to avoid.

These two models are not quite compatible with each other.

> I would have to simulate some calls on the VMs behalf to set up the internal state of DOS for program execution.

I think it is better and more flexible to do the duplication thing. I thought it was insane prior, but it is 100% workable.

The BIOS ROM may be an issue, but is not a deal-breaker.

## The Instance State

- Initialization data (allocated using specialized MCB chain)
- DOS Data Segment
- The INDOS semaphore (most likely in DOSSEG)
- List of lists
- BIOS data area (slightly special)
- Interrupt vector table (also special)

The idea is that each VM creates a full instance of the modifiable data components used by DOS, while code/mixed components are shared globally.

The instance state is copied into a template snapshot created soon after startup and is partially modified as needed to support MCBs and other things.

The BDA is a challenge and many programs actually use it to get certain information, such as the port addresses of COM/LPT and other things.

It also contains global state that requires specialized emulation, such as the caps-lock state and the keyboard buffer, which are impossible to realistically duplicate across VMs.

The keyboard driver would not deal with the BIOS at all, but maybe it could somehow handle it. Probably not because of the scan code used by default.

I should write some code with Open Watcom that tests some of this stuff, but the idea is not actually that bad.

> Duplicating DOS data makes VME useful on Pentium processors, because reentrnacy is less of a problem
> The problem is that drivers are not actually duplicated. I wonder if they should be.

## Duplicate drivers?

Drivers have to access physical resources which require arbitration and provide no real benefit when doing DDR (DOS data replication) since only one can be active at a time.

Drivers also have data segments, so duplication is kind of required.

So basically: some 64k-200k of memory is always duplicated?

The advantage is more accurate emulation and easier use of VME.

And BTW it is possible to unregister drivers by manipulating the linked list in the headers. I may not actually need HIMEM anymore after boot, which saves 6K of memory, if that matters. Very few drivers will be needed, and a 32-bit ATAPI driver can easily delete the memory used by MSCDEX or equivilent.

Keep in mind that MSCDEX is a TSR and it actually creates a network drive using the redirector.

## The other model

I can also have a hooking layer which properly handles everything that matters and acts like an actual VM monitor.

Plus the thing about the extended error is not actually that important. We can just call the real function as part of a hook. This is only needed for errors, and not otherwise.

## How It Would Work

Unless a function code is explicitly supported, it cannot be called by a program, whether it has other handlers or not.

This the the "VM" approach. Implement a layer on top of the DOS API.

The DOS kernel does everything in 40k of memory, and 32-bit programs are actually not that much bigger, so a simple layer cannot be that much of a problem.

As far as ASE goes, it only needs to happen at the end of the chain for functions that need it and in that particular situation when we are in protected mode.

Extensions when fully controlling an interface happen by converting the segment and offset into a linear address and possibly validating counts. There are instructions that make this simpler, such as VERR, VERW and LSL.

There may need to be additional validation in case there is a buffer overflow, but I would not bother with this unless it is something important like disk/file access which could cause data loss if a crash were to happen.

The extended error state will be thread local and when passing to DOS, and can be set after a hooked and consumed function like `DSS_SetExtError`.

## Array of function code info

INT 21H can have an array to get information about function codes using bit flags.

- FC_SUPPORTED      Is the function supported for multitasking DOS sessions?
                    Program is forcefully terminated if not.

- FC_SEND2DOS       Can this be sent to DOS at the end of the chain
                    or does it need to run a special routine?

- FC_ASE_EXT_DS_DX  DS:(E)DX is used as the address.

DOS usually uses DS:DX for addresses but there are exceptions.

Anyway, this is just some ideas. The actual central dispatcher will be very different.

To avoid code duplication, I considered that DS:(E)DX addressing is standard with exceptions.

I may want to futher group the functions into defined types based on the kinds of parameters they take.

For example,

the read/write group:
- DS:(E)DX for buffer, ECX for buffer size

the keyboard input:
- DS:(E)DX is the buffer, and the length is encoded inside the structure.

register-based:
- The majority

variable-length string (null or $):
- AH=9
- open file with ASCIIZ name

Some are strange but also uncommon enough to special case on their own.

Remember that buffers in the DOS meaning have segments, offsets, and sizes, all of which are extended for protected mode.

Exceptions to the rules can be switch/cased or if-ed out.

This all will require a full listing of some sort.

Also, whether or not it has an error code is another thing.

# July 31

## PCI Bus scans by user software?

I wonder if some games do PCI bus scans to detect graphics cards and other devices.

It is theoretically possible to do the equivalent of PCI passthrough to a DOS session. The device can then be released after the program finishes.

This is a bus-specific feature that I would not make available to other buses as part of any interface, but there is nothing stopping PCI.DRV from trapping configuration register access.

As long as nothing is modified, there are no problems.

Consider that there is the Oak CDROM driver, which does scan the bus. It does not modify anything most likely, which is acceptable.

# August 1

## Load kernel in HMA

This is really disappointing in a way because I will have to retire my current bootloader.

But I cannot really argue against it. The only problem is limiting the kernel to 64K of memory.

However, it ensures I make maximal use of the HMA rather than allocating the whole thing or whatever.

I will still have the kernel in the higher half for very obvious reasons. The reason being that I do not really want the kernel being directly accessible by V86 processes.

That is something I can consider though, because sharing the RM addressible range means slightly streamlined communication with real mode.

> BTW whatever I have for creating the segment descriptors is totally dumb and needs to be size-optimized.

Some pages have to be ring-0 or everything will obviously not work, so I am not really conerned.

I think HMA load fullfills the essential features of a higher-half kernel without not being higher-half. It is always mapped and the address range is reserved.

ATM I am not even considering putting drivers in their own address space. They will just get ring-0 pages from the same virtual address pool as userspace.

The only downside is the inability to know if a pointer is ring-0 or ring-3, but this can be checked correctly by comparing against the page tables.

Not a bad idea, really. It limits the kernel size, but the boot process and memory detection is massively streamlined.

The only issue is the GDT and LDT which have to be ready right away.

I think the LDT can be dynamically resizable with a simple malloc. It does not event have to be available right away, and only user programs need it. I would designate it as part of the DPMI/DOS subsystem.

The IDT and GDT are essential and will be a bit large. The IDT is 8K and all 256 vectors are required in order to get the correct index and place the IRQ vectors at a safe place.

BSS cannot be in the kernel though, and I also do not think that write protecting the kernel image will work either.

BSS can be mapped to a different virtual address for safety reasons and to use less HMA space.

The sections will be the usual code and data.

BTW the BSS should not be in conventional memory, although the IDT may have to be for size reasons.

### HMA Load Continued

Switching to real mode for IRQ reflection is a bit easier. It is also possible to insert all structures necessary for VME shims (if I decide to do that) in the binary itself.

# August 2, 2025

The BSS section is a challenge. Normally, BIOS ROMs just zero out data like this in the image because they have to be self-contained.

The amount of BSS space that needs to be allocated is a matter of programming practices. If it must take up space, I should take measures to avoid having to auto-generate things.

There are things that unfortunately take up a lot of space.

- IVT: 8192
- GDT: 64
- V86 chain array: 1024
- Init PTables and PD: 8192

The chains can take up more if I try to have a stub for redirection, but this is probably not needed.

Only 1 page table is needed because everything can be mapped in the first 4M. PDir remains the same.

So in total, 17,472 are completely unavailable.

## Alternative

I wonder if I could actually not HMA load at all, do the same thing I am currently doing, but not depend on the HMA for the init page tables and instead generate them in real mode memory. The kernel can replace them once the memory manager takes control.

Then we can use DOS=HIGH,NOUMB to save as much conventional memory as possible.

The modifications to the bootloader are simple enough. Generate the page tables in the program address range (three still needed), calculate their physical addresses and set them, and then do the rest the same way.

This is not that easy but is done for the GDT, so this must happen.

## What to do with the HMA after

DOS=HIGH,NOUMB will be the standard setup. The whole HMA wis expected to be used.

## Changes to reflection

Reflecting interrupts will have to be rewritten, unfortunately. It currently assumes its position, and this may not actually be possible.

## New Idea: EMM386 compatibility

If I can hijack EMM386, there can be support for loading DOS drivers in a UMB.

There are different options:
- Use a VCPI-compatible EMM and enter protected mode
    - Then I do some detection on the page tables to find out where the EMBs are mapped
    - It should be possible
- Require the use of a GEMMIS EMM like that of Microsoft

I will probably use jemmex instead. It is open source, so I can package it with OS/90.

## jemmex

It can be loaded and unloaded at any time, and requires that no XMS driver is currently installed. It is much more managable that anything else.

As long as I disable DMA buffering, it should be a net gain.

On FreeDOS, a jemmex configuration with COMMAND moved partly to extended memory, 500K are available for programs.

This does not include DOS=HIGH.

I was able to get it to have 620K available! The kernel also allocated some disk buffers (the BUFFERS directive) in the UMBs too.

This is really quite important because it makes maximum use of conventional memory for multitasking DOS sessions.

Disk buffers don't need that much memory though. I would rather load drivers there if possible.

I will probably need to develop a setup program for all this.

## How to do this

I will need to reconfigure the bootloader substantially. Most code cannot really be reused.

First of all, I have to use VCPI to switch to protected mode. Then I have to use its services to allocate enough memory for the kernel or use XMS (either is fine)

> btw jemmex is only 30K in size. It has XMS and EMS in one driver
> It has to be DEVICE loaded or it

There are different ways to get memory. In this sceme, everything is handled by one single support driver.

VCPI actually makes allocation easy, because it allows allocating any number of pages from extended memory (all of them actually).

The allocation must be done by repeatedly calling a function to map a free page to a virtual address. Each one can fail.

The idea is I do a loop where I allocate and map pages to 0xC0000000.

Copying the kernel is difficult though because I cannot access the memory directly.

## Actually that is wrong

It only returns the physical address and does not map the memory. That has to be put in the page tables into a special mode switch structure.

VCPI just won't work. XMS is available, so just use that ATP.

## The changes to the loader

VCPI must be used to perform the mode switch. I also need to generate the page tables differently so that it uses enough memory.

It is really sad, but I may have to scrap the current bootloader. It can be used for other purposes.

> Or I can use UMBs in the kernel and load any drivers after boot.

Not all drivers work on the UMB, btw.

DOS allows literally anything to go in the UMBs.
- Buffers
- LASTDRIVE drive tables
- Certain drivers
- Stacks

It is something that must be done at startup.

There can be several UMBs.

Anyway, I should really recognize the importance of all this. Putting things high saves a lot of space for DOS programs.

jemmex should have an API entry point so I can get the information. It is really good at getting UMB locations and detecting memory.

> I don't need mtools to build on macOS. I can just mount it.
> Linux can do this too, but a partition offset is required. I already know this information and it can be obtained.

## Better configuration

UMBs don't seem to work correctly unless DOS=HIGH,UMB is used. This means the HMA is theoretically possible to use, but it is also used by buffers.

If I want to salvage some code, it really needs to be available.


## Information about pusha

Apparently this instruction executed in 5 clocks on the pentium and 11 on the 486.

Okay, but the 486 and pentium are also pipelined and the pipeline gets flushed on an interrupt.

pushad will still require 4 clocks overhead, so it is not actually that fast on either.

But also consider that this way before they really optimized the stack operations, and push/pop caused an interlock.

https://people.computing.clemson.edu/~mark/330/colwell/case_486.html

There is some information there about the pipeline here.

The pipeline used by the i486 is not actually a classic RISC one.

It goes:
- Fetch
- Decode 1
- Decode 2
- Execute
- Register writeback

The clocks in the manual are probably referring to the execute phase, which iis the best case scenario of how long it will take.

So less instructions is actually GOOD for the 486.

On the other hand, a set of push instructions requires an address calculation at D2, and may require waiting for the ESP to change. This means each push will take 3 clocks instead of one.

I highly doubt that register writeback needs to happen before the ESP is accessible. That would have been a terrible decision.

Anyway, to sum it up, pushad should be faster than a series of pushes on the 486.

The pentium has a different pipeline, but it is extremely fast.

Point is that interrupts can use pushad and popad.

Not quite. popad is extremely slow on the 386, but pops are just slow anyway.

# August 1

I need to find the Jemmex API entry point.

> I may really need to support Virtual DMA Services, on top of emulating the DMA controller for programs.

https://github.com/Baron-von-Riedesel/Jemm/blob/5f902082b4fd222ae2d47a92118e11236084c17f/src/JEMM.INC#L100

This seems to have some of the function codes. Both are supposed to use a special file name, but Jemmex seems to use IOCTL codes.

Here is some official information on this:
https://www.lo-tech.co.uk/wiki/LIM_Expanded_Memory_Specification_V4:_Appendix_B

IDK they make it really difficult and a very long procedure is detailed to access some simple data. Plus the format of it is not even standardized and jemmex uses a different format.

## Different approach

Once I enter ring-0, I am basically in control, and can perform reclaimation of memory at some point.

Somehow I need to enumerate the UMBs and take control of them. The bootloader can fullfill this requirement.

It's really a total mess. XMS needs to be eliminated eventually and full control must be taken as some point of memory. Eventually XMS calls will be illegal.

I REALLY need to enumerate all the things that need to be loaded high. Jemmex provides useful services but makes it all difficult with the IOCTL interface.

The main purpose of this EMM device string is to confirm installation.

> I can actually call programs like jemmex.exe or mem to enumerate memory regions. They can be copied to a file and then the kernel can scan the data

> I might as well support GEMMIS now.

> Is this all even necessary? I can allocate UMBs after startup easily. Allocations and programs can even allocate this memory transparently and I can also have real mode translations to avoid the pointer normalization of DOS.

BTW there were computers with special patches that allowed for a non-compatible 720K mode, all transparently.

## Better Idea?

A better idea is to not offer UMBs since programs that use them probably handle XMS too.

I have considered this a while ago, but here is the idea. Transparently allocate memory in the upper memory area!

This allows programs to access up to 630K of memory without needing to do all that EMS stuff.

The only thing I have to confirm is that DOS does not do normalization of pointers in the UMA. If it does I have to perform translation.

> Try printing hello world at a location in the HMA.

The memory has to use MCBs but this is completely doable. A text mode only program should be able to use 98,304 bytes underneath the mode 3 text buffer.

It means programs can actually get more memory than 640K under unusual configurations.

This achieves the same thing as whatever else I was planning to do, but without the complexity. The working space is still extended to the full theoretical maximum.

Segment C000 is where the video BIOS usually is. Some BIOSes require more than the usual 4K though and only E820 can detect this.

Anyway, the memory available in the UMA to map is 0xA0000-0xB7FFF. The text buffer only needs 8K and maybe more for some unusual text modes.

0xBA000-0xC0000 is 24,576 bytes.

In total, we can reliably allocate 122,880 bytes transparently, which is larger than any typical DOS configuration.

Even with only 327,680 (half) of conventional memory left, this gives a total of 450,560.

I tested it and the HMA is 100% accessible. I printed hello world from the HMA and it worked!

I am not sure if file IO would work, especially if the BIOS is involved. DOS should use its own buffers for IO though.

## Text Mode Interface?

> libcaca?

I found a nice font that allows for 106x40 text graphics. This may be doable.
https://int10h.org/oldschool-pc-fonts/fontlist/font?dos-v_re_jpn12
There is also a 7x15 one, but the M looks really bad on it.

With 640x480, I can display 106x40, which is better than the default DOS, and still maintain readability.

The difficulty will be writing the code that can blit it onto the screen despite the poor alignment.

There will be an issue with how much UMA space I get on the main process, but it won't matter at all.

Graphical modes will probably be recommended or standard because the cursor is easier to support.

## Fast Drawing?

Drawing at 6-pixel boundaries with that font will be difficult to optimize, and it negates the predictable boundaries advantage of a typical TUI.

There are actually even smaller fonts at 6x8! I probably won't take it that far though.

Anyway, the idea is we have to fetch bit lines for the glyphs and also render the background colors too.

So yeah, text is extremely suboptimal. But VGA does not support glyphs with less than 8 columns.

Well, there is TM.EXE and I have no idea what magic it does, but it supports 90x80 text mode (not 4:3 so don't use). I wonder if it works on real hardware.

TM.EXE works in 86Box, which is a more accurate emulator. It should be possible to do modesetting with it, but this software is unfortunately freeware.

I can do some execution tracing and reverse engineer a mode set to a 4:3 with 90 columns. It has to be carefully calculated though, so that the font does not get truncated and is either 8x16 or 8x8.

## Info on UMA

The video BIOS is not actually needed most of the time. Most of the operations do not use memory addresses and are entirely register based. In the event that they do, it is rare enough that copying to some other location to make an INT 10H call for one program is enough.

Several INT 10H functions are controlled by the supervisor for performance reasons. The rest can be accessed using this method.

Using this hack, I can guarantee that modern computers with larger VGA ROMs can work correctly and free 32K more memory

That leaves us at 152K which is added to the conventional memory. The user can use this much under DOS and still have 639K of memory.

It is also entirely possible to have MORE memory that is normally allowed. This means programs like MEM can actually fail if they assume there can only be 640K or that this memory must be in the low region only.

Time will tell, and if it is a problem, I can detect MEM.EXE and direct it to something better.

# August 4

## The DOS Configuration format

- Transparent Upper Memory Area [AUTOUMA]
- Allow more than 640K of conventional memory if available [CMINCOMPAT]
    - Allows a single block to be larger too.
    - The 16-bit paragraph count 100% of the DOS memory to be addressed, so there should not be any problems. Programs may allocate 640K to get the amount that is free, and may not notice all the memory, but it is still more than could be available under plain DOS.
- Conventional memory size restrict (can be disabled) [CMSIZE]
- Maximum XMS handles (XMSHNDS)
- Extended memory pages accessible
- Extended memory pages lockable
- Extended memory pages pre-locked

The user-level options are:
- Can run in background window

Something like that. It could be specified in as struct or set of headers.

## Swapping or Demand Paging

The idea of having a demand paging buffer is not actually bad, although it is not as optimal for memory-mapped files.

The alternative to a buffer is removing data from memory to the swap and deallocating the memory. Instead, we can access the swap through a window, instead of having to create multiple "windows" and delete them too.

The buffer is a list of page frames that get mapped to wherever disk-backed memory is being accessed. The page frames can go to different threads too because multiple sectors can go in each.

The buffer size must be modifiable at run-time.

This will be the approach taken. It avoids reallocation, which will not be that fast.

## Kernel strategy

The strategy will need updates.

Here is a new one:

- Implement logging through port E9
-

> I can get even more memory for DOS if I use the HMA! The only problem is the A20 gate, but I already don't care about that.

## HMA For more memory?

The HMA is already being used by several things. However, there is no reason why page tables should be resident there permanently if programs are never supposed to access them.

The real mode IRQ reflection code 100% needs to be there.

> What ring were the pages??? Makes me wonder, can the CPU execute R3 pages in R0?

The reflector is reserved 8K of memory, and it has to reserve RAM for the IRQ handling.

However, DOS has its own stacks that it switches to upon interrupts, so I can probably reduce this to maybe 4K.

I could give 1K for the startup INIT stack. Then I need some memory for far calls.

Implementing far call interfaces will be done using the INT instruction to access the monitor. Each segment goes to an INT instruction, and the saved CS:IP when recieving a hooked INT call represent the far call in question.

That would be used to dispatch to some table, which does not have to be in the HMA at all.

1K is enough to provide 512 callbacks to all programs. I consider this to be sufficient.

Only about 6K really needs to be used. The rest can allocatable pages after reclaimation.

## UMA Strategy

Programs like MEM.EXE will probably not work correctly because they assume a standard DOS layout. I probably have to catch attempts to run MEM and use a custom one.

Will memory control blocks exist?

There is little reason to access the MCBs directly unless it is to implement something like MEM.EXE. Keep in mind that JEMMEX actually does not use an MCB-style allocation system for UMBs, which is actually incompatible with MS-DOS internals, but client software does not care.

> Instead, is uses XMS memory to contain the allocation information.

Most likely I will keep allocation information in high memory.

## DOS Functions Using Each Other?

EXEC for example needs to allocate memory.

So does this mean that DOS functions need to be implemented with a regular C function?

VMM32 did things like this. It had simulate calls which were done on the VM.

As for the file-related part of EXE loading, I am quite sure that programs access the same space of file handles and share the same mappings except for special handles.

```
Vdk_Alloc
Vdk_Free
Vdk_Read
```

There can be layers to it, to prevent having to theoretically have a program execute an INT out of band.

But some programs hook things, so there really is not much of a choice.

```
void StealthInt(DOSPROG* d, uint vector);
```

This will simulate an INT instruction on the behalf of a task, which will actually run the code in question as needed, but no modifications happen to the registers, hence the name Stealth.

Doing this without enhanced SV86 would be very strange, but completely possible. It is basically just SV86 but we use a local hook instead if available. SV86 only changes the thread local call buffer, not the actual saved user context.

I have not yet decided, but VME support is better that way.

Actually VME is not really good at all. It redirects to the LITERAL interrupt vector table. This is not a thread-local context. Unless I copy the real interrupt vector table.

The IVT and BDA cannot be mapped using a single page table.

## VME

VME was documented in Appendix H, which required signing an NDA. That is why IBM was able to add it to OS/2 Warp.

The fact it uses the IVT makes it infeasible, and the only thing it actually makes faster is INT calls which are not trapped by the monitor, which is almost none of ones that are commonly used. INT 21H will always fault.

The advantage is that the virtual interrupt flag is kept in EFLAGS. In fact, I can support VME just for that, and capture all INT calls.

It is also possible to have some slipstream method to add real mode modules into a program's address space, and use this kind of like how Linux uses VDSO.

Things like memory allocation of even text mode IO can be implemented inside a compact module, and provide enhanced performance under VME. I can have the kernel reduce the size of it by providing certain services.

Yeah, it is basically vDSO. Not a bad idea actually, but not useful for a 386.

And also, the IVT is still at address zero, so it does not help much.

I probably have to copy the IVT and add additional memory for the DOS machine context. The BDA also poses a major problem, but it is necessarily unsafe for multitasking, so if some unhooked function modifies it out-of-band, we are already screwed.

Actually, there is no problem with a global interrupt vector table. It already is global, and modifiable though not directly by clients. SV86 uses it currently.

But as stated before, the benefit is non-existent for most INT calls due to hooking, so the vDSO approach is the way to go.

## More on vDSO-style modules

The things I would insert into it would be primarily related to string and character output for general smoothness. Not every program is very efficient with character output.

Memory allocation could theoretically be there too, but I prefer to keep that in the kernel.

Overall, not that many opportunities. VME is not that useful.

But the idea of inserting modules into the local address space of a program is not too bad.

But keep in mind that text output is suboptimal in real mode. DOS will likely call INT 10H for each character. If the screen is not overflowed and there is no need for scrolling, the data can just be copied with a simple loop. DOS does not word wrap.

# August 5, 2025

VME should not be that hard to add. I will not think about it that much for now.

## Logger

The logger is the next component I will work on.

Remember the features:
- 80-character lines for log records.
- printf-like format options
- size specifiers too (maybe different)

```
trace("Hello, world" XWORD(value))
```

Why not do the implicit space concatentation?

## Memory manager allocator

I am thinking of using a tagged bit array structure where the population count is part of an entry.

The size of an entry is therefore arbitrary, because bit scanning is done with a 256-byte lookup table.

The difference is that we know if the entry is worth looking at or not.

110,592 bytes of memory can be represented in a 32-bit entry with 5-bit tag.

However, the 32-bit size is arbitrary and does not matter if it is larger.

It can be a structure of bytes instead, with the first one being special and containing the count.

6-bit count and 58-bit map means 237,568 bytes are covered by one counted entry.

Having too many may negate certain advantages though.

### Alternative way

Why not just allocate blocks contiguously until it is no longer possible, and bind them using linked lists?

Because an allocation binds virtual addresses to physical ones permanently, page table entries can convey the usual information but with some extra data.

Yeah, but the page tables have nothing to do with physical memory. So no, that is not true.

> Or group several pages together into variable length but limited size segments. Win16 did this on the 286 and it made swapping easier because every segment is no larger than 64K.
> No, just do the linked list thing. Just keep fragmentation in check.

### With linked list

This is WAY faster in the majority of cases. It completely avoids per-page allocation and is WAY faster due to best case complexity being based on the number of allocations (assuming to fragments) rather than pages, which is WAY better.

Based on how programs will usually access memory, this is the best.

The memory block structure of older designs can be repurposed and include a count field. The memory use will pretty much be exactly as before, but with speed enhancements.

Here will be some enhancements:
- Keep track of the top of memory at all times, or register it as a block itself like DOS does
- Count trapped page frames after each alloc/dealloc in that area

The second one is not really needed. But after freeing blocks, they can just be linked with each other to form a chain of free ones. So we don't even have to iterate through allocated ones. Right?

## SV86?

I demonstrated earlier that VME has very limited benefits, and this is the case for SV86 being process local.

VME has the ability to boost display updates of CPUs that are fast enough for that not to matter, and only on command line text programs!

So VME may not even be used at all. Remember that OS/2 used it but it also is its own operating system much more so than Windows or OS/90.

# August 6

## Strategy

I intend to make some changes to the kernel design:
- Improved linked list method of XM allocation
- Full support for multiple real mode address spaces.

Getting full DOS programs to does not have to be that far from now. A working scheduler and some way of controlling the active screen, like a really primitive desqview, could actually work.

The lack of a keyboard driver would mean I have to use INT 21H from the kernel, which is REALLY weird, but there is no real difference if I just stay in text mode.

(reading a key with no echo is supported)

I need a way to allocate memory suitable for the conventional area. The current idea for M_Alloc does not really work because it returns an address.

However, it can be used to allow for access to the process address space from different threads, so it can work.

The memory manager is not that complicated. I just need OOM killing capability and for the allocation to work as expected using most of what I already have.

I have no plans to change the existing API, maybe just a few updates and clarifications.

BTW supporting more than 14M of XM above the ISA hole is NOT of prime importance. It can be added later if I write good good.

MM is really holding me back from doing the DOS stuff, so I should finish up the specs for it soon.

Also, I need a scheduler whitepaper. I have some good ideas to make 1MS time slices viable.

## The scheduler idea

I can change the IRQ#0 handler in the IDT to a particular handler that does it based on the necessary switch action.

Right now the scheduler is all about avoiding the typical push/pop ISR handling and instead uses direct register transfers based on the current mode of the thread.

> The scheduler does not currently handle the low 1M. Each thread will copy this, but it is not thread-local, only process-local.

The switch action is based on where we are switching from and what is being switched to. Currently, it requires the use of a table to find a handler for that exact situation.

Because I control all attempts to change modes, it is possible to have tasks contain dependent information about the next one. The switch action can be set to depend on the next task.

And this switch action can take the form of an IRQ#0 handler optionally, although I am not so sure that is necessary.

Threads (called tasks in code) are in a circular linked lists and I control every insertion or deletion. It is a bit slower to create and delete tasks, but it can also be WAY faster to context switch, without the initial overhead, which may save time for time slice calculation which I have been struggling to fit into it.

```
S_SwitchToV86Mode
S_SwitchToProtMode
```

This may be private entirely. DOS APIs that are listed in RBIL seem to have a tendency toward spilling out all the internal details. VMM32 also does this.

There are advantages in early design to this, such as allowing for future flexibility, and  understanding how it all comes together without having to worry too much about abstraction.

Anyway, I do not think that functions for changing the switch action should be used.

That is quite literally all these functions do. For that reason, they should NEVER be exported or used by anything other than the kernel.

Tasks however cannot literally switch mode, that is turn VM on or off in EFLAGS, because doing so without updating the switch action would break context switching.

So NO, these functions do actually finalize a mode switch.

While we are in the kernel though, there is no concern with turning off interrupts except when changing the thread blocks. The user context can be modified and prepared for the switch and it won't be scheduled until leaving the kernel.



> Volatile for thread block?
> Consider .90 as a file extension for drivers

```
FAT.90

DIRCACHE.90
DSKCACHE.90
FIOCACHE.90

PCI.90
ISA.90

```

## Implementing switch actions

The switch code needs to save some common registers and perform time slice checking. We just increment a count and if it does not exceed the count, we can early out without unique code at all.

> I changed the regdump structure slightly. It should be the same size, but I used macros to shorten it. Verify the size btw.

The complexity is added when we start creating tasks. They can start in any mode, so that has to be part of the interface.

FYI, a task be in 3 different scheduler modes with a total of 9 switch actions

This means 9 procedures to implement.

3 modes means 2 bits to represent, although 2 bits can allows for 4 options ofc.

16-entry table can be used which just uses the value as a direct offset. Preferably it can even have the base added into it so we can just jump to it after doing the preliminary time slice logic.

## Strategy again

First of all, the kernel needs to compile and do something. All systems need to be tested, including SV86 to make sure I don't have any weird code rot.

YK it is actually quite tempting to put everything in reserve and start from stratch. But I will not.

Anyway, further development will be strictly linear and active. The OS must "run" somehow. I will also add things incrementally.

First of all, I need a logger.

# August 8, 2025

Do I even need time slices at all?

In the context of a UI, I can just have a background task and a foreground task. Some tasks can mark themselves as background so that their threads are suspended (IE cut out of the chain).

The OS will also yield when performing IO.

We will still have preemptive multitasking, but I am saying that there are many opportunities to do scheduling implicitly and with higher granularity than a time slice.

Most DOS programs for example literally just wait for IO unless they are command line apps.

> It is not really that safe to suspend a VM while it's in supervisor mode.
> This could cause deadlocks, because the thread could already be holding a mutex.
> There should be a wait to wait for the program to reach user mode before suspending it, although I wonder if that is safe also.

Good information to keep in mind. Terminating or even suspending a client is important to support.

I still don't think threads should be suspended for any reason. THey should just have priority dropped.

## VBE?

VBE used defined mode numbers up to 1994, and most video cards supported them even after.

I got a CL GD543X specsheet and it confirms the existence of the 1024x768 8bpp resolution.

# August 9

## Human interface and latency

Interrupts are kind of required, but if the CPU is under heavy use (such as running a fullscreen game) then it is not viable.

I have not really talked about how KM input actually will work that much.

My preferred workflow is:
- IRQ goes to driver
- Program which "controls the keyboard" is context switched to in the hopes that it polls the keyboard at some point and catches the event.
- This program then redirects the keyboard input to ANOTHER program, such as a virtual DOS session, or uses it internally.

I cannot allow the keyboard to be accessed directly, which is fine.

But I am concerned that if the CPU is maxed out, I need a way of recieving and dispatching keystrokes in all the various ways that DOS programs require.

But if a program is running in full screen, I think it should somehow be given priority so that it can recieve keystrokes quickly.

I am just thinking about how most operating systems can have thousands of threads in the background (can still be done with some changes) and have a totally responsive mouse cursor and minimum input lag.

I believe most games specifically poll the keyboard though. All games have a main loop which makes it look like things are going on at once. This means they will make a system call that gets what is on the keyboard.

I think I have the wrong picture of things. Keyboard IO is not some kind of "event" and it requires software polling, which is done by every program that needs input.

The IRQ handler queues each keystroke until the keys pressed are drained (usually at once) from the buffer and taken wherever else.

A game will repeatedly call the kernel and ask for the recent keystroke. That is if it does not directly control the keyboard, but that seems unlikely. The BIOS should be capable of non-blocking IO.

INT 16H AH=1 does exactly that. It returns in the zero flag

## Extended BIOS Data Area

This actually causes some major problems. 128K is the official size, which is HUGE. There is no specification of the size and if the system has no E820 or equivalent for accurate detection, there really is no way.

Actually, that is not really accurate. It has a header with a size. I should test this though.

> Note: I can call the conventional memory "base memory" which is closer to BIOS terminology and is shorter.

> BTW you DON'T yield when the software is polling for characters. This would cause pointless context switching. It is much better to let the program keep processing, it has other things to do.

## Tests on EBDA

I found a map of the header and I wrote a program which I tested on 86Box.

It is 1024 bytes on the Pentium BIOS, which is the minimum size. The segment also makes sense too.

The EBDA cannot be reclaimed like the VGABIOS though.

## Can the whole BIOS ROM be reclaimed?

First of all, the video BIOS may handle IRQs. There is no way to prove otherwise. In this case, there are no problem because we switch to real mode with no paging.

Theoretically, OS/90 should be able to provide a full 1M minus the usual things which cannot be used.

By that logic, even DOS can be mapped out and then we do translation. But this is WAY too much work.

But the BIOS is also extremely complex.

So bad idea.

> YK the "latch" thing can actually be used
> Write mode 1 does this. All reads full a 32-bit (maybe more) latch register, which can accelerate bitblitting.
> Writes under mode 1 store what is in the latch register.
> This bypasses the 16-bit data bus.
> It is faster on real hardware. Not sure if alignment is a factor though.

> Really though?

## Latch registers?

In theory, using the 32-bit internal bus of the VGA card should be an immediate improvement. The memory used by the VGA card is much slower than main memory. The was part of the reason for...

# August 10

I will not write any UI notes here anymore. But I will consider using the latch.

## Important note

The EBDA detection code was WRONG!

The segment returned is 1024 bytes below the end of the base memory. This makes no sense.

I know the memory detection call returns 639K but that is because it excludes the IVT because it was in the ROM originally.

Actually that is WRONG.

According to RBIL, it is the number of 1K blocks after 0000:0000. Which means it includes the IVT.

In fact, the MEM command in FreeDOS actually counts the IVT in the total when running `MEM /P /D`.

OSDev also confirms this.

The EBDA on most vintage computers is 1024 but Smo

> So no, it is 100% correct. The EBDA cuts into conventional memory, leaving the upper memory layout intact.

## More on upper memory

The VGA BIOS is not used that often, so it can be reclaimed to allow for more extended memory. (This requires VESA too)

> Random note: the EBDA may not actually exist at all. It was added on the PS/2 and older 386 system will not have it. A simple sanity check can confirm if it exist or not, or the conventional memory size function, which DOES include the IVT.

# August 16

## Alternative designs and the current one

OS/90 is currently headed toward being a DOS multiplexer at the fundamental level. It makes different programs think they are the only thing running, and passes requests through a single real program.

For stability reasons, I have to incrementally support things. If certain function calls are not supported from common APIs, errors may be generated.

My theory is:
- Some DOS/BIOS system calls are register-based and are like "library" calls. They can be made 32-bit for speed and are not dependent on anything like the PSP or files.
- Some calls do file/device IO and we have to impersonate the virtual DOS session. The real JFT is always the same and so is the real current PSP.
    - The get/set PSP interface is simulated for user programs and not when the kernel/SV86 call it.
- Functions that return extended error codes are "post-hooked" upon completion to check if they caused any error at all, and the error code is captured. It is a global context, sadly.

My idea is that everything else is 100% context-independent. Even if it does have global state, it won't matter because all requests are serialized/protected by the non-preemptive nature of SV86, and in the case of hooks, thread-safety is guaranteed.

I do worry about the execution of INT calls happening within a hook. It could theoretically cause a deadlock scenario.



### The other ideas

1. .SYS driver that bootstraps the entire OS from there.
2. Take over the entire boot process

They don't really provide any real advantages.

### Continuing

There are APIs that need process-local contexts, post-hooks, calling other INT calls, and other things. It is not as clean as I would like, but I chose this.

I will need much more detailed specs though. That means documenting INT 21H and INT 2Fh and explaining how it is handled.

## INT 21 U - DOS 3.0+ internal - GET ADDRESS OF DOS SWAPPABLE DATA AREA

AX = 5D06h

This thing is crazy!!!!

It returns exactly that, the DOS swappable data area. It literally has everything. More than anything in the List of Lists.

Current PSP, current DTA, driver stuff, LITERALLY EVERYTHING.

It is the full non-reentrant state of DOS. It is NOT the non-reentrant state of the drivers though, nor the BIOS.

That is fine. The goal is not reentrancy, but the ability to control the internal context of DOS efficiently. I do wonder if FreeDOS supports it though.

I also wonder how safe it is overall. It should cover basically every local context. Reentrancy is not the goal here at all. Global variables are still allowed, I just need to control the things that cannot be shared safely.

# August 17

## Analyzing the Swap Data Area More

I could in theory do DOS duplication and swap the entire region, but the size is extremely large, so this is not feasible.

Instead, it is acceptable enough to only touch the fields that matter.

This includes the current PSP, DTA, and many other things which do not really multitask well.

These can either be changed upon system calls, which makes sense because they are meant to store that sort of temporary information, or they can be written in and forget everything else. Only certain fields really matter.

I think the former is fine.

The structure has useful fields which can bypass having to call DOS for certain information. The current PSP is one example.

The PSP is a complicated issue. Windows uses PSP segments to represent Windows programs (no idea why). This actually allowed DOS to run out of memory even on Windows 95.

Windows gets/sets the current PSP. This is a bit of an issue, because we rely on swapping the address space to switch between DOS sessions.

The begining of the multitasking region is a PSP (which is swapped too) that represents the parent process of the VM.

I can either switch the current PSP to the current one local to the process or hook the calls and virtualize them.

Regardless, the PSP's are localized.

The list of lists or the swap data must convey the correct data.

But the LoL contains only things that basically never change in normal operations.

## Handling of PSP

Subprocesses get local PSPs which are set directly in the SDA before INT calls.

## Programs that access the SDA

This is required for SHARE. The best fix is to make it think this is totally not necessary. I cannot tolerate anything that tries to manipulate this data. And as stated before, it CANNOT be simply copied.

Well actually it can, even if it is not page-aligned nor granular.

It involves duplicating the unaligned or extraneous data into extended memory and including it as part of the local mapping. The data around it is included too and read/write.

Okay, but would this work for the BDA?

The BDA can also be virtualized, so long as the memory after it is not being used for memory allocations.

The problem with the BDA is that it kind of needs to exist. The BIOS WILL depend on it. Simulating it for programs is great, but it does need to be physically present for certain operations to work.

But in a way, the BDA is kind of the BIOS version of the SDA, although I have to be careful with the BIOS.

Okay, but the problem is that the data which is NOT being "hooked" cannot be directly accessed at all.

That data which is not being controlled needs to still be global in scope.

Using page not present does not work because there is no way to reset it.

> There has to be some way to get the size of the DOS data segment, in case I ever decide to do that.
> I thought I was not doing that?

# August 18

## Critical problem with DPMI approach

I considered doing subprocesses as entirely separate programs for a while, and that is for good reason.

I need to reconsider this.

A DPMI program can execute another DPMI program, and both start in real mode and go to protected mode. They also have instance information that is NOT shared, such as the protected mode interrupt vectors, virtual interrupt flag, and other things.

Far pointer callbacks and memory handles do not need to work this way, but others things do.

So a VM, session, or whatever I want to call it needs to have a concept of a subprocess or create new processes with some ability to share resources.

Multiple DOS address spaces makes this complicated.

A note on the IVT though. I do not think DPMI specifies this specifically, but the IVT may be an exception and different than the PM handlers.

# August 28

## Back to OS/90

I need to solve a vexing problem. The handling of drive letters for BIOS/DOS compatibility and protected mode extensibility is a challenge.

## How does DOS represent drives?

DOS returns a structure for drive parameters and it is passed by reference, so it can be modified if needed.

The AH=32h function should NOT be used because the LoL has it and 32h requires an immediate disk access, which is not desirable.

It contains a bunch of FAT parameter block things. It also has the driver header pointer.

This can be used to enumerate each drive and their drivers (which may be IO.SYS or the BIOS, somehow).

### A solution

One way to deal with drive letters is to prevent the drive letter from being used by any other block device.

This alone cannot work because each accessible disk is already using a blockdev driver.

So we have the ability to reclaim a drive letter and get rid of a driver entirely. This would also be non-reentrant and require a lock.

It is also necessary to tell the driver to flush all buffers and prepare to be disabled (there is an API for this provided by DRIVER.SYS).

There are also drivers which need to be reclaimed by name. For example, OAKCDROM or something like that.

Then there is the issue of a redirector, but technically this is not that bad.

MSCDEX is a TSR that implements the filesystem used by the CD and also provides the drive letter.

The way it works is still unknown.

## Reading DOS Internals...

I found out that DOS returns special key combinations with a null byte as an escape when using getch.

I also learned that a driver called CON is responsible for console IO.

Apparently this driver calls a Windows/386 specific 2Fh call to set the focus of keyboard IO to handle these key sequences.

I never really considered this. But the CON driver is still active and accessible, and it really does do complex input cooking worth looking at.

Most of the time we interact with console IO using standard handles or functions for that purpose.

But CON is technically a special device with an actual driver WITH A HEADER BTW and apparently needs to handle a multitasking environment as with windows.

I think it even has a file name. Chardevs also have driver names which carry significance.

CON is a reserved device. It works like `/dev/stdout`.

I am not sure this will be a major problem, but keyboard IO has to be handled in the cooked DOS manner. I do NOT know how that works at the moment.

This is a major TODO.

I found the source code from FreeDOS. It is written in assembly.

https://github.com/FDOS/kernel/blob/master/kernel/console.asm

Actually, I am not so sure this is so complicated. I think only CTRL-P has the special encoding, and these require special handling. That should be enough.

## Drive letters continued

There is a function for redirection which takes a pointer to a drive letter character with a colon after it, or null for other uses (automatic?).

# August 30

I made some very significant changes to the source code. First of all, source files are now captialized and mostly DOS-cased.

Also, the names of each module is changed with numeric order based on relevance or position on the overall software stack.

This happened a while ago, but the main OS/90 repository cannot be immediately used.

There are a few changes to be made to the "build system":
- Update to use the new naming
- Maybe change build.sh to run.sh, which I am doing in other projects

# August 31

I do not like how the current initialization code works...

NO. It barely takes up any space and uses procedures that the DOS subsystem will also use. No need to bother with that.

There is also no other way to deal with some things like interrupt vectors.

There is a problem though. INT 32 or INT 20H has an incorrect vector which is the DOS exit program call before AH=4Ch. It should be the range 0,31.

All other vectors except the IRQ one are to be assigned the fast system entry vector, which applies to all DOS calls but not the BIOS.

Anyway, I should also clearly state that I am very proud of my initialization code and I really like how I wrote it.

## DOS swappable data

I am really concerned about how SHARE.EXE uses the SDA. It does return an error code. I assume it is used to make it possible to cancel an operation by hooking an interrupt handler and catching some keyboard event.

Cannot really think of another reason why it would be used.

DOS also has a critical section flag which makes SDA swapping impossible, so if I decide to leverage it for safer multitasking, this has to be caught and count as a global preemption disable.

The SDA is part of the DOS data segment. It CAN be instanced and does not have the same problems as the BDA.

The SDA and any extraneous data can be paged as R/W data.

But the problem is that we have to deal with drivers which must be replicated as well.

Drivers can actually be destroyed and simulated if I feel like going that far. Using the SDA may improve stability but I do not want to duplicate driver data because a lot of it is redundant.

And at that point I might as well manually insert them into individual programs and let the user configure this. Something like ANSI.SYS can be added to a program that needs it while not wasting CM for each program.

That would be an improvement over Windows.

> This does complicate drive letters (or maybe simplify depending on how I handle it)
> How can I check if an INT call was made directly by a program or by DOS?
> This can be used to debug programs that use unsafe or undocumented calls.
> Can I save all registers for kernel calls? VMM uses this for calling conventions, and so does Linux and DOS for userspace. I know push/pop are not that fast, but they are also not that slow and have to be done anyway.

> Also, pusha/popa is not slow on old CPUs, and is good for code density.
> In fact, pushad is literally 5 clocks on the pentium, which is quite ideal. The 386 is a bit slower than manual pushing but on 486 it is worth it.
> But on the pentium pushing is also quite fast.

## The SDA

SHARE.EXE cannot be allowed to access the SDA. If it is accessed directly, it MUST fail, along with any attempt to access the DDS.

Critical sections are fine though and have to be controlled by the kernel. Windows consumes INT 2Ah by default so I will do the same.

Critical sections are used quite frequently by DOS, especially when calling the BIOS. Printing a character for example requires one because the OS is executing BIOS code which cannot possibly be made reentrant.

## Can non-page-granular regions be swapped safely?

Whether this can work or not in the context of the SDA requires some considerations of where the SDA may be and if the pages it is part of may overlap with other data we don't want it to overlap with. Some of this data may be accessed by other software such as the BIOS or drivers and not actually be instancable.

A quick not on instancing: it makes VME viable because DOS can be arbitrarily reentered. The nuance is that once again critical sections have to be trapped and disable preemption as well.

DOS has a way of allocating initialization memory.

What about things like buffers and drivers?

DOS uses things like this which are not at all reentrant-safe. That is why critical sections exist, and depending on the DOS version, they can be quite frequently used.

Those types of things CANNOT and SHOULD NOT be instanced. That is, the data inside non-instanced data should be preserved across task switches.

# September 1

DOS will always assert the critical section flag when it is required. This is done before calling BIOS functions for example, because those are 100% non-reentrant.

> BIOS code also usually disables interrupts entirely except when it explicitly yields. This allows BIOS code to be exectuted in an ISR but has obvious problems for system performance. The BIOS can obviously wait for a specific IRQ as well. Not required but commonly done.

I think I should clarify what the SDA exists for. It is intended to contain all global data used by DOS that is not directly accessible, which makes it non-reentrant.

Reentrance from an ISR can be done by swapping the SDA with a totally different one and DOS will basically use it like a long list of parameters and variables to do whatever operation.

The SDA could permit multitasking in theory.

It does NOT make VME that viable in fact, because even the simplest IO requires the assumption that INT 10H is non-reentrant and needs a critical section.

Critical sections basically say that software should not reenter DOS by changing the SDA because it is executing code that uses global data which is not included in the SDA, such as the BIOS and drivers.

DOS is very careful about this and real software has been using the features for a long time.

Drivers do not have to be replicated or anything like that because they are not accessed outside a critical section. Any IO will Use INT 2AH.

The purpose of using the SDA is not to make VME more viable. It cannot be of any use when it will have to assert the critical section flag most of the time.

The SDA is in the DOS data segment. It is not page aligned or page granular. The fact that uninstanced data can exist after this data means we somehow cannot replicate it and allow different threads to have different uninstanced data.

The SDA is also way too large to be copied and may even be multiple pages long.

Is copying extraneous data actually bad? The problem is that it must be preserved across entrances to DOS. Duplicating more data could cause an incoherency, but I do not yet have an example of how it could fail.

> Consider a fictitious DOS call that just prints "Hello, world!" (default) from a string in memory that is set by another call, and this memory is NOT in the SDA, but MAYBE the SDA points to it.

References are a major problem though. The SDA or the LoL, the latter not being swappable...

The whole idea of the SDA is that it is self-contained though. So I doubt this is a problem.

## The way it works

DOS reports regions of the SDA by how many copies there are (probably just one) and if it's swap always or swap in DOS.

The SDA itself is a contiguous chunk and the descriptors are only for information.

Swap-always means it remains valid between calls to INT 21H, meaning that it is not merely used as a temporary variable.

We are still trying to switch the entire thing though.

Also, the problem with instancing data outside the SDA is that certain INT calls may theoretically malfunction because of the virtual address space.

Maybe go back to the previous example.

But I am suggesting the possibility that instancing the entire DOS data segment could be incorrect.

Yes, but DOS uses a critical section any time it has to access data outside the SDA. SO this is basically a non-issue?

If I want to instance the entire DOS data segment, there should be ways of finding how much memory the kernel is using.

I do wonder if disabling preemption can be a choice, so if the kernel calls an INT that goes to SV86, it can leave preemption on until DOS signals it to be turned off.

Yes, but the plan is that it is a ring-3 context running as a subset of kernel mode, and the scheduler cannot handle this.

We need a way to disable preemption and return to ring-3 though.

Okay, I know DOS gets a critical section, but there are still things which could be problematic, such as differences in the address spaces causing uninstanced data to vary between threads.

What that implies is that duplicating the entire DOS data segment, assuming no further granularity issues, supposedly would NOT work.

Actually there is a potential issue with the data that comes before the official SDA. It contains the printer echo flag, switch character, and other things.

The DTA also may have to be copied to a private buffer, except we don't need that because it only uses virtual addresses in the banking region.

But the unofficial components of the SDA which come before it must somehow be instanced, for example.

So why not instance the entire DOS data segment?

No we don't instance drivers or caches. Instancing DOS is needed because of other reasons.

And what would those be? I understand preventing race conditions are failures to multitask, but DOS is non-reentrant anyway. Most operations can be done regardless of what is in the SDA using wrapper hooks.

I also have not specified what the SDA is supposed to contain anyway, upon program startup.

I don't know what Windows does, but I know it uses some MS-DOS specific features which are not fully documented and I cannot depend on when writing a free software OS.

# September 2

The wrapper hook idea could work but also consider that INT 21H has a lot of undocumented calls which may use features I am trying to control, such as PSPs or the extended error state. The network actually uses both.

DOS may use these internally. DOS instancing avoids this problem.

# September 3

> Switching out the BDA may actually need to happen. Some programs access the keyboard buffer or check them flags in it, especially GUI-like ones. Windows standard mode almost certainly accesses it. In fact, it even reserved a segment for it to be accessed in real and standard mode with 40h as the selector.
> The BDA is immensely complicated but could in theory be switched out. I may need to use a safe cloning method, or write/map the bootstrap BDA when doing SV86 calls.
> It is also assumed to work correctly in all situations.

Interesting notes here. The BDA contains bits related to drive motors, screen dimensions and a bunch of other things. Not sure if all of it matters though.

The size of the BDA is 256 bytes.

There are problems that can occur when programs use the BDA to get mode information. The FreeDOS editor can handle different text mode parameters.

It is not entirely robust, but still possible. And different programs having different setting either requires changing this as part of multitasking or instancing the whole BDA.

# September 4

Some things actually SHOULD be kept coherent in the SDA. It contains information about CHS parameters and free/used clusters.

This needs to be the same or malfunctions could actually occur. Free space determination uses these values and updating across SDAs is actually wrong.

Information about drives also needs to be global.

Some things which the DOS kernel may used internally without calling itself by INT but has in the SDA include:
- Current PSP
- JFT location
- DTA location
- DOS semaphore (maybe because used by some TSRs)

These can be updated in real time without having to add them to the task switch mechanism. It is enough to check if they are already updated upon system entry.

The BDA can also be patched as awell. The only things I am concerned about are video mode and text mode dimensions. There is the keyboard buffer though.

Windows most likely did not rely on the buffer. I am quite sure it had input drivers for KB/M.

So I do not think it is important.

I am not sure MOUSE.COM necessarily used the BDA for the screen parameters though. These had to be manually changed by the program.

MOUSE.COM will NOT be used by the OS though. I have no intention to have a real mode driver at all.

> DOS allows character devices to have special names. They can be added in by drivers. CON is one example. But also remember that EMM also creates a character device too.

# September 6

## How to test DOS multitasking

It is difficult to test true concurrency, such as when interfaces are used at the same time. Regardless, having a basic task switching functionality is a good start.

I need some way of interactively switching programs. Most likely before I even develop a keyboard driver. OS/90 will start off as a preemptive full screen glitchy task switcher.

I could have a command that switches a program. It can call a trapped function that toggles the current window.

In the background, it will be 100% time sharing.

As said before, it will be extremely basic and run the simplest programs. No UI and the keyboard is still controlled by just one program exclusively.

> The keyboard will still block the whole OS though. Not great for testing concurrency or performance. Not be mention the shell will literally freeze everything until it is exited. Really bad stuff. I need an actual keyboard driver.

# September 7

## Keyboard driver?

I cannot really think of a way to have the PS/2 mouse use a different driver than the keyboard considering the same controller is used. It is obviously possible, as it is with MOUSE.COM.

MOUSE.COM is interesting because it actually seizes an IRQ to operate, and can actually claim an entire serial port as well.

There is no way a 16-bit mouse driver will ever be used. I have no reason to support that.

A feature of OS/90 is that aside from being able to use legacy drivers in protected mode, I can also have PM drivers basically eject legacy drivers once they are no longer needed.

That is why IRQs are considered reclaimable and 16-bit.

The mouse driver itself can also be completely ejected, but there are concerns such as having two mice in the computer (bad idea) and one is serial and the other is PS/2.

I don't think even MOUSE will support something like this.

### Polling vs IRQ

Polling is a waste of CPU cycles, but IRQs are not that great either.

It depends on several factors that decide which one makes the most sense:
- User mode or kernel mode driver
- Preemptive or cooperative UI

IRQs are 100% not viable for user mode because IRQs will not be passed directly to the program. This is just impossible.

Cooperative multitasking for the UI (not command line apps) changes things because each program will have a main loop that is asking for new events to process. In this case, polling isn't too bad because the program cannot get events unless it is requesting it.

Although events like strokes from the keyboard require some kind of queueing. The UI is not the only thing running in the system.

So in the end, we do need IRQs. They need to access locked memory and write recent keystrokes. Programs then drain the queue to get keystrokes.

The IRQ is used to send every single byte. There are times in which input cannot be processed at all because the data is incomplete.

The thing that should be considered here is latency. Some people type really fast. I do not, but latency still needs to be controlled properly.

Input processing requires some kind of host serving the data to a client. Processes waiting for input will enter the kernel and wait for input to be sent somehow.

One part of the keyboard driver is a hook for anything that could ever request keyboard input, which includes the BIOS routines...

> VMM32 had so such problem with direct access to the keyboard in this theoretical situation. I have no plans to emulate the keyboard and I don't think VMM did this either.
> In fact V86 could be multithreaded and required a critical section for safety reasons.
> SV86 as it is now is NOT multitaskable because it runs from ring-0 and switched to ring 3 V86 mode.

## SV86

This is a theoretical situation where the keyboard IO calls of the BIOS do not block the whole system, or potentially real mode IO or drivers.

Windows could do this back then so I am not sure why OS/90 shouldn't.

Well the BIOS actually disables interrupts for the duration of its execution most of the time. The only time it does not is when "yielding" such as when it is awaiting an interrupt (which is useful).

That is why my old version of a mouse driver was able to print to the screen correctly when moving.

> DOS .SYS drivers can be added into the VM dynamically. They are actually just a chain anyway.

I think the way VMM did real mode calls is that a critical section was aquired and then the context was altered to switch control flow. This would prevent task switching and a subsequent race condition.

In OS/90, this could be disabling preemption and letting the program execute the INT call.

But OS/90 cannot really do this safely. Disabling preemption cannot be done for long due to the higher reliance on preemptive multitasking.

That is not really the issue. It just gets really complicated after that.

## Keyboard driver structure

A queue in the logical data structure to use, or a backward stack read from the start in memory.

Each key stroke in scan code set 2 is broken by a F0 code with few exceptions.

## Advanced scheduler design

Like a microkernel design, the part that decides time slices and CPU can be in the userspace and make decisions on a secondly basis.

Scheduling can be done by collecting statistics on the following things:
- Disk IO bandwidth on hardware side
- Requested disk IO bandwidth from software side
- Total system calls per second
- Instruction pointer bytes delta (this may need a higher sample frequency)

The disadvantage is that 1 second is a very large quantum.

> I can work on the UI using SDL and the back-end rendering.

# September 9

## Process control block allocation

There are two types of records: thread blocks (TASK) and DOS processes. Both require 4K blocks.

I think there should be a special address space for these. They should not waste linked list entries in the global allocator and slow down the system.

> My journal has reached 640K in size! I think it is time to move to volume 2.

## DMA emulation

Virtual device drivers need to emulate transfers using the DMA subsystem. Sometimes, buffers used for DMA must be entirely on the software side.

Consider a soundblaster emulator. DMA transfers have to be simulated for that to work. SB emulation was used to allow DOS programs to play sounds while using incompatible devices.
