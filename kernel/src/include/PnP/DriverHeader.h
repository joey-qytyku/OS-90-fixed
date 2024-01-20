///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef PNP_DRIVER_HEADER
#define PNP_DRIVER_HEADER

#include "Event.h"

typedef struct {
    char       *name,
               *description,
               *author,
               *license,
               *command_line;
    U32         drv_ver_bcd;
    U32         min_os_ver_bcd;

    MBHND       default_mbox;

    U8          is_pnp;

}DRVHDR,*P_DRVHDR;

// Not to be used by drivers
extern DRVHDR g_kernel_driver_header;

#endif /* PNP_DRIVER_HEADER */
