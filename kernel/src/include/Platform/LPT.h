#ifndef LPT_H
#define LPT_H

#define LPT1_BASE 0x378
#define LPT2_BASE
#define LPT3_BASE

enum {
    ST_BUSY        = 128,
    ST_ACK         = 64,
    ST_PAPER_OUT   = 32,
    ST_SELECT_IN   = 16,
    ST_ERROR       = 8,
    ST_IRQ         = 4
};

enum {
    CR_STROBE      = 1,
    CR_AUTO_LF     = 2,
    CR_INITIALISE  = 4,
    CR_SELECT      = 8,
    CR_IRQACK      = 16,
    CR_BIDI        = 32
};


#endif /* LPT_H */
