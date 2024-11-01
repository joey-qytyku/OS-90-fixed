DRIVER_DECL(
	NAME("EATPAGES"),
	LICENSE("GPLv2"),
	AUTHORS("Joey Qytyku"),
	CNOTICE("(C) Joey Qytyku 2024"),
	GEVENTHND(DriverEvents)
);

static PVOID alloced_region;

static VOID DoEatPages(VOID)
{
	alloced_region = API.Vm_Alloc(NULL, M_GetInfo(1), 0, PG_W);
}

static VOID FreePages()
{
	API.Vm_Free(alloced_region);
}

VOID DriverEvents(PEVENT_BLOCK peb)
{
	swich (peb->code)
	{
		case GE_LOAD:
			DoEatPages();
		break;

		case GE_UNLOAD:
			FreePages();
		break;
	}
}
