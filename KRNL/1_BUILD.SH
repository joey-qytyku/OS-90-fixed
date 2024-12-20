# Build the kernel binary first.

cd krnl/src

rm *.o

${KRNL_CC} ${KRNL_CFLAGS} -c *.c

${KRNL_LD} -T ../1_link.ld --oformat=binary -o ../krnl.bin

rm *.o

# Build bootloader
cd ../boot

# Convert kernel binary to hex include file
xxd -i ../krnl.bin | sed -n '2,$ p' | sed -n '$ ! p' | sed -n '$ ! p' > data.h

# Compile the
