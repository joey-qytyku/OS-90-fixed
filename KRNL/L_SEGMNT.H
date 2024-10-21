#ifndef DESC_H
#define DESC_H

typedef struct __attribute__((packed)) {
	SHORT   limit1;
	SHORT   s_base1;
	BYTE    b_base2;

	BYTE    access_byte;
	BYTE    extaccess_byte;
	BYTE    b_base3;
}SEGMENT_DESCRIPTOR,*PSEGMENT_DESCRIPTOR;

API_DECL(VOID, L_SegmentCreate,
	SHORT   selector,
	LONG    base_addr,
	LONG    limit,
	LONG    access,
	LONG    exaccess);

API_DECL(VOID, L_SegmentSetLimit,
	SHORT selector,
	LONG limit
	);


#endif /* DESC_H */
