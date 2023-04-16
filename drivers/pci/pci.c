#include <Type.h>

#define HEADER_STD 0
#define HEADER_PCI_TO_PCI 1
#define HEADER_CARDBUS_TO_PCI 2

typedef struct {
        dword id_vendor;
        byte header_type;
        byte multifunc;
}       PciDevInfo,*PPciDevInfo;

typedef struct {
        dword bus, dev, func, offset;
}       PciDevAddr,*PPciDevAddr;

dword MakeAddress(PPciDevAddr pcia)
{
    if (pcia->offset & 0x3 > 0)
        ;
    dword ret = (pcia->func << 8) | pcia->dev << 11 | pcia->bus << 16 | 1<<31;
    return ret
}

byte PciCfgReadByte()
{}

byte PciCfgReadWord()
{}

byte PciCfgReadDword(PPciDevAddr addr)
{}

// 0-Does not exist
Status IdentifyPciDev(PciDevInfo);
