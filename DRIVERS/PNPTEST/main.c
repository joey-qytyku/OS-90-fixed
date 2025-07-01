#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

typedef _Packed struct {
	unsigned __int8		sig[4];
	unsigned __int8		revision;
	unsigned __int8		length;
	unsigned __int16	control_field;
	unsigned __int8		checksum;
	unsigned __int32	event_ptr;
	unsigned __int16	rm_entry_off;
	unsigned __int16	rm_entry_seg;

	unsigned __int16	pm_entry_off;
	unsigned __int32	pm_entry_seg_base;

	unsigned __int32	oem_id;

	unsigned __int16	rm_data_seg;
	unsigned __int32	pm_data_seg_base;

}PNP_HDR;

// printf("%Ws"); // const char *far

static char pnp_sig[] = "$PnP";

static PNP_HDR __far * FindPnPHeader(void)
{
	char far * rom = 0xF000:>0x0000;
	unsigned __int16 i;
	PNP_HDR far *hdr;
	unsigned char sum = 0;

	for (i = 0; i <= 0xFFF0; i += 16)
	{
		if (_fmemcmp(rom + i, pnp_sig, 4) == 0) {
			hdr = 0xF000:>i;
			printf("Found the PnP header at address %Wp\n", hdr);
			break;
		}
	}
	/* Compute checksum */
	for (i = 0; i < hdr->length; i++)
	{
		sum += * (((char *__far)hdr)+i);
	}
	if (sum == 0)
	{
		puts("Checksum passed");
	}
	return hdr;
}

typedef int __cdecl far (*ENTRY_POINT)(int function, ...);

static ENTRY_POINT PNP;

static ENTRY_POINT CreateRmEntryPoint(PNP_HDR __far* hdr)
{
	void __far * r = MK_FP(hdr->rm_entry_seg, hdr->rm_entry_off);
	printf("RM entry point = %Wp\n", r);
	return r;
}

int main(int argc, char** argv)
{
	unsigned char  __far nn = 0;
	unsigned int   __far ns = 0;

	PNP_HDR far* hdr = FindPnPHeader();
	PNP = CreateRmEntryPoint(hdr);

	PNP(0, &nn, &ns, hdr->rm_data_seg);

	printf("%u device nodes with max size of %u bytes\n", nn, ns);
	return 0;
}
