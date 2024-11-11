#ifndef E_DOS_H
#define E_DOS_H

//
// DOS functions (21H) that must be hooked.
//
enum {
	DOS_ALLOC       = 0x48,
	DOS_FREE        = 0x49,
	DOS_SETBLOCK    = 0x4A,
	DOS_EXEC        = 0x4B,
	DOS_TERMINATE   = 0x4C,
	DOS_GET_RETCODE = 0x4D
};

typedef struct {
	SHORT   off;
	SHORT   seg;
}FPTR16;

typedef _Packed struct {
	SHORT   seg;
	LONG    off;
}FPTR32;

//
// The memory control block structure. This is used for freeing memory
// upon forced program termination.
//
typedef _Packed struct {
	BYTE    type;
	USHORT  psp_seg;

//      Number of paragraphs
	USHORT  size;
	BYTE    _resv[3];

//      Display name, not really used.
	BYTE    name[8];
}MEMCB,*P_MEMCB;

typedef _Packed struct {
//
//      A sequence which leads to the termination of a program.
//      INT 20H is not to be used by any DPMI program while in protected mode.
//
	SHORT   int20h[2];


//
//      Segment of the first byte available to a program. Mostly useful for EXE
//      programs since the PSP is allocated separately.
//
	SHORT   progmem_top_seg;

	BYTE    reserved;

//
//      Seems to be deprecated. Supposedly a far call to DOS.
//
	BYTE    call5[5];

//
//      These are mostly deprecated.
//
	FPTR16  terminate;      /* These are supposed to be deprecated */
	FPTR16  ctrlC;
	FPTR16  critical;

	SHORT   _parent_psp;    /* Internal. Program should not access. */

//
//      The default job file table. The size corresponds directly with the
//      maximum number of open handles. The number of openable files is
//      decided by the FILES=xxx directive in CONFIG.SYS.
//      This default table is not used because 20 is far too small.
//
	BYTE    jft[20];        /* Default, 0,1,2,3,4 */

//
//      Segment of the environment block. Because programs are free to
//      change whatever they want in the environment, it is totally impossible
//      to make it global. A new one is created for each program. The shell
//      is supposed to instantiate the environment variables, though it should
//      NOT be the standard autoexec.bat file. Default name is OS90AUTO.BAT.
//
	SHORT   env_seg;

//      This is used by DOS internally because it switches stacks when
//      calling INT 21H functions. It will automatically do this,
//      which is fine.
//
	LONG    _saved_stack;

//      Size of the JFT in bytes, or how many handles
	SHORT   jft_size;

//      SEG:OFF pointer to the JFT. This will not be within the actual PSP.
//      OS/90 resizes it and sets the same value for every program.
//      This is necessary to prevent any kind of catastrophic situation
//      where the same file is opened.
//
	FPTR16  jft_ptr;

//      If COMMAND.COM runs a program, this would point to the shell.
	FPTR16  prev_psp;

//
//      DOS version in BCD format. OS/90 has no need for SETVER.
//      since it is done automatically.
//
	SHORT   dosver;

	BYTE    reserved[14];

//      Sequence:
//              INT 21H
//              RET
//
	BYTE    int21h_retf[3];

	BYTE    more_reserved[9];
//
//      OS/90 does not use file control blocks. They should generally
//      work but there is no guarantee. Ever since the multitasking DOS
//      craze of the mid 80's, FCBs were essentially no more.
//
	BYTE    fcb1[16];
	BYTE    fcb2[20];

//      Length of command tail excluding the carriage return
	BYTE    command_len;
//      Command tail. Terminated by carriage return (\r or 0Dh)
	BYTE    command[127];
}PSP,*P_PSP;

// Note: The Disk Transfer Area is always at offset 80h of the PSP by default.
// This is for CP/M compatibility.
// Also note: OS/90 does not support file control blocks!

typedef _Packed struct {
	BYTE    internal[11];
	BYTE    attr;
	SHORT   time;
	SHORT   date;
	LONG    size;
	BYTE    fname_asciiz[13];
}DTA;

#define LOCAL_XMS_HANDLES 24 /* MUST BE MULTIPLE OF 8 */


/////
// The following vectors are handled in protected mode whenever possible.
// Calling them directly will always be an error.
//

//
// Called when a ^C is encountered at some point in execution.
//
//
#define CTRL_C_VECTOR    0x23

//
// INT 1Ch is used by DOS. It is called periodically when the timer ticks.
// On OS/90, this is not an accurate timer but must nonetheless be supported.
// The idle thread is hooked by the DOS manager to perform context changes
// so that the interrupt can be handled, if there is a handler.
//
#define TICK_VECTOR     0x1C

//
// Critical error. The handler for this is ordinarily implemented by the
// command shell, where it displays the infamous fail prompt.
// If the program runs standalone, the call will go unhandled and cause the
// VM to forcefully terminate.
//
// This is not CALLED. It is jumped to more than anything, like EXIT.
//
//
#define CRIT_ERR_VECTOR 0x24

/////////
// Exit vector is different because the real mode version can be called
// even if a protected mode handler is active.
//
#define EXIT_VECTOR 0x22

typedef struct {
	LONG    largest_free;
	LONG    max_unlocked_pgalloc;
	LONG    addrspace_pages;
	LONG    freepages;
	LONG    addrspace_free;
	LONG    pagefile_pages;
	BYTE    reserved_FF[12];
}DPMI_FREEMEM_STRUC;

// Add IRET action to avoid IF?
	// Why not make them all local and copy the defaults in?
	// Yeah, but how? Do I pass the IVT in to INTxH?
	// Consider it. Having global hooks

// MUST BE UNDER 4096 BYTES
// Note that ^C does not require return in real mode.
typedef _Packed struct {
	//
	// A full local interrupt vector table. This is looked up first
	// before reflecting the INT to actual real mode.
	//
	// Real mode exceptions are never reflected through SV86 since it
	// will never handle properly. If there is no handler set in RM,
	// the exception will go uncaught and kill the program.
	//
	// DPMI requires that any IRQ that goes to the program uses the IVT
	// or IDT according to the current mode, and goes through the reflection
	// process as needed. Most programs will insert a callback routine
	// in the IVT to ensure the same handling in both modes.
	//
	// Vectors such as exit, break, and critical error are all handled in
	// real mode by default. The DPMI spec mandates that these vectors
	//
	FPTR16  virtual_ivt[256];

	// The IDT applies to IRQs and INT calls (or related opcodes).
	// Because the IBM PC overlaps the 286+ exceptions with BIOS
	// interrupts, it is necessary to separate them.
	//
	// The IRQ vectors can be anywhere that makes sense. In our case
	// they coincide exactly with the actual INT vectors.
	FPTR32  virtual_idt[256];

//      DPMI requires 32 exception handlers to be supported for modification.
//      This does NOT mean that all of them can be caught. OS/90 does not
//      permit page faults from being caught since those are _too_ close to
//      the memory manager.
//
// Full list of supported exceptions:
//
//      - Divide by zero
//      - Trap fault
//              - unhandled and NOT reflected to RM
//              - Unless set in PM, will fail!
//      - Stack segment fault
//              - Similar to a #GP. Should work
//
//      - General protection fault:
//              - Only if the OS does not handle it.
//              - Recommended for segfaults
//              - Also sent if a coprocessor overrun happens, even on 80387
//
//      - Floating point exception (even with 80387, same one)
//      - SIMD stuff:
//              - Not sure what you're going to do with these
//              - Should technically work
//              - The kernel must support SIMD. Not currently the case.
//
// NOT SUPPORTED:
//
//      - Alignment check
//              - This must be enabled in a control register. It is not
//                thread-local so not supported.
//      - Page fault
//              - DPMI 1.0 permits this, though it is not required.
//              - OS/90 has its own way of handling page faults.
//              - Not supported.
//
//      DOS does not handle exceptions properly so the default behavior
//      for these is really just to terminate the VM instead of reflecting
//      to real mode.
//
	FPTR32  pm_except[0x20];

//      Pointer to the program segment prefix. Can be directly
//      converted to a real mode segment as needed.
//      Programs are allowed to change the current PSP using the
//      "undocumented" interface, which behaves differently
//      because OS/90 completely hijacks it.
//
//      Under OS/90, the current PSP, kind of like the DTA, is made
//      a global context.
//
//      Windows for example, changes the current PSP because it makes a new
//      one for each program. If Windows terminates however, it will delete
//      the PSPs it creates and exit out with the one that it originally had.
//
//      The data in MS-DOS that contains the current PSP is totally
//      internal. The INT 21H interface is the only way to control it.
//
	P_PSP   psp;

//
//      This state is normally global, so it must be kept here.
//      It is obtained using INT 21H, AH=
//
	LONG    extended_error;

//
//      Programs can execute new programs, whether they are in real mode
//      or protected mode. This creates a brand-new instance and also
//      freezes the threads of the parent process.
//
	PVOID   parent_prog;

//      TODO: define
	LONG    prog_flags;

//
//      Each program needs a stack for calling real mode services.
//      Needed?
//
	PSHORT  dpmi_rm_stack;

//      Zero == unused. Maps XMS memory to chains.
//      24-bit compressed values.
//
//      XMS is only allowed to allocate physical memory
//
	BYTE    xms_handle_table[LOCAL_XMS_HANDLES * 3];
	LONG    xms_kb_allowance; //???
	LONG    xms_kb_inuse;     //???

//      Memory deallocation is done by parsing the memory control blocks.

//      OS/90 supports DOS multithreading. Crazy right?
	PTASK    threads[32];

//      AH=4Dh uses this value. Set on subprocess termination.
//      Actually 8-bit value.
	SHORT   subprog_retcode;

	//
	// Directly convertible to BIOS disks addresses because the IBM PC
	// only has two floppies.
	//
	SHORT   current_drive; // A=1, B=2, ...
	// DOS only supports 64-byte paths.
	// See INT 21h,AH=47h for more details.
	//
	// The format is the same as DOS but without the leading c:\ type prefix
	//
	// DOS also keeps a table of the path that each drive is currently at.
	// This must be simulated. 1,664 bytes are used.
	//
	BYTE    path[64][26];

	// Bytes: 3,940
}EMU_CONTEXT,*PEMU_CONTEXT;
/*

>>> Use page table-style file caching approach?
*/
#endif /* E_DOS_H */