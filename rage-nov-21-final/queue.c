/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "abus.h"
#include "io.h"
#include "mkio.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

struct queue *qroot = NULL;

pthread_mutex_t queue_mtx;

int queue_init()
{
	int rv;
	pthread_mutexattr_t attr;

	rv = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	rv = pthread_mutex_init(&queue_mtx, &attr);
	if (rv) {
		assert_perror(rv);
		return -1;
	}
}

int queue_fatal(const char *str)
{
	unsigned char i;

	if (!(strcmp(str, msg[47])))
		return 1;

	for (i = 93; i <= 106; i++)
		if (!(strcmp(str, msg[i])))
			return 1;

	return 0;
}

void *queue_daemon(void *arg)
{
	int rv;
	struct queue *qptr;

	while (1) {
		qptr = queue_pop();

		while (qptr != NULL) {
			if (qptr->len == 16) {
				rv = io_send_msg(qptr->msg, qptr->word);
				if (rv == -1)
					pthread_exit(&rv);

				rv = abus_send_msg(R1, queue_fatal(qptr->msg) ? 0xe1e1 : 0xe0e0, qptr->msg, 16);
				if (rv == -1)
					pthread_exit(&rv);
			} else {
				rv = io_send_buf(qptr->msg, qptr->word, qptr->len);
				if (rv == -1)
					pthread_exit(&rv);
			}

			free(qptr);

			qptr = queue_pop();
		}

		usleep(QUEUE_DELAY);
	}
}

int queue_push(char *str, const unsigned short word, const int len)
{
	int rv;
	struct queue *ptr, *tmp;

	rv = pthread_mutex_lock(&queue_mtx);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	tmp = (struct queue *)malloc(sizeof(struct queue));
	if (tmp == NULL) {
		fprintf(stderr, "malloc() == NULL\n");

		rv = pthread_mutex_unlock(&queue_mtx);
		if (rv) {
			assert_perror(rv);
			return -1;
		}

		return -1;
	}

	tmp->len = len;
	tmp->msg = str;
	tmp->word = word;
	tmp->next = NULL;

	if (qroot != NULL) {
		ptr = qroot;

		while (ptr->next != NULL)
			ptr = ptr->next;

		ptr->next = tmp;
	} else {
		qroot = tmp;
	}

	rv = pthread_mutex_unlock(&queue_mtx);
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	return 0;
}

struct queue *queue_pop()
{
	int rv;
	struct queue *ptr = NULL;

	rv = pthread_mutex_lock(&queue_mtx);
	if (rv) {
		assert_perror(rv);
		exit(-1);
	}

	if (qroot != NULL) {
		ptr = qroot;

		if (ptr->next != NULL)
			qroot = ptr->next;
		else
			qroot = NULL;
	}

	rv = pthread_mutex_unlock(&queue_mtx);
	if (rv) {
		assert_perror(rv);
		exit(-1);
	}

	return ptr;
}
