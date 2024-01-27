#ifndef SCHEDULER_PROC_H
#define SCHEDULER_PROC_H

#include <Type.h>
#include <Misc/log2.h>

// Can I make the system entry in ASM. Make the PCB a bit more ASM friendly?

// Relates to the thread state variable.
//
//
//

#define MAX_SUBPROCS 8

typedef enum {
    TH_KERNEL = 0,
    TH_USER   = 1
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
}TREGS,*P_TREGS;

#define OP_INT3 0xCC
#define OP_INT  0xCD
#define OP_INTO 0xCE
#define OP_IRET 0xCF

// Is this right?
// Should be. Maybe add some convenience union fields for registers?

// Capitalize names?

/* ADD PADDING FOR HIGH, another struct? */
#define _DWB_R(n)\
    union {\
        U32 E##n##X;\
        union {\
            U8 n##L;\
            struct { U8 :8; U8 n##H; };\
        };\
        U16 n##X;\
    };

#define _DW_R(n)\
    union {\
        U32 E##n;\
        U16 n;\
    }

// Standard register dump structure. User registers,
tstruct {
    _DWB_R(A);
    _DWB_R(B);
    _DWB_R(C);
    _DWB_R(D);

    _DW_R(SI);
    _DW_R(DI);
    _DW_R(BP);

    _DW_R(IP);

    U32   CS;
    _DW_R(FLAGS);
    _DW_R(SP);
    U32   SS;

    U32   pm_ES;
    U32   pm_DS;
    U32   pm_FS;
    U32   pm_GS;

    U32   v86_ES;
    U32   v86_DS;
    U32   v86_FS;
    U32   v86_GS;
}RD,*P_RD;

typedef struct {
    U16 off;
    U16 seg;
}FAR_PTR_16;

tpkstruct {
    U32     handler_eip;
    U8      type            :3;
    U16     handler_cseg    :13;
}LOCAL_PM_IDT_ENTRY;

// This will be 100% assembly soon
typedef U32 IRET_GPHND_DO(P_RD);

///////////////////////////////////////////////////////////////////
///P r o c e s s   C o n t r o l   B l o c k   S t r u c t u r e///
///////////////////////////////////////////////////////////////////

struct PCB_COMMON { // 28 bytes
    RD    restore_context;          // +0
    PVOID next;                     // +4
    PVOID last;                     // +8
    U32 psp_segment;                // +12
    THREAD_STATE thread_state:8;    // +13
    DPMI_BITNESS bitness     :8;    // +14
    U16 _;                          // +15
    U32 _sepint_actions[3];         // +27
};
// TODO, IRET actions

struct PCB_INT_VECTORS {
    LOCAL_PM_IDT_ENTRY local_idt[256];
    FAR_PTR_16 rm_local_ivt[256];
};

struct PCB_DOS_INFO {
    // When the exec function finishes it returns this somewhere.
    // Local to each DOS VM.
    U16 last_subproc_exit_code;

    U32 subprocess_stack[MAX_SUBPROCS];
    FAR_PTR_16 ctrl_c_handler_seg_off;
    FAR_PTR_16 crit_error_seg_off;
    U8   current_working_dir[80];
    // The command line remembers the current disk path
    // Default behavior is to CD to the root. (I think)
    // Regardless, there is no global CWD because OS/90 multitasks.
    // This includes the drive letter as "[LETTER]:"
};

tstruct {
    struct PCB_COMMON       std;
    struct PCB_INT_VECTORS  intv;
    struct PCB_DOS_INFO     dosinf;
}PCB, *P_PCB;

//static int x = sizeof(PCB);

// Add a way to throw a system exit by jumping, that way we dont have to
// return from procedures and can just kill the kernel context.

// TODO TODO TODO TODO
#define Get_Current_PCB()
//(P_PCB)(__asm__("sp") & (~0x1FFFL))

#endif /* SCHEDULER_PROC_H */
