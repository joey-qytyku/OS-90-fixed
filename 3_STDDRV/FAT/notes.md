# FAT Driver Structure

## Structure of FAT Filesystem

BOOT | Reserved? | FAT(s) | Root Dir | .... |

## Differences Between FAT Versions

The main difference is the addressing used is with different bits.

## Driver Organization

Every filesystem requires open, close, read, write, and other things. These must be implemented at the lowest level in the driver.

Access to the disk is done by calling INT 10H using an extended interface that is expected to be there. HDPMI can be used for testing on DOS.

### Disk Interface
