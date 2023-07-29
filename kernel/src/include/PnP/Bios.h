///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     Copyright (C) 2023, Joey Qytyku                       //
//                                                                           //
// This file is part of OS/90 and is published under the GNU General Public  //
// License version 2. A copy of this license should be included with the     //
// source code and can be found at <https://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef PNP_BIOS_H
#define PNP_BIOS_H

#include <Type.h>

typedef struct {
    U32   signature;
    U8   version;
    U8   length;
    WORD    control_field;
    U8   checksum;
    PVOID   event_notification;
    WORD    _real_mode_code_off;
    WORD    _real_mode_code_seg;
    WORD    protected_off;
    U32   protected_base;
    U32   oem_device_id;
    WORD    _real_mode_data_seg;
    WORD    protected_data_base;
}*PPNP_INSTALL_CHECK;

#endif /* PNP_BIOS_H */
