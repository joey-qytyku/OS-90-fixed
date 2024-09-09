set -e

CFLAGS="-Os -mgeneral-regs-only -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra"

i686-elf-gcc -x c -include TYPE.H $CFLAGS -c *.C

for x in *.ASM
do
	nasm $x -felf
done

i686-elf-ld -T 1_LINK.LD *.o

rm *.o
