/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#ifndef CHAIN_H
#define CHAIN_H
#include "page.h"

typedef struct {
        WORD    rel_index   :15;
        WORD    f_inuse     :1;

        SIGSHORT    next;
        SIGSHORT    prev;
}MB,*P_MB;

LONG M_Alloc(LONG bytes_commit, LONG bytes_uncommit);
VOID M_Free(LONG chain);

SIGLONG M_ResizeWithCommit(LONG chain, SIGLONG delta_bytes);
SIGLONG M_ExtendUncommit(LONG bytes);

#endif /* CHAIN_H */
