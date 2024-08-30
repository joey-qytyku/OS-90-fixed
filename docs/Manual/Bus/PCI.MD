## OS/90 PCI Support

OS/90 has a driver called `PCI.RSX` that communicates with all device drivers that implement a PCI device.

64-bit PCI devices are not supported.

Features include:
- Proper handling of duplicate devices
- Handles unconfigurable devices like VGA
- Device-to-driver binding
- Automatic reconfiguration for ports and MMIO
- Single bus scan
- Support for legacy drivers
- Cache-enabled MMIO mapping
- Line-based interrupt handling
- PCI configuration cached to disk

### Calling Interface

PCI functions are exposed using a call table. The calling conventions are cdecl.

### API Listing

Device calls:

// LONG BarIORead(BYTE bar_and_size, LONG offset);
// VOID BarIOWrite(BYTE bar_and_size, LONG offset, );


```
BYTE PCI_CountDevices(LONG v_d);
```
Returns the number of devices in the PCI bus that have a certain identification code, which includes the vendor ID and the device ID in the exact way it appears in the PCI configuration space.

Note that a byte is returned.

```
VOID PCI_OpenDevices(LONG v_d, HPCIDEV *handles, BYTE count);
```
Opens all devices with this ID and returns the handles in a buffer. If the driver really does not want to handle multiple devices, it can decide the exact count, but the order of detection is not predictable.

```
STAT PCI_DeviceSetIrqState(HPCIDEV dev, LONG enable);
```
Enable or disable interrupt requests from the PCI device.

```
LONG PCI_DeviceReadConfig(HPCIDEV dev, BYTE offset);
```

```
VOID PCI_DeviceWriteConfig(HPCIDEV dev, BYTE offset, LONG value);
```

```
VOID PCI_BarSetMemoryMode(PCI_MEM_MODE mode);
```

```
STAT PCI_CloseDevices(HPCIDEV *handles, BYTE num_handles);
```

```
STAT PCI_DeleteDevices(LONG v_d);
```
