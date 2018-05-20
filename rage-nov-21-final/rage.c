/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "abus.h"
#include "data.h"
#include "eth.h"
#include "event.h"
#include "io.h"
#include "list.h"
#include "lstack.h"
#include "mkio.h"
#include "queue.h"
#include "rage.h"
#include "rc.h"
#include "serial.h"
#include "system.h"
#include "source/net.h"
#include "src-1450/baselib.h"
#include "src-1450/heater.h"
#include "src-1450/tech.h"

char citem;
char interrupt = 0;
int timedelta;
pthread_t stpn_id;
pthread_t system_id;
struct reject rej[17];
time_t timestart;
timer_t modetimer_id;

int setrej(const unsigned char id, const unsigned char mode, unsigned char item)
{
	unsigned char i;
	char citem_local = citem;

	if (id > 16) {
		TRACE(id);
		return -1;
	}

	if (!timestart) {
		if (lstack_push(id + 1) == -1)
			return -1;

		if (lstack_push(mode) == -1)
			return -1;

		if (lstack_push(item) == -1)
			return -1;

		return 0;
	}

	if (!citem)
		citem_local = 3;

	if (item != 3)
		item = ~item & 0x3;

	if (item != citem_local)
		return 0;

	for (i = 1; i <= 2; i++)
		if (i & item) {
			if (id)
				rej[id].msg1 = msg[90 + i];

			if (event_add(rej[id].msg1, 8 + 16 * (i - 1), rej[id].deadline[mode][citem]) == -1)
				return -1;

			if (rej[id].msg2 != NULL) {
				if (event_add(rej[id].msg2, 16 + 16 * (i - 1), rej[id].deadline[mode][citem]) == -1)
					return -1;
			}
		}

	return 0;
}

void initrej()
{
	unsigned char i;

	memset(rej, 0, sizeof(rej));

	rej[0].msg1 = msg[46];
	rej[0].msg2 = msg[47];

	rej[1].msg2 = msg[93];
	rej[2].msg2 = msg[94];
	rej[3].msg2 = msg[95];
	rej[4].msg2 = msg[96];
	rej[5].msg2 = msg[97];
	rej[6].msg2 = msg[98];
	rej[7].msg2 = msg[99];
	rej[8].msg2 = msg[100];
	rej[9].msg2 = msg[101];
	rej[10].msg2 = msg[96];
	rej[11].msg2 = msg[99];
	rej[12].msg2 = msg[102];
	rej[13].msg2 = msg[103];
	rej[14].msg2 = msg[104];
	rej[15].msg2 = msg[105];
	rej[16].msg2 = msg[106];

	for (i = 0; i < 3; i++) {
		rej[0].deadline[3][i] = 2;
		rej[0].deadline[4][i] = 2;
		rej[0].deadline[12][i] = 2;
		rej[0].deadline[14][i] = 5;
		rej[0].deadline[15][i] = 2;
		rej[0].deadline[21][i] = 2;

		rej[0].deadline[16][i] = 2;
		rej[0].deadline[17][i] = 2;
		rej[0].deadline[18][i] = 2;

		rej[1].deadline[3][i] = 6;
		rej[2].deadline[3][i] = 6;
		rej[3].deadline[3][i] = 6;

		rej[4].deadline[3][i] = 5;
		rej[5].deadline[3][i] = 5;
		rej[6].deadline[3][i] = 5;
	}

	rej[0].deadline[2][0] = 5;
	rej[0].deadline[2][1] = 4;
	rej[0].deadline[2][2] = 4;

	rej[0].deadline[13][0] = 4;
	rej[0].deadline[13][1] = 4;
	rej[0].deadline[13][2] = 5;


	rej[1].deadline[2][0] = 10;
	rej[1].deadline[2][1] = 8;
	rej[1].deadline[2][2] = 8;

	rej[1].deadline[4][0] = 8;
	rej[1].deadline[4][1] = 6;
	rej[1].deadline[4][2] = 6;

	rej[1].deadline[13][0] = 11;
	rej[1].deadline[13][1] = 8;
	rej[1].deadline[13][2] = 9;

	rej[1].deadline[14][0] = 11;
	rej[1].deadline[14][1] = 9;
	rej[1].deadline[14][2] = 9;

	rej[1].deadline[15][0] = 8;
	rej[1].deadline[15][1] = 6;
	rej[1].deadline[15][2] = 6;

	rej[1].deadline[21][0] = 9;
	rej[1].deadline[21][1] = 6;
	rej[1].deadline[21][2] = 6;


	rej[2].deadline[2][0] = 11;
	rej[2].deadline[2][1] = 6;
	rej[2].deadline[2][2] = 6;

	rej[2].deadline[4][0] = 9;
	rej[2].deadline[4][1] = 6;
	rej[2].deadline[4][2] = 6;

	rej[2].deadline[13][0] = 11;
	rej[2].deadline[13][1] = 8;
	rej[2].deadline[13][2] = 8;

	rej[2].deadline[14][0] = 11;
	rej[2].deadline[14][1] = 9;
	rej[2].deadline[14][2] = 9;

	rej[2].deadline[15][0] = 8;
	rej[2].deadline[15][1] = 6;
	rej[2].deadline[15][2] = 6;

	rej[2].deadline[21][0] = 9;
	rej[2].deadline[21][1] = 6;
	rej[2].deadline[21][2] = 6;


	rej[3].deadline[2][0] = 9;
	rej[3].deadline[2][1] = 8;
	rej[3].deadline[2][2] = 9;

	rej[3].deadline[4][0] = 8;
	rej[3].deadline[4][1] = 5;
	rej[3].deadline[4][2] = 6;

	rej[3].deadline[13][0] = 11;
	rej[3].deadline[13][1] = 8;
	rej[3].deadline[13][2] = 9;

	rej[3].deadline[14][0] = 11;
	rej[3].deadline[14][1] = 9;
	rej[3].deadline[14][2] = 9;

	rej[3].deadline[15][0] = 8;
	rej[3].deadline[15][1] = 6;
	rej[3].deadline[15][2] = 6;

	rej[3].deadline[21][0] = 9;
	rej[3].deadline[21][1] = 6;
	rej[3].deadline[21][2] = 6;


	rej[4].deadline[2][0] = 10;
	rej[4].deadline[2][1] = 7;
	rej[4].deadline[2][2] = 8;

	rej[4].deadline[4][0] = 7;
	rej[4].deadline[4][1] = 5;
	rej[4].deadline[4][2] = 5;

	rej[4].deadline[13][0] = 10;
	rej[4].deadline[13][1] = 7;
	rej[4].deadline[13][2] = 8;

	rej[4].deadline[14][0] = 10;
	rej[4].deadline[14][1] = 8;
	rej[4].deadline[14][2] = 8;

	rej[4].deadline[15][0] = 7;
	rej[4].deadline[15][1] = 5;
	rej[4].deadline[15][2] = 5;

	rej[4].deadline[21][0] = 8;
	rej[4].deadline[21][1] = 6;
	rej[4].deadline[21][2] = 5;


	rej[5].deadline[2][0] = 11;
	rej[5].deadline[2][1] = 8;
	rej[5].deadline[2][2] = 9;

	rej[5].deadline[4][0] = 7;
	rej[5].deadline[4][1] = 5;
	rej[5].deadline[4][2] = 5;

	rej[5].deadline[13][0] = 11;
	rej[5].deadline[13][1] = 8;
	rej[5].deadline[13][2] = 9;

	rej[5].deadline[14][0] = 11;
	rej[5].deadline[14][1] = 9;
	rej[5].deadline[14][2] = 9;

	rej[5].deadline[15][0] = 8;
	rej[5].deadline[15][1] = 6;
	rej[5].deadline[15][2] = 6;

	rej[5].deadline[21][0] = 9;
	rej[5].deadline[21][1] = 6;
	rej[5].deadline[21][2] = 6;


	rej[6].deadline[2][0] = 10;
	rej[6].deadline[2][1] = 8;
	rej[6].deadline[2][2] = 8;

	rej[6].deadline[4][0] = 8;
	rej[6].deadline[4][1] = 5;
	rej[6].deadline[4][2] = 6;

	rej[6].deadline[13][0] = 10;
	rej[6].deadline[13][1] = 7;
	rej[6].deadline[13][2] = 8;

	rej[6].deadline[14][0] = 11;
	rej[6].deadline[14][1] = 9;
	rej[6].deadline[14][2] = 9;

	rej[6].deadline[15][0] = 7;
	rej[6].deadline[15][1] = 5;
	rej[6].deadline[15][2] = 5;

	rej[6].deadline[21][0] = 8;
	rej[6].deadline[21][1] = 5;
	rej[6].deadline[21][2] = 6;


	rej[7].deadline[2][0] = 46;
	rej[7].deadline[2][1] = 16;
	rej[7].deadline[2][2] = 16;

	rej[7].deadline[3][0] = 25;
	rej[7].deadline[3][1] = 10;
	rej[7].deadline[3][2] = 10;

	rej[7].deadline[4][0] = 20;
	rej[7].deadline[4][1] = 5;
	rej[7].deadline[4][2] = 5;

	rej[7].deadline[13][0] = 45;
	rej[7].deadline[13][1] = 22;
	rej[7].deadline[13][2] = 22;

	rej[7].deadline[14][0] = 25;
	rej[7].deadline[14][1] = 10;
	rej[7].deadline[14][2] = 10;

	rej[7].deadline[15][0] = 46;
	rej[7].deadline[15][1] = 23;
	rej[7].deadline[15][2] = 23;


	rej[8].deadline[2][0] = 46;
	rej[8].deadline[2][1] = 32;
	rej[8].deadline[2][2] = 31;

	rej[8].deadline[3][0] = 44;
	rej[8].deadline[3][1] = 29;
	rej[8].deadline[3][2] = 29;

	rej[8].deadline[4][0] = 39;
	rej[8].deadline[4][1] = 24;
	rej[8].deadline[4][2] = 24;

	rej[8].deadline[13][0] = 65;
	rej[8].deadline[13][1] = 41;
	rej[8].deadline[13][2] = 41;

	rej[8].deadline[14][0] = 45;
	rej[8].deadline[14][1] = 29;
	rej[8].deadline[14][2] = 29;

	rej[8].deadline[15][0] = 66;
	rej[8].deadline[15][1] = 42;
	rej[8].deadline[15][2] = 42;


	rej[9].deadline[2][0] = 29;
	rej[9].deadline[2][1] = 12;
	rej[9].deadline[2][2] = 12;

	rej[9].deadline[4][0] = 20;
	rej[9].deadline[4][1] = 4;
	rej[9].deadline[4][2] = 4;

	rej[9].deadline[13][0] = 46;
	rej[9].deadline[13][1] = 22;
	rej[9].deadline[13][2] = 22;

	rej[9].deadline[14][0] = 25;
	rej[9].deadline[14][1] = 10;
	rej[9].deadline[14][2] = 10;

	rej[9].deadline[15][0] = 47;
	rej[9].deadline[15][1] = 23;
	rej[9].deadline[15][2] = 23;


	rej[10].deadline[2][0] = 162;
	rej[10].deadline[2][1] = 118;
	rej[10].deadline[2][2] = 118;

	rej[10].deadline[13][0] = 172;
	rej[10].deadline[13][1] = 128;
	rej[10].deadline[13][2] = 129;

	rej[10].deadline[14][0] = 152;
	rej[10].deadline[14][1] = 117;
	rej[10].deadline[14][2] = 116;

	rej[10].deadline[15][0] = 173;
	rej[10].deadline[15][1] = 130;
	rej[10].deadline[15][2] = 130;


	rej[11].deadline[2][0] = 150;
	rej[11].deadline[2][1] = 118;
	rej[11].deadline[2][2] = 118;

	rej[11].deadline[13][0] = 172;
	rej[11].deadline[13][1] = 129;
	rej[11].deadline[13][2] = 129;

	rej[11].deadline[14][0] = 152;
	rej[11].deadline[14][1] = 116;
	rej[11].deadline[14][2] = 116;

	rej[11].deadline[15][0] = 173;
	rej[11].deadline[15][1] = 130;
	rej[11].deadline[15][2] = 130;


	rej[12].deadline[2][0] = 160;
	rej[12].deadline[2][1] = 118;
	rej[12].deadline[2][2] = 118;

	rej[12].deadline[13][0] = 172;
	rej[12].deadline[13][1] = 128;
	rej[12].deadline[13][2] = 128;

	rej[12].deadline[14][0] = 152;
	rej[12].deadline[14][1] = 117;
	rej[12].deadline[14][2] = 117;

	rej[12].deadline[15][0] = 173;
	rej[12].deadline[15][1] = 130;
	rej[12].deadline[15][2] = 134;


	rej[13].deadline[2][0] = 156;
	rej[13].deadline[2][1] = 119;
	rej[13].deadline[2][2] = 121;

	rej[13].deadline[13][0] = 173;
	rej[13].deadline[13][1] = 130;
	rej[13].deadline[13][2] = 129;

	rej[13].deadline[14][0] = 154;
	rej[13].deadline[14][1] = 118;
	rej[13].deadline[14][2] = 118;

	rej[13].deadline[14][0] = 175;
	rej[13].deadline[14][1] = 131;
	rej[13].deadline[14][2] = 131;


	rej[14].deadline[2][0] = 171;
	rej[14].deadline[2][1] = 122;
	rej[14].deadline[2][2] = 125;

	rej[14].deadline[13][0] = 180;
	rej[14].deadline[13][1] = 132;
	rej[14].deadline[13][2] = 132;

	rej[14].deadline[14][0] = 159;
	rej[14].deadline[14][1] = 120;
	rej[14].deadline[14][2] = 120;

	rej[14].deadline[15][0] = 181;
	rej[14].deadline[15][1] = 133;
	rej[14].deadline[15][2] = 133;


	rej[15].deadline[2][0] = 170;
	rej[15].deadline[2][1] = 160;
	rej[15].deadline[2][2] = 123;

	rej[15].deadline[13][0] = 178;
	rej[15].deadline[13][1] = 131;
	rej[15].deadline[13][2] = 132;

	rej[15].deadline[14][0] = 159;
	rej[15].deadline[14][1] = 120;
	rej[15].deadline[14][2] = 120;

	rej[15].deadline[15][0] = 180;
	rej[15].deadline[15][1] = 133;
	rej[15].deadline[15][2] = 134;


	rej[16].deadline[2][0] = 180;
	rej[16].deadline[2][1] = 127;
	rej[16].deadline[2][2] = 126;

	rej[16].deadline[13][0] = 182;
	rej[16].deadline[13][1] = 133;
	rej[16].deadline[13][2] = 133;

	rej[16].deadline[14][0] = 162;
	rej[16].deadline[14][1] = 122;
	rej[16].deadline[14][2] = 122;

	rej[16].deadline[15][0] = 184;
	rej[16].deadline[15][1] = 135;
	rej[16].deadline[15][2] = 135;
}

void modetimer_action()
{
	interrupt = 1;
}

int modetimer_delete()
{
	if (timer_delete(modetimer_id) == -1) {
		assert_perror(errno);
		return -1;
	}

	return 0;
}

int modetimer_adjust(const unsigned int deadline)
{
	struct itimerspec it_mode;
	struct sigevent sigev;

	it_mode.it_value.tv_sec = deadline;
	it_mode.it_value.tv_nsec = 0;
	it_mode.it_interval.tv_sec = 0;
	it_mode.it_interval.tv_nsec = 0;

	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGRTMIN + 2;

	if (signal(SIGRTMIN + 2, modetimer_action) == SIG_ERR) {
		assert_perror(errno);
		return -1;
	}

	if (timer_create(CLOCK_REALTIME, &sigev, &modetimer_id) == -1) {
		assert_perror(errno);
		return -1;
	}

	if (timer_settime(modetimer_id, 0, &it_mode, NULL) == -1) {
		assert_perror(errno);
		return -1;
	}

	return 0;
}

int ftimes(const int mode, const char item)
{
	unsigned char i;
	unsigned char m[TIMESN] = {3, 4, 16, 18, 21};
	unsigned char t[TIMESN][3] = {{160, 140, 140}, {150, 130, 130}, {153, 133, 133}, {7, 7, 7}, {7, 7, 7}};

	if (item > 3) {
		TRACE(item);
		return -1;
	}

	for (i = 0; i < TIMESN; i++)
		if (mode == m[i])
			return (int)(t[i][(int)item]);

	return -1;
}

int the_doors()
{
	unsigned char i;

	for (i = 0; i < 12; i++) {
		if (ntohs(ab30.donesSUGP) & VD_DOORS_AND_WINDOWS_CLOSED)
			return 0;

		if (io_send_msg(msg[151], 32) == -1)
			return -1;

		delay(1000, 50);

		if (io_send_msg("                ", 32) == -1)
			return -1;

		if (interrupt)
			return -1;

		delay(1000, 50);
	}

	return -1;
}

int job(const unsigned char state)
{
	unsigned char i;
	unsigned short buf;
	unsigned int cmd, st;

	switch (state) {
	case 0:
		buf = ntohs(bc10.stateAPK);
		/*buf &= 0xf7ff;*/
		buf &= 0xfbff;
		buf = htons(buf);

		if (data_copy(&bc10, &buf, sizeof(struct BC10)) == -1)
			return -1;

		if (data_send(CL_APK, 0xbc10) == -1)
			return -1;


		st = ntohl(bb30.stateOEP_2);
		st >>= 2;
		st &= 0xfff0;

		cmd = ntohl(bb10.stateOEP);
		cmd &= 0xfffb000f;

		cmd |= st;

		bb10.stateOEP = htonl(cmd);

		if (data_send(CL_OEP, 0xbb10) == -1)
			return -1;

		break;
	case 1:
		buf = ntohs(bc10.stateAPK);
		buf |= 0x400;
		buf = htons(buf);

		if (data_copy(&bc10, &buf, sizeof(struct BC10)) == -1)
			return -1;

		if (data_send(CL_APK, 0xbc10) == -1)
			return -1;


		st = ntohl(bb30.stateOEP_2);
		st >>= 2;
		st &= 0xfff0;

		cmd = ntohl(bb10.stateOEP);
		cmd |= 0x20000;
		cmd &= 0xffff000f;

		cmd |= st;

		cmd = htonl(cmd);

		if (data_copy(&bb10.stateOEP, &cmd, sizeof(int)) == -1)
			return -1;

		if (data_send(CL_OEP, 0xbb10) == -1)
			return -1;

		for (i = 0; i < (3000000 / 50000); i++) {
			usleep(50000);

			if (ntohl(bb30.stateOEP_2) & 0x20000)
				break;
		}
		/* По инициативе КБМ: */
		/*if (!(ntohl(bb30.stateOEP_2) & 0x20000)) {
			if (io_send_msg(msg[164], 8) == -1)
				return -1;

			if (abus_send_msg(R1, 0xe1e1, msg[164], 16) == -1)
				return -1;

			return -1;
		}*/

		break;
	default:
		TRACE(state);
		return -1;
	}

	return 0;
}

void *sugp(void *arg)
{
	int rv;
	unsigned short i, buf, cmd;
	unsigned char j;
	unsigned int tmp;

	cmd = VKL_SUGP;
	buf = htons(cmd);

	rv = data_copy(&ab10.command, &buf, sizeof(short));
	if (rv == -1)
		pthread_exit(&rv);

	rv = data_send(CL_SUGP, 0xab10);
	if (rv == -1)
		pthread_exit(&rv);

	for (i = 0; i < 1580000 / 50000; i++) {
		usleep(50000);

		buf = ntohs(ab30.donesSUGP);

		if (buf & VD_SUGP_VKL)
			break;
	}
	if (!(buf & VD_SUGP_VKL)) {
		if (io_send_msg(msg[143], 8) == -1)
			pthread_exit(&rv);

		if (io_send_msg(msg[144], 16) == -1)
			pthread_exit(&rv);

		if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
			pthread_exit(&rv);

		if (abus_send_msg(R1, 0xe1e1, msg[144], 16) == -1)
			pthread_exit(&rv);

		interrupt = 2;

		pthread_exit(&rv);
	}

	switch (*((int *)arg)) {
	case 3:
	case 12:
	case 15:
		cmd |= VYVES_AGREGAT;
		buf = htons(cmd);

		rv = data_copy(&ab10.command, &buf, sizeof(short));
		if (rv == -1)
			pthread_exit(&rv);

		rv = data_send(CL_SUGP, 0xab10);
		if (rv == -1)
			pthread_exit(&rv);

		for (i = 0; i < (55000000 / 50000); i++) {
			usleep(50000);

			buf = ntohs(ab30.donesSUGP);

			if (buf & VD_AGREGAT_VYVESH)
				break;
		}
		if (!(buf & VD_AGREGAT_VYVESH)) {
			if (io_send_msg(msg[143], 8) == -1)
				pthread_exit(&rv);

			if (io_send_msg(msg[145], 16) == -1)
				pthread_exit(&rv);

			if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
				pthread_exit(&rv);

			if (abus_send_msg(R1, 0xe1e1, msg[145], 16) == -1)
				pthread_exit(&rv);

			interrupt = 2;

			pthread_exit(&rv);
		}

		if (*((int *)arg) == 15) {
			buf = htons(OTKL_SUGP);

			rv = data_copy(&ab10.command, &buf, sizeof(short));
			if (rv == -1)
				pthread_exit(&rv);

			rv = data_send(CL_SUGP, 0xab10);
			if (rv == -1)
				pthread_exit(&rv);

			for (i = 0; i <= 100000 * 30 / 50000; i++) {
				usleep(50000);

				buf = ntohs(ab30.donesSUGP);

				if (!(buf & VD_SUGP_VKL))
					break;
			}
			if (buf & VD_SUGP_VKL) {
				if (io_send_msg(msg[143], 8) == -1)
					pthread_exit(&rv);

				if (abus_send_msg(R1, 0xe1e1, msg[143], 16) == -1)
					pthread_exit(&rv);

				interrupt = 2;

				pthread_exit(&rv);
			}

			if (!cd10.bit)
				for (i = 0; i < 55; i++) {
					delay(1000, 50);

					if (interrupt)
						pthread_exit(&rv);
				}
			else
				timedelta += 55;

			tmp = ntohl(ab10.stateSESandSUGP);
			tmp &= 0xfffffffc;
			tmp |= 0x4;

			if (!citem)
				citem = 3;

			if (citem & 1)
				tmp |= 16;

			if (citem & 2)
				tmp |= 8;

			if (citem == 3)
				citem = 0;

			tmp = htonl(tmp);

			rv = data_copy(&ab10.stateSESandSUGP, &tmp, sizeof(int));
			if (rv == -1)
				pthread_exit(&rv);

			cmd = VKL_SUGP;
			buf = htons(cmd);

			rv = data_copy(&ab10.command, &buf, sizeof(short));
			if (rv == -1)
				pthread_exit(&rv);

			rv = data_send(CL_SUGP, 0xab10);
			if (rv == -1)
				pthread_exit(&rv);

			for (i = 0; i < 1580000 / 50000; i++) {
				usleep(50000);

				buf = ntohs(ab30.donesSUGP);

				if (buf & VD_SUGP_VKL)
					break;
			}
			if (!(buf & VD_SUGP_VKL)) {
				if (io_send_msg(msg[143], 8) == -1)
					pthread_exit(&rv);

				if (io_send_msg(msg[144], 16) == -1)
					pthread_exit(&rv);

				if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
					pthread_exit(&rv);

				if (abus_send_msg(R1, 0xe1e1, msg[144], 16) == -1)
					pthread_exit(&rv);

				interrupt = 2;

				pthread_exit(&rv);
			}

			cmd ^= VYVES_AGREGAT;
		}

		if ((*((int *)arg) == 3) || (*((int *)arg) == 12))
			break;

		cmd ^= VYVES_AGREGAT;

		if (interrupt)
			pthread_exit(&rv);
	case 2:
	case 13:
	case 14:
		cmd |= ZAVYAZ_AGK;
		buf = htons(cmd);

		rv = data_copy(&ab10.command, &buf, sizeof(short));
		if (rv == -1)
			pthread_exit(&rv);

		rv = data_send(CL_SUGP, 0xab10);
		if (rv == -1)
			pthread_exit(&rv);

		/*for (i = 0; i < (8000000 / 50000); i++) {*/
		for (i = 0; i < (24000000 / 50000); i++) {
			usleep(50000);

			buf = ntohs(ab30.donesSUGP);

			if (buf & VD_AGK_ZAVYAZ)
				break;
		}
		if (!(buf & VD_AGK_ZAVYAZ)) {
			if (io_send_msg(msg[143], 8) == -1)
				pthread_exit(&rv);

			if (io_send_msg(msg[163], 16) == -1)
				pthread_exit(&rv);

			if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
				pthread_exit(&rv);

			if (abus_send_msg(R1, 0xe1e1, msg[163], 16) == -1)
				pthread_exit(&rv);

			interrupt = 2;

			pthread_exit(&rv);
		}

		cmd ^= ZAVYAZ_AGK;

		if (job(1) == -1) {
			interrupt = 2;

			pthread_exit(&rv);
		}

		if (!citem)
			citem = 3;

		for (i = 1; i <= 2; i++)
			if (i & citem) {
				cmd |= (i == 1) ? OPEN_LEFT_ROOF : OPEN_RIGHT_ROOF;
				buf = htons(cmd);

				rv = data_copy(&ab10.command, &buf, sizeof(short));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_SUGP, 0xab10);
				if (rv == -1)
					pthread_exit(&rv);

				for (j = 0; j < (10000000 / 50000); j++) {
					usleep(50000);

					buf = ntohs(ab30.donesSUGP);

					if (buf & (i == 1 ? VD_LEFT_ROOF_OPENED : VD_RIGHT_ROOF_OPENED))
						break;
				}
				if (!(buf & (i == 1 ? VD_LEFT_ROOF_OPENED : VD_RIGHT_ROOF_OPENED))) {
					if (io_send_msg(msg[143], 8) == -1)
						pthread_exit(&rv);

					if (io_send_msg(msg[145 + i], 16) == -1)
						pthread_exit(&rv);

					if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
						pthread_exit(&rv);

					if (abus_send_msg(R1, 0xe1e1, msg[145 + i], 16) == -1)
						pthread_exit(&rv);

					interrupt = 2;

					pthread_exit(&rv);
				}

				cmd ^= (i == 1) ? OPEN_LEFT_ROOF : OPEN_RIGHT_ROOF;

				if (interrupt)
					pthread_exit(&rv);

				cmd |= (i == 1) ? LIFT_LEFT_GUIDE : LIFT_RIGHT_GUIDE;

				buf = htons(cmd);

				rv = data_copy(&ab10.command, &buf, sizeof(short));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_SUGP, 0xab10);
				if (rv == -1)
					pthread_exit(&rv);

				for (j = 0; j < (31000000 / 50000); j++) {
					usleep(50000);

					buf = ntohs(ab30.donesSUGP);

					if (buf & (i == 1 ? VD_LEFT_GUIDE_LIFTED : VD_RIGHT_GUIDE_LIFTED))
						break;
				}
				if (!(buf & (i == 1 ? VD_LEFT_GUIDE_LIFTED : VD_RIGHT_GUIDE_LIFTED))) {
					if (io_send_msg(msg[143], 8) == -1)
						pthread_exit(&rv);

					if (io_send_msg(msg[147 + i], 16) == -1)
						pthread_exit(&rv);

					if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
						pthread_exit(&rv);

					if (abus_send_msg(R1, 0xe1e1, msg[147 + i], 16) == -1)
						pthread_exit(&rv);

					interrupt = 2;

					pthread_exit(&rv);
				}

				cmd ^= (i == 1) ? LIFT_LEFT_GUIDE : LIFT_RIGHT_GUIDE;

				if (interrupt)
					pthread_exit(&rv);

				if (the_doors() == -1) {
					if (io_send_msg(msg[151], 16) == -1)
						pthread_exit(&rv);

					if (abus_send_msg(R1, 0xe0e0, msg[151], 16) == -1)
						pthread_exit(&rv);

					interrupt = 2;

					pthread_exit(&rv);
				}
			}

		if (!e110.breakMask || (e110.breakMask && cd10.bit)) {
			delay(2000, 50);

			for (i = 1; i <= 2; i++)
				if (i & citem) {
					buf = htons((unsigned short)i);

					rv = data_copy(&cf00, &buf, sizeof(struct CF00));
					if (rv == -1)
						pthread_exit(&rv);

					rv = data_send(CL_RMI, 0xcf00);
					if (rv == -1)
						pthread_exit(&rv);

					delay(5000, 50);

					if (io_send_msg(msg[130 + i], 8 + i * 8) == -1)
						pthread_exit(&rv);

					if (abus_send_msg(R1, 0xe0e0, msg[130 + i], 16) == -1)
						pthread_exit(&rv);

					if (*((int *)arg) == 2) {
						if ((i == 2) && (citem == 3)) {	
							rv = rc_offitem(0, 0);
							if (rv == -1)
								pthread_exit(&rv);
						} else {
							rv = rc_offitem(i, 0);
							if (rv == -1)
								pthread_exit(&rv);
						}
					}
				}

			if (citem == 3)
				citem = 0;

			delay(2000, 50);
		}
	case 17:
		cmd |= SVERN_AGREGAT;

		buf = htons(cmd);

		rv = data_copy(&ab10.command, &buf, sizeof(short));
		if (rv == -1)
			pthread_exit(&rv);

		rv = data_send(CL_SUGP, 0xab10);
		if (rv == -1)
			pthread_exit(&rv);

		for (i = 0; i < (119000000 / 50000); i++) {
			usleep(50000);

			buf = ntohs(ab30.donesSUGP);

			if (buf & VD_AGREGAT_PO_POHOD)
				break;
		}
		if (!(buf & VD_AGREGAT_PO_POHOD)) {
			if (io_send_msg(msg[143], 8) == -1)
				pthread_exit(&rv);

			if (io_send_msg(msg[150], 16) == -1)
				pthread_exit(&rv);

			if (abus_send_msg(R1, 0xe0e0, msg[143], 16) == -1)
				pthread_exit(&rv);

			if (abus_send_msg(R1, 0xe1e1, msg[150], 16) == -1)
				pthread_exit(&rv);

			interrupt = 2;

			pthread_exit(&rv);
		}

		break;
	default:
		rv = -1;
		pthread_exit(&rv);
	}

	buf = htons(OTKL_SUGP);

	rv = data_copy(&ab10.command, &buf, sizeof(short));
	if (rv == -1)
		pthread_exit(&rv);

	rv = data_send(CL_SUGP, 0xab10);
	if (rv == -1)
		pthread_exit(&rv);

	for (i = 0; i <= 100000 * 30 / 50000; i++) {
		usleep(50000);

		buf = ntohs(ab30.donesSUGP);

		if (!(buf & VD_SUGP_VKL))
			break;
	}
	if (buf & VD_SUGP_VKL) {
		if (io_send_msg(msg[143], 8) == -1)
			pthread_exit(&rv);

		if (abus_send_msg(R1, 0xe1e1, msg[143], 16) == -1)
			pthread_exit(&rv);

		interrupt = 2;

		pthread_exit(&rv);
	}

	buf = 0;
	rv = data_copy(&ab10.command, &buf, sizeof(short));
	if (rv == -1)
		pthread_exit(&rv);

	rv = data_send(CL_SUGP, 0xab10);

	if ((*((int *)arg) == 3) && (!cd10.bit)) {

	} else {
		interrupt = 1;
	}

	pthread_exit(&rv);

	return NULL;
}

char check_items()
{
	char citem_local = citem;

	if (!citem_local)
		citem_local = 3;

	if ((citem_local & 1) && (!dc10.rch1)) {
		citem_local ^= 1;

		if (io_send_msg(msg[91], 8) == -1)
			return -1;

		if (io_send_msg(msg[165], 16) == -1)
			return -1;

		if (abus_send_msg(R1, 0xe0e0, msg[91], 16) == -1)
			return -1;

		if (abus_send_msg(R1, citem_local ? 0xe0e0 : 0xe1e1, msg[165], 16) == -1)
			return -1;
	}

	if ((citem_local & 2) && (!dc10.rch2)) {
		citem_local ^= 2;

		if (io_send_msg(msg[92], 24) == -1)
			return -1;

		if (io_send_msg(msg[165], 32) == -1)
			return -1;

		if (abus_send_msg(R1, 0xe0e0, msg[92], 16) == -1)
			return -1;

		if (abus_send_msg(R1, citem_local ? 0xe0e0 : 0xe1e1, msg[165], 16) == -1)
			return -1;
	}

	switch (citem_local) {
	case 0:
		interrupt = 2;
		break;
	case 3:
		citem_local = 0;
	}

	return citem_local;
}

int modestart(int mode)
{
	int rv, rv_rc, rv_sugp, sec = 0;
	char flag = 0, state = 0;
	unsigned char i, j;
	unsigned short tmp[14];
	struct itimerspec elapsed;
	pthread_t rc_id, sugp_id;

	if (event_timer_init() == -1)
		return -1;

	if (time(&timestart) == ((time_t) -1)) {
		assert_perror(errno);
		return -1;
	}

	if (io_timer_adjust() == -1)
		return -1;

	tmp[0] = htons((unsigned short)mode);

	if (data_copy(&c110.regID, tmp, sizeof(short)) == -1)
		return -1;

	tmp[0] = htons((unsigned short)(0x3 & (~citem)));

	if (data_copy(&c110.prizIzd, tmp, sizeof(short)) == -1)
		return -1;

	if (data_send(CL_RMI, 0xc110) == -1)
		return -1;

	if (lstack_release() == -1)
		return -1;

	if (e110.breakMask) {
		if (!cd10.bit)
			if (setrej((unsigned char)(ntohs(e110.breakMask) - 1), (unsigned char)mode, (unsigned char)(0x3 & (~citem))) == -1)
				return -1;
	} else {
		if ((mode == 3) || (mode == 4) || (mode == 16) || (mode == 18) || (mode == 21)) {
			if ((mode == 3) && (cd10.bit)) {

			} else {
				if (((mode == 4) || (mode == 16)) && cd10.bit) {
					sec = 30;
				} else {
					sec = ftimes(mode, citem);
					if (sec == -1) {
						TRACE(sec);
						return -1;
					}
				}

				if (modetimer_adjust(sec) == -1)
					return -1;
			}
		}
	}

	if (lstack_release() == -1)
		return -1;

	if (io_screen_clear(0) == -1)
		return -1;

	switch (mode) {
	case 1:
		if (io_send_msg(msg[137], 8) == -1)
			return -1;

		break;
	case 2:
		if (io_send_msg(msg[44], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[44], 16) == -1)
				return -1;

		break;
	case 3:
		if (io_send_msg(msg[122], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[122], 16) == -1)
				return -1;

		break;
	case 4:
		if (io_send_msg(msg[123], 8) == -1)
			return -1;

		break;
	case 12:
		if (io_send_msg(msg[124], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[124], 16) == -1)
				return -1;

		break;
	case 13:
		if (io_send_msg(msg[125], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[125], 16) == -1)
				return -1;

		break;
	case 14:
		if (io_send_msg(msg[31], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[31], 16) == -1)
				return -1;

		break;
	case 15:
		if (io_send_msg(msg[126], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[126], 16) == -1)
				return -1;

		break;
	case 16:
		if (io_send_msg(msg[127], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[127], 16) == -1)
				return -1;

		break;
	case 17:
		if (io_send_msg(msg[128], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[128], 16) == -1)
				return -1;

		break;
	case 18:
		if (io_send_msg(msg[129], 8) == -1)
			return -1;

		if (abus_send_msg(R1, 0xc1c1, msg[129], 16) == -1)
				return -1;

		break;
	case 21:
		if (io_send_msg(msg[130], 8) == -1)
			return -1;

		break;
	case 254:
		if (io_send_msg(msg[167], 8) == -1)
			return -1;

		break;
	case 255:
		if (io_send_msg(msg[168], 8) == -1)
			return -1;

		break;
	default:
		TRACE(mode);
		return -1;
	}

	switch (citem) {
	case 0:
		if (io_send_msg(msg[25], 16) == -1)
			return -1;
		break;
	case 1:
		if (io_send_msg(msg[24], 16) == -1)
			return -1;
		break;
	case 2:
		if (io_send_msg(msg[23], 16) == -1)
			return -1;
		break;
	default:
		TRACE(citem);
		return -1;
	}

	if (abus_get_state())
		if ((mode == 3) || (mode == 13) || (mode == 15) || (mode == 16)) {
			if (mkio_recv(ABUS, R2, 1, tmp) == -1)
				return -1;

			tmp[0] = 0xdffd;

			if (mkio_send(ABUS, R1, 1, tmp) == -1)
				return -1;

			if (io_send_msg(msg[155], 32) == -1)
				return -1;

			if (abus_send_msg(R1, 0xe0e0, msg[155], 16) == -1)
				return -1;

			delay(10000, 50);
			/*rv = sleep(10);
			TRACE(rv);*/

			sec += 10;

			if (abus_send_msg(R1, 0xaeee, "                                                          ", 58) == -1)
				return -1;

			if (io_send_msg(msg[156], 32) == -1)
				return -1;

			if (abus_send_msg(R1, 0xe0e0, msg[156], 16) == -1)
				return -1;
		}

	if (mode == 1) {
		delay(1000, 50);

		if (!citem)
			citem = 3;

		for (i = 1; i <= 2; i++)
			if (i & citem)
				if (io_send_msg(msg[134 + i], 8 + i * 8) == -1)
					return -1;

		interrupt = 2;
	}

	if (mode == 255) {
		if (sys_state) {
			if (!cd10.bit)
				delay(8000, 50);
			else
				timedelta += 8;

			if (io_send_msg(msg[169], 8) == -1)
				return -1;

			if (io_send_msg(msg[170], 16) == -1)
				return -1;

			interrupt = 2;
		} else {
			if (!citem)
				citem = 3;

			for (i = 0; i < 3; i++) {
				switch (i) {
				case 0:
					if (!cd10.bit)
						delay(7000, 50);
					else
						timedelta += 7;

					break;
				case 1:
					if (!cd10.bit)
						delay(40000, 50);
					else
						timedelta += 40;

					break;
				case 2:
					if (!cd10.bit)
						delay(12000, 50);
					else
						timedelta += 12;
				}

				for (j = 1; j <= 2; j++) {
					if (j & citem)
						if (io_send_msg(msg[(i % 2 ? 157 : 159) + j], 8 * j) == -1)
							return -1;

					if ((j == 1) && (citem == 3)) {
						if (!cd10.bit)
							delay(5000, 50);
						else
							timedelta += 5;
					}
				}
			}

			if (io_send_msg(msg[171], 16) == -1)
				return -1;

			interrupt = 1;
		}
	}

	if ((mode == 2) && (!interrupt)) {
		rv = pthread_create(&rc_id, NULL, rc, NULL);
		if (rv)
			pthread_exit(&rv);

		rv = pthread_join(rc_id, (void **)&rv_rc);

		if (rv) {
			assert_perror(rv);
			return -1;
		}
		if (rv_rc == -1)
			interrupt = 2;
	}

	if (!interrupt)
		if ((mode == 2) || (mode == 3) || (mode == 13) || (mode == 14) || (mode == 15) || (mode == 16))
			if (io_stpn_recv(tmp) == -1) {
				if (io_send_msg(msg[138], 8) == -1)
					return -1;

				if (io_send_msg(msg[153], 16) == -1)
					return -1;

				if (abus_send_msg(R1, 0xe0e0, msg[138], 16) == -1)
					return -1;

				if (abus_send_msg(R1, 0xe1e1, msg[153], 16) == -1)
					return -1;

				interrupt = 2;
			}

	if (!interrupt)
		citem = check_items();

	if (citem == -1)
		return -1;

	if (!interrupt) {
		if (system_check(mode) == -1)
			return -1;

		if (mode == 254)
			interrupt = 1;
	}

	if (!interrupt)
		if ((mode == 3) || (mode == 12) || (mode == 15) || (mode == 17)) {
			rv = pthread_create(&sugp_id, NULL, sugp, &mode);
			if (rv) {
				assert_perror(rv);
				return -1;
			}
		}

	while (1) {
		if (!interrupt) {
			if (mode != 1) {
				tmp[0] = io_wait_push();
			} else {
				if (!state)
					tmp[0] = CAI_END_MODE;
				else
					tmp[0] = io_wait_push();
			}
		} else {
			tmp[0] = CAI_END_MODE;
		}

		switch (tmp[0]) {
		case CAI_END_MODE:
			if (!state) {
				if (mode == 2) {
					rv = pthread_cancel(rc_id);
					if (rv)
						if (rv != ESRCH) {
							assert_perror(rv);
							return -1;
						}

					if (interrupt) {
						if (!citem)
							citem = 3;

						for (i = 0; i <= 2; i++)
							if (i & citem) {
								if ((i == 2) && (citem == 3)) {	
									rv = rc_offitem(0, 1);
									if (rv == -1)
										pthread_exit(&rv);
								} else {
									rv = rc_offitem(i, 1);
									if (rv == -1)
										pthread_exit(&rv);
								}
							}

						if (citem == 3)
							citem = 0;
					}
				}

				rv = pthread_join(sugp_id, (void **)&rv_sugp);
				if (rv)
					if (rv != ESRCH) {
						assert_perror(rv);
						return -1;
					}
				if (rv_sugp == -1)
					return -1;

				tmp[0] = ntohs(ab30.donesSUGP);

				if (tmp[0] & VD_SUGP_VKL) {
					tmp[0] = htons(OTKL_SUGP);

					if (data_copy(&ab10.command, tmp, sizeof(short)) == -1)
						return -1;

					if (data_send(CL_SUGP, 0xab10) == -1)
						return -1;

					delay(2000, 50);

					tmp[0] = 0;

					if (data_copy(&ab10.command, tmp, sizeof(short)) == -1)
						return -1;

					if (data_send(CL_SUGP, 0xab10) == -1)
						return -1;

					usleep(200000);
				}

				if (job(0) == -1)
					return -1;

				if (!e110.breakMask)
					if ((mode == 3) || (mode == 4) || (mode == 16) || (mode == 18) || (mode == 21)) {
						if ((mode == 3) && (cd10.bit)) {

						} else {
							if (timer_delete(modetimer_id) == -1) {
								assert_perror(errno);
								return -1;
							}
						}
					}

				if (io_timer_delete() == -1)
					return -1;

				if (event_timer_delete() == -1)
					return -1;

				if (interrupt != 2) {
					if (citem != 3)
						tmp[0] = ~citem & 0x3;
					else
						tmp[0] = citem;

					if (abus_send_msg(R1, 0xaaaa, (char *)tmp, 2) == -1)
						return -1;
				}

				if (abus_send_msg(R1, 0xcccc, msg[45], 16) == -1)
					return -1;

				if (mode != 1)
					switch(interrupt) {
					case 0:
						tmp[0] = htons(102);

						if (data_copy(&cc10.regID, tmp, sizeof(short)) == -1)
							return -1;

						if (io_send_msg(msg[32], 8) == -1)
							return -1;

						break;
					case 1:
						tmp[0] = htons((unsigned short)mode);

						if (data_copy(&cc10.regID, tmp, sizeof(short)) == -1)
							return -1;

						if (io_send_msg(msg[45], 8) == -1)
							return -1;
					case 2:
						tmp[0] = htons(1);

						if (data_copy(&e070.otkazID, tmp, sizeof(short)) == -1)
							return -1;
				} else {
					if (!interrupt)
						if (io_send_msg(msg[122], 16) == -1)
							return -1;
				}

				if (mode == 21) {
					if (!citem)
						citem = 3;

					for (i = 1; i <= 2; i++)
						if (i & citem)
							if (io_send_msg(msg[132 + i], 8 + i * 8) == -1)
								return -1;
				}

				if (io_send_msg(msg[33], 40) == -1)
					return -1;

				if (data_send(CL_RMI, (interrupt != 2) ? 0xcc10 : 0xe070) == -1)
					return -1;

				flag = 1;
				state = 1;
				interrupt = 0;
			}

			break;
		case CAI_RESET:
			if (state) {
				if (io_send_msg(msg[37], 16) == -1)
					return -1;

				if (io_send_msg(msg[34], 40) == -1)
					return -1;

				tmp[0] = 0x380;

				if (mkio_send(11, 1, 1, tmp) == -1)
					return -1;

				delay(1000, 50);

				while (1) {
					if (mkio_recv(CAI, REG_ST, 1, tmp) == -1)
						return -1;

					if (tmp[0] == 0xffff) {
						break;
					} else {
						rv = pthread_yield();
						if (rv) {
							assert_perror(rv);
							return -1;
						}
					}
				};

				return 0;
			}

			break;
		case ((unsigned short) -1):
			return -1;
		}

		if (((mode == 2) || (mode == 13) || (mode == 14)) && (!flag)) {
			if ((sec >= 122) || (cd10.bit)) {
				if (cd10.bit)
					timedelta += 122;

				rv = pthread_create(&sugp_id, NULL, sugp, &mode);
				if (rv) {
					assert_perror(rv);
					return -1;
				}

				flag = 1;
			}

			sec++;
		}

		/*sleep(1);*/
		usleep(100000);
	}

	return 0;
}

void *server(void *arg)
{
	int rv;
	struct Client_Info c[CL_NUMBER];

	strcpy(c[0].addr, CL_KSAU);
	strcpy(c[1].addr, CL_RMI);
	strcpy(c[2].addr, CL_OEP);
	strcpy(c[3].addr, CL_SUGP);
	strcpy(c[4].addr, CL_APK);

	c[0].handler = eth_ksau;
	c[1].handler = eth_rmi;
	c[2].handler = eth_oep;
	c[3].handler = eth_sugp;
	c[4].handler = eth_apk;

	rv = sim_server_up("10.130.59.1", 2222, c, CL_NUMBER);

	pthread_exit(&rv);

	return NULL;
}

int multiloader()
{
	if (io_send_msg("   T 00.00.00   ", 0) == -1)
		return -1;

	delay(1000, 50);

	if (ntohs(dc10.rch1))
		switch (ntohs(dc10.rch1)) {
		case 1:
			if (io_send_msg(dc10.stateMask & 0x4 ? msg[110] : msg[109], 16) == -1)
				return -1;
			break;
		case 2:
			if (io_send_msg(dc10.stateMask & 0x4 ? msg[112] : msg[111], 16) == -1)
				return -1;
			break;
		case 3:
			if (io_send_msg(dc10.stateMask & 0x4 ? msg[114] : msg[113], 16) == -1)
				return -1;
			break;
		default:
			if (io_send_msg(msg[115], 16) == -1)
				return -1;
			return -1;
		}

	if (ntohs(dc10.rch2))
		switch (ntohs(dc10.rch2)) {
		case 1:
			if (io_send_msg(dc10.stateMask & 0x4 ? msg[110] : msg[109], 24) == -1)
				return -1;
			break;
		case 2:
			if (io_send_msg(dc10.stateMask & 0x4 ? msg[112] : msg[111], 24) == -1)
				return -1;
			break;
		case 3:
			if (io_send_msg(dc10.stateMask & 0x4 ? msg[114] : msg[113], 24) == -1)
				return -1;
			break;
		default:
			if (io_send_msg(msg[115], 24) == -1)
				return -1;
			return -1;
		}

	delay(2000, 50);

	if (dc10.rch1 && dc10.rch2)
		if (io_send_buf("                                ", 16, 32) == -1)
			return -1;

	if (dc10.rch1)
		if (io_send_msg("                ", 16) == -1)
			return -1;

	if (dc10.rch2)
		if (io_send_msg("                ", 24) == -1)
			return -1;

	return 0;
}

int main(int argc, char **argv)
{
	unsigned char i;
	int arg, rv, rv_list;
	pthread_t list_id, server_id, queue_id;
	unsigned short buf[2];
	char name[43];
	char *s;


	pthread_t heater_tr;
	char str[10];


	if (getopt(argc, argv, "-v") != -1) {
		printf("%s %s\n", __DATE__, __TIME__);
		return 0;
	}

	if (queue_init() == -1)
		return -1;

	if (data_init() == -1)
		return -1;

	rv = pthread_create(&server_id, NULL, server, NULL);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	progress_bar();

	s = ptime() - 11;

	for (i = 0; s[i] != '\0'; i++)
		if (s[i] == ' ')
			s[i] = '_';

	strncpy(name, "/home/rage/log/log_", 19);
	name[19] = '\0';
	strcat(name, s);
	strcat(name, ".txt");

	if (freopen(name, "w", stdout) == NULL) {
		assert_perror(errno);
		return -1;
	}

	if (freopen(name, "w", stderr) == NULL) {
		assert_perror(errno);
		return -1;
	}

	rv = pthread_create(&queue_id, NULL, queue_daemon, NULL);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	if (mkio_init() == -1)
		return -1;

	dc10.codeZH = htonl(0x88418841);

	rv = pthread_create(&system_id, NULL, system_poll, NULL);
	if (rv) {
		assert_perror(rv);
		pthread_exit(&rv);
	}

	rv = pthread_create(&stpn_id, NULL, stpn_poll, NULL);
	if (rv) {
		assert_perror(rv);
		pthread_exit(&rv);
	}

	if (io_initmsg() == -1)
		return -1;

	initrej();


	if(pthread_create(&heater_tr,NULL,heating,NULL))
	{
		fprintf(stderr,"pthread_create()!=0\n");
		return -1;
	}

	bLockPUITrd=1;
	rv = pthread_create(&pui_tr, NULL, puitrd, &arg);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	if(!strcmp(argv[argc-1],"-to_aoo"))
	{
		to_aoo=1;
		fprintf(stderr,"TO AOO\n");
	}

	if(to_aoo)
	{
		printf("Enter 'q' for exit\n");
		APK=1;
		do
		{
			fgets(str,sizeof(str),stdin);
			if(!strcmp(str,"q\n")) break;
			usleep(1000);
		}while(1);
		printf("END OF MODE!\n");
		buf[0]=0xFFFF;
		buf[1]=0xFFFF;
		mkio_send(HEATER,R1,2,buf);
		/*pthread_cancel(server_id);*/
		pthread_cancel(heater_tr);
		return 0;
	}


	while (1) {
		timedelta = 0;

		while (1) {
			if (mkio_recv(CAI, REG_ST, 1, buf) == -1)
				return -1;

			if (buf[0] != 0xffff) {
				break;
			} else {
				rv = pthread_yield();
				if (rv) {
					assert_perror(rv);
					return -1;
				}
			}
		};

		buf[0] = ntohs(a120.stateMask);
		buf[0] |= 4;
		buf[0] = htons(buf[0]);

		if (data_copy(&a120, buf, sizeof(struct A120)) == -1)
			return -1;

		if (data_send(CL_RMI, 0xa120) == -1)
			return -1;

		delay(3000, 50);

		if (io_devices_init() == -1)
			return -1;

		if (io_open_code_u82() == -1)
			return -1;

		/*if (system_refresh() == -1)
			return -1;*/

		if (io_removesec() == -1)
			return -1;

		if (io_screen_clear(1) == -1)
			return -1;


		bLockPUITrd=0;


		rv = multiloader();

		if ((ntohs(dc10.rch1) == 1) || (ntohs(dc10.rch2) == 1)) {
			timestart = 0;

			if (!rv)
				arg = abus_poll();
			else if (rv == -1)
				arg = -1;

			if ((arg == ABUS_MSG_STATE) || (arg == -1) || (arg == 20)) {
				if (io_send_msg(msg[33], 40) == -1)
					return -1;

				while (1) {
					buf[0] = io_wait_push();

					if (buf[0] == CAI_RESET)
						break;

					if (buf[0] == ((unsigned short) -1)) {
						return -1;
					} else {
						rv = pthread_yield();
						if (rv) {
							assert_perror(rv);
							return -1;
						}
					}
				}

				if (io_send_msg(msg[37], 16) == -1)
					return -1;

				if (io_send_msg(msg[34], 40) == -1)
					return -1;

				buf[0] = 0x380;

				if (mkio_send(11, 1, 1, buf) == -1)
					return -1;

				delay(1000, 50);

				while (1) {
					if (mkio_recv(CAI, REG_ST, 1, buf) == -1)
						return -1;

					if (buf[0] == 0xffff) {
						break;
					} else {
						rv = pthread_yield();
						if (rv) {
							assert_perror(rv);
							return -1;
						}
					}
				};
			} else {
				rv = pthread_create(&list_id, NULL, list, &arg);
				if (rv) {
					assert_perror(rv);
					return -1;
				}

				rv = pthread_join(list_id, (void **)&rv_list);
				TRACE(rv);
				TRACE(rv_list);
				if (rv) {
					assert_perror(rv);
					return -1;
				}
				if (rv_list == -1)
					return -1;

				if (abus_get_state())
					abus_set_state(0);
				else
					list_free();

				lstack_destroy();
			}


		} else {
			arg=0;
			
			if(pthread_create(&list_id,NULL,vednz,&arg)!=0)
			{
				fprintf(stderr,"pthread_create()!=0\n");

				return -1;
			}
			/*printf("Greate PRREG thread...\n");
			if(pthread_create(&pr_tr,NULL,prreg,&arg)!=0)
			{
				fprintf(stderr,"pthread_create()!=0\n");

				return -1;
			}*/
			puts("Ожидание завершения потока list_id (функция vednz)...");
			pthread_join(list_id,NULL);
			puts("Поток list_id завершен.");
			ABUSFlag=0;
		}

		bLockPUITrd=1;


		buf[0] = ntohs(a120.stateMask);
		buf[0] &= 0xfb;
		buf[0] = htons(buf[0]);

		if (data_copy(&a120, buf, sizeof(struct A120)) == -1)
			return -1;

		if (data_send(CL_RMI, 0xa120) == -1)
			return -1;

		switch (ntohs(c110.regID)) {
		case 1:
		case 2:
			buf[0] = 0;

			if (data_copy(&ba10, buf, sizeof(struct BA10)) == -1)
				return -1;

			if (data_send(CL_RMI, 0xba10) == -1)
				return -1;
		}

		if (system_refresh() == -1)
			return -1;
	}

	if (mkio_destroy() == -1)
		return -1;

	return 0;
}
