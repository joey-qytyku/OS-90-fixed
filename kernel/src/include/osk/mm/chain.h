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
#include <Type.h>
#include "page.h"

//
// Every mapped page has a handler procedure associated with the
// physical block table entry. When an access to a hooked page
// is captured or the allocator decides to discard a collateral
// page, this handler is called.
//
typedef VOID (PAGE_PROC*)(
        LONG    page_table_entry,
        PVOID   virtual_address
);

typedef struct {
        PAGE_PROC       page_proc;
        SHORT           extra;          // Use as hint for how many contigous blocks after?
        SHORT           rel_index;
        SIGSHORT        next;
        SIGSHORT        prev;
}MB,*P_MB;

//
// Maybe I am onto something with the contigous blocks idea.
// Perhaps I can keep the PBT and the MB, but allow for a single
// entry to represent more than one page frame.
// This keeps the structure largely the same but improves
// effificiency.
//

LONG M_Alloc(LONG bytes_commit, LONG bytes_uncommit);
VOID M_Free(LONG chain);

SIGLONG M_ResizeWithCommit(LONG chain, SIGLONG delta_bytes);
SIGLONG M_ExtendUncommit(LONG bytes);

#endif /* CHAIN_H */
