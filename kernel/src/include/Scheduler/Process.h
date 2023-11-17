#ifndef SCHEDULER_PROC_H
#define SCHEDULER_PROC_H

#include <Type.h>
#include <Misc/log2.h>

// Relates to the thread state variable.
//
//
//
enum {
    THREAD_DEAD,
    THREAD_IN_KERNEL,
    THREAD_BLOCKED,
    THREAD_RUN_V86,
    THREAD_RUN_PM,
    THREAD_TYPE_MASK = 0b111 // All other bits reserved
};

enum {
    // Bitness of programs is determined by how they are initialized, not by their
    // code segment descriptor size. If a program enters 32-bit DPMI, it will be
    // recognized as 32-bit and some function calls are slightly different.

    // Even if a program raw switches to real mode

    PROGRAM_PM_32,      // DOS program in 32-bit PM
    PROGRAM_PM_16,      // DOS program in 16-bit PM
    PROGRAM_V86,        // DOS program in real mode
};

//
// UREGS is a complete context for ring-3 code.
//
tstruct {
    U32 es;
    U32 ds;
    U32 fs;
    U32 gs;
    U32 ss;

    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
    U32 esi;
    U32 edi;
    U32 ebp;
    U32 esp;
    U32 eflags;
    U32 eip;
}UREGS,*P_UREGS;

//
// Context of a kernel thread. Segment registers are omitted because the kernel
// is garaunteed to use the flat model
//
tstruct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
    U32 esi;
    U32 edi;
    U32 ebp;
    U32 esp;
    U32 eflags;
    U32 eip;
}KREGS;

typedef struct { U16 off; U16 seg; }FAR_PTR_16;

typedef struct PACKED
{
    U32     handler_eip;
    U8      type            :3;
    U16     handler_cseg    :13;
}LOCAL_PM_IDT_ENTRY;

///////////////////////////////////////////////////////////////////
// P r o c e s s   C o n t r o l   B l o c k   S t r u c t u r e //
///////////////////////////////////////////////////////////////////

// Very performance sensitive. Beware of structure ordering.
// This structure is NOT packed by default.

// Create "methods" for this?
tstruct
{
    UREGS   user_regs;
    KREGS   kern_regs;

    PVOID   next;
    PVOID   last;
    U16     psp_segment;    // Placed here for cache locality

    U32   thread_state;
    U32   procflags;

    LOCAL_PM_IDT_ENTRY  local_idt[256];

    // Real mode control section
    FAR_PTR_16 rm_local_ivt[256];

    // This is the SS:SP to be used when performing DPMI translation services.
    // Because they are not SV86, it is local to each process.
    U32 rm_dpmi_xlat_ss_sp;

    U16 rm_subproc_exit_code;

    U32 ctrl_c_handler_seg_off;
    U32 crit_error_seg_off;

    // Add subprocess stack. It contains:
    // * PSP
    // * Size of allocation

    U8   current_working_dir[80];
    // The command line remembers the current disk path
    // Default behavior is to CD to the root. (I think)
    // This includes the drive letter as "[LETTER]:C"
}PCB, *P_PCB;

// static int x = sizeof(PCB);

static inline P_PCB GetCurrentPCB(VOID)
{
    register U32 sp __asm__("sp");
    return (P_PCB)(sp & (~0x1FFFL));
}

// "Methods" for the PCB because it has so many members without a clear
// indication as to how they are meant to be accessed.
// These are all static inlines and the compiler will optimize most of the
// calls away with direct access. If necessary, it could also generate static
// procedures.
// IGNORE THESE FOR DRIVERS!

#define MK_GETTER(rettype, funcname, member)\
    rettype PcbGet_##funcname(P_PCB pcb)\
    { return pcb->member; }\

#define MK_SETTER(type, funcname, member)\
    VOID PcbSet_##funcname(P_PCB pcb, type value)\
    { pcb->member = value; }

// Get/set next process
MK_GETTER(P_PCB, Next, next);
MK_SETTER(P_PCB, Next, next);

// Get/set last process
MK_GETTER(P_PCB, Last, last);
MK_SETTER(P_PCB, Last, last);

// Get/set PM IDT entries

static inline VOID PcbSetLocalIdtEntry(P_PCB p, U8 i, LOCAL_PM_IDT_ENTRY e)
{ p->local_idt[i] = e; }

static inline LOCAL_PM_IDT_ENTRY PcbGetLocalIdtEntry(P_PCB p, U8 i)
{ return p->local_idt[i]; }

// Get/set real mode IVT vectors

static inline VOID PcbSetLocalRmVector(P_PCB p, U8 i, FAR_PTR_16 v)
{ p->rm_local_ivt[i] = v; }

// Remove this?
PVOID KERNEL ProcSegmentToLinearAddress(
    P_PCB,
    U16,
    U32
);

#endif /* SCHEDULER_PROC_H */
