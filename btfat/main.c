#include "btfat.h"

int main()
{
	int fd, i;
	char buf[13] = {'f'}, buf_read[13] = {0};
	off_t offset;
	size_t count;

	if (btfat(0xb8000000, 0) == -1) {
		btfat_perror();
		return -1;
	}

	fd = btfat_open("foo.txt");
	if (fd == -1) {
		btfat_perror();
		return -1;
	}

	memcpy(buf, "Hello, world!", 13);

	count = btfat_write(fd, buf, 13);

	count = btfat_read(fd, buf_read, 13);
	for (i = 0; i < 13; i++)
		printf("%c", buf_read[i]);
	printf("\n");

	memset(buf_read, 0, 13);

	offset = btfat_lseek(fd, 0, SEEK_SET);

	count = btfat_read(fd, buf_read, 7);
	for (i = 0; i < 7; i++)
		printf("%c", buf_read[i]);
	printf("\n");

	count = btfat_read(fd, buf_read + 7, 6);
	for (i = 0; i < 13; i++)
		printf("%c", buf_read[i]);
	printf("\n");

	btfat_close(fd);

	/*offset = btfat_lseek(fd, 0, SEEK_SET);
	printf("offset = %d\n", offset);
	if (offset == -1) {
		btfat_perror();
		return -1;
	}*/

	return 0;
}
