xxd -i ../KRNL/KERNEL.BIN | sed -n '2,$ p' | sed -n '$ ! p' | sed -n '$ ! p' > data.h

${WCL} -mc -lr -3 os90.c

rm data.h
