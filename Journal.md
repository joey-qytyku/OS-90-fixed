
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

I like using the Whatever_Case style, but maybe I should just go back to the snake_case. It looks good with acronyms, which do not need to be capitalized, and is very readable. Most importantly This_Case is much easier to type that snake_case and does not require excessive pressing of the shift key.

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
