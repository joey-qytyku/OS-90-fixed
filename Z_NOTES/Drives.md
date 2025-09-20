# Drive Letters and BIOS Numbers (WIP)

The following obstactles exist when implementing block devices in 32-bit software:
- BIOS emulation of USB drives or other media as floppy disks or hard disks (which is implementation-defined)
- CD-ROM/ATAPI drivers such as MSCDEX allow any drive to be used (it has a drive argument) and this is done at run time.
- INT 13H is still the primary block device access method for compatibility reasons, which means the numbering must be the same.

Drive letters are also a sort of resource that is allocated for each unit on a block device.

> I am not sure INT 13H is actually good for developing FS drivers. I clearly need a better interface for issuing requests that only uses INT 13H when needed.

## Legacy emulation

INT 13H has an interface to determine if a device is emulated or not. This requires the 1998 extended INT 13H function AH=48h. If this is not there, it is assumed the device cannot do any floppy/HD emulation of other block devices like USB.

If the device is emulated, then there is nothing to really be done except allow a potential 32-bit driver to reclaim the drive letter/number.

> I highly doubt I will EVER make USB host controller drivers, or even block device support.

## ATAPI and removable devices

At boot time, each drive

> It is possible that I may have to use ATAPI drivers for DOS at the same time as 32-bit disk access. This would require the CD drive to be in the slave controller and only the master to use 32-bit disk access.

> Why not scan each drive letter at the DOS level to get the information?

> AH=32h allows getting the information about a drive.

> There are drivers for DOS that we may have to safely take control from. ATAPI can be used as described above. The real mode drivers can be detected.


