#ifndef _DATA_H
#define _DATA_H 1

#include "source/structs.h"

#define CL_NUMBER 5

#define CL_KSAU "10.130.59.1"
#define CL_RMI  "10.130.59.2"
#define CL_OEP  "10.130.59.5"
#define CL_SUGP "10.130.59.6"
#define CL_APK  "10.130.59.7"

struct clients {
	int s;
	char ip[16];
};

struct data {
	unsigned short head;
	void *addr;
	unsigned int len;
	pthread_mutex_t mtx;
	struct data *next;
};

/*extern struct clients cl[CL_NUMBER];
extern struct data *data_first;*/

extern struct A120 a120;
extern struct AA10 aa10;
extern struct AB10 ab10;
extern struct AB30 ab30;
extern struct AD20 ad20;
extern struct B112 b110;
extern struct B130 b130;
extern struct B310 b310;
extern struct BA10 ba10;
extern struct BB10 bb10;
extern struct BB30 bb30;
extern struct BC10 bc10;
extern struct BC40 bc40;
extern struct C110 c110;
extern struct CC10 cc10;
extern struct CD10 cd10;
extern struct CF00 cf00;
extern struct DA10 da10;
extern struct DB20 db20;
extern struct DB30 db30;
extern struct DC10 dc10;
extern struct E010 e010;
extern struct E030 e030;
extern struct E070 e070;
extern struct E110 e110;

extern char *ptime();
extern int cladd(const char *ip, const int s);
extern int clrem(const int s);
extern int data_ands(void *, const unsigned short);
extern int data_copy(void *, const void *, const unsigned int);
extern int data_init();
extern int data_send(const char *, const unsigned short);
extern int fclient(const char *ip);
extern struct data *data_register(const unsigned short, void *, const unsigned int);
extern struct SIM_UserData *data_receive(const char *);
extern void progress_bar();

#endif /* !_DATA_H */