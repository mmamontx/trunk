#ifndef _EVENT_H
#define _EVENT_H 1

#include <time.h>

struct event {
	char *msg;
	unsigned char word;
	struct itimerspec it;
	struct event *next;
};

/*extern struct event *topev;

extern timer_t evtimer_id;*/

extern int event_add(char *, const unsigned char, const unsigned int);
extern int event_timer_delete();
extern int event_timer_init();
extern void event_timer_action();

#endif /* !_EVENT_H */