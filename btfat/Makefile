all: btfat main
	bt23d-ld *.o -G 0 -r -o 9.o

btfat:
	bt23d-gcc -c -I/home/mtov/OS3/include -I/home/mtov/OS3/bsp/bt205/lib/include -G 0 -nostdlib -r3000 btfat.c

main:
	bt23d-gcc -c -I/home/mtov/OS3/include -I/home/mtov/OS3/bsp/bt205/lib/include -G 0 -nostdlib -r3000 main.c

install:
	cp 9.o /tmp

clean:
	rm btfat.o
	rm main.o
	rm 9.o