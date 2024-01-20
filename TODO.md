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
