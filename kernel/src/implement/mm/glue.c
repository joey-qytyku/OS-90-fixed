#include <osk/sd/basicatomic.h>
#include <osk/sd/sv86.h>

#include <osk/mc/pio.h>

#include <osk/db/debug.h>

static SHORT GetCmosBytePair(SHORT base)
{
        BYTE b1, b2;

        PREEMPT_INC();

        BYTE old = inb(0x70);

        outb(0x70, base+1);
        b1 = inb(0x71);

        outb(0x70, base);
        b2 = inb(0x71);

        outb(0x70, old);

        PREEMPT_DEC();
        return (b1 << 8) | b2;
}

LONG g_extended_pages;

LONG GetExtendedMemPages()
{
        SHORT size_simple = GetCmosBytePair(0x17) >> 2;

        if (size_simple == 0xFFFF) {
                // In this case, the system has more than 64MB of memory.
                // Only a BIOS function can figure this out.
                static STDREGS regs;
        }

        return g_extended_pages;
}

extern int END_BSS;

VOID M_Init(VOID)
{
        // Remember to include memory hole in PBT

        g_extended_pages = GetExtendedMemPages();

        PLONG page_dir;

        __asm__ volatile ("movl %%cr3,%0":"=r"(page_dir)::"memory");
        page_dir = (PLONG)( ((LONG)page_dir) & 0xFFFFF000 );

        //
        // Find where the kernel was loaded and its size in pages
        //
        PVOID kernel_phys_base = (*(PLONG)(page_dir[512] & 0xFFFFF000)) & 0xFFFFF000;

        debug_log("Kernel load address: %p\n", kernel_phys_base);
}
