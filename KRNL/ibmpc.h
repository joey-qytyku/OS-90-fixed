typedef enum {
	GSC_40x25 		= 0,
	COL_40x25 		= 1,
	GSC_80x25 		= 2,
	COL_80x25 		= 3,
	COL_320x200_2BIT 	= 4,
	GSC_320x200_2BIT	= 5,
	MON_640x200		= 6,
	MON_80x25		= 7,
	COL_160x200_4BIT 	= 8,
	COL_320x200_4BIT 	= 9,
	MON_640x350		= 0xF,
	COL_640x200_4BIT	= 0xA,
	MON_640x480		= 0x11,
	COL_640x480_4BIT	= 0x12,
	COL_320x200_8BIT	= 0x13

}BIOS_VMODE;

// Pack everything ahead, including substructures.
#pragma pack(push, 1)

typedef struct
{
	WORD	comport[4];
	WORD	lptport[3]; // [1]
	WORD	ebdaseg;

	// Because of little endianness, the bits appear
	// essentially in the order that one would expect.
	// No need for structure nonsense.
	union {
		struct {
			WORD	e_ipl_diskette	:1;
			WORD	e_math_coproc	:1;
			WORD	e_mouse_ps2	:1;
			WORD	e_default_vmode	:2;
			WORD	e_num_flp_drives:2;
			WORD	:1;

			WORD	e_dma		:1;
			WORD	e_num_com_ports	:3;
			WORD	e_game_adapter	:1;
			WORD	e_num_lpt_ports	:2;
			WORD	:1;
		};
		WORD equip;
	};

	// Used by the PCjr. Undefined here.
	BYTE	:8;

	// Conventional memory size. I recommend not using
	// this and calling INT 12H, which it matches with.
	WORD	mem_kilos;
	BYTE	:8;

	//
	BYTE	ps2_bios_control;

	union {
		struct {
		WORD kf_rshift:1;
		WORD kf_lshift:1;
		WORD kf_ctrl:1;		/* Any CTRL key */
		WORD kf_alt:1;		/* Any ALT key*/
		WORD kf_scrllock:1;
		WORD kf_numlock:1;
		WORD kf_capslock:1;
		WORD kf_insert:1;
		};
		WORD kflags;
	};
	BYTE	_40_19;
	WORD	kbuff_offset_head;
	WORD	kbuff_offset_tail;
	BYTE	kbuff[32];
	union {

		BYTE drv_recal_status:1;
	};
};

#pragma pack()

// NOTE: Place the bit flag defines inline.

/*
[1]:
This is not used on PS/2 and greater systems because they cannot use a 4th one
one and use it for the EBDA. This is standard for PS/2 and better.

OS/90 supports 4 LPT ports on devices that support it by checking if the value
is within the 10-bit ISA-addressable port range. Anything higher than 1024
or 0x400 does not make sense because 0x400 cannot be an address within the
BIOS ROM (where EBDA is).

The only computer with 4 LPTs that can run OS/90 is the Compaq Deskpro 386.

*/
