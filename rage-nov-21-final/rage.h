#ifndef _RAGE_H
#define _RAGE_H 1

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define TRACE(var) fprintf(stderr, #var" = %d 0x%x\n", var, var)

#define TIMESN 5

struct reject {
	char *msg1;
	char *msg2;
	unsigned char deadline[22][3];
};

extern char citem;
extern char interrupt;
extern int timedelta;
extern pthread_t stpn_id;
extern pthread_t system_id;
extern struct reject rej[17];
extern time_t timestart;
/*extern timer_t modetimer_id;*/

extern char check_items();
extern int ftimes(const int, const char);
extern int job(const unsigned char);
extern int modestart(int);
extern int modetimer_adjust(const unsigned int);
extern int multiloader();
extern int setrej(const unsigned char, const unsigned char, unsigned char);
extern int the_doors();
extern void *server(void *);
extern void *sugp(void *);
extern void initrej();
extern void modetimer_action();

#endif /* !_RAGE_H */