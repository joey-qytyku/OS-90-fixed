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

#endif /* PNP_EVENT_H */
