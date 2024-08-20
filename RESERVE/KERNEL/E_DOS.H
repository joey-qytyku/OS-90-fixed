#ifndef

typedef struct {
	SHORT   off;
	SHORT   seg;
}FPTR16;

typedef _Packed struct {
	SHORT   seg;
	LONG    off;
}FPTR32;

typedef _Packed struct {
	SHORT   int20h[2];
	SHORT   progmem_top_seg;
	BYTE    reserved;
	BYTE    call5[5];

	FPTR16  terminate;      /* These are supposed to be deprecated */
	FPTR16  ctrlC;
	FPTR16  critical;

	SHORT   _parent_psp;    /* Internal. Program should not access. */

	BYTE    jft[20];        /* Default, 0,1,2,3,4 */
	SHORT   env_seg;
	LONG    _saved_stack;   /* Internal */

	SHORT   _jft_size;      /* Normally internal. */
	LONG    _jft_ptr;       /* Internal */
	FPTR16  prev_psp;       /* Supposedly used by SHARE. */
	SHORT   dosver;
	BYTE    reserved[14];
	BYTE    int21h_retf[3];

	BYTE    more_reserved[9];
	BYTE    fcb1[16];
	BYTE    fcb2[20];
	BYTE    command_len;
	BYTE    command[127];   /* Terminated by carriage return (0Dh) */
}PSP,*P_PSP;

// Note: The Disk Transfer Area is always at offset 80h of the PSP by default.
// This is for CP/M compatibility.

typedef _Packed struct {
	BYTE    internal[11];
	BYTE    attr;
	SHORT   time;
	SHORT   date;
	LONG    size;
	BYTE    fname_asciiz[13];
}DTA;

#define LOCAL_XMS_HANDLES 24 /* MUST BE MULTIPLE OF 8 */

#define BREAK_VECTOR
#define CRIT_ERR_VECTOR
#define EXIT_VECTOR

typedef struct {
	LONG    largest_free;
	LONG    max_unlocked_pgalloc;
	LONG    addrspace_pages;
	LONG    freepages;
	LONG    addrspace_free;
	LONG    pagefile_pages;
	BYTE    reserved[12]; // Set to FF
}DPMI_FREEMEM_STRUC;

// Add IRET action to avoid IF?

// MUST BE UNDER 4096 BYTES
// Note that ^C does not require return in real mode.
typedef _Packed struct {
	P_PSP   psp;
	FPTR16  virtual_ivt[256];
	FPTR32  virtual_idt[256];
	FPTR32  pm_except[0x20];

	LONG    extended_error;

	PVOID   parent_prog;
	LONG    prog_flags;
	PSHORT  dpmi_rm_stack;

	// Zero == unused. Maps XMS memory to chains.
	// 24-bit compressed values.
	BYTE    xms_handle_table[LOCAL_XMS_HANDLES * 3];
	LONG    xms_kb_allowance;
	LONG    xms_kb_inuse;

	// OS/90 supports DOS multithreading. Crazy right?
	PTASK    threads[24];

	// DOS only supports 64-byte paths.
	// See INT 21h,AH=47h for more details.
	LONG    current_drive; // A=1, B=2, ...
	BYTE    path[64];
}EMU_CONTEXT,*PEMU_CONTEXT;
