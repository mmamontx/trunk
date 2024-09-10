#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "balloc.h"

int mempool_init()
{
	handle = malloc(sizeof(struct balloc_handle));
	if (handle == NULL) {
		fprintf(stderr, "malloc() returned NULL\n");
		return -1;
	}

	handle->fd = open("mempool", O_CREAT | O_RDWR, 0666);
	if (handle->fd == -1) {
		fprintf(stderr, "open() returned -1\n");
		return -1;
	}

	handle->membase = mmap(NULL, BLOCK_SIZE * BLOCK_NUM + BLOCK_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, handle->fd, 0);
	if (handle->membase == MAP_FAILED) {
		fprintf(stderr, "mmap() returned MAP_FAILED");
		return -1;
	}

	handle->map = (char *)handle->membase + BLOCK_SIZE * BLOCK_NUM;

	return 0;
}

// This one is not very pretty as it is O(n) - since the occupied blocks are managed in place by the user there is no entity that maintains a list of free blocks, which would make it O(1)
int bseek() {
	int i;

	for (i = 0; i < BLOCK_NUM; i++) {
		if (handle->map[i] == 0)
			return i;
	}

	return -1;
}

void *balloc() {
	int n;

	if (handle == NULL)
		mempool_init();

	n = bseek();
	if (n == -1) {
		fprintf(stderr, "bseek() returned -1\n");
		return NULL;
	}

	handle->map[n] = 1; // In use

	void *addr = (char *)(handle->membase) + n * BLOCK_SIZE;
#if VERBOSE == 1
	printf("Allocated a single memory block of size %d at address %p.\n", BLOCK_SIZE, addr);
	printf("n = %d\n", n);
#endif

	return addr;
}

int bfree(void *ptr) {
	if (handle == NULL)
		mempool_init();

	int offset = (char *)ptr - (char *)(handle->membase);

	if (offset % BLOCK_SIZE) {
		fprintf(stderr, "bfree() got misaligned address.\n");
		return -1;
	}

	int n = offset / BLOCK_SIZE;
	handle->map[n] = 0;

#if VERBOSE == 1
	printf("Freed a block at address %p.\n", ptr);
#endif

	return 0;
}

void bclear() {
	if (handle == NULL)
		mempool_init();

	memset(handle->membase, 0, BLOCK_SIZE * BLOCK_NUM + BLOCK_NUM);
}

