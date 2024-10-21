set -e

CFLAGS="-Os -mgeneral-regs-only -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra -mpreferred-stack-boundary=2"

i686-elf-gcc -x c -include TYPE.H $CFLAGS -c *.C

for x in *.ASM
do
	nasm $x -felf
done

i686-elf-ld -T 1_LINK.LD *.o --oformat binary -o KERNEL.BIN

rm *.o
