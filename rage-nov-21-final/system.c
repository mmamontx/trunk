/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "abus.h"
#include "data.h"
#include "io.h"
#include "mkio.h"
#include "rage.h"
#include "system.h"

#include <math.h>

int sys_state = 0;

int system_control(const unsigned short *buf)
{
	int rv;
	unsigned char i;
	char citem_local;
	unsigned int key;
	unsigned short tmp[3] = {2, 0, 0};

	abus_set_state(1);

	rv = pthread_cancel(system_id);
	if (rv)
		if (rv != ESRCH) {
			assert_perror(rv);
			return -1;
		}

	if (buf[buf[1] - 2] != 0)
		citem_local = (char)(buf[buf[1] - 2] ^ 3);

	if (citem_local == 0)
		citem_local = 3;

	if (io_psk_cmd(0x101) == -1)
		return -1;

	delay(1000, 50);

	if (system_refresh() == -1)
		return -1;

	if (!cd10.bit)
		delay(5000, 50);
	else
		timedelta += 5;

	key = (unsigned int)buf[4] << 16;
	key |= (unsigned int)buf[5];

	if ((key != 0xffffffff) && sys_state) {
		if (io_send_msg(msg[157], 16) == -1)
			return -1;

		if (abus_send_msg(R1, 0xe1e1, msg[157], 16) == -1)
			return -1;

		return 0;
	}

	if (!cd10.bit)
		delay(12000, 50);
	else
		timedelta += 12;

	if (key != 0xffffffff) {
		if (!cd10.bit)
			delay(5000, 50);
		else
			timedelta += 5;

		key = htonl(key);

		memcpy(tmp + 1, &key, sizeof(int));

		key = ntohl(key);

		for (i = 2; i < 6; i++)
			*((unsigned char *)tmp + i) = ((*((unsigned char *)tmp + i) & 0xf) << 4) | ((*((unsigned char *)tmp + i) & 0xf0) >> 4);

		if (mkio_send(3, 1, 3, tmp) == -1)
			return -1;

		do {
			delay(1000, 50);

			if (mkio_recv(3, 1, 1, tmp) == -1)
				return -1;
		} while (tmp[0] & 2);

		if (!cd10.bit)
			delay(10000, 50);
		else
			timedelta += 10;
	}

	if (citem_local & 1) {
		if (io_send_msg((key != 0xffffffff) ? msg[158] : msg[160], 24) == -1)
			return -1;

		if (abus_send_msg(R1, 0xe0e0, (key != 0xffffffff) ? msg[158] : msg[160], 16) == -1)
			return -1;
	}

	if (!cd10.bit)
		delay(5000, 50);
	else
		timedelta += 5;

	if (citem_local & 2) {
		if (io_send_msg((key != 0xffffffff) ? msg[159] : msg[161], 32) == -1)
			return -1;

		if (abus_send_msg(R1, 0xe0e0, (key != 0xffffffff) ? msg[159] : msg[161], 16) == -1)
			return -1;
	}

	if (key != 0xffffffff) {
		sys_state = 1;
	} else {
		sys_state = 0;
	}

	if (sys_state)
		tmp[0] = 3;
	else
		tmp[0] = 0;

	if (abus_send_msg(R1, 0xaaaa, (char *)tmp, 2) == -1)
		return -1;

	if (io_send_msg(msg[162], 32) == -1)
		return -1;

	if (abus_send_msg(R1, 0xcccc, msg[162], 16) == -1)
		return -1;

	if (io_psk_cmd(0x201) == -1)
		return -1;

	if (io_psk_cmd(0x101) == -1)
		return -1;

	delay(1000, 50);

	if (system_refresh() == -1)
		return -1;

	tmp[0] = 0;

	rv = data_copy(&b310.stateMask, tmp, sizeof(short));
	if (rv == -1)
		pthread_exit(&rv);

	/*key = htonl(key);*/
	key = 0;

	rv = data_copy(&b310.keyZSH, &key, sizeof(int));
	if (rv == -1)
		pthread_exit(&rv);

	rv = data_send(CL_RMI, 0xb310);
	if (rv == -1)
		pthread_exit(&rv);

	rv = pthread_create(&system_id, NULL, system_poll, NULL);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	abus_set_state(0);

	return 0;
}

int system_check(const int mode)
{
	char i, citem_local;

	if (!citem)
		citem_local = 3;
	else
		citem_local = ~citem & 0x3;

	switch (mode) {
	case 254:
		delay(4000, 50);
	case 3:
	case 4:
	case 12:
	case 16:
	case 17:
	case 18:
	case 21:
		if (sys_state) {
			for (i = 1; i < 3; i++)
				if (i & citem_local) {
					if (io_send_msg(msg[140 + i], 8 + 16 * (i - 1)) == -1)
						return -1;

					if (abus_send_msg(R1, 0xe1e1, msg[140 + i], 16) == -1)
						return -1;
				}

			interrupt = 2;
		}

		if (mode != 254)
			break;
	case 2:
	case 13:
	case 14:
	case 15:
		if (!sys_state) {
			for (i = 1; i < 3; i++)
				if (i & citem_local) {
					if (io_send_msg(msg[138 + i], 8 + 16 * (i - 1)) == -1)
						return -1;

					if (abus_send_msg(R1, 0xe1e1, msg[138 + i], 16) == -1)
						return -1;
				}

			interrupt = 2;
		}

		break;
	default:
		return -1;
	}

	return 0;
}

int system_refresh()
{
	unsigned char i;
	unsigned short buf[4] = {1, 0, 0, 3};

	*((unsigned int *)(buf + 1)) = ntohl(dc10.codeZH);

	if ((SYSTEM_MAXDELAY / SYSTEM_DELAY) > pow(256, sizeof(i)) - 1) {
		fprintf(stderr, "%d > %d\n", SYSTEM_MAXDELAY / SYSTEM_DELAY, pow(256, sizeof(i)) - 1);
		return -1;
	}

	for (i = 1; i <= (SYSTEM_MAXDELAY / SYSTEM_DELAY); i++) {
		if (sys_state)
			buf[3] = 0;
		else
			buf[3] = 3;

		if (mkio_send(3, 1, 4, buf) == -1)
			return -1;

		usleep(SYSTEM_DELAY);

		if (mkio_recv(3, 1, 1, buf + 3) == -1)
			return -1;

		if (buf[3] != 0xffff) {
			if (sys_state) {
				if (buf[3] == 0x1) {
					return 0;
				}
			} else {
				if (buf[3] == 0x3) {
					return 0;
				}
			}
		}
	}

	TRACE(i);

	return -1;
}

void *system_poll(void *arg)
{
	int rv;
	unsigned int key;
	unsigned char i;
	unsigned short count, buf[4] = {0};

	while (1) {
		count = 0;
		key = 0;

		while (1) {
			rv = mkio_recv(3, 1, 4, buf);
			if (rv == -1)
				pthread_exit(&rv);

			if (buf[0] != 0xffff) {
				if (sys_state) {
					if (buf[0] & 2) {
						if ((buf[0] == 0x2) && (!buf[1])) {

						} else {
							count = htons(7);

							rv = data_copy(&b310.stateMask, &count, sizeof(short));
							if (rv == -1)
								pthread_exit(&rv);

							key = htonl(2550136832);

							rv = data_copy(&b310.keyZSH, &key, sizeof(int));
							if (rv == -1)
								pthread_exit(&rv);

							rv = data_send(CL_RMI, 0xb310);
							if (rv == -1)
								pthread_exit(&rv);

							count = 0;
							key = 0;
							sys_state = 0;
						}
					}
				} else {
					if (buf[2] > count) {
						key |= buf[1] << (4 * (buf[2] - 1));
						TRACE(key);
						count = buf[2];
					}
				}
			}

			if ((!sys_state) && (count == 8))
				break;

			if (sys_state)
				delay(1000, 50);
			else
				usleep(50000);
		}

		if (key == ntohl(dc10.codeZH)) {
			count = 0;
			sys_state = 1;
		} else {
			count = htons(7);
		}

		rv = data_copy(&b310.stateMask, &count, sizeof(short));
		if (rv == -1)
			pthread_exit(&rv);

		key = htonl(key);

		rv = data_copy(&b310.keyZSH, &key, sizeof(int));
		if (rv == -1)
			pthread_exit(&rv);

		rv = data_send(CL_RMI, 0xb310);
		if (rv == -1)
			pthread_exit(&rv);

		memset(buf, 0, sizeof(buf));

		delay(10000, 50);
	}

	return NULL;
}
