# Virtual DMA Services and ISA DMA

OS/90 has an API that allows for plug-and-play allocation of DMA channels and programming DMA transfers.

Drivers can allocate DMA buffers at startup with a function that is ALLOWED TO FAIL but almost never will.

There are programs that will program DMA transfers directly or use VDS. The latter was in fact used, and does have to be supported.

## Description of DMA


> Programs that use DMA channels are usually utilizing non-PnP hardware, but it is possible to see the current assignments and do a pass-through with the correct number.

```
Virtual device drivers need to emulate transfers using the DMA subsystem. Sometimes, buffers used for DMA must be entirely on the software side.

Consider a soundblaster emulator. DMA transfers have to be simulated for that to work.
```
