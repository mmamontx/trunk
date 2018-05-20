/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "event.h"
#include "rage.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

timer_t evtimer_id;
struct event *topev = NULL;

int event_timer_delete()
{
	if (timer_delete(evtimer_id) == -1) {
		assert_perror(errno);
		return -1;
	}

	topev = NULL;

	return 0;
}

int event_timer_init()
{
	struct sigevent sigev;

	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGRTMIN + 3;

	if (signal(SIGRTMIN + 3, event_timer_action) == SIG_ERR) {
		assert_perror(errno);
		return -1;
	}

	if (timer_create(CLOCK_REALTIME, &sigev, &evtimer_id) == -1) {
		assert_perror(errno);
		return -1;
	}

	return 0;
}

void event_timer_action()
{
	struct event *ptr = topev;

	if (queue_push(topev->msg, topev->word, 16) == -1)
		return;

	if (topev->next != NULL) {
		topev = topev->next;

		/*if (topev->it.it_value.tv_sec != ptr->it.it_value.tv_sec) {*/
		if (topev->it.it_value.tv_sec) {
			interrupt = 2;

			free(ptr);

			ptr = topev;

			while (ptr->next != NULL) {
				topev = ptr;
				ptr = ptr->next;

				free(topev);
			}

			topev = NULL;
		} else {
			if (timer_settime(evtimer_id, 0, &topev->it, NULL) == -1) {
				free(ptr);
				assert_perror(errno);
				return;
			}
		}
	} else {
		interrupt = 2;

		topev = NULL;
	}

	free(ptr);
}

int event_add(char *msg, const unsigned char word, const unsigned int deadline)
{
	long int delta;
	struct itimerspec timeleft;
	struct event *ev, *ptr, *temp;

	if (!deadline) {
		TRACE(deadline);
		return 0;
	}

	ev = (struct event *)malloc(sizeof(struct event));
	if (ev == NULL) {
		fprintf(stderr, "malloc() == NULL\n");
		return -1;
	}

	ev->it.it_interval.tv_sec = 0;
	ev->it.it_interval.tv_nsec = 0;

	ev->msg = msg;
	ev->word = word;
	ev->next = NULL;

	if (topev != NULL) {
		ptr = topev;

		while (1) {
			if (deadline < ptr->it.it_value.tv_sec) {
				if (ptr != topev) {
					temp->next = ev;

					ev->it.it_value.tv_sec = temp->it.it_value.tv_sec - deadline;

					if (!ev->it.it_value.tv_sec) {
						ev->it.it_value.tv_nsec = temp->it.it_value.tv_nsec + 330000000;

						if (ev->it.it_value.tv_nsec > 999999999) {
							ev->it.it_value.tv_nsec -= 1000000000;
							ev->it.it_value.tv_sec += 1;
						}
					} else {
						ev->it.it_value.tv_nsec = 0;
					}

					ev->next = ptr;
				} else {
					if (timer_gettime(evtimer_id, &timeleft) == -1) {
						free(ev);
						assert_perror(errno);
						return -1;
					}

					delta = ptr->it.it_value.tv_sec - timeleft.it_value.tv_sec;

					if (delta > deadline) {
						free(ev);
					} else {
						ev->it.it_value.tv_sec = deadline - (unsigned int)delta;
						ev->it.it_value.tv_nsec = timeleft.it_value.tv_nsec;

						ptr->it.it_value.tv_sec -= deadline;

						if (timer_settime(evtimer_id, 0, &ev->it, NULL) == -1) {
							free(ev);
							assert_perror(errno);
							return -1;
						}

						ev->next = ptr;
						topev = ev;
					}
				}

				return 0;
			} else {
				if (ptr->next != NULL) {
					temp = ptr;
					ptr = ptr->next;
				} else {
					ptr->next = ev;

					ev->it.it_value.tv_sec = deadline;
					ev->it.it_value.tv_sec = ev->it.it_value.tv_sec - ptr->it.it_value.tv_sec;

					if (!(ev->it.it_value.tv_sec))
						ev->it.it_value.tv_nsec = ptr->it.it_value.tv_nsec + 330000000;
					else
						ev->it.it_value.tv_nsec = 0;

					return 0;
				}
			}
		}
	} else {
		ev->it.it_value.tv_sec = deadline;
		ev->it.it_value.tv_nsec = 0;

		if(timer_settime(evtimer_id, 0, &ev->it, NULL) == -1) {
			free(ev);
			assert_perror(errno);
			return -1;
		}

		topev = ev;
	}

	return 0;
}
