/**
 * @file Disk.c
 * @author Joey Qytyku
 * @brief ATA support, floppy drives TODO
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * Unfinished:
 * ATA_SetParam
 * ATA_Write
 * Identification for cache flushing
**/

#include <PnP_Mgr.h>
#include <Platform/ATA.h>
#include <Platform/IA32.h>
#include <Platform/IO.h>
#include <Type.h>

typedef struct { // TODO: What does this mean?
    word th;
    word ts;
    word tc;
    word cyls_per_head;

    word h, c, s;
}CHS_Params,*PCHS_Params;

typedef struct
{
    byte    exists:1;    /* If the status register is FF, no drive */
    byte    atapi:1;     /* If identify fails, it is ATAPI */
    byte    dsel:1;      /* Which disk is selected */
        /* This is because drive selection is slow
        // and requires a 15 ns delay (according to OSDev.org) */
    byte    flush_cache_cmd; /* 0x00 (NOP) if not supported */
}ATA_Drive;

ATA_Drive atapri, atasec;

dword CHS2LBA(PCHS_Params p)
{
    dword lba;
    // I *obviously* did not come up with this
    // https://pcrepairclass.tripod.com/cgi-bin/datarec1/chstolba.html
    lba = (p->c * p->th * p->ts) + (p->h * p->ts) + (p->s - 1);
    return lba;
}

// Returns the IO base or zero if error
static word SetParam(byte drive_num, word sectors, dword lba)
{
    byte bsectors, dsel;
    word io;

    // Drive numbers start at 2 for hard drives
    if (drive_num < 2 || sectors > 255)
        return 0;
    else if (sectors == 256)
        bsectors = 0;

    //Drive selecting requires reading the status 15 times
    dsel = drive_num & 1;
    if (drive_num)
        /*TODO*/;

    outb(io+2, sectors);
    outb(io+3, (byte)(lba &  0xFF));
    outb(io+4, (byte)(lba >> 8));
    outb(io+5, (byte)(lba >> 16));

    return io;
}

/**
 * @param drive     The drive number starting from 2
 * @return          Two meanings
 * @retval          -1 if no error but parameters invalid
 * @retval          Error byte if error (never zero if error)
*/
Status ATA_Read(const byte drive,
word sectors, const dword lba, const pvoid to)
{
    const word io = SetParam(drive, sectors, lba);
    byte stat;

    // Disable interrupts

    /**
     * If parameters invalid, fail with code -1
     * This means the error register was not read
     * but there is still an error
    **/
    if (!io)
        return -1;

    outb(io+7, CMD_READ_SECTORS);

    do {
        if (sectors == 0)
            break;

        rep_insw(to, 256, io); // ?
        stat = inb(io+7);

        if ((stat & STAT_ERR) >0) // Error Bit set?
            return (dword)inb(io+1);

        sectors--;
    } while (STAT_BSY != 0 && (stat & STAT_DRQ) > 1);
}

void ATA_Write(const pvoid from, const byte drive,
word sectors, const dword lba)
{
    // Flush cache is only supported on ATA-4 from 1998
    // IDENTIFY
}

void EarlyInitATA()
{
    if (inb(ATAP+7) != 0xFF)
        atapri.exists = 1;
    if (inb(ATAS+7) != 0xFF)
        atasec.exists = 1;
}
