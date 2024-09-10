#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "balloc.h"

int main() {
	int i;
	void *ptr;

	bclear(); // Erasing mem

	// Out of memory test

	// Allocate all blocks
	for (i = 0; i < BLOCK_NUM; i++) {
		ptr = balloc();
	}

	// Try allocate one more and ensure that it can't be done
	printf("Out of memory test: %s\n", (balloc() == NULL) ? "SUCCESS" : "FAIL");

	// Free test

	bfree(ptr);

	ptr = balloc();
	printf("bfree() test: %s\n", (ptr != NULL) ? "SUCCESS" : "FAIL");

	if (BLOCK_SIZE > 1) {
		// Misaligned test
	
		printf("Misaligned free() test: %s\n", (bfree(ptr + 1) == -1) ? "SUCCESS" : "FAIL");
	}

	bclear();

	// Multiprocess write & read test
	
	switch (fork()) {
	case -1:
		printf("fork() returned -1\n");
		exit(-1);
	case 0: // Child goes first and modifies a block
		ptr = balloc();
		((unsigned char *)ptr)[0] = 0xaa;
		bfree(ptr);
		exit(0);
	default: // Parent waits for child then verifies data
		wait(NULL);
		ptr = balloc(); // Since there are only two processes and bfree() doesn't reset the memory contents, we expect the same block to be allocated here having the same data
		printf("Multiprocess write & read test: %s\n", ((((unsigned char *)ptr)[0]) == 0xaa) ? "SUCCESS" : "FAIL");
	}

	return 0;
}

