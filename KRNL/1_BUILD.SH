CFLAGS="-Os -mgeneral-regs-only -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra -mpreferred-stack-boundary=2"

function cleanup() {
	rm -rf *.o
	rm -rf L_SWITCH.BIN
	rm -rf KERNEL.BIN
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

rm -f *.o L_SWITCH.BIN L_SWITCH.H

nasm -fbin L_SWITCH.MSA -o L_SWITCH.BIN
check_errors $?

xxd -i L_SWITCH.BIN > L_SWITCH.H
check_errors $?

i686-elf-gcc -x c -include TYPE.H $CFLAGS -c *.C
check_errors $?

for x in *.ASM
do
	nasm $x -felf
	check_errors $?
done

i686-elf-ld -T 1_LINK.LD *.o --oformat binary -o KERNEL.BIN
check_errors $?

cleanup
