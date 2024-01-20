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

#define PNP_ROM_STRING BYTESWAP(0x24506e50) /* "$PnP" */

tpkstruct {
    U32     signature;
    U8      version;
    U8      length;
    U16     control_field;
    U8      checksum;
    PVOID   event_notification;
    U16     _real_mode_code_off;
    U16     _real_mode_code_seg;
    U16     protected_off;
    U32     protected_base;
    U32     oem_device_id;
    U16     _real_mode_data_seg;
    U16     protected_data_base;
}*PPNP_INSTALL_CHECK;

#endif /* PNP_BIOS_H */
