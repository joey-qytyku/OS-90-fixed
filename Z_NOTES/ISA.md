# ISA Plug-and-play

## Read data port

This port supposedly uses 12-bit decode. That makes basically zero sense though because it's relocatable range is between 0x000-0x3FF, which is 10-bit addressable.

It seems like 10 is difficult to mistype as 12, so I think it may have been intentional. Or maybe there was a reason, perhaps to allow it to exceed the 10-bit range but no more than that.

We unfortunately have to put up with garbage because whoever wrote the spec wrote 12 instead of 16 for something that is already part of the chipset!

The OS/90 PnP resource manager therefore supports 10-bit, 12-bit, and 16-bit decode.

## Structure representation of ISA card

Each logical device has a limit to how many resources it is allowed to use. This has no bearing on the resource data read from the card because that is mainly to provide possibilities or limitations.

From the standard:

* Memory Address Base registers (up to four non-contiguous ranges)
* I/O Address Base registers (up to eight non-contiguous ranges)
* Interrupt Level Select registers (up to two separate interrupt levels)
* DMA Channel Select registers (up to two DMA channels )

This means an ISA device can be defined in one structure:


```c
// TODO
typedef struct {
	// Follows bit pattern defined in 6.2.2.5
	uchar dma_info_1;
	uchar dma_info_2;

	// Selects no more that 2 IRQs.
	ushort irq;

}ISADEV;
```

> ISA PnP allows for level triggered interrupts but ISA is primarily edge-triggered. This feature is not supported.
