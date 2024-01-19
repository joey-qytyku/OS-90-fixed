#ifndef PNP_EVENT_H
#define PNP_EVENT_H

#include <Type.h>
#include <Scheduler/Sync.h>

#define EV_CC_NOT_DONE 0xFFFFFFFF
#define EVP_INITIALIZER { ._access_lock =  (ATOMIC)(0) }

// The event packet structure is universal to user and kernel
// It is as self contained as possible and does not require additional
// data to handle the event. The entire event is described with this
// type of object.
//
// Whether the sender/reciever is user or kernel is to be determined internally.
// Currently, MBHND is simply a pointer. Anything above 0xC0000000 is kernel.
//
// This does not matter to the drivers though.
//
// Setting `intr` to NULL means interferences are ignored and a no-op procedure
// is not needed.
//

typedef VOID (*EV_INTERFERE_CALLBACK)(PVOID);
typedef VOID (*MB_STKOVF_CALLBACK)(PVOID);
typedef VOID (*MB_DISPATCH_CALLBACK)(VOID);

tstruct {
    U16     info;
    U8      stack_len;
    U8      stack_top;
    PU32    ptr_to_stack;
    MB_STKOVF_CALLBACK on_overflow;
    MB_DISPATCH_CALLBACK disp;
    PVOID *next;
}MBOX,*P_MBOX,*MBHND; // MBHND is just a pointer to a mailbox

tstruct {
    U16     func;
    U16     payload_size;
    PVOID   exit_code;
    EV_INTERFERE_CALLBACK intr;

    MBHND   sender;
    MBHND   reciever;
    ATOMIC  _access_lock;
    U8      payload[];
}EVENT_PACKET,*P_EVENT_PACKET;

VOID Raise_Event(
    MBHND           mailbox,
    P_EVENT_PACKET  to_send
);

VOID Raise_Urgent(
    MBHND           mailbox,
    P_EVENT_PACKET  to_send
);

VOID Interrupt_Event(
    P_EVENT_PACKET  to_intr,
    U16             intr_code
);


VOID Forward_Request(
    P_EVENT_PACKET  packet,
    MBHND           to
);

#endif /* PNP_EVENT_H */
