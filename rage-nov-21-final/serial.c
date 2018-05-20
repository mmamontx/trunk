/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "data.h"
#include "io.h"
#include "rage.h"
#include "serial.h"
#include "src-1450/baselib.h"

#include <fcntl.h>
#include <math.h>
#include <termios.h>

unsigned char sindex = 0;
char flood = 0;
char stflag = 0;
struct _c c;

int serial_test(const int open_flags)
{
	int i, fd, rv;
	struct t task;
	char buf[18], str[7] = {'t', 'e', 's', 't', '\0', '0' + sindex % 10, '\0'};

	fd = serial_init(open_flags);
	if (fd == -1)
		return -1;

	task.flag = 0x10;
	task.id = 0x71;
	task.index = sindex + 1;
	task.reserved = 0;
	task.subcode1 = 0x7;
	task.subcode2 = 0x1;
	task.params = 0;

	memcpy(buf, &task, sizeof(task));
	memcpy(buf + sizeof(task), str, 7);

	*((unsigned short *)buf + 7) = serial_csum((unsigned short *)buf + 1, 6);

	buf[16] = 0x10;
	buf[17] = 0x3;

	for (i = 0; i < 18; i++)
		printf("buf[%d] = 0x%x\n", i, buf[i]);

	rv = write(fd, buf, 18);
	if (rv == -1) {
		close(fd);
		assert_perror(errno);
		return -1;
	}

	for (i = 0; i < 5; i++) {
		usleep(50000);

		rv = read(fd, buf, 12);

		TRACE(rv);

		if (rv == -1) {
			if (errno != EAGAIN) {
				close(fd);
				assert_perror(errno);
				return -1;
			} else {
				continue;
			}
		} else {
			if (rv > 0) {
				sindex++;
				break;
			}
		}
	}

	if (rv <= 0) {
		close(fd);
		return -1;
	}

	return fd;
}

unsigned short serial_csum(unsigned short *buf, const unsigned int len)
{
	unsigned int i;
	unsigned short csum = 0;

	for (i = 0; i < len; i++)
		csum ^= buf[i];

	return csum;
}

void *stpn_poll(void *arg)
{
	int rv;
	char flag = 0;
	unsigned int cmdh;
	unsigned short buf[14];
	double t1, t2;

	while (1) {
		usleep(50000);

		rv = io_stpn_recv(buf);

		if (rv == -1) {
			if (flag) {
				buf[0] = ntohs(a120.stateMask);
				buf[0] &= 0xfffe;
				buf[0] = htons(buf[0]);

				rv = data_copy(&a120, buf, sizeof(struct A120));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_RMI, 0xa120);
				if (rv == -1)
					pthread_exit(&rv);

				flag = 0;
			}

			if (ntohs(cd10.regID) == 101)
				continue;
			else
				delay(1000, 50);
		} else {
			if (!flag) {
				buf[0] = ntohs(a120.stateMask);
				buf[0] |= 1;
				buf[0] = htons(buf[0]);

				rv = data_copy(&a120, buf, sizeof(struct A120));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_RMI, 0xa120);
				if (rv == -1)
					pthread_exit(&rv);

				flag = 1;
			}

			if ((ntohs(cd10.regID) == 101) && flood) {
				t1 = pow(id_cards[ntohs(cd10.taskID) - 1].X1 - (long)c.x, 2);	
				t2 = pow(id_cards[ntohs(cd10.taskID) - 1].Y1 - (long)c.y, 2);
				t1 = sqrt(t1 + t2);

				if (t1 < 20) {
					t1 = pow((int)c.x - stpn_data.x, 2);
					t2 = pow((int)c.y - stpn_data.y, 2);
					t1 = sqrt(t1 + t2);

					if (t1 < 2)
						stflag = 1;

					flood = 0;

					continue;
				}

				if (((int)c.x < 0) || ((unsigned int)c.y == 0)) {
					printf("(int)c.x = %d\n(unsigned int)c.y = %d\n", (int)c.x, (unsigned int)c.y);
					continue;
				}

				cmdh = htonl((int)c.x);

				rv = data_copy(&db20.X_nap, &cmdh, sizeof(int));
				if (rv == -1)
					pthread_exit(&rv);

				cmdh = htonl((uint)c.y);

				rv = data_copy(&db20.Y_nap, &cmdh, sizeof(int));
				if (rv == -1)
					pthread_exit(&rv);

				buf[0] = htons((short)c.z);

				rv = data_copy(&db20.H_nap, buf, sizeof(short));
				if (rv == -1)
					pthread_exit(&rv);

				cmdh = htonl(stpn_data.x);

				rv = data_copy(&db20.X_stpn, &cmdh, sizeof(int));
				if (rv == -1)
					pthread_exit(&rv);

				cmdh = htonl((uint)stpn_data.y);

				rv = data_copy(&db20.Y_stpn, &cmdh, sizeof(int));
				if (rv == -1)
					pthread_exit(&rv);

				buf[0] = htons(stpn_data.h);

				rv = data_copy(&db20.H_stpn, buf, sizeof(short));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_RMI, 0xdb20);
				if (rv == -1)
					pthread_exit(&rv);
			} else {
				delay(1000, 50);
			}
		}
	}
}

int serial_init(const int open_flags)
{
	struct termios t;
	struct pollfd fd;

	fd.fd = open("/dev/ttyS0", open_flags);
	if (fd.fd == -1) {
		assert_perror(errno);
		return -1;
	}

	fd.events = POLLIN;
	fd.revents = 0;

	t.c_cflag = MINICOM_CFG;

	if (cfsetispeed(&t, B115200) == -1) {
		assert_perror(errno);
		return -1;
	}

	if (cfsetospeed(&t, B115200) == -1) {
		assert_perror(errno);
		return -1;
	}

	if (tcsetattr(fd.fd, TCSANOW, &t) == -1) {
		assert_perror(errno);
		return -1;
	}

	return fd.fd;
}

void *serial(void *arg)
{
	int fd, b, rv, shift = 0, len, lenr, buflen;
	char tr, f = 0, st = 0;
	struct a action;
	struct t task;
	unsigned char flag = 0, tail = 0;
	unsigned char buf[BUFSIZE];
	unsigned char convert[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	unsigned char *ptr;
	unsigned short i, route, csum;

	memset(&c, 0, sizeof(c));

	while (1) {
		fd = serial_test(O_RDWR | O_NONBLOCK);

		TRACE(fd);

		if (fd != -1)
			break;

		delay(15000, 50);

		if (!f) {
			sindex++;
			f = 1;
		} else {
			sindex--;
			f = 0;
		}
	}

	close(fd);

	fd = serial_init(O_RDWR);
	if (fd == -1) {
		TRACE(fd);
		pthread_exit(&fd);
	}

	task.flag = 0x10;
	task.id = 0x71;
	task.index = sindex++;
	task.reserved = 0;
	task.subcode1 = 0x7;
	task.subcode2 = 0x1;
	/*task.params = ntohs(da10.DKT);*/
	task.params = 0;

	if (ntohs(da10.routeID) < 10) {
		*((char *)&route) = convert[ntohs(da10.routeID)];
	} else {
		*((char *)&route) = convert[ntohs(da10.routeID) / 10];
		*((char *)&route + 1) = convert[ntohs(da10.routeID) % 10];
	}

	lenr = strlen((char *)&route);
	len = strlen((char *)da10.mapID);

	buflen = sizeof(task) + (len + 1) + (lenr + 1) + 4;

	ptr = (unsigned char *)calloc(1, buflen + buflen % 2);
	if (ptr == NULL) {
		fprintf(stderr, "calloc() == NULL");
		rv = -1;
		pthread_exit(&rv);
	}

	memcpy(ptr, &task, sizeof(task));
	memcpy(ptr + sizeof(task), &da10.mapID, len);
	memcpy(ptr + sizeof(task) + (len + 1), &route, sizeof(ushort));

	csum = serial_csum((unsigned short *)ptr + 1, (buflen + buflen % 2 - 6) / sizeof(short));

	*((unsigned short *)ptr + (buflen + buflen % 2 - 4) / sizeof(short)) = csum;

	ptr[buflen + buflen % 2 - 2] = 0x10;
	ptr[buflen + buflen % 2 - 1] = 0x3;

	/*for (i = 0; i < buflen + buflen % 2; i++)
		printf("ptr[%d] = 0x%x\n", i, ptr[i]);*/

	action.flag = 0x10;
	action.id = 0x71;
	action.reserved = 0;
	action.subcode1 = 0x7;
	action.subcode2 = 0x2;
	action.cmd = 1;
	action.eflag = 0x10;
	action.end = 0x3;

	rv = write(fd, ptr, buflen + buflen % 2);
	if (rv == -1) {
		assert_perror(errno);
		pthread_exit(&rv);
	}

	free(ptr);

	do {
		delay(1000, 50);

		b = read(fd, buf, BUFSIZE);
		if (b == -1) {
			assert_perror(errno);
			pthread_exit(&b);
		}
	} while (!b);

	/*for (i = 0; i < b; i++)
		printf("buf[%d] = 0x%x\n", i, buf[i]);*/

	if (buf[b - 6] != 0) {
		switch (buf[6]) {
		case 1:
			fprintf(stderr, "Не найден файл карты %s.\n", ((unsigned char *)&da10.mapID));
			break;
		case 2:
			fprintf(stderr, "Не найден файл маршрута %d.\n", ntohs(da10.routeID));
			break;
		case 3:
			fprintf(stderr, "Не найден файл контрольных точек для маршрута %d.\n", ntohs(da10.routeID));
			break;
		case 4:
			fprintf(stderr, "Маршрут %d не соответствует карте %s (точки маршрута не попадают в границы файла карты).\n", ntohs(da10.routeID), (unsigned char *)&da10.mapID);
			break;
		default:
			TRACE(buf[6]);
		}

		pthread_exit(NULL);
	}

	rv = data_send(CL_RMI, 0xdb20);
	if (rv == -1)
		pthread_exit(&rv);

	while (!(ntohs(bc40.stateGear) & 4))
		delay(1000, 50);

	flood = 1;

	while (1) {
		action.index = sindex++;
		action.csum = serial_csum((unsigned short *)&action + 1, (sizeof(action) - 6) / sizeof(short));

		ptr = (unsigned char *)&action;

		for (i = 1; i < sizeof(action) - 2; i++)
			if (ptr[i] == 0x10)
				break;

		if (i != sizeof(action) - 2) {
			ptr = (unsigned char *)malloc(sizeof(action) * 2);
			if (ptr == NULL)
				pthread_exit(NULL);

			memcpy(ptr, &action, sizeof(action));

			for (i = 1; i < sizeof(action) + tail - 2; i++)
				if (ptr[i] == 0x10) {
					memmove(ptr + i + 1, ptr + i, sizeof(action) * 2 - i - 1);
					ptr[i] = 0x10;
					tail++;
					i += 2;
				}
		}

		rv = write(fd, tail ? ptr : (unsigned char *)&action, sizeof(action) + tail);
		if (rv == -1) {
			assert_perror(errno);
			pthread_exit(&rv);
		}

		if (tail) {
			free(ptr);
			tail = 0;
		}

		do {
			delay(1000, 50);

			b = read(fd, buf + shift, BUFSIZE);
			if (b == -1) {
				assert_perror(errno);
				pthread_exit(&b);
			}
		} while (!b);

		if (!flag) for (i = 0; i < b; i++)
			if ((buf[i] == 0x10) && (buf[i + 1] == 0x71)) {
				flag = buf[i + 1];
				shift = b - i;

				memmove(buf, buf + i, shift);

				shift = 0;

				break;
			}

		tr = b - 192;

		if (tr) for (i = 0; i < shift + b - 1; i++)
			if ((buf[i] == 0x10) && (buf[i + 1] == 0x10)) {
				memmove(&buf[i], &buf[i + 1], shift + b - i - 1);

				tr--;

				if (!tr)
					break;
			}

		if (flag) for (i = 0; i < shift + b; i++)
			if ((buf[i] == 0x10) && (buf[i + 1] == 0x3)) {
				memcpy(&c, buf + 7, sizeof(c));

				flag = 0;
				shift = 0;

				break;
			}

		if (flag)
			shift = i;

		if ((!(ntohs(bc40.stateGear) & 0x4)) || (stflag)) {
			if (!st)
				st = 1;

			action.cmd = 2;
		} else {
			if (st)
				st = 0;

			action.cmd = 3;
		}
	}
}
