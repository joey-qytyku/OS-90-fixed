#include "scheduler/task.h"

typedef PVOID (*PAGE_GET)(VOID);
typedef VOID (*KTHREAD_PROC)(PVOID);

HTASK TsT2Sd_CreateKernelThread(KTHREAD_PROC ktp)
{}

//
// init_line is formatted as a subsystem name, the absolute path of the
// executable to run, and any additional parameters.
// The line must be no longer than 255 characters including NULL.
//
// fail_msg_ptr is a pointer to a const char * which will be the string
// that represents the error message if there is one. Outputs NULL
// if none. This is for debugging or for user troubleshooting. Pass
// NULL to ignore.
//
// page: a function that returns a void pointer to 4K of locked memory.
//       allows for creating process controls blocks anywhere.
//
// Return value is the handle. Equal to ~0 if failed.
//
// Example:
//      CreateTask("VDOS.DRV C:\COMMAND.COM /M:128");
//

HTASK TsT2Sd_CreateTask(
        PAGE_GET page,
        const char *init_line,
        const char **fail_msg_ptr
){}

//
// Completely expunges the task after calling the exit hook.
//
LONG TsT2Sd_TerminateTask(HTASK handle)
{}

//
// Returns whatever is passed to it because handles are pointers.
// Always use this in driver code.
//
TASK *TsT2_TASKPtrFromHTASK(HTASK handle)
{
        return (TASK*)handle;
}
