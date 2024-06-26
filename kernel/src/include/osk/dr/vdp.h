#ifndef VDP_H
#define VDP_H
// SIZE:

// UPDATE THIS
typedef struct {
        // SHORT   port;
        // SHORT   str;            // 0=Not string, 1=forward, 2=backward
        // SHORT   in_or_out;      // 0=input, 1=output
        // SHORT   iosize;         // 1=Byte, 2=word, 4=byte

        // LONG    es;
        // LONG    ds;

        // LONG    iters;
        // LONG    datum;
        // PVOID   source;
        // PVOID   dest;
}VDP_IO_PACKET,*P_VDP_IO_PACKET;
typedef VOID (IOHANDLER)(LONG times, PVOID source, PVOID dest);

typedef struct {
        IOHANDLER       ioh;
        SHORT           port;
        BYTE            byte_count;
}IOP_REGION;


typedef struct {
        char    name[8];
        LONG*   mmio_regions;
        SHORT*  iop_regions;
}VIODEV,*P_VIODEV;

//
// The addresses are valid in all modes and are calculated with the segment bases.
// Any invalid memory access is prevented and will not generate a VDP IO request.
//

OS_RegisterDevice()
{}

#endif
