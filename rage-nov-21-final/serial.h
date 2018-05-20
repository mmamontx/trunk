#ifndef _SERIAL_H
#define _SERIAL_H 1

#define BUFSIZE 65536

#define MINICOM_CFG 0x18b2

struct t {
	char flag;
	char id;
	unsigned char index;
	char reserved;
	char subcode1;
	char subcode2;
	char params;
};

struct a {
	char flag;
	char id;
	unsigned char index;
	char reserved;
	char subcode1;
	char subcode2;
	char cmd;
	char zero;
	unsigned short csum __attribute__ ((packed));
	char eflag;
	char end;
};

struct _c {
	double         time   __attribute__ ((packed));
	short          sysc   __attribute__ ((packed));
	unsigned short pr     __attribute__ ((packed));
	double         x      __attribute__ ((packed));
	double         y      __attribute__ ((packed));
	double         z      __attribute__ ((packed));
	double         o_time __attribute__ ((packed));
	short          o_sysc __attribute__ ((packed));
	unsigned short o_pr   __attribute__ ((packed));
	double         o_x    __attribute__ ((packed));
	double         o_y    __attribute__ ((packed));
	double         o_z    __attribute__ ((packed));
};

extern char stflag;
extern char flood;
/*extern unsigned char sindex;*/

extern int serial_init(const int);
extern int serial_test(const int);
extern unsigned short serial_csum(unsigned short *, const unsigned int);
extern void *serial(void *);
extern void *stpn_poll(void *);

#endif /* !_SERIAL_H */
