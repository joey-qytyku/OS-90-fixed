#ifndef __CENTCOM_H__
#define __CENTCOM_H__

// This file should not be included by drivers.

// PSTDREGS is passed in EDX for assembly handlers.
// May want to change.

typedef void (*EXC_HANDLER)(REGS*);

extern EXC_HANDLER handler_table[EXCEPTIONS_SUPPORTED];

void RemapPIC(void);

#endif /* __CENTCOM_H__ */
