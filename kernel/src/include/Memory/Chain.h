////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef MM_CHAIN_H
#define MM_CHAIN_H

#include <Type.h>
#include "MemDefs.h"

// Current config:
// - About 2GB of memory addressing (more than Win98)
// - 128MB per allocation max
// Pretty good, ought to be enough for anybody.

tstruct
{
    U32     rel_index   :15;
    U32     owner_pid   :17;
    U16     next;
    U16     prev;
}PFE,*P_PFE;

// The PID is just a pointer to the process control block in OS/90.
// But some of the fields do not actually matter.
// The top of the address will always be 0xC and the bottom 13 bits will
// always be zero due to 8K alignment. 32-13-2 means 17 significant bits
// that actually need to be stored.

// But do you even need these?
#define UNPACK_PID(pfepid) ((PID)(((pfepid)<<13) | 0xC0000000))
#define PACK_PID(realpid) ((PID)(((realpid)>>13)& 0x1FFFF));

#define PFE_FREE(ptr) ((ptr)->prev == 0)
#define PFE_LAST_IN_CHAIN(ptr) ((ptr)->next == 0)

static int x = sizeof (PFE);

kernel CHID Chain_Alloc(
    U32 bytes,
    PID owner_pid
);

U32 Chain_Size(CHID chain);

STATUS Get_Chain_Physical_Addresses(
    CHID    chain,
    U32     base_index,
    U32     num_pages,
    PVOID*  out_computed
);

PVOID Chain_Walk(CHID id, U32 req_index);
VOID Chain_Init(VOID);

#endif /* MM_CHAIN_H */