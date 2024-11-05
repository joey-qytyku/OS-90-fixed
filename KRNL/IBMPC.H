typedef struct __attribute__((packed)) {
	SHORT   comport[4];
	SHORT   lptport[3]; // [1]
	SHORT   ebdaseg;
	BYTE    equip[2];

	BYTE    :8;
	SHORT   mem_kilos;      // Conventional
	BYTE    :8;
	BYTE    bios_control_flags;

}BDA, *P_BDA;

// NOTE: Place the bit flag defines inline.

/*
[1]: This is not used on PS/2 and greater systems because they cannot use a
	fourth one and may repurpose this byte.

*/
