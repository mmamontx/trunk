int mempool_init();
void *balloc();
int bfree(void *ptr);
void bclear();

struct balloc_handle {
	int fd;
	void *membase;
	char *map; // The size of the metadata can be reduced in 8 by using bits instead of bytes causing some extra calculations
};

struct balloc_handle *handle = NULL;

