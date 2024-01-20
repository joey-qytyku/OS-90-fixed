#include "../DriverHeader.h"

#define R2_16 0
#define R2_32 1

typedef VOID (*ISR)(VOID);

typedef enum {
    IRQ_UNDEFINED = 0,
    IRQ_FREE      = 1,  // Available interrupt
    IRQ_INUSE_32  = 2,  // 32-bit interrupt controlled by a BUS
    IRQ_RECL_16   = 3   // Legacy DOS driver interrupt, can be reclaimed
}ICLASS;


typedef struct __attribute__((packed))
{
    // Each level is two bits, which means that all interrupt levels fit in
    // a single 32-bit U32

    U32     class_bmp;
    ISR     handlers[16];
    PVOID   owners[16];
}INTERRUPTS,
*PINTERRUPTS;

kernel ISR Get_IRQ_Handler(U32 irq);
kernel STATUS Acquire_IRQ(U8  vec, ISR hnd, P_DRVHDR owner);
kernel VOID Release_IRQ(U8 irq, U32 to);
kernel ICLASS Get_IRQ_Class(U8 irq);

VOID Init_PnP_IRQ();

