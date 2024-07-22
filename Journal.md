
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

> VFC tags: maybe add messages that explain why that tag was selected or where to find it? For example, a video can have a tag and a time with it to where in the video something is mentioned.

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
```

```
Okay, I thought about this for a moment. I do not technically have to do this. If the DOS subsystem is made a mandatory part of the OS, I can simply have an autoexec file run using the default command interpreter.
