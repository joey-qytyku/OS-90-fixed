#include <Platform/IO.h>
#include <Debug/Debug.h>
#include <PnP/Resource.h>
#include <Platform/LPT.h>

// Line printer driver. Uses the first LPT port and reserves resources
// so there is no conflict with a real (TM) driver.

VOID LptPutchar(char ch)
{
    delay_outb(LPT1_BASE+2, 0);
    delay_outb(LPT1_BASE, 0);

    // Post the character to print
    delay_outb(LPT1_BASE, ch);

    // Wait for printer to not be busy, active low
    while (delay_inb(LPT1_BASE+1) & ST_BUSY == 0);

    // Perform handshake by pulsing the strobe pin
    U8 cr = delay_inb(LPT1_BASE+2);
    delay_outb(LPT1_BASE+2, cr | 1);
    delay_outb(LPT1_BASE+2, cr);

    // The printer is notified. Wait until done.

    while (delay_inb(LPT1_BASE+1) & ST_BUSY == 0);

    delay_outb(LPT1_BASE+2, 0);
    delay_outb(LPT1_BASE, 0);
}
