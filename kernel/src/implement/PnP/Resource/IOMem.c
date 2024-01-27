///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <PnP/Resource/IOMem.h>

static U32          cur_iorsc = 0;
static IO_RESOURCE  resources[MAX_IO_RSC];

kernel STATUS Add_IOMem_Rsc(PIO_RESOURCE new_rsc)
{
    if (cur_iorsc >= MAX_IO_RSC)
        return -1;
    resources[cur_iorsc] = *new_rsc;
    cur_iorsc++;
    return 0;
}

// Constructor for IO_RESOURCE
kernel VOID New_IOMem_Rsc( // Change prototype
    P_DRVHDR owner,
    PIO_RESOURCE i,
    U32     start,
    U32     size,
    U16     flags
){
    i->flags = flags;
    i->owner = owner;
    i->start = start;
    i->size  = size;
}

// BRIEF:
//      Allocate IO port space. Returns all ones (-1) if failed. Otherwise
//      returns base IO port.
//
//
kernel U32 Allocate_IO_Ports(U16 num, U8 align)
{
}

VOID Init_PnP_IOMem()
{
}
