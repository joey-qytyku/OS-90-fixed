#ifndef PNP_CORE_H
#define PNP_CORE_H

#include "Resource.h"
#include "Event.h"
#include "Bios.h"

#include <Type.h>

// Non-PnP drivers must be loaded first

#define DRV_IMPLEMENT_BUS        1
#define DRV_IMPLEMENT_DEV_PNP    2
#define DRV_IMPLEMENT_DEV_LEGACY 3

typedef struct {
    PIMUSTR     driver_name;
    PIMUSTR     description;
    PBYTE       cmdline;
    DWORD       driver_flags;
    PVOID       next_driver;
    FP_EVENT_HANDLER event_handler;
}DRIVER_HEADER,*PDRIVER_HEADER;

extern VOID InitPnP(VOID);

#endif /* PNP_CORE_H */
