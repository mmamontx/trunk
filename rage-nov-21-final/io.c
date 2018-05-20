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
	msg[0] = "קשגעבפר פינ עוצ.";
	msg[1] = "  עבגן‏יך       ";
	msg[2] = "  ףםומב חןפןקמ. ";
	msg[3] = "  ןגףלץציקבמיס  ";
	msg[4] = "  פוטמןלןחי‏.   ";
	msg[5] = "     נח1        ";
	msg[6] = "     נח2        ";
	msg[7] = "     נח3        ";
	msg[8] = "     נח1קנ      ";
	msg[9] = "     מפם        ";
	msg[10] = "  ןנעןף 1פ141   ";
	msg[11] = "     ח3-ח1      ";
	msg[12] = "     ח3-ח2      ";
	msg[13] = "     ח1-ח2      ";
	msg[14] = "     ח1,ח2-ח3   ";
	msg[15] = "     ח2-ח1      ";
	msg[16] = "  פן1 9ץ55-9נ78 ";
	msg[17] = "  כןמפעןלר      ";
	msg[18] = "  כןמפעןלר תû   ";
	msg[19] = "  פן2 בü1944    ";
	msg[20] = "  עוחלבםומפ     ";
	msg[21] = "  קקןה בגמ      ";
	msg[22] = "  קקןה-קשקןה    ";
	msg[23] = "  יתהוליו 2     ";
	msg[24] = "  יתהוליו 1     ";
	msg[25] = "  יתהוליו 2+1   ";
	msg[26] = "  לוקןו  מבנע.  ";
	msg[27] = "  נעבקןו מבנע.  ";
	msg[28] = "  ןגב    מבנע.  ";
	msg[29] = "  תבהבפר עוצים  ";
	msg[30] = "  קשגעבפר ןג'וכפ";
	msg[31] = "עוצים נח1       ";
	msg[32] = "עוצים נעוכעב‎ומ ";
	msg[33] = "מבצםי כל. ףגעןף ";
	msg[34] = "מבצםי כמ. קשכל  ";
	msg[35] = "עוצים קקןה בגמ  ";
	msg[36] = "קשגןע נןהעוציםב ";
	msg[37] = "9ץ55 יףט        ";
	msg[38] = " קקןה ‏יףלב     ";
	msg[39] = " ימהיכבדיס ‏יףלב";
	msg[40] = "םבףףיק          ";
	msg[41] = "üלוםומפ         ";
	msg[42] = "  פן 9ז694      ";
	msg[43] = "כןמפעןלר 9ץ55   ";
	msg[44] = "עוצים נח1קנ     ";
	msg[45] = "עוצים ןכןמ‏ומ   ";
	msg[46] = "ןפכבת עד        ";
	msg[47] = "כעד1            ";
	msg[48] = "קקןה יה1        ";
	msg[49] = "X1              ";
	msg[50] = "Y1              ";
	msg[51] = "H1              ";
	msg[52] = "X2              ";
	msg[53] = "Y2              ";
	msg[54] = "H2              ";
	msg[55] = "כןמפע.בתיםץפ    ";
	msg[56] = "כןמפע.הבלרמןףפר ";
	msg[57] = "ףכןעןףפר קופעב  ";
	msg[58] = "בתיםץפ קופעב    ";
	msg[59] = "כןמזיחץעבדיס יה ";
	msg[60] = "פינ פן‏כי זימיûב";
	msg[61] = "פינ פעבוכפןעיי  ";
	msg[62] = "ûפבפמןףפר       ";
	msg[63] = "DHK5            ";
	msg[64] = "קשףןפב פנ       ";
	msg[65] = "DXפנ            ";
	msg[66] = "מכף             ";
	msg[67] = "כלא‏            ";
	msg[68] = "מבצבפר כק       ";
	msg[69] = "יה קקוהומש      ";
	msg[70] = "  נח3           ";
	msg[71] = "  נח2           ";
	msg[72] = "  נח1           ";
	msg[73] = "  נח1 קנ        ";
	msg[74] = "  מפם           ";
	msg[75] = "  יתהוליו N2    ";
	msg[76] = "  יתהוליו N1    ";
	msg[77] = "  יתהוליו N1+N2 ";
	msg[78] = "  ח3->ח1        ";
	msg[79] = "  ח3->ח2        ";
	msg[80] = "  ח2->ח1        ";
	msg[81] = "  ח1,ח2->ח3     ";
	msg[82] = "  ח1->ח2        ";
	msg[83] = "  פן1 ץ55-נ78   ";
	msg[84] = "  פן1 û142      ";
	msg[85] = "  פן2 û142      ";
	msg[86] = "  פן2 ü1944     ";
	msg[87] = "  נח3 ינ        ";
	msg[88] = "  כןמפעןלר בחכ  ";
	msg[89] = "  אףפיעןקכב ןףנב";
	msg[90] = "  תבניףר חןפןקמ.";
	msg[91] = "ןפכבת 9ם728(1)  ";
	msg[92] = "ןפכבת 9ם728(2)  ";
	msg[93] = "מופ יףט. נכ     ";
	msg[94] = "מופ יףט. תכ     ";
	msg[95] = "מופ יףט. פנכ    ";
	msg[96] = "מופ יףט גלננ 1  ";
	msg[97] = "מופ יףט. ג‏     ";
	msg[98] = "מופ נע ג‏ 1,2   ";
	msg[99] = "מופ יףט גלננ 2  ";
	msg[100] = "מופ הןמ. ה יףנעק";
	msg[101] = "מופ הןמ. י קכל. ";
	msg[102] = "מו ףמספש גלננ   ";
	msg[103] = "מופ ןפכעשפיס נכ.";
	msg[104] = "מופ ה. גינ קתקוה";
	msg[105] = "מופ ןפכעשפיס תכ.";
	msg[106] = "מופ ףטןהב 9ם728 ";
	msg[107] = "ןפכבת ןûיגכב כף ";
	msg[108] = "ק כןםבמהו C0C0  ";
	msg[109] = " יתה 9ם728      ";
	msg[110] = " יתה 9ם728 ץפ   ";
	msg[111] = " יתה 9ם723      ";
	msg[112] = " יתה 9ם723 ץפ   ";
	msg[113] = " יתה 9ם723-1    ";
	msg[114] = " יתה 9ם723-1 ץפ ";
	msg[115] = " מויתק יתהוליו  ";
	msg[116] = "ןפכבת 9ץ87ם ץ85ם";
	msg[117] = "כקנ.ה כןמפע נק  ";
	msg[118] = "כןמפע נק כמ1 כמ3";
	msg[119] = "כןמפע נק כמ2 כמ3";
	msg[120] = "כןמפע נק כמ1 כמ2";
	msg[121] = " ףןףפ. נועוהבמן ";
	msg[122] = "עוצים ח3-ח1     ";
	msg[123] = "עוצים עוחלבםומפ ";
	msg[124] = "עוצים ח3-ח2     ";
	msg[125] = "עוצים נח2       ";
	msg[126] = "עוצים נח3       ";
	msg[127] = "עוצים ח2-ח1     ";
	msg[128] = "עוצים ח1,ח2-ח3  ";
	msg[129] = "עוצים ח1-ח2     ";
	msg[130] = "עוצים כןמפעןלר  ";
	msg[131] = "נץףכ 1 נעןקוהומ ";
	msg[132] = "נץףכ 2 נעןקוהומ ";
	msg[133] = "  תû N1 תבגלןכ  ";
	msg[134] = "  תû N2 תבגלןכ  ";
	msg[135] = "ןפכבת זףק    (1)";
	msg[136] = "ןפכבת זףק    (2)";
	msg[137] = "עוצ. פן1 ץ55-נ78";
	msg[138] = "    ןפכבת 1פ141 ";
	msg[139] = "תû נעבק. תבגלןכ ";
	msg[140] = "תû לוק. תבגלןכ  ";
	msg[141] = "תû נעבק. עבתגלןכ";
	msg[142] = "תû לוק. עבתגלןכ ";
	msg[143] = "ןפכבת ףץחנ      ";
	msg[144] = "ףץחנ מו קכלא‏ומן";
	msg[145] = "בחעוח מו קשקוûומ";
	msg[146] = "לוק כעû מו ןפכע ";
	msg[147] = "נעבק כעû מו ןפכע";
	msg[148] = "לוק מבנ מו נןהמ ";
	msg[149] = "נעבק מבנ מו נןהמ";
	msg[150] = "בחעוח מו נן נטהמ";
	msg[151] = "תבכעשפר הקועי   ";
	msg[152] = "כקנ.כ נןהכל נק  ";
	msg[153] = "מופ מןעםש םכין  ";
	msg[154] = "עוצים ץנע. ףת   ";
	msg[155] = "נועוהב‏ב םבףףיקב";
	msg[156] = "םבףףיקש נועוהבמש";
	msg[157] = "ןûיגכב ץנעבקלומ ";
	msg[158] = "עבתגלןכ 1944 ל  ";
	msg[159] = "עבתגלןכ 1944 נ  ";
	msg[160] = "יףטןהמןו 1944 ל ";
	msg[161] = "יףטןהמןו 1944 נ ";
	msg[162] = "מןעםב עוצ ןכןמ  ";
	msg[163] = "בחכ מו תבקסתבמ  ";
	msg[164] = "נץ תבקוףב מו תגכ";
	msg[165] = "מופ יףט         ";
	msg[166] = "ןפגןך עוציםב    ";
	msg[167] = "עוצים כןמפע תû  ";
	msg[168] = "עוצים פן2 ü1944 ";
	msg[169] = "ןפכבת 1944      ";
	msg[170] = "קה 33           ";
	msg[171] = "מןעםב בü1944    ";

	if (strcmp(msg[MSGCOUNT - 1], "מןעםב בü1944    ")) {
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
