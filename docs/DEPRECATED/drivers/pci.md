# PCI Support in OS/90

A bus driver called PCI.DRV implements PCI support. It allows for IRQ steering and memory and IO space configuration.

## Memory BAR Handling

The size of the bar is always a power of 2 and is determined by:
1. Save the value of the BAR
2. Write FFFFFFFF to the BAR
3. Read the value back
4. Mask the information bits (varies between IO and mem)
5. Bitwise not
6. Increment by one

Steps 5 and 6 are done because the return of the operation is a mask (ISA PnP has this feature too) and should be converted to a number. The size also indicates the same alignment and bits that do not match the aligment should not be set.



