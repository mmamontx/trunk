#ifndef _QUEUE_H
#define _QUEUE_H 1

#include <pthread.h>

#define QUEUE_DELAY 50000

struct queue {
	int len;
	unsigned int word;
	char *msg;
	struct queue *next;
};

/*extern struct queue *qroot;

extern pthread_mutex_t queue_mtx;*/

extern int queue_fatal(const char *);
extern int queue_push(char *, const unsigned short, const int);
extern struct queue *queue_pop();
extern void *queue_daemon(void *);

#endif /* !_QUEUE_H */