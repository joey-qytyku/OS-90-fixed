#ifndef DESC_H
#define DESC_H

typedef _Packed struct {
	SHORT   limit1;
	SHORT   s_base1;
	BYTE    b_base2;
	union {
		BYTE byte;
		struct _Packed {
			BYTE type:4;
			BYTE system:1;
			BYTE ring:2;
			BYTE present:1;
		}
	}access;

	struct _Packed {
		BYTE limit2:4;
		BYTE avl:1;
		BYTE :1;
		BYTE is_32bit:1;
		BYTE page_granular:1;
		BYTE
	}extended;

	BYTE    b_base3;
}SEGMENT_DESCRIPTOR,*PSEGMENT_DESCRIPTOR;

#endif /* DESC_H */
