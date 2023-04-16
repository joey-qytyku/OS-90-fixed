#ifndef FARCALL_H
#define FARCALL_H

//
// To make the interface simpler, the V86 chain link struct
// is embedded here. This structure is part of a linked list.
// The difference is that it is indexed by number rather than probing
// functions iteratively as with V86.
//

#include <Scheduler.h>
#include <Type.h>

#define MAX_SUPPORTED_FAR_CALLS 16
#define FAR_CALL_BASE_SEG 0xF001
#define FAR_CALL_BASE_ADDR (0xF001*16)
#define FAR_CALL_UPPER_BOUND_OFFSET (MAX_SUPPORTED_FAR_CALLS-1)

//
// Far call handler does not need to return anything. The kernel
// knows exactly which one to call by index
//
typedef VOID (*DOS_FCALL_HANDLER)(PDWORD);

extern DWORD ScInsertFarCallHandler(DOS_FCALL_HANDLER fcall_handler);
extern VOID HandleFcallAfterPF(WORD, const PDWORD);

#endif
