#include "data.h"
#include "io.h"
#include "mkio.h"
#include "rage.h"
#include "rc.h"

int rc_connect()
{
	unsigned char i;
	unsigned short buf, cmd = 0x8000;

	for (i = 0; i < 5; i++) {
		if (mkio_send(BE, R2, 1, &cmd) == -1)
			return -1;

		if (mkio_recv(BE, R2, 1, &buf) == -1)
			return -1;

		if ((buf & cmd) == cmd)
			break;

		delay(1000, 50);
	}

	if (i == 5) {
		if (io_send_msg(msg[152], 8) == -1)
			return -1;

		return -1;
	}

	delay(1000, 50);

	if (mkio_recv(BE, R1, 1, &buf) == -1)
		return -1;

	if (!(buf & 0x2000)) {
		if (io_send_msg(msg[116], 8) == -1)
			return -1;

		if (io_send_msg(msg[117], 16) == -1)
			return -1;

		return -1;
	}

	cmd = 1;

	if (mkio_send(BE, R1, 1, &cmd) == -1)
		return -1;

	delay(1000, 50);

	if (mkio_recv(BE, R1, 1, &buf) == -1)
		return -1;

	if (buf & 0x2000) {
		if (io_send_msg(msg[116], 8) == -1)
			return -1;

		if (io_send_msg(msg[117], 16) == -1)
			return -1;

		return -1;
	}

	buf = htons(1);

	if (data_copy(&ba10.stateMask, &buf, sizeof(short)) == -1)
		return -1;

	if (data_send(CL_RMI, 0xba10) == -1)
		return -1;

	return 0;
}

int rc_offitem(const unsigned char item, const unsigned char r)
{
	unsigned short buf;

	switch (item) {
		case 0:
			buf = r ? 0x380 : 0x70;
			buf = htons(buf);

			if (data_copy(&ba10, &buf, sizeof(short)) == -1)
				return -1;

			if (data_send(CL_RMI, 0xba10) == -1)
				return -1;

			buf = 6;

			break;
		case 1:
			buf = r ? 0x180 : 0x30;
			buf = htons(buf);

			if (data_copy(&ba10, &buf, sizeof(short)) == -1)
				return -1;

			if (data_send(CL_RMI, 0xba10) == -1)
				return -1;

			buf = 4;

			break;
		case 2:
			buf = r ? 0x280 : 0x50;
			buf = htons(buf);

			if (data_copy(&ba10, &buf, sizeof(short)) == -1)
				return -1;

			if (data_send(CL_RMI, 0xba10) == -1)
				return -1;

			buf = 5;

			break;
		default:
			TRACE(item);
			return -1;
	}

	if (mkio_send(BE, R1, 1, &buf) == -1)
		return -1;

	return 0;
}

void *rc(void *arg)
{
	int rv;
	unsigned int i;
	unsigned short buf;

	rv = rc_connect();
	if (rv == -1) {
		interrupt = 2;

		pthread_exit(&rv);
	}

	for (i = 0; i < 108000; i++, usleep(100000)) {
		rv = mkio_recv(BE, R1, 1, &buf);
		if (rv == -1)
			pthread_exit(&rv);

		if ((buf & 0x1c00) == 0x1400)
			break;

		if ((buf & 0x1800) == 0x1800)
			break;
	}

	if (i != 108000) {
		if ((buf & 0x1c00) == 0x1400) {
			buf = 0x1402;
			buf = htons(buf);

			rv = data_copy(&ba10, &buf, sizeof(short));
			if (rv == -1)
				pthread_exit(&rv);

			rv = data_send(CL_RMI, 0xba10);
			if (rv == -1)
				pthread_exit(&rv);

			buf = 2;

			rv = mkio_send(BE, R1, 1, &buf);
			if (rv == -1)
				pthread_exit(&rv);

			buf = htons(buf);

			rv = data_copy(&ba10, &buf, sizeof(short));
			if (rv == -1)
				pthread_exit(&rv);

			rv = data_send(CL_RMI, 0xba10);
		}

		if ((buf & 0x1800) == 0x1800) {
			buf = 0x1808;
			buf = htons(buf);

			rv = data_copy(&ba10, &buf, sizeof(short));
			if (rv == -1)
				pthread_exit(&rv);

			rv = data_send(CL_RMI, 0xba10);
			if (rv == -1)
				pthread_exit(&rv);

			buf = 0x5b;

			rv = mkio_send(BE, R1, 1, &buf);
			if (rv == -1)
				pthread_exit(&rv);

			buf = 8;
			buf = htons(buf);

			rv = data_copy(&ba10, &buf, sizeof(short));
			if (rv == -1)
				pthread_exit(&rv);

			rv = data_send(CL_RMI, 0xba10);
			if (rv == -1)
				pthread_exit(&rv);

			rv = io_send_msg(msg[166], 8);

			interrupt = 2;
		}
	} else {
		rv = io_send_msg(msg[116], 8);
		if (rv == -1)
			pthread_exit(&rv);

		rv = io_send_msg(msg[118], 16);
	}

	pthread_exit(&rv);

	return NULL;
}
