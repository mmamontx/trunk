block_size = 100
block_num = 128
file_size = $(shell echo $(block_size) \* $(block_num) + $(block_num) | bc)
current_dir = $(shell pwd)
verbose = 1 # Set to 0 to omit extra messages

balloc.o: balloc.c
	gcc -c balloc.c -o balloc.o -fPIC -W -Wall -pedantic -DBLOCK_SIZE=${block_size} -DBLOCK_NUM=${block_num} -DVERBOSE=${verbose}

libballoc.so: balloc.o
	gcc -shared -o libballoc.so balloc.o

mempool:
	dd if=/dev/zero of=mempool bs=$(file_size) count=1

test: test.c
	gcc test.c -o test -lballoc -L$(current_dir) -DBLOCK_SIZE=${block_size} -DBLOCK_NUM=${block_num}

all: libballoc.so test mempool

clean:
	rm balloc.o
	rm libballoc.so
	rm test
	rm mempool

