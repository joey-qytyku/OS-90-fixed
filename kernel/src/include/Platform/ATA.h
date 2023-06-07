#ifndef ATA_H
#define ATA_H

#define ATAP 0x1F0
#define ATAS 0x170

#define CTLP (ATAP + 0x200)
#define CTLS (ATAS + 0x200)

// And plus != to zero to check if set
// Error register
enum ATA_ERROR {
    ERR_AMNF  = 1,
    ERR_TKZNF = 2,
    ERR_ABRT  = 4,
    ERR_MCR   = 8,
    ERR_IDNF  = 16,
    ERR_MC    = 32,
    ERR_UNC   = 64,
    ERR_BBK   = 128,
};

enum ATA_STATUS {
    STAT_ERR    = 1,
    STAT_IDX    = 2,
    STAT_CORR   = 4,
    STAT_DRQ    = 8,
    STAT_SRV    = 16,
    STAT_DF     = 32,
    STAT_RDY    = 64,
    STAT_BSY    = 128
};

#define CMD_NOP 0x00
#define CMD_READ_SECTORS  0x20
#define CMD_WRITE_SECTORS 0x30

// Only ATA-4+ has this, introduced 1998
#define CMD_FLUSH_CACHE 0xE7

/***
 * Supported on all drives after 1987
 * any 386 PC from the late 80's (mostly Compaq)
 * is garaunteed to support IDENTIFY
***/
#define CMD_IDENTIFY 0xEC

#endif /* ATA_H */
