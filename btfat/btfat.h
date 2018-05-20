	/*
	 * BageT File Allocation Table.
	 * Sep 30 2014.
	 */

#ifndef _BTFAT_H
# define _BTFAT_H 1

#include <stdio.h>

#define BLOCK_SIZE  524288/* Block size: 512KB. */
#define BLOCK_FIRST 0xb8080000

#define FS_SIZE    sizeof(int) * (2 + block_count) + sizeof(struct fatfd) * file_count
#define FS_MAXSIZE sizeof(int) * (2 + block_count) + sizeof(struct fatfd) * block_count

#define POOLSIZE 1024

enum {
	BTFAT_ENOMEM = 1,
	BTFAT_EBADF,
	BTFAT_ENFILE,
	BTFAT_EFAULT,
	BTFAT_EIO,
}btfat_errno;

struct fatfd {/* File Allocation Table File Descriptor. */
	char *name;
	int fi;
	size_t len;
};

struct fatl {/* File Allocation Table List. */
	int fd;
	off_t offset;
	struct fatfd info;
	struct fatl *next;
};

int btfat_close(int fd);
int btfat_open(char *name);
off_t lseek(int fd, off_t offset, int whence);
ssize_t btfat_read(int fd, void *buf, size_t count);
ssize_t btfat_write(int fd, void *buf, size_t count);
struct fatl *fatl_push(char *name);

#endif /* !_BTFAT_H */
