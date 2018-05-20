/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "mkio.h"

#include <errno.h>
#include <fcntl.h>

int fd;
pthread_mutex_t mkio = PTHREAD_MUTEX_INITIALIZER;

int mkio_send(const unsigned int addr, const unsigned int saddr, const unsigned int len, const unsigned short *buf)
{
	int rv, rv_trylock, stat;
	unsigned char i;
	unsigned short out[33];

	if (addr > 31)
		return -1;
	if (saddr > 31)
		return -1;
	if (len > 32)
		return -1;
	if (!len)
		return 0;

	do {
		rv_trylock = pthread_mutex_trylock(&mkio);
		if (!rv_trylock) {
			out[0] = (addr << 11) | (saddr << 5);

			if (len != 32)
				out[0] |= len;

			for(i = 0; i < len; i++)
				out[i + 1] = buf[i];

			if (write(fd, out, len + 1) == -1) {
				assert_perror(errno);
				return -1;
			}

			while (1) {
				rv = ioctl(fd, KK_CHECK_READY, NULL);
				if (rv > 0) {
					break;
				} else if (rv == -1) {
					assert_perror(errno);
					return -1;
				}
			}

			stat = ioctl(fd, KK_POST_WRITE, out);

			rv = pthread_mutex_unlock(&mkio);
			if (rv) {
				assert_perror(rv);
				return -1;
			}

			/*if (stat == -1) {
				assert_perror(errno);
				return -1;
			}*/
		} else {
			if (rv_trylock == EBUSY) {
				rv = pthread_yield();
				if (rv) {
					assert_perror(rv);
					return -1;
				}
			} else {
				assert_perror(rv_trylock);
				return -1;
			}
		}
	} while (rv_trylock);

	return stat;
}

int mkio_recv(const unsigned int addr, const unsigned int saddr, const unsigned int len, unsigned short *buf)
{
	int rv, rv_trylock, stat;
	unsigned char i;
	unsigned short in[33], j;

	if (addr > 31)
		return -1;
	if (saddr > 31)
		return -1;
	if (len > 32)
		return -1;
	if (!len)
		return 0;

	do {
		rv_trylock = pthread_mutex_trylock(&mkio);
		if (!rv_trylock) {
			in[0] = (addr << 11) | (saddr << 5) | 0x400;

			if (len != 32)
				in[0] |= len;

			for (i = 0; i < 5; i++) {
				if (read(fd, in, len + 1) == -1) {
					assert_perror(errno);
					return -1;
				}

				for (j = 0; j < 2000; j++) {
					rv = ioctl(fd, KK_CHECK_READY, NULL);
					if (rv > 0) {
						break;
					} else if (rv == -1) {
						assert_perror(errno);
						return -1;
					}
				}

				if(rv > 0)
					break;

				delay(1000, 50);
			}

			if (rv <= 0) {
				rv = pthread_mutex_unlock(&mkio);
				if (rv)
					assert_perror(rv);

				return -1;
			}

			stat = ioctl(fd, KK_POST_READ, in);
			if (stat == -1) {
				assert_perror(errno);
				return -1;
			} else {
				if (stat < 0)
					for (i = 0; i < len; i++)
						buf[i] = 0xffff;
				else
					for (i = 0; i < len; i++)
						buf[i] = in[i];
			}

			rv = pthread_mutex_unlock(&mkio);
			if (rv) {
				assert_perror(rv);
				return -1;
			}
		} else {
			if (rv_trylock == EBUSY) {
				rv = pthread_yield();
				if (rv) {
					assert_perror(rv);
					return -1;
				}
			} else {
				assert_perror(rv_trylock);
				return -1;
			}
		}
	} while (rv_trylock);

	return stat;
}

int mkio_init()
{
	fd = open(MANCHESTER_DEVICE, O_RDWR);
	if (fd == -1) {
		assert_perror(errno);
		return -1;
	}

	if (ioctl(fd, KK_SET_MODE, 0) == -1) {
		assert_perror(errno);
		return -1;
	}

	return 0;
}

int mkio_destroy()
{
	int rv;

	if (close(fd) == -1) {
		assert_perror(errno);
		return -1;
	}

	rv = pthread_mutex_destroy(&mkio);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	return 0;
}
