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

typedef U32 CHID;

// 8-U8structure
tpkstruct
{
    WORD    rel_index   :14;
    U8   f_free      :1;
    WORD    next;
    WORD    prev;
    WORD    owner_pid;      // Only matters for first entry?
}MB,*P_MB;

//static int x = sizeof (MB);

CHID KERNEL ChainAlloc(
    U32        bytes,
    PID          owner_pid
);

STATUS KERNEL ChainExtend(
    CHID         id,
    U32        bytes_uncommit,
    U32        bytes_commit
);

PVOID ChainWalk(CHID id, U32 req_index)

#endif /* MM_CHAIN_H */