all: clean
	nasm reflect.asm -o REFLECT.BIN
	nasm boot386.asm -o OS90.COM

install-unix:
	mcopy OS90.COM C:/OS90/OS90.COM

clean:
	-rm OS90.COM
	-rm REFLECT.BIN
