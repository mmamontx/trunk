/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "abus.h"
#include "data.h"
#include "io.h"
#include "mkio.h"
#include "rage.h"
#include "system.h"

#include "src-1450/baselib.h"

#include <math.h>
#include <stdio.h>

unsigned char abus = 0;

unsigned char abus_get_state()
{
	return abus;
}

void abus_set_state(const unsigned char _abus)
{
	abus = _abus;
}

unsigned short abus_csum(const unsigned short *buf, const unsigned short len)
{
	unsigned int csum = 0;
	unsigned short i;

	for (i = 0; i < len; i++)
		if ((csum += buf[i]) > 0xffff) {
			csum++;
			csum &= 0xffff;
		}

	return (unsigned short)csum;
}

int abus_send_msg(const unsigned char r, const unsigned short id, const char *str, const unsigned char len)
{
	unsigned char i, shift;
	unsigned short *buf;

	if (!abus)
		return 0;

	if ((id == 0xe0e0) || (id == 0xe1e1))
		shift = 1;
	else
		shift = 0;

	buf = (unsigned short *)malloc(sizeof(short) * (len / 2 + len % 2 + 3 + shift));
	if (buf == NULL) {
		fprintf(stderr, "malloc() == NULL\n");
		return -1;
	}

	buf[0] = id;
	buf[1] = len / 2 + len % 2 + 3 + shift;

	for (i = 0; i < len; i++)
		*((char *)buf + sizeof(short) * (2 + shift) + i) = str[i];

	buf[buf[1] - 1] = abus_csum(buf, buf[1] - 1);

	if (mkio_send(ABUS, r, buf[1], buf) == -1) {
		free(buf);
		return -1;
	}

	free(buf);

	return 0;
}

int abus_poll()
{
	float az;
	unsigned int ccount = 3, i;
	unsigned short buf[15];

	for (i = 0; i < ccount; i++) {
		if (mkio_recv(ABUS, R2, 9, buf) == -1)
			return -1;

		switch (buf[0]) {
		case ABUS_MSG_WAIT:
			buf[0] = ABUS_MSG_WAIT;
			buf[1] = 3;
			buf[2] = abus_csum(buf, 2);

			ccount = 3600 * 3 * (1000000 / ABUS_DELAY) + 1;

			if (mkio_send(ABUS, R1, 3, buf) == -1)
				return -1;

			break;
		case ABUS_MSG_STATE:
			buf[0] = ABUS_MSG_STATE_NEW;
			buf[1] = 15;
			buf[2] = ntohs(dc10.got1) << 3;
			buf[2] |= ntohs(dc10.got2);

			if (ntohs(dc10.rch1)) {
				buf[3] |= 2;
				buf[4] = 022;

				if (!sys_state)
					buf[6] |= 2;
			}

			if (ntohs(dc10.rch2)) {
				buf[3] |= 1;
				buf[5] = 022;

				if (!sys_state)
					buf[6] |= 1;
			}

			if (io_stpn_recv(buf) == -1) {
				if (io_send_msg(msg[138], 8) == -1)
					return -1;

				if (io_send_msg(msg[153], 16) == -1)
					return -1;

				abus_set_state(1);

				if (abus_send_msg(R3, 0xe0e0, msg[138], 16) == -1)
					return -1;

				if (abus_send_msg(R3, 0xe1e1, msg[153], 16) == -1)
					return -1;

				abus_set_state(0);

				return -1;
			}

			if (cd10.regID != 101) {
				az = id_cards[ntohs(cd10.taskID) - 1].AZ[0];
				az += (float)(id_cards[ntohs(cd10.taskID) - 1].AZ[1]) / 60;
				az += (float)(id_cards[ntohs(cd10.taskID) - 1].AZ[2]) / 3600;
				az /= 180 * M_PI;

				buf[7] = *((unsigned short *)(&az) + 0);
				buf[8] = *((unsigned short *)(&az) + 1);

				buf[9] = id_cards[ntohs(cd10.taskID) - 1].X1 & 0xffff;
				buf[10] = (id_cards[ntohs(cd10.taskID) - 1].X1 & 0xffff0000) >> 16;

				buf[11] = id_cards[ntohs(cd10.taskID) - 1].Y1 & 0xffff;
				buf[12] = (id_cards[ntohs(cd10.taskID) - 1].Y1 & 0xffff0000) >> 16;

				buf[13] = id_cards[ntohs(cd10.taskID) - 1].H1;
			}

			buf[14] = abus_csum(buf, 14);

			if (mkio_send(ABUS, R3, 15, buf) == -1)
				return -1;

			if (io_send_msg(msg[121], 8) == -1)
				return -1;

			return ABUS_MSG_STATE;
		case ABUS_MSG_CNTR_COM:
			if (abus_csum(buf, buf[1] - 1) != buf[buf[1] - 1]) {
				if (io_send_msg(msg[107], 8) == -1)
					return -1;

				if (io_send_msg(msg[108], 16) == -1)
					return -1;

				abus_set_state(1);

				if (abus_send_msg(R3, 0xe0e0, msg[107], 16) == -1)
					return -1;

				if (abus_send_msg(R3, 0xe1e1, msg[108], 16) == -1)
					return -1;

				abus_set_state(0);

				return -1;
			}

			if (buf[buf[1] - 2] != 0)
				citem = (char)(buf[buf[1] - 2] ^ 3);
			else
				citem = 0;

			if (buf[2] == 20) {
				if (io_send_msg(msg[154], 8) == -1)
					return -1;

				abus_set_state(1);

				if (abus_send_msg(R1, 0xc1c1, msg[154], 16) == -1)
					return -1;

				abus_set_state(0);

				if (time(&timestart) == ((time_t) -1)) {
					assert_perror(errno);
					return -1;
				}

				if (io_timer_adjust() == -1)
					return -1;

				if (system_control(buf) == -1)
					return -1;

				if (io_timer_delete() == -1)
					return -1;
			}

			abus_set_state(1);

			return buf[2];
		}

		usleep(ABUS_DELAY);
	}

	return 0;
}