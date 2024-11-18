CFLAGS="-Os -mgeneral-regs-only -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra -mpreferred-stack-boundary=2"

function check_errors() {
	if [[$? -ne 0]]
	then
		rm *.o
		rm L_SWITCH.BIN
	fi
	echo KRNL: ERRORS DURING BUILD
	exit 1
}

set -e

rm -f *.o L_SWITCH.BIN L_SWITCH.H

nasm -fbin L_SWITCH.MSA -o L_SWITCH.BIN
check_errors

xxd -i L_SWITCH.BIN > L_SWITCH.H
check_errors

i686-elf-gcc -x c -include TYPE.H $CFLAGS -c *.C
check_errors

for x in *.ASM
do
	nasm $x -felf
	check_errors
done

i686-elf-ld -T 1_LINK.LD *.o --oformat binary -o KERNEL.BIN
check_errors

rm -rf *.o
