#ifndef PNP_EVENT_H
#define PNP_EVENT_H

#include <Type.h>

typedef enum {
    MJ_PNP_UNLOAD,
    MJ_PNP_LOAD,
    MJ_PNP_DISABLE,
    MJ_PNP_ENABLE,
    MJ_REQUEST_INTERRUPT
}PNP_EVENT_CODE;

typedef struct
{
    WORD    major;
    WORD    minor;

}DRIVER_EVENT_PACKET,
*PDRIVER_EVENT_PACKET;

typedef VOID (*FP_EVENT_HANDLER) (PVOID);

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


#endif /* PNP_EVENT_H */
