#ifndef SCHEDULER_PROC_H
#define SCHEDULER_PROC_H

#include <Type.h>

// Bitness of programs is determined by how they are initialized, not by their
// code segment descriptor size. If a program enters 32-bit DPMI, it will be
// recognized as 32-bit and some function calls are slightly different.

enum {
    PROGRAM_PM_32,      // DOS program in 32-bit PM
    PROGRAM_PM_16,      // DOS program in 16-bit PM
    PROGRAM_V86,        // DOS program in real mode
};

//
// UREGS is a complete context for ring-3 code.
//
tstruct {
    DWORD es; DWORD ds;
    DWORD fs; DWORD gs;
    DWORD ss;

    DWORD eax;    DWORD ebx;
    DWORD ecx;    DWORD edx;
    DWORD esi;    DWORD edi;
    DWORD ebp;    DWORD esp;
    DWORD eflags; DWORD eip;
    DWORD  _was_v86; // For internal use, do not touch
}UREGS;

// How do we fix ScVirtual_Int? Separate V86 context?
//
// Context of a kernel thread. Segment registers are omitted because the kernel
// is garaunteed to use the flat model
//
tstruct {
    DWORD eax;    DWORD ebx;
    DWORD ecx;    DWORD edx;
    DWORD esi;    DWORD edi;
    DWORD ebp;    DWORD esp;
    DWORD eflags; DWORD eip;
}KREGS;

// In the 8086, if we fetch the little endian value
// AD DE from the memory, AH will be DE and AL will be AD.
tpkstruct {
    union {
        struct {
            BYTE al;
            BYTE ah;
        };
        WORD ax;
    };
    union {
        struct {
            BYTE bl;
            BYTE bh;
        };
        WORD bx;
    };
    union {
        struct {
            BYTE cl;
            BYTE ch;
        };
        WORD cx;
    };
    union {
        struct {
            BYTE dl;
            BYTE dh;
        };
        WORD dx;
    };
    WORD si;
    WORD di;
    WORD bp;

    WORD es;
    WORD ds;

    WORD ip;
    WORD cs;
    WORD flags;
    WORD sp;
    WORD ss;
}DREGW,*P_DREGW;

///////////////////////////////////////////////////////////////////
// P r o c e s s   C o n t r o l   B l o c k   S t r u c t u r e //
///////////////////////////////////////////////////////////////////

tpkstruct {
    UREGS   user_thread_context;
    KREGS   kernel_thread_context;

    PVOID   mem_mirror;

    DWORD   kernel_pm_stack; // What to do?
    DWORD   user_page_directory_entries[64];
    // Exceptions share the same IDT. This might make standard compliance
    // a bit shaky, but it is more space efficient and a program should not
    // bother with these vectors anyway.
    LOCAL_PM_IDT_ENTRY  local_idt[256];

    // Real mode control section
    DWORD   rm_local_ivt[256];
    DWORD   rm_kernel_ss_sp;
    WORD    rm_subproc_exit_code;
    WORD    psp_segment;

    DWORD   ctrl_c_handler_seg_off;
    DWORD   crit_error_seg_off;

    WORD    vpic_mask;  // By default, all IRQs are masked

    // Flags related to the process
    BYTE    program_type            :2; // [1]
    BYTE    virtual_irq_on          :1;
    BYTE    fake_irq_in_progress    :1;
    BYTE    fake_irq_pending        :1;
    BYTE    use87                   :1;
    BYTE    proc_state              :3;
    BYTE    protected_mode          :1;

    BYTE    vector_to_invoke:4; // [2]

    // [1]: The type of program. If the program enters protected
    // mode, it will permanently become a 16/32-bit protected mode
    // program. Raw mode switching will not change this, but it will
    // change protected_mode
    //
    // [2]: Vector to use for an exception or interrupt

    // Add subprocess stack. It contains:
    // * PSP
    // * Size of allocation

    BYTE    current_working_dir[80];
    // The command line remembers the current disk path
    // Default behavior is to CD to the root. (I think)
    // This includes the drive letter.

	PVOID   x87env;
    PVOID   next;
    PVOID   last;

}PCB,*P_PCB;

//static int x = sizeof(PCB);

#endif /* SCHEDULER_PROC_H */
