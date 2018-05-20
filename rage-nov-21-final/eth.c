/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include <fcntl.h>

#include "data.h"
#include "eth.h"
#include "rage.h"
#include "serial.h"
#include "system.h"

char flags[6];

int solve(const char *ip, const unsigned short head, int s, const struct SIM_UserData *t)
{
	/*unsigned int delays = 0;*/
	char *str;
	unsigned char fi, i, j;
	unsigned short temp;
	struct SIM_UserData tmp;

	switch (head) {
	case 0xbc30: fi = 0; break;
	case 0xbb50: fi = 1; break;
	case 0xab50: fi = 2; break;
	case 0xbc10: fi = 3; break;
	case 0xbb10: fi = 4; break;
	case 0xab10: fi = 5; break;
	default: return 1;
	}

	flags[fi] = 0;

	for (i = 0; i < POLL_COUNT; i++) {
		memcpy(&tmp, t, sizeof(struct SIM_UserData));

		if (sim_send(s, &tmp) == -1)
			return -1;

		str = ptime();

		if (str != NULL)
			printf("%s %s Отправлен пакет с заголовком 0x%x. (%d)\n", str, ip, head, i + 1);

		for (j = 0; j < POLL_MAXDELAY / POLL_DELAY; j++) {
			usleep(POLL_DELAY);

			/*printf(".");

			fflush(stdout);*/

			if (flags[fi])
				break;
		}

		/*printf("\n");*/

		/*if (!flags[i])
			delays += POLL_MAXDELAY / POLL_DELAY;
		else
			delays += j + 1;*/

		if (!flags[fi]) {
			if (i != (POLL_COUNT - 1)) {
				continue;
			} else {
				/*if (close(s) == -1) {
					assert_perror(errno);
					return -1;
				}*/
				close(s);

				if (clrem(s) == -1)
					return -1;

				for (j = 0; j < POLL_MAXDELAY / POLL_DELAY; j++) {
					usleep(POLL_DELAY);

					s = fclient(ip);

					if (s)
						break;
				}
				if (s) {
					i = 0;
					/*delays += j + 1;*/
					continue;
				}

				switch (fi) {
				case 0:
				case 3:
					temp = htons(7);
					break;
				case 1:
				case 4:
					temp = htons(5);
					break;
				case 2:
				case 5:
					temp = htons(6);
					break;
				default:
					TRACE(fi);
				}

				if (data_copy(&aa10, &temp, sizeof(struct AA10)) == -1)
					return -1;

				if (data_send(CL_RMI, 0xaa10) == -1)
					return -1;

				fprintf(stderr, "flags[%d] = 0x%x\n", fi, flags[fi]);

				return -1;
			}
		} else {
			break;
		}
	}

	/*printf("%d мс.\n", delays * 50);*/

	return 0;
}

void *eth_sugp(void *arg)
{
	int rv;
	unsigned int tmp;
	struct SIM_UserData *p;

	while (1) {
		p = data_receive(CL_SUGP);
		if (p) {
			switch (p->head) {
			case 0xab20:
				flags[5] = 1;

				break;
			case 0xab30:
				flags[2] = 1;

				rv = data_send(CL_SUGP, 0xab40);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&ab30, p->pData, sizeof(struct AB30));
				if (rv == -1)
					pthread_exit(&rv);

				if (ntohs(ab30.stateMask2) & 0x4) {
					tmp = bb10.stateOEP;
					tmp |= htonl(0x20000);

					rv = data_copy(&bb10.stateOEP, &tmp, sizeof(int));
					if (rv == -1)
						pthread_exit(&rv);

					rv = data_send(CL_OEP, 0xbb10);
					if (rv == -1)
						pthread_exit(&rv);
				}

				rv = data_send(CL_RMI, 0xab30);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xad20:
				rv = data_ands(&ad20, *((unsigned short *)p->pData));
				if (rv == -1)
					pthread_exit(&rv);
			}

			if (p->pData)
				free(p->pData);

			free(p);
		}
	}

	return NULL;
}

void *eth_oep(void *arg)
{
	int rv;
	struct SIM_UserData *p;

	while (1) {
		p = data_receive(CL_OEP);
		if (p) {
			switch (p->head) {
			case 0xbb20:
				flags[4] = 1;

				break;
			case 0xbb30:
				flags[1] = 1;

				rv = data_send(CL_OEP, 0xbb40);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&bb30, p->pData, sizeof(struct BB30));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_RMI, 0xbb30);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xad20:
				rv = data_ands(&ad20, *((unsigned short *)p->pData));
				if (rv == -1)
					pthread_exit(&rv);
			}

			if (p->pData)
				free(p->pData);

			free(p);
		}
	}

	return NULL;
}

void *eth_apk(void *arg)
{
	int rv;
	char flag = 0;
	unsigned int tmp;
	struct SIM_UserData *p;

	while (1) {
		p = data_receive(CL_APK);
		if (p) {
			switch (p->head) {
			case 0xbc20:
				flags[3] = 1;

				break;
			case 0xbc40:
				flags[0] = 1;

				rv = data_send(CL_APK, 0xbc50);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&bc40, p->pData, sizeof(struct BC40));
				if (rv == -1)
					pthread_exit(&rv);

				if (ntohl(bc40.stateAPK) & 0x20000000) {
					if (!flag) {
						tmp = ab10.stateSESandSUGP;
						tmp |= htonl(0x80);
						tmp &= htonl(0xffffff8);

						rv = data_copy(&ab10.stateSESandSUGP, &tmp, sizeof(int));
						if (rv == -1)
							pthread_exit(&rv);

						rv = data_send(CL_SUGP, 0xab10);
						if (rv == -1)
							pthread_exit(&rv);

						flag = 1;
					}
				} else {
					if (flag) {
						tmp = ab10.stateSESandSUGP;
						tmp &= htonl(0xffffff7f);

						rv = data_copy(&ab10.stateSESandSUGP, &tmp, sizeof(int));
						if (rv == -1)
							pthread_exit(&rv);

						rv = data_send(CL_SUGP, 0xab10);
						if (rv == -1)
							pthread_exit(&rv);

						flag = 0;
					}
				}

				rv = data_send(CL_RMI, 0xbc40);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xad20:
				rv = data_ands(&ad20, *((unsigned short *)p->pData));
				if (rv == -1)
					pthread_exit(&rv);
			}

			if (p->pData)
				free(p->pData);

			free(p);
		}
	}

	return NULL;
}

void *eth_ksau(void *arg)
{
	int rv;
	struct SIM_UserData *p;

	while (1) {
		p = data_receive(CL_KSAU);
		if (p) {
			if (p->head == 0xe030) {
				rv = data_copy(&e030, p->pData, sizeof(struct E030));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_RMI, 0xe030);
				if (rv == -1)
					pthread_exit(&rv);
			}

			if (p->pData)
				free(p->pData);

			free(p);
		}
	}

	return NULL;
}

int collect(const unsigned char flag)
{
	int rv;
	unsigned int tmp_uint;
	unsigned char i, j;
	unsigned short mask, tmp, buf[14] = {0};

	if (data_send(CL_OEP, 0xbb50) == -1)
		return -1;

	if (data_send(CL_SUGP, 0xab50) == -1)
		return -1;

	if (data_send(CL_APK, 0xbc30) == -1)
		return -1;

	delay(1000, 50);

	mask = ntohs(a120.stateMask);

	if (io_stpn_recv(buf) != -1)
		mask |= 1;
	else
		mask &= 0xfffe;

	rv = serial_test(O_RDWR | O_NONBLOCK);
	if (rv != -1) {
		close(rv);
		mask |= 2;
	} else {
		mask &= 0xfffd;
	}

	mask |= 8;/* АБУС :) */

	tmp = ntohs(b130.stateMaskPUO);
	if (tmp & 1)
		mask |= 0x10;

	if (!flag) {
		tmp_uint = ntohl(bc40.stateAPK);
		if (tmp_uint & 0x800)
			mask |= 0x20;

		tmp_uint = ntohl(bb30.stateOEP_1);
		if (tmp_uint & 1)
			mask |= 0x40;

		tmp_uint = ntohl(ab30.stateMask1);
		if (tmp_uint & 0x10)
			mask |= 0x80;
	} else {
		if (!flags[0])
			mask |= 0x20;

		if (!flags[1])
			mask |= 0x40;

		if (!flags[2])
			mask |= 0x80;
	}

	mask = htons(mask);

	if (data_copy(&a120, &mask, sizeof(struct A120)) == -1)
		return -1;

	if (data_send(CL_RMI, 0xa120) == -1)
		return -1;

	return 0;
}

void *eth_rmi(void *arg)
{
	int rv;
	unsigned short tmp;
	struct SIM_UserData *p;
	pthread_t serial_id;

	while (1) {
		p = data_receive(CL_RMI);
		if (p) {
			switch (p->head) {
			case 0xa110:
				rv = collect(0);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_RMI, 0xb130);
				if (rv == -1)
					pthread_exit(&rv);	

				break;
			case 0xa210:
				rv = data_send(CL_RMI, 0xa220);
				if (rv == -1)
					pthread_exit(&rv);

				if (!cd10.mode) {
					rv = data_send(CL_KSAU, 0xcd10);
					if (rv == -1)
						pthread_exit(&rv);

					rv = data_send(CL_KSAU, 0xdc10);
					if (rv == -1)
						pthread_exit(&rv);

					rv = data_send(CL_KSAU, 0xa210);
					if (rv == -1)
						pthread_exit(&rv);
				}

				if (ntohs(cd10.regID) == 101) {
					rv = pthread_create(&serial_id, NULL, serial, NULL);
					if (rv) {
						assert_perror(rv);
						pthread_exit(&rv);
					}
				}

				break;
			case 0xab10:
				rv = data_copy(&ab10, p->pData, sizeof(struct AB10));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_SUGP, 0xab10);
				if (rv == -1)
					pthread_exit(&rv);

				if (rv != 2) {
					rv = data_send(CL_RMI, 0xab20);
					if (rv == -1)
						pthread_exit(&rv);
				}

				break;
			case 0xab50:
				rv = data_send(CL_RMI, 0xab60);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_SUGP, 0xab50);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xACDC:/* \m/ */
				rv = collect(1);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xad10:
				tmp = htons(2 | 0x200 | 0x400);

				rv = data_copy(&ad20, &tmp, sizeof(struct AD20));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_OEP, 0xad10);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_SUGP, 0xad10);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_APK, 0xad10);
				if (rv == -1)
					pthread_exit(&rv);

				/*sleep(5);*/

				rv = data_send(CL_RMI, 0xad20);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xb110:
				rv = data_send(CL_RMI, 0xb120);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&b110, p->pData, sizeof(struct B112));
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xbb10:
				rv = data_copy(&bb10, p->pData, sizeof(struct BB10));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_OEP, 0xbb10);
				if (rv == -1)
					pthread_exit(&rv);

				if (rv != 2) {
					rv = data_send(CL_RMI, 0xbb20);
					if (rv == -1)
						pthread_exit(&rv);
				}

				break;
			case 0xbb50:
				rv = data_send(CL_RMI, 0xbb60);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_OEP, 0xbb50);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xbc10:
				rv = data_copy(&bc10, p->pData, sizeof(struct BC10));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_APK, 0xbc10);
				if (rv == -1)
					pthread_exit(&rv);

				if (rv != 2) {
					rv = data_send(CL_RMI, 0xbc20);
					if (rv == -1)
						pthread_exit(&rv);
				}

				break;
			case 0xbc30:
				rv = data_send(CL_RMI, 0xbc60);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_send(CL_APK, 0xbc30);
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xcb10:
				rv = data_send(CL_RMI, 0xcb20);
				if (rv == -1)
					pthread_exit(&rv);

				if (ntohs(cd10.regID) == 101) {
					/*rv = pthread_cancel(stpn_id);
					if (rv)
						if (rv != ESRCH) {
							assert_perror(rv);
							pthread_exit(&rv);
						}*/

					stflag = 0;
					flood = 0;

					rv = pthread_cancel(serial_id);
					if (rv)
						if (rv != ESRCH) {
							assert_perror(rv);
							pthread_exit(&rv);
						}
				}

				break;
			case 0xcd10:
				rv = data_send(CL_RMI, 0xcd20);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&cd10, p->pData, sizeof(struct CD10));
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xda10:
				rv = data_send(CL_RMI, 0xda20);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&da10, p->pData, sizeof(struct DA10));
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xdb30:
				rv = data_send(CL_RMI, 0xdb40);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&db30, p->pData, sizeof(struct DB30));
				if (rv == -1)
					pthread_exit(&rv);

				break;
			case 0xdc10:
				rv = data_send(CL_RMI, 0xdc20);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&dc10, p->pData, sizeof(struct DC10));
				if (rv == -1)
					pthread_exit(&rv);

				sys_state = 0;

				break;
			case 0xe110:
				rv = data_send(CL_RMI, 0xe120);
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&e110, p->pData, sizeof(struct E110));
				if (rv == -1)
					pthread_exit(&rv);

				rv = data_copy(&e070, p->pData, sizeof(struct E070));
				if (rv == -1)
					pthread_exit(&rv);
			}

			if (p->pData)
				free(p->pData);

			free(p);
		}
	}

	return NULL;
}
