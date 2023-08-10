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

tpkstruct
{
    U16     rel_index   :14;
    U8      f_free      :1;
    U16     next;
    U16     prev;
    U16     owner_pid;
}MB,*P_MB;

//static int x = sizeof (MB);

CHID KERNEL ChainAlloc(
    U32 bytes,
    PID owner_pid
);

U32 ChainSize(CHID chain);

STATUS KERNEL ChainExtend(
    CHID    id,
    U32     bytes_uncommit,
    U32     bytes_commit
);

PVOID ChainWalk(CHID id, U32 req_index);
VOID ChainInit(VOID);

#endif /* MM_CHAIN_H */