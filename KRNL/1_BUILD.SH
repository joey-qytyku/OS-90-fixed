CFLAGS="-x c -Os -mgeneral-regs-only -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra -mpreferred-stack-boundary=2"

function cleanup() {
	rm -rf *.o
	rm -rf l_switch.bin
}

function check_errors() {
	if [ $1 -ne 0 ]
	then
		cleanup
		echo KRNL: ERRORS DURING BUILD
		exit 1
	fi
}

set -e

rm -f *.o l_switch.bin l_switch.h

nasm -fbin L_SWITCH.MSA -o l_switch.bin
check_errors $?

xxd -i l_switch.bin > l_switch.h
check_errors $?

i686-elf-gcc -include TYPE.H $CFLAGS -c *.c
check_errors $?

for x in *.asm
do
	nasm $x -felf
	check_errors $?
done

i686-elf-ld -T 1_link.ld *.o --oformat binary -o KERNEL.BIN
check_errors $?

cleanup
