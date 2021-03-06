/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "data.h"
#include "io.h"
#include "mkio.h"
#include "queue.h"
#include "rage.h"

#include "src-1450/baselib.h"
#include "src-1450/tech.h"

#include <errno.h>
#include <math.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

timer_t timer_id;

struct geodata stpn_data;

char *msg[MSGCOUNT];
pthread_mutex_t screen = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stpn = PTHREAD_MUTEX_INITIALIZER;

int io_stpn_recv(unsigned short *buf)
{
	int rv, zone;
	float azfl;
	double a0, a1, az, cosbp1, cosbp1cosbp1, cosq1, cosq1cosq1, b1, bp1, f1, f1f1, gamv, np1, q1;
	unsigned char i, j;
	unsigned short crc = 0;
	unsigned short *ptr = (unsigned short *)(&stpn_data);

	rv = pthread_mutex_lock(&stpn);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	for (i = 0; i < 5; i++) {
		if (mkio_recv(STAN, GEO_DATA, sizeof(stpn_data) / sizeof(short), (unsigned short *)(&stpn_data)) == -1) {
			rv = pthread_mutex_unlock(&stpn);
			if (rv) {
				assert_perror(rv);
				return -1;
			}

			return -1;
		}

		for (j = 0; j < (sizeof(stpn_data) - sizeof(crc)) / sizeof(crc); j++)
			crc = (~crc & ptr[i]) | (~ptr[i] & crc);

		if (crc == stpn_data.crc)
			break;

		usleep(100000);
	}

	if (stpn_data.crc == 0xffff) {
		/*fprintf(stderr, "stpn_data.crc == 0xffff\n");*/
		rv = pthread_mutex_unlock(&stpn);
		if (rv) {
			assert_perror(rv);
			return -1;
		}

		return -1;
	}

	zone = stpn_data.y / 1000000;

	stpn_data.y -= zone * 1000000;

	q1 = stpn_data.x / 6367558.4968;

	cosq1 = cos(q1);
	cosq1cosq1 = cosq1 * cosq1;

	bp1 = 0.0000002346304 + 0.000000002686829;
	bp1 *= cosq1cosq1 * cosq1cosq1;
	bp1 = (-0.00002936228 - bp1) * cosq1cosq1;
	bp1 = (0.005022174 - bp1) * cosq1 * sin(q1);
	bp1 += q1;

	cosbp1 = cos(bp1);
	cosbp1cosbp1 = cosbp1 * cosbp1;

	np1 = 6399698.9 / sqrt(1.0 + 0.0067385248 * cosbp1cosbp1);

	f1 = (stpn_data.y - 500000.0) / np1 * cosbp1;
	f1f1 = f1 * f1;

	a0 = (0.005615437 * cosbp1cosbp1 + 0.161612773) * cosbp1cosbp1 + 0.25;
	a1 = (0.003369262 * cosbp1cosbp1 + 0.5) * cosbp1 * sin(bp1);

	b1 = 1.0 - (a0 - 0.125 * f1f1) * f1f1;
	b1 = bp1 - a1 * b1 * f1f1;

	gamv = (stpn_data.y - 500000.0) * sin(b1) / cos(b1) / 6371210.0;

	az = stpn_data.du * M_PI / 32768.0 + gamv;
	azfl = az;

	stpn_data.y += zone * 1000000;

	buf[7] = *((unsigned short *)(&azfl));
	buf[8] = *((unsigned short *)(&azfl) + 1);

	buf[9] = stpn_data.x & 0xffff;
	buf[10] = (stpn_data.x & 0xffff0000) >> 16;

	buf[11] = stpn_data.y & 0xffff;
	buf[12] = (stpn_data.y & 0xffff0000) >> 16;

	buf[13] = stpn_data.h;

	rv = pthread_mutex_unlock(&stpn);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	return 0;
}

int io_open_code_u82()
{
	unsigned short code = 0x4000;

	if (mkio_send(BKD, R1, 1, &code) == -1)
		return -1;

	usleep(50000);

	code = 0;

	if (mkio_send(BKD, R1, 1, &code) == -1)
		return -1;

	usleep(50000);

	code = 0x4000;

	if (mkio_send(BKD, R1, 1, &code) == -1)
		return -1;

	usleep(200000);

	code = 0;

	if (mkio_send(BKD, R1, 1, &code) == -1)
		return -1;

	return 0;
}

int io_psk_cmd(const unsigned short cmd)
{
	unsigned char i;
	unsigned short buf;

	for (i = 0; i < 4; i++) {
		if (mkio_send(BKD, R1, 1, &cmd) == -1)
			return -1;

		usleep(10000);

		if (mkio_recv(BKD, R1, 1, &buf) == -1)
			return -1;

		if ((buf & cmd) == cmd)
			break;

		usleep(10000);
	}

	if ((buf & cmd) != cmd) {
		TRACE(cmd);
		TRACE(buf);
		return -1;
	}

	usleep(50000);

	buf = 0;

	if (mkio_send(BKD, R1, 1, &buf) == -1)
		return -1;

	return 0;
}

int io_devices_init()
{
	const unsigned char r[3] = {R1, R5, R8};
	unsigned char i;
	unsigned short buf = 0, cmd = 0x2ff;

	for (i = 0; i < 3; i++)
		if (mkio_send(BKD, r[i], 1, &buf) == -1)
			return -1;

	for (i = 1; i <= 3; i++)
		if (mkio_send(BE, i, 1, &buf) == -1)
			return -1;

	buf = 7;

	if (mkio_send(BKD, R11, 1, &buf) == -1)
		return -1;

	if (mkio_send(BE, R11, 1, &buf) == -1)
		return -1;

	usleep(50000);

	buf = 0;

	if (mkio_send(BKD, R11, 1, &buf) == -1)
		return -1;

	if (mkio_send(BE, R11, 1, &buf) == -1)
		return -1;

	for (i = 0; i < 3; i++) {
		if (mkio_recv(BKD, r[i], 1, &buf) == -1)
			return -1;

		if (buf) {
			TRACE(buf);
			return -1;
		}
	}

	for (i = 1; i <= 3; i++) {
		if (mkio_recv(BE, i, 1, &buf) == -1)
			return -1;

		if (i == 1)
			buf &= 0xfcff;

		if (buf) {
			TRACE(buf);
			return -1;
		}
	}

	for (i = 0; i < 4; i++, cmd += 0x200) {
		if (io_psk_cmd(cmd) == -1)
			return -1;

		if (mkio_recv(BKD, R3, 1, &buf) == -1)
			return -1;

		/*if (!(buf & 1)) {
			TRACE(buf);
			return -1;
		}*/
	}

	return 0;
}

unsigned short io_wait_push()
{
	int rv, rv_trylock;
	unsigned short buf = 0, stat;


	if(USE_PUI_THREAD)
	{
		buf=KEYCODE;
		if(KEYCODE) KEYCODE=0;
		return buf;
	}


	do {
		rv_trylock = pthread_mutex_trylock(&screen);
		if (!rv_trylock) {
			if (mkio_recv(CAI, REG_ST, 1, &stat) == -1) {
				rv = pthread_mutex_unlock(&screen);
				if (rv)
					assert_perror(rv);

				return ((unsigned short) -1);
			}

			if (stat & 4) {
				usleep(10000);

				rv = pthread_mutex_unlock(&screen);
				if (rv) {
					assert_perror(rv);
					return ((unsigned short) -1);
				}

				return 4;
			}

			if (mkio_send(CAI, REG_A1, 1, &buf) == -1) {
				rv = pthread_mutex_unlock(&screen);
				if (rv)
					assert_perror(rv);

				return ((unsigned short) -1);
			}

			if (mkio_recv(CAI, REG_BUF_OUT, 1, &buf) == -1) {
				rv = pthread_mutex_unlock(&screen);
				if (rv)
					assert_perror(rv);

				return ((unsigned short) -1);
			}

			buf &= 0xff;
			stat |= 4;

			if (mkio_send(CAI, REG_ST, 1, &stat) == -1) {
				rv = pthread_mutex_unlock(&screen);
				if (rv)
					assert_perror(rv);

				return ((unsigned short) -1);
			}

			rv = pthread_mutex_unlock(&screen);
			if (rv) {
				assert_perror(rv);
				return ((unsigned short) -1);
			}
		} else {
			if (rv_trylock == EBUSY) {
				rv = pthread_yield();
				if (rv) {
					assert_perror(rv);
					return ((unsigned short) -1);
				}
			} else {
				assert_perror(rv_trylock);
				return ((unsigned short) -1);
			}
		}
	} while (rv_trylock);

	return buf;
}

int io_initmsg()
{
	msg[0] = "������� ��� ���.";
	msg[1] = "  �������       ";
	msg[2] = "  ����� ������. ";
	msg[3] = "  ������������  ";
	msg[4] = "  ����������.   ";
	msg[5] = "     ��1        ";
	msg[6] = "     ��2        ";
	msg[7] = "     ��3        ";
	msg[8] = "     ��1��      ";
	msg[9] = "     ���        ";
	msg[10] = "  ����� 1�141   ";
	msg[11] = "     �3-�1      ";
	msg[12] = "     �3-�2      ";
	msg[13] = "     �1-�2      ";
	msg[14] = "     �1,�2-�3   ";
	msg[15] = "     �2-�1      ";
	msg[16] = "  ��1 9�55-9�78 ";
	msg[17] = "  ��������      ";
	msg[18] = "  �������� ��   ";
	msg[19] = "  ��2 ��1944    ";
	msg[20] = "  ���������     ";
	msg[21] = "  ���� ���      ";
	msg[22] = "  ����-�����    ";
	msg[23] = "  ������� 2     ";
	msg[24] = "  ������� 1     ";
	msg[25] = "  ������� 2+1   ";
	msg[26] = "  �����  ����.  ";
	msg[27] = "  ������ ����.  ";
	msg[28] = "  ���    ����.  ";
	msg[29] = "  ������ �����  ";
	msg[30] = "  ������� ��'���";
	msg[31] = "����� ��1       ";
	msg[32] = "����� ��������� ";
	msg[33] = "����� ��. ����� ";
	msg[34] = "����� ��. ����  ";
	msg[35] = "����� ���� ���  ";
	msg[36] = "����� ��������� ";
	msg[37] = "9�55 ���        ";
	msg[38] = " ���� �����     ";
	msg[39] = " ��������� �����";
	msg[40] = "������          ";
	msg[41] = "�������         ";
	msg[42] = "  �� 9�694      ";
	msg[43] = "�������� 9�55   ";
	msg[44] = "����� ��1��     ";
	msg[45] = "����� �������   ";
	msg[46] = "����� ��        ";
	msg[47] = "���1            ";
	msg[48] = "���� ��1        ";
	msg[49] = "X1              ";
	msg[50] = "Y1              ";
	msg[51] = "H1              ";
	msg[52] = "X2              ";
	msg[53] = "Y2              ";
	msg[54] = "H2              ";
	msg[55] = "�����.������    ";
	msg[56] = "�����.��������� ";
	msg[57] = "�������� �����  ";
	msg[58] = "������ �����    ";
	msg[59] = "������������ �� ";
	msg[60] = "��� ����� ������";
	msg[61] = "��� ����������  ";
	msg[62] = "���������       ";
	msg[63] = "DHK5            ";
	msg[64] = "������ ��       ";
	msg[65] = "DX��            ";
	msg[66] = "���             ";
	msg[67] = "����            ";
	msg[68] = "������ ��       ";
	msg[69] = "�� �������      ";
	msg[70] = "  ��3           ";
	msg[71] = "  ��2           ";
	msg[72] = "  ��1           ";
	msg[73] = "  ��1 ��        ";
	msg[74] = "  ���           ";
	msg[75] = "  ������� N2    ";
	msg[76] = "  ������� N1    ";
	msg[77] = "  ������� N1+N2 ";
	msg[78] = "  �3->�1        ";
	msg[79] = "  �3->�2        ";
	msg[80] = "  �2->�1        ";
	msg[81] = "  �1,�2->�3     ";
	msg[82] = "  �1->�2        ";
	msg[83] = "  ��1 �55-�78   ";
	msg[84] = "  ��1 �142      ";
	msg[85] = "  ��2 �142      ";
	msg[86] = "  ��2 �1944     ";
	msg[87] = "  ��3 ��        ";
	msg[88] = "  �������� ���  ";
	msg[89] = "  ��������� ����";
	msg[90] = "  ������ ������.";
	msg[91] = "����� 9�728(1)  ";
	msg[92] = "����� 9�728(2)  ";
	msg[93] = "��� ���. ��     ";
	msg[94] = "��� ���. ��     ";
	msg[95] = "��� ���. ���    ";
	msg[96] = "��� ��� ���� 1  ";
	msg[97] = "��� ���. ��     ";
	msg[98] = "��� �� �� 1,2   ";
	msg[99] = "��� ��� ���� 2  ";
	msg[100] = "��� ���. � �����";
	msg[101] = "��� ���. � ���. ";
	msg[102] = "�� ����� ����   ";
	msg[103] = "��� �������� ��.";
	msg[104] = "��� �. ��� �����";
	msg[105] = "��� �������� ��.";
	msg[106] = "��� ����� 9�728 ";
	msg[107] = "����� ������ �� ";
	msg[108] = "� ������� C0C0  ";
	msg[109] = " ��� 9�728      ";
	msg[110] = " ��� 9�728 ��   ";
	msg[111] = " ��� 9�723      ";
	msg[112] = " ��� 9�723 ��   ";
	msg[113] = " ��� 9�723-1    ";
	msg[114] = " ��� 9�723-1 �� ";
	msg[115] = " ����� �������  ";
	msg[116] = "����� 9�87� �85�";
	msg[117] = "���.� ����� ��  ";
	msg[118] = "����� �� ��1 ��3";
	msg[119] = "����� �� ��2 ��3";
	msg[120] = "����� �� ��1 ��2";
	msg[121] = " ����. �������� ";
	msg[122] = "����� �3-�1     ";
	msg[123] = "����� ��������� ";
	msg[124] = "����� �3-�2     ";
	msg[125] = "����� ��2       ";
	msg[126] = "����� ��3       ";
	msg[127] = "����� �2-�1     ";
	msg[128] = "����� �1,�2-�3  ";
	msg[129] = "����� �1-�2     ";
	msg[130] = "����� ��������  ";
	msg[131] = "���� 1 �������� ";
	msg[132] = "���� 2 �������� ";
	msg[133] = "  �� N1 ������  ";
	msg[134] = "  �� N2 ������  ";
	msg[135] = "����� ���    (1)";
	msg[136] = "����� ���    (2)";
	msg[137] = "���. ��1 �55-�78";
	msg[138] = "    ����� 1�141 ";
	msg[139] = "�� ����. ������ ";
	msg[140] = "�� ���. ������  ";
	msg[141] = "�� ����. �������";
	msg[142] = "�� ���. ������� ";
	msg[143] = "����� ����      ";
	msg[144] = "���� �� ��������";
	msg[145] = "����� �� �������";
	msg[146] = "��� ��� �� ���� ";
	msg[147] = "���� ��� �� ����";
	msg[148] = "��� ��� �� ���� ";
	msg[149] = "���� ��� �� ����";
	msg[150] = "����� �� �� ����";
	msg[151] = "������� �����   ";
	msg[152] = "���.� ����� ��  ";
	msg[153] = "��� ����� ����  ";
	msg[154] = "����� ���. ��   ";
	msg[155] = "�������� �������";
	msg[156] = "������� ��������";
	msg[157] = "������ �������� ";
	msg[158] = "������� 1944 �  ";
	msg[159] = "������� 1944 �  ";
	msg[160] = "�������� 1944 � ";
	msg[161] = "�������� 1944 � ";
	msg[162] = "����� ��� ����  ";
	msg[163] = "��� �� �������  ";
	msg[164] = "�� ������ �� ���";
	msg[165] = "��� ���         ";
	msg[166] = "����� ������    ";
	msg[167] = "����� ����� ��  ";
	msg[168] = "����� ��2 �1944 ";
	msg[169] = "����� 1944      ";
	msg[170] = "�� 33           ";
	msg[171] = "����� ��1944    ";

	if (strcmp(msg[MSGCOUNT - 1], "����� ��1944    ")) {
		fprintf(stderr, "strcmp() != 0\n");
		return -1;
	}

	return 0;
}

int io_send_msg(char *buf, const unsigned short word)
{
	int rv;
	unsigned short stat;


	if((word*sizeof(unsigned short)+16)>sizeof(PUIBUF)) 
	{
		printf("ERROR: io_send_msg: out of PUIBUF range\n");
		return -1;
	}
	pthread_mutex_lock(&pui_mtx);
	memcpy(PUIBUF+word*sizeof(unsigned short),buf,16);
	pthread_mutex_unlock(&pui_mtx);


	rv = pthread_mutex_trylock(&screen);
	if (!rv) {
		while (1) {
			if (mkio_recv(CAI, REG_ST, 1, &stat) == -1) {
				rv = pthread_mutex_unlock(&screen);
				if (rv)
					assert_perror(rv);

				return -1;
			}

			if (!(stat & 1))
				break;
			else
				usleep(10000);
		}

		if (mkio_send(CAI, REG_A2, 1, &word) == -1) {
			rv = pthread_mutex_unlock(&screen);
			if (rv)
				assert_perror(rv);

			return -1;
		}

		if (mkio_send(CAI, REG_BUF_IN, 8, (unsigned short *)buf) == -1) {
			rv = pthread_mutex_unlock(&screen);
			if (rv)
				assert_perror(rv);

			return -1;
		}

		stat |= 1;

		if (mkio_send(CAI, REG_ST, 1, &stat) == -1) {
			rv = pthread_mutex_unlock(&screen);
			if (rv)
				assert_perror(rv);

			return -1;
		}

		rv = pthread_mutex_unlock(&screen);
		if (rv) {
			assert_perror(rv);
			return -1;
		}
	} else {
		if (rv == EBUSY) {
			if (queue_push(buf, word, 16) == -1)
				return -1;
		} else {
			assert_perror(rv);
			return -1;
		}
	}

	return 0;
}

int io_send_buf(char *buf, const unsigned short word, const int len)
{
	int rv;
	unsigned short stat;


	if((word*sizeof(unsigned short)+len)>sizeof(PUIBUF)) 
	{
		printf("ERROR: io_send_buf: out of PUIBUF range\n");
		return -1;
	}
	pthread_mutex_lock(&pui_mtx);
	memcpy(PUIBUF+word*sizeof(unsigned short),buf,len);
	pthread_mutex_unlock(&pui_mtx);


	rv = pthread_mutex_trylock(&screen);
	if (!rv) {
		while (1) {
			if (mkio_recv(CAI, REG_ST, 1, &stat) == -1) {
				rv = pthread_mutex_unlock(&screen);
				if (rv)
					assert_perror(rv);

				return -1;
			}

			if (!(stat & 1))
				break;
			else
				usleep(10000);
		}

		if (mkio_send(CAI, REG_A2, 1, &word) == -1) {
			rv = pthread_mutex_unlock(&screen);
			if (rv)
				assert_perror(rv);

			return -1;
		}

		if (mkio_send(CAI, REG_BUF_IN, len / 2, (unsigned short *)buf) == -1) {
			rv = pthread_mutex_unlock(&screen);
			if (rv)
				assert_perror(rv);

			return -1;
		}

		stat |= 1;

		if (mkio_send(CAI, REG_ST, 1, &stat) == -1) {
			rv = pthread_mutex_unlock(&screen);
			if (rv)
				assert_perror(rv);

			return -1;
		}

		rv = pthread_mutex_unlock(&screen);
		if (rv) {
			assert_perror(rv);
			return -1;
		}
	} else {
		if (rv == EBUSY) {
			if (queue_push(buf, word, len) == -1)
				return -1;
		} else {
			assert_perror(rv);
			return -1;
		}
	}

	return 0;
}

void io_update_time()
{
	int tdelta, hour, min, sec;
	char string[16] = {0};
	unsigned char convert[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	unsigned short buf;
	time_t timemode;

	if (time(&timemode) == ((time_t) -1)) {
		assert_perror(errno);
		return;
	}

	tdelta = (int)difftime(timemode, timestart) + timedelta;

	hour = tdelta / 3600;
	min = tdelta / 60 - hour * 60;
	sec = tdelta % 60;

	if (mkio_recv(CAI, REG_ST, 1, &buf) == -1)
		return;

	buf &= 0xffef;

	if (mkio_send(CAI, REG_ST, 1, &buf) == -1)
		return;

	if ((ntohs(dc10.rch1) == 1) || (ntohs(dc10.rch2) == 1)) {
		string[3] = 'T';
		string[5] = convert[hour / 10];
		string[6] = convert[hour % 10];
		string[7] = '.';
		string[8] = convert[min / 10];
		string[9] = convert[min % 10];
		string[10] = '.';
		string[11] = convert[sec / 10];
		string[12] = convert[sec % 10];


	}else if(((ntohs(dc10.rch1)==0)&&(ntohs(dc10.rch2)==0))||((ntohs(dc10.rch1)==3)&&(ntohs(dc10.rch2)==3))||((ntohs(dc10.rch1)==0)&&(ntohs(dc10.rch2)==3))||((ntohs(dc10.rch1)==3)&&(ntohs(dc10.rch2)==0)))
	{
		string[3]='T';
		string[4]='=';
		string[5]=convert[hour / 10];
		string[6]=convert[hour % 10];
		string[7]='.';
		
		string[8]=convert[min / 10];
		string[9]=convert[min % 10];
		string[10]='.';
		
		string[11]=convert[sec / 10];
		string[12]=convert[(sec - sec % 5) % 10];
	}else
	{
		string[4]='T';
		string[6]=convert[hour / 10];
		string[7]=convert[hour % 10];
		string[8]='.';
		
		string[9]=convert[min / 10];
		string[10]=convert[min % 10];
		string[11]='.';
		
 		string[12]=convert[sec / 10];
		string[13]=convert[(sec - sec % 5) % 10];
	}


	if (io_send_msg(string, 0) == -1)
		return;


	if((dc10.rch1!=1)&&(dc10.rch2!=1))
	{
		if((ABUSFlag&0x5656)==0x5656)
			WWREG(string,0xF9F9,16);
	}
}

void io_timer_handler()
{
	int rv;

	rv = pthread_mutex_trylock(&screen);
	if (!rv) {
		rv = pthread_mutex_unlock(&screen);
		if (rv) {
			assert_perror(rv);
			return;
		}

		if (raise(SIGRTMIN + 1))
			return;
	} else {
		if (rv != EBUSY) {
			assert_perror(rv);
			return;
		}
	}
}

int io_timer_delete()
{
	if (timer_delete(timer_id) == -1) {
		assert_perror(errno);
		return -1;
	}

	return 0;
}

int io_timer_adjust()
{
	struct itimerspec it;
	struct sigevent sigev;

	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGRTMIN;

	if (signal(SIGRTMIN, io_timer_handler) == SIG_ERR) {
		assert_perror(errno);
		return -1;
	}

	if (signal(SIGRTMIN + 1, io_update_time) == SIG_ERR) {
		assert_perror(errno);
		return -1;
	}

	if (timer_create(CLOCK_REALTIME, &sigev, &timer_id) == -1) {
		assert_perror(errno);
		return -1;
	}

	it.it_value.tv_sec = 1;
	it.it_value.tv_nsec = 0;

	it.it_interval.tv_sec = 1;
	it.it_interval.tv_nsec = 0;

	if (timer_settime(timer_id, 0, &it, NULL) == -1) {
		assert_perror(errno);
		return -1;
	}

	return 0;
}

int io_screen_clear(const int timer_clear)
{
	int rv, rv_trylock;
	/*int rv;*/
	unsigned short stat, buf[48] = {0x20};

	do {
		rv_trylock = pthread_mutex_trylock(&screen);
		if (!rv_trylock) {
			if (mkio_recv(CAI, REG_ST, 1, &stat) == -1)
				return -1;

			if (timer_clear)
				buf[0] = 8;

			buf[0] = timer_clear ? 0 : 8;

			if (mkio_send(CAI, REG_A2, 1, buf) == -1)
				return -1;

			if (timer_clear)
				buf[0] = 0;

			if (mkio_send(CAI, REG_BUF_IN, timer_clear ? 24 : 16, buf) == -1)
				return -1;

			if (mkio_send(CAI, REG_BUF_IN, 24, buf) == -1)
				return -1;

			stat |= 1;

			if (mkio_send(CAI, REG_ST, 1, &stat) == -1)
				return -1;

			rv = pthread_mutex_unlock(&screen);
			if (rv) {
				assert_perror(rv);
				return -1;
			}

			rv = pthread_mutex_lock(&pui_mtx);
			if (rv) {
				assert_perror(rv);
				return -1;
			}

			memset(timer_clear ? PUIBUF : PUIBUF + 16, 0x20, timer_clear ? 96 : 80);

			rv = pthread_mutex_unlock(&pui_mtx);
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

	return 0;
}

int io_removesec()
{
	unsigned short buf;

	if (mkio_recv(CAI, REG_ST, 1, &buf) == -1)
		return -1;

	buf &= 0xff7;

	if (mkio_send(CAI, REG_ST, 1, &buf) == -1)
		return -1;

	return 0;
}
