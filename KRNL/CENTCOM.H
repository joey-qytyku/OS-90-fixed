#ifndef __CENTCOM_H__
#define __CENTCOM_H__

// This file should not be included by drivers.

// PSTDREGS is passed in EDX for assembly handlers.
// May want to change.
typedef (VOID)(*EXC_HANDLER)(PSTDREGS);

extern EXC_HANDLER [];

#endif /* __CENTCOM_H__ */
