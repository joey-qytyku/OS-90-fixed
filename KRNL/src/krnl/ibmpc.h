typedef struct __attribute__((packed)) {
	SHORT   comport[4];
	SHORT   lptport[3]; // [1]
	SHORT   ebdaseg;

	#define BIOS_EQP_IPL_DISKETTE   (BIT(1))
	#define BIOS_EQP_MATH_COPROC    (BIT(2))
	#define BIOS_EQP_MOUSE_PS2      (BIT(3)) /*Don't actually trust this*/
	#define BIOS_EQP_DEFAULT_VMODE  (BIT(4) | BIT(5))
	#define BIOS_EQP_NUM_FLP_DRIVES (BIT(6) | BIT(7))

	#define BIOS_EQP_DMA            (BIT(8))
	#define BIOS_EQP_NUM_COM_PORTS  (BIT(9) | BIT(10) | BIT(11))
	#define BIOS_EQP_GAME_ADAPTER   (BIT(12))
	#define BIOS_EQP_NUM_LPT_PORTS  (BIT(14) | BIT(15))
	BYTE    equip[2];

	BYTE    :8;
	SHORT   mem_kilos;      // Conventional
	BYTE    :8;
	BYTE    ps2_bios_control;

	#define BIOS_KF_RSHIFT          (BIT(0))
	#define BIOS_KF_LSHIFT          (BIT(1))
	#define BIOS_KF_CTRL            (BIT(2)) /* Any CTRL key */
	#define BIOS_KF_ALT             (BIT(3)) /* Any ALT key*/
	#define BIOS_KF_SCRLLOCK        (BIT(4))
	#define BIOS_KF_NUMLOCK         (BIT(5))
	#define BIOS_KF_CAPSLOCK        (BIT(6))
	#define BIOS_KF_INSERT          (BIT(7))
	SHORT   kbflags;
	BYTE    unknown_1; // Not sure




}BDA, *P_BDA;

// NOTE: Place the bit flag defines inline.

/*
[1]:
This is not used on PS/2 and greater systems because they cannot use a 4th one
one and use it for the EBDA. This is standard for PS/2 and better.

OS/90 supports 4 LPT ports on devices that support it by checking if the value
is within the 10-bit ISA-addressable port range. Anything higher than 1024
or 0x400 does not make sense because 0x4000 cannot be an address within the
BIOS ROM (where EBDA is).

*/
