#ifndef PNP_BIOS_H
#define PNP_BIOS_H

#include <Type.h>

typedef struct {
    DWORD   signature;
    BYTE    version;
    BYTE    length;
    WORD    control_field;
    BYTE    checksum;
    PVOID   event_notification;
    WORD    _real_mode_code_off;
    WORD    _real_mode_code_seg;
    WORD    protected_off;
    DWORD   protected_base;
    DWORD   oem_device_id;
    WORD    _real_mode_data_seg;
    WORD    protected_data_base;
}*PPNP_INSTALL_CHECK;

#endif /* PNP_BIOS_H */
