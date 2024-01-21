///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <PnP/BIOS/BIOS.h>
#include <IA32/PnpSeg.h>

// Scan the ROM space for "$PnP" at a 2K boundary
STATUS Setup_PnP_BIOS(VOID) // sussy
{
    // ROM space should not be prefetched or written
    volatile PPNP_INSTALL_CHECK checkstruct = (PPNP_INSTALL_CHECK)0xF0000;
    BOOL supports_pnp;
    U8   compute_checksum;

    // Find the checkstruct
    for (U32 i = 0; i<0x800*32; i++)
    {
        if (checkstruct->signature == PNP_ROM_STRING)
        {
            supports_pnp=1;
            break;
        }
        checkstruct += 0x800; //????
    }
    if (!supports_pnp)
        return OS_FEATURE_NOT_SUPPORTED;

    PnSetBiosDsegBase(checkstruct->protected_data_base);
    PnSetBiosCsegBase(checkstruct->protected_base);

    return OS_OK;
}

