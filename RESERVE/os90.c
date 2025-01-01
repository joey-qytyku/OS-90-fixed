
/*

WARNING! What you are about to see is the most unportable C code in exisitence.
It is also as messy as one would expect from a DOS C program.
Nothing here is the result of bad practice, just the requirements of the
project being met.

*/

#include <stdint.h>
#include <dos.h>

/* Require small model */

static uint8_t kdata [] = {
	#include "data.h"
};

static uint32_t kernel_phys_base;
static uint32_t kernel_bytes;

uint32_t xms;

static inline void setup_page_tables()
{
	__segment hma = 0xFFFF;

	uint32_t __based(hma) *page_dir = 0;
	uint32_t __based(hma) *page_tab_conv = 4096;
	uint32_t __based(hma) *page_tab_krnl = 8192;

	int i;

	kernel_phys_base &= ~0xFFF;

	/* Clear all bytes of the relevant HMA portion */
	for (i = 0; i < 4096*3 / 4; i++)
		(hma:>page_dir)[i] = 0;

	/* Conventional memory mapping are identity mapped
	 * ring-3, writable, and cache enabled.
	*/
	for (i = 0; i < 1024; i++) {
		(hma:>page_tab_conv)[i] = i | 7u;
		(hma:>page_tab_krnl)[i] = (kernel_phys_base+(1u<<14u)) | 3u;
	}
}

static inline void alloc_memory()
{
	_asm {

	}
}

static inline void copy_kernel()
{
}

int __declspec(noreturn) main()
{
	return 0;
}
