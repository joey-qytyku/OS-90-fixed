#ifndef MMSWAP_H
#define MMSWAP_H

enum {
	SWAPCTL_ENABLE_SWAP,
	//
	// Enable the swap file at root of boot disk.
	// The FS driver should call it if it finds a valid swap file
	// in the root directory.
	//

	SWAPCTL_NOPHYS,
	//
	// Called by system to indicate that there are no page frames for the
	// allocator.
	//
};

// Why not make it transparent? Allocate on swap using M_Alloc?

typedef LONG (*MMSWAP_CTLPROC)();

#endif /* MMSWAP_H */
