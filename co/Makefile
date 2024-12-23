all:
	./co.pl
	cat header.asm main.asm footer.asm >foo.asm
	nasm -f elf64 foo.asm
	ld foo.o -o foo

clean:
	rm main.asm
	rm foo\.*
	rm foo
	rm *~
