///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <IA32/PnpSeg.h>

#include <Platform/IO.h>    /* Accessing interrupt mask register */

#include <Misc/Linker.h>    /* Accessing the PnP BIOS structure */

#include <PnP/Core.h>

#include <Debug/Debug.h>

DRVHDR g_kernel_driver_header = {
    .author      = "Joey Qytyku",
    .license     = "GPLv2",
    .description = "The OS/90 kernel."
};

VOID Init_PnP(VOID)
{
    // Clear all interrupt and resource entries. Zeroing them ensures they
    // are recognized as not in use.
}
