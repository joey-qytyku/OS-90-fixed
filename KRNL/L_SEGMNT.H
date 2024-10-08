#ifndef DESC_H
#define DESC_H

#define PACK __attribute__((packed))

typedef struct {
	SHORT   limit1;
	SHORT   s_base1;
	BYTE    b_base2;
	union {
		BYTE byte;
		struct {
			BYTE type:4;
			BYTE system:1;
			BYTE ring:2;
			BYTE present:1;
		}__attribute__((packed));
	}access;

	union {
		BYTE    byte;
		struct {
			BYTE limit2:4;
			BYTE avl:1;
			BYTE :1;
			BYTE is_32bit:1;
			BYTE page_granular:1;
		}__attribute__((packed));
	}extended;

	BYTE    b_base3;
}SEGMENT_DESCRIPTOR,*PSEGMENT_DESCRIPTOR;

API_DECL(VOID, L_SegmentCreate,
	SHORT   selector,
	LONG    base_addr,
	LONG    limit,
	BYTE    access,
	BYTE    exaccess);

API_DECL(VOID, L_SegmentSetLimit,
	SHORT selector,
	LONG limit
	);

API_DECL(VOID, L_SegmentCreate,
	SHORT   selector,
	LONG    base_addr,
	LONG    limit,
	BYTE    access,
	BYTE    exaccess
	);

#endif /* DESC_H */
