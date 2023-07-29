#ifndef IA32_PNPSEG_H
#define IA32_PNPSEG_H

#include "Segment.h"

#define PnSetBiosDsegBase(base)\
    {IaAppendAddressToDescriptor(&aqwGlobalDescriptorTable + GDT_PNP_BIOS_DS*8,\
    base);}

#define PnSetOsDsegBase(base)\
    {IaAppendAddressToDescriptor(&aqwGlobalDescriptorTable + GDT_PNP_OS_DS*8\
    (U32)base);}

#define PnSetBiosCsegBase(base)\
    {IaAppendAddressToDescriptor(&aqwGlobalDescriptorTable + GDT_PNPCS*8,\
    (U32)base);}

#endif /* IA32_PNPSEG_H */
