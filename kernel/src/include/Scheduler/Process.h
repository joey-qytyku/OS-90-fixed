#ifndef SCHEDULER_PROC_H
#define SCHEDULER_PROC_H

#include <Type.h>
#include <Misc/log2.h>

// Can I make the system entry in ASM. Make the PCB a bit more ASM friendly?

// Relates to the thread state variable.
//
//
//

typedef enum {
    TH_DEAD = 0
}THREAD_STATE;

typedef enum {
    // Bitness of programs is determined by how they are initialized, not by their
    // code segment descriptor size. If a program enters 32-bit DPMI, it will be
    // recognized as 32-bit and some function calls are slightly different.

    // Even if a program raw switches to real mode, it will still be a protected
    // mode program in the eyes of DPMI.
    //! The actual mode is determined only by looking at the VM flag and the
    //! segment descriptor!

    PROGRAM_PM_32,
    PROGRAM_PM_16,
    PROGRAM_V86,
}DPMI_BITNESS;

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

#define OP_INT3 0xCC
#define OP_INT  0xCD
#define OP_INTO 0xCE
#define OP_IRET 0xCF

tpkstruct {
    U32   eax;
    U32   ebx;
    U32   ecx;
    U32   edx;
    U32   esi;
    U32   edi;
    U32   ebp;

    U32   eip;
    U32   cs;
    U32   eflags;
    U32   esp;
    U32   ss;

    U32   pm_es;
    U32   pm_ds;
    U32   pm_fs;
    U32   pm_gs;

    U32   v86_es;
    U32   v86_ds;
    U32   v86_fs;
    U32   v86_gs;
}IRET_FRAME,*P_IRET_FRAME;

typedef struct {
    U16 off;
    U16 seg;
}FAR_PTR_16;

typedef struct PACKED {
    U32     handler_eip;
    U8      type            :3;
    U16     handler_cseg    :13;
}LOCAL_PM_IDT_ENTRY;

typedef U32 INT_SEPDO(P_IRET_FRAME);
typedef U32 EXC_SEPDO(P_IRET_FRAME);

typedef U32 IRET_GPHND_DO(P_IRET_FRAME);

///////////////////////////////////////////////////////////////////
///P r o c e s s   C o n t r o l   B l o c k   S t r u c t u r e///
///////////////////////////////////////////////////////////////////

// Very performance sensitive. Beware of structure ordering.
// This structure is NOT packed by default.
// Must be compatible with assembly definition.

// Create "methods" for this?

// It might be better to just use bytes for some of the process flags.
// That requires less instructions to manipulate and allows for using ASM.
tstruct
{
    UREGS   user_regs;
    KREGS   kern_regs;

    PVOID   next;
    PVOID   last;
    U32     psp_segment;

    THREAD_STATE    thread_state:8;
    DPMI_BITNESS    bitness     :8;

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
    // Regardless, there is no global CWD because OS/90 multitasks.
    // This includes the drive letter as "[LETTER]:C"
}PCB, *P_PCB;

// static int x = sizeof(PCB);

// TODO TODO TODO TODO
#define Get_Current_PCB()
//(P_PCB)(__asm__("sp") & (~0x1FFFL))

#endif /* SCHEDULER_PROC_H */
