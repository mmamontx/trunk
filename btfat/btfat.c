#include <stdio.h>
#include <errno.h>

#include <flashLib.h>

#include "btfat.h"

int block_count;
int file_count;
int *fat;
void *superblock;
struct fatl *fatlist;

	/*
	 * Looking  for  given  name  in  FAT  list,  if not present: allocating
	 * memory,  initializing structure and looking for free file descriptor,
	 * then pushing to list.
	 */
struct fatl *fatl_push(char *name)
{
	int i, rv, fc;
	struct fatl *fl, *ptr;
	unsigned char flag;
	unsigned char buf[FS_MAXSIZE + POOLSIZE];

	ptr = fatlist;
	while (ptr != NULL)
		if (strcmp(ptr->info.name, name) == 0) {
			if (ptr->fd == -1)
				ptr->fd = find_free_fd();

			return ptr;
		} else {
			ptr = ptr->next;
		}

	fl = (struct fatl *)malloc(sizeof(struct fatl));
	if (fl == NULL) {
		btfat_errno = BTFAT_ENOMEM;
		return NULL;
	}

	fl->offset = 0;
	fl->info.fi = -1;
	fl->info.len = 0;
	fl->next = NULL;

	fl->info.name = (char *)malloc(strlen(name) + 1);
	if (fl->info.name == NULL) {
		btfat_errno = BTFAT_ENOMEM;
		return NULL;
	}

	strcpy(fl->info.name, name);
	name[strlen(name)] = '\0';

	if (fatlist == NULL) {
		fatlist = fl;

		fl->fd = 1;
	} else {
		fl->fd = find_free_fd();

		ptr = fatlist;
		while (ptr->next != NULL)
			ptr = ptr->next;

		ptr->next = fl;
	}

	rv = flashReadBlock((char *)buf, (char *)superblock, FS_MAXSIZE + POOLSIZE);
	if (rv != 0) {
		perror(NULL);
		btfat_errno = BTFAT_EIO;
		return NULL;
	}

	fc = file_count;
	for (i = 0; fc > 0; i++)
		if (buf[FS_SIZE + i] == '\0')
			fc--;

	printf("i = %d\n", i);
	strcpy(buf + FS_MAXSIZE + i, name);

	file_count++;

	*((int *)buf + 1) = file_count;

	rv = flashEraseSector(((int)superblock - 0xb8000000) / BLOCK_SIZE);
	if (rv != 0) {
		perror(NULL);
		btfat_errno = BTFAT_EIO;
		return NULL;
	}

	rv = flashWriteBlock((char *)superblock, (char *)buf, FS_MAXSIZE + POOLSIZE, 1, 0);
	if (rv != 0) {
		perror(NULL);
		btfat_errno = BTFAT_EIO;
		return NULL;
	}

	return fl;
}

int btfat_open(char *name)
{
	struct fatl *fl;

	fl = fatl_push(name);
	if (fl == NULL)
		return -1;

	return fl->fd;
}

int btfat_close(int fd)
{
	struct fatl *fl;

	fl = fatlist;
	while (fl != NULL)
		if (fl->fd == fd) {
			fl->fd = -1;
			break;
		} else {
			fl = fl->next;
		}

	if (fl == NULL) {
		btfat_errno = BTFAT_EBADF;
		return -1;
	}

	return 0;
}

void btfat_perror()
{
	switch (btfat_errno) {
	case 0:
		fprintf(stderr, "Succes.\n");
		break;
	case BTFAT_ENOMEM:
		fprintf(stderr, "Out of memory.\n");
		break;
	case BTFAT_EBADF:
		fprintf(stderr, "Bad file number.\n");
		break;
	case BTFAT_ENFILE:
		fprintf(stderr, "File table overflow.\n");
		break;
	case BTFAT_EFAULT:
		fprintf(stderr, "Bad address.\n");
		break;
	case BTFAT_EIO:
		fprintf(stderr, "I/O error.\n");
	}
}

int find_free_fd()
{
	int i;
	struct fatl *fl;
	unsigned char flag;

	for (i = 1; ; i++) {
		flag = 0;

		fl = fatlist; 
		do {
			if (fl->fd == i) {
				flag = 1;
				break;
			}

			fl = fl->next;
		} while (fl != NULL);

		if (!flag)
			break;
	}

	return i;
}

int find_free_block()
{
	int i;

	for (i = 0; i < block_count; i++)
		if (fat[i] == -1)
			break;

	return i;
}

int update_superblock()
{
	int i, fc;
	int buf[FS_MAXSIZE + POOLSIZE / sizeof(int)];
	struct fatl *fl;

	printf("Updating superblock %d...", ((int)superblock - 0xb8000000) / BLOCK_SIZE);

	buf[0] = block_count;
	buf[1] = file_count;

	memcpy(buf + 2, fat, block_count * sizeof(int));

	fl = fatlist;
	for (i = 0; fl != NULL; i++) {
		memcpy(buf + 2 + block_count + sizeof(struct fatfd) * i, &(fl->info), sizeof(struct fatfd));

		fc = i;
		while (fc > 0) {

			fc--;
		}

		fl = fl->next;
	}

	if (flashEraseSector(((int)superblock - 0xb8000000) / BLOCK_SIZE) != 0) {
		perror(NULL);
		btfat_errno = BTFAT_EIO;
		return -1;
	}

	if (flashWriteBlock((char *)superblock, (char *)buf, FS_MAXSIZE + POOLSIZE, 1, 0) != 0) {
		perror(NULL);
		btfat_errno = BTFAT_EIO;
		return -1;
	}

	puts(" Done.");

	for (i = 0; i < block_count + 2; i++)
		printf("%d ", buf[i]);
	puts("");

	return 0;
}

int btfat(void *addr, unsigned char flag)
{
	int i, rv;
	struct fatl *fl;
	int buf[BLOCK_SIZE / sizeof(int)];

	if ((int)addr < 0xb8000000) {
		btfat_errno = BTFAT_EFAULT;
		return -1;
	}

	superblock = addr;

	if (flag == 0) {
		rv = flashReadBlock((char *)buf, (char *)addr, BLOCK_SIZE / 2);
		if (rv != 0) {
			perror(NULL);
			btfat_errno = BTFAT_EIO;
			return -1;
		}

		block_count = buf[0];
		file_count = buf[1];
		printf("block_count = %d\n", block_count);
		printf(" file_count = %d\n", file_count);

		if (file_count > block_count) {
			btfat_errno = BTFAT_ENFILE;
			return -1;
		}

		fat = (int *)malloc(block_count * sizeof(int));
		if (fat == NULL) {
			btfat_errno = BTFAT_ENOMEM;
			return -1;
		}

		memcpy(fat, buf + 2, block_count * sizeof(int));

		if (file_count > 0)
			for (i = 0; i < file_count; i++) {
				fl = (struct fatl *)malloc(sizeof(struct fatl));
				if (fl == NULL) {
					btfat_errno = BTFAT_ENOMEM;
					return -1;
				}

				fl->fd = -1;
				fl->offset = 0;

				memcpy(&(fl->info), buf + 2 + block_count, sizeof(struct fatfd));

				if (i != file_count - 1) {
					fl = (struct fatl *)malloc(sizeof(struct fatl));
					if (fl == NULL) {
						btfat_errno = BTFAT_ENOMEM;
						return -1;
					}
				} else {
					fl->next = NULL;
				}
			}
	} else {
		block_count = 244 - 1;
		file_count = 0;

		fat = (int *)malloc(block_count * sizeof(int));
		if (fat == NULL) {
			btfat_errno = BTFAT_ENOMEM;
			return -1;
		}

		for (i = 0; i < block_count; i++)
			fat[i] = -1;

		if (update_superblock() == -1)
			return -1;
	}

	return 0;
}

off_t btfat_lseek(int fd, off_t offset, int whence)
{
	struct fatl *fl;

	fl = fatlist;
	while (fl != NULL)
		if (fl->fd == fd)
			break;
		else
			fl = fl->next;

	if (fl == NULL) {
		btfat_errno = BTFAT_EBADF;
		return -1;
	}

	switch (whence) {
	case SEEK_SET:
		fl->offset = offset;
		break;
	case SEEK_CUR:
		fl->offset += offset;
		break;
	case SEEK_END:
		fl->offset = fl->info.len + offset;
	}

	return fl->offset;
}

	/*
	 * Looking  for  given  file descriptor in FAT list, if present: reading
	 * file block by block.
	 */
ssize_t btfat_read(int fd, void *buf, size_t count)
{
	int block_count, fi, i, rv;
	off_t offset;
	size_t b;
	struct fatl *fl;

	if (count == 0)
		return 0;

	fl = fatlist;
	while (fl != NULL)
		if (fl->fd == fd)
			break;
		else
			fl = fl->next;

	if (fl == NULL) {
		btfat_errno = BTFAT_EBADF;
		return -1;
	}

	if (fl->info.fi == -1)
		return 0;

	if (count > fl->info.len - fl->offset)
		count = fl->info.len - fl->offset;

	offset = fl->offset;

	fi = fl->info.fi;
	i = 0;
	i += (int)floor((double)offset / BLOCK_SIZE);
	offset -= BLOCK_SIZE * i;
	while (count > 0) {
		if (count > BLOCK_SIZE - offset)
			b = BLOCK_SIZE - offset;
		else
			b = count;

		rv = flashReadBlock((char *)buf, (char *)BLOCK_FIRST + fi * BLOCK_SIZE + offset, b);
		if (rv != 0) {
			perror(NULL);
			btfat_errno = BTFAT_EIO;
			return -1;
		}

		if (fat[fi] != -1) {
			fi = fat[fi];
			offset = 0;
			i++;
			count -= b;
		} else {
			break;
		}
	}

	fl->offset += BLOCK_SIZE * i + b - offset;
	if (fl->offset > fl->info.len)
		fl->offset = fl->info.len;

	return count;
}

	/*
	 * Looking for given file descriptor in FAT list, if present: allocating
	 * first block (if need to) and writing file block by block.
	 */
ssize_t btfat_write(int fd, void *buf, size_t count)
{
	int block_count, fi, i, rv;
	off_t offset;
	size_t b;
	struct fatl *fl;

	if (count == 0)
		return 0;

	fl = fatlist;
	while (fl != NULL)
		if (fl->fd == fd)
			break;
		else
			fl = fl->next;

	if (fl == NULL) {
		btfat_errno = BTFAT_EBADF;
		return -1;
	}

	if (fl->info.fi == -1) {
		fl->info.fi = find_free_block();

		if (update_superblock() == -1)
			return -1;
	}

	offset = fl->offset;

	fi = fl->info.fi;
	i = 0;
	i += (int)floor((double)offset / BLOCK_SIZE);
	offset -= BLOCK_SIZE * i;

	while (count > 0) {
		if (count > BLOCK_SIZE - offset)
			b = BLOCK_SIZE - offset;
		else
			b = count;

		rv = flashEraseSector(fi);
		if (rv != 0) {
			perror(NULL);
			btfat_errno = BTFAT_EIO;
			return -1;
		}

		rv = flashWriteBlock((char *)BLOCK_FIRST + fi * BLOCK_SIZE + offset, (char *)buf + BLOCK_SIZE * i, b, 1, 0);
		if (rv != 0) {
			perror(NULL);
			btfat_errno = BTFAT_EIO;
			return -1;
		}

		count -= b;

		if (count > 0) {
			fi = find_free_block();
			fat[fi] = fi;
			offset = 0;
		}
	}

	fl->info.len = BLOCK_SIZE * i + b;

	return count;
}
