> Idea: drivers do not get missing symbols errors until that symbol is called. A stub can be used. This way, PCI can be used only if available on legacy drivers.

# OS/90 PCI Support

OS/90 has a driver called `PCI.RZM` that communicates with all device drivers that implement a PCI device.

The PCI bus driver requires handling of multiple devices of the same type.

64-bit PCI devices are not supported.

Features include:
- Proper handling of duplicate devices
- Handles unconfigurable devices like VGA
- Device-to-driver binding
- Automatic reconfiguration for ports and MMIO
- Single bus scan
- Support for legacy drivers
- Cache-enabled MMIO mapping
- Port and MMIO indiscriminant device IO based on BARs.
- Line-based interrupt handling
- PCI configuration cached to disk

Device calls:
```
// LONG BarIORead(BYTE bar_and_size, LONG offset);
// VOID BarIOWrite(BYTE bar_and_size, LONG offset, );


BYTE PCI_GetNumDevsWithSpec(LONG v_d);

HPCIDEV *PCI_OpenDevices(LONG v_d);

STAT PCI_DeviceSetIrqState(HPCIDEV dev, LONG enable);
LONG PCI_ReadConfig(HPCIDEV dev, LONG offset);
VOID PCI_WriteConfig(HPCIDEV dev, LONG offset, LONG value);
VOID PCI_BarSetMemoryMode(PCI_MEM_MODE mode);

STAT PCI_CloseDevices(HPCIDEV *handles, BYTE num_handles);
STAT PCI_DeleteDevices(LONG v_d);

```
