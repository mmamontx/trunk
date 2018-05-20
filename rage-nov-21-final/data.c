/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "data.h"
#include "rage.h"

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct clients cl[CL_NUMBER];
struct data *data_first;

struct A120 a120;
struct AA10 aa10;
struct AB10 ab10;
struct AB30 ab30;
struct AD20 ad20;
struct B112 b110;
struct B130 b130;
struct B310 b310;
struct BA10 ba10;
struct BB10 bb10;
struct BB30 bb30;
struct BC10 bc10;
struct BC40 bc40;
struct C110 c110;
struct CC10 cc10;
struct CD10 cd10;
struct CF00 cf00;
struct DA10 da10;
struct DB20 db20;
struct DB30 db30;
struct DC10 dc10;
struct E010 e010;
struct E030 e030;
struct E070 e070;
struct E110 e110;

void progress_bar()
{
	int i, j, count, n;

	i = 0;
	while (1) {
		sleep(1);

		system("clear");

		for (j = 0, count = 0; j < CL_NUMBER; j++)
			if (cl[j].s != -1)
				count++;

		if (count > i + 1)
			count = i + 1;

		printf("%3.0f%[", (float)count / CL_NUMBER * 100);

		if (count > 0)
			for (j = 0; j < floor((double)38 / CL_NUMBER * count); j++)
				printf("=");
		printf(">");

		n = 38 - j;
		for (j = 0; j < n; j++)
			printf(" ");

		printf("]\n");

		if (i < 4) {
			i++;
		} else {
			if (count == 5) {
				puts("Готовность имитатора изделия 9П78-1");
				return;
			}
		}
	}
}

int fclient(const char *ip)
{
	unsigned char i;

	for (i = 0; i < CL_NUMBER; i++)
		if (!strcmp(cl[i].ip, ip))
			return cl[i].s;

	return 0;
}

int cladd(const char *ip, const int s)
{
	unsigned char i = 0;

	while (cl[i].s != -1)
		i++;

	if (i > (CL_NUMBER - 1)) {
		TRACE(i);
		return -1;
	}

	cl[i].s = s;

	memcpy(cl[i].ip, ip, sizeof(char) * 16);

	return 0;
}

int clrem(const int s)
{
	unsigned char i = 0;

	while (cl[i].s != s)
		i++;

	if (i > (CL_NUMBER - 1)) {
		TRACE(i);
		return -1;
	}

	close(s);

	cl[i].s = -1;

	memset(cl[i].ip, '\0', sizeof(char) * 16);

	return 0;
}

char *ptime()
{
	char *s;
	time_t t;

	if (time(&t) == ((time_t) -1)) {
		assert_perror(errno);
		return NULL;
	}

	s = ctime(&t);
	if (s == NULL) {
		fprintf(stderr, "ctime() == NULL\n");
		return NULL;
	}

	s[19] = '\0';

	return s + 11;
}

int data_init()
{
	unsigned char i = 0;

	do {
		cl[i].s = -1;
		i++;
	} while (i < CL_NUMBER);

	if (data_register(0xa120, &a120, sizeof(struct A120)) == NULL)
		return -1;

	if (data_register(0xaa10, &aa10, sizeof(struct AA10)) == NULL)
		return -1;

	if (data_register(0xab10, &ab10, sizeof(struct AB10)) == NULL)
		return -1;

	if (data_register(0xab30, &ab30, sizeof(struct AB30)) == NULL)
		return -1;

	if (data_register(0xad20, &ad20, sizeof(struct AD20)) == NULL)
		return -1;

	if (data_register(0xb110, &b110, sizeof(struct B112)) == NULL)
		return -1;

	if (data_register(0xb130, &b130, sizeof(struct B130)) == NULL)
		return -1;

	if (data_register(0xb310, &b310, sizeof(struct B310)) == NULL)
		return -1;

	if (data_register(0xba10, &ba10, sizeof(struct BA10)) == NULL)
		return -1;

	if (data_register(0xbb10, &bb10, sizeof(struct BB10)) == NULL)
		return -1;

	if (data_register(0xbb30, &bb30, sizeof(struct BB30)) == NULL)
		return -1;

	if (data_register(0xbc10, &bc10, sizeof(struct BC10)) == NULL)
		return -1;

	if (data_register(0xbc40, &bc40, sizeof(struct BC40)) == NULL)
		return -1;

	if (data_register(0xc110, &c110, sizeof(struct C110)) == NULL)
		return -1;

	if (data_register(0xcc10, &cc10, sizeof(struct CC10)) == NULL)
		return -1;

	if (data_register(0xcd10, &cd10, sizeof(struct CD10)) == NULL)
		return -1;

	if (data_register(0xcf00, &cf00, sizeof(struct CF00)) == NULL)
		return -1;

	if (data_register(0xda10, &da10, sizeof(struct DA10)) == NULL)
		return -1;

	if (data_register(0xdb20, &db20, sizeof(struct DB20)) == NULL)
		return -1;

	if (data_register(0xdb30, &db30, sizeof(struct DB30)) == NULL)
		return -1;

	if (data_register(0xdc10, &dc10, sizeof(struct DC10)) == NULL)
		return -1;

	if (data_register(0xe010, &e010, sizeof(struct E010)) == NULL)
		return -1;

	if (data_register(0xe030, &e030, sizeof(struct E030)) == NULL)
		return -1;

	if (data_register(0xe070, &e070, sizeof(struct E070)) == NULL)
		return -1;

	if (data_register(0xe110, &e110, sizeof(struct E110)) == NULL)
		return -1;

	return 0;
}

struct data *data_register(const unsigned short head, void *addr, const unsigned int len)
{
	struct data *ptr, *tmp;
	pthread_mutex_t mtx_tmp = PTHREAD_MUTEX_INITIALIZER;

	ptr = (struct data *)malloc(sizeof(struct data));
	if (ptr == NULL) {
		fprintf(stderr, "malloc() == NULL\n");
		return NULL;
	}

	ptr->addr = addr;
	ptr->head = head;
	ptr->len = len;
	ptr->mtx = mtx_tmp;
	ptr->next = NULL;

	if (data_first != NULL) {
		tmp = data_first;

		while (tmp->next != NULL)
			tmp = tmp->next;

		tmp->next = ptr;
	} else {
		data_first = ptr;
	}

	return ptr;
}

int data_ands(void *dest, const unsigned short mask)
{
	int rv;
	struct data *ptr;

	if (data_first == NULL) {
		fprintf(stderr, "data_first == NULL\n");
		return -1;
	}

	ptr = data_first;

	while (dest != ptr->addr)
		if (ptr->next != NULL) {
			ptr = ptr->next;
		} else {
			fprintf(stderr, "ptr->next == NULL\n");
			return -1;
		}

	rv = pthread_mutex_lock(&(ptr->mtx));
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	*((unsigned short *)dest) &= mask;

	rv = pthread_mutex_unlock(&(ptr->mtx));
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	return 0;
}

int data_copy(void *dest, const void *src, const unsigned int len)
{
	int rv;
	struct data *ptr;
	unsigned char shift;

	if (dest == src)
		return 0;

	if (data_first == NULL) {
		fprintf(stderr, "data_first == NULL\n");
		return -1;
	}

	ptr = data_first;

	while (1) {
		for (shift = 0; shift < ptr->len / 2; shift++)
			if (((unsigned short *)ptr->addr + shift) == dest) {
				shift++;
				break;
			}

		shift--;

		if (((unsigned short *)ptr->addr + shift) == dest)
			break;

		if (ptr->next != NULL) {
			ptr = ptr->next;
		} else {
			fprintf(stderr, "ptr->next == NULL\n");
			return -1;
		}
	}

	if ((ptr->len - shift * 2) < len) {
		TRACE(len);
		return -1;
	}

	rv = pthread_mutex_lock(&(ptr->mtx));
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	memcpy(dest, src, len);

	rv = pthread_mutex_unlock(&(ptr->mtx));
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	return 0;
}

struct SIM_UserData *data_receive(const char *ip)
{
	char *s;
	unsigned char i;
	struct data *ptr;
	struct SIM_UserData *p;

	for (i = 0; i < CL_NUMBER; i++)
		if (!strcmp(ip, cl[i].ip))
			break;

	if (i > (CL_NUMBER - 1)) /*{
		TRACE(i);*/
		return NULL;
	/*}*/

	p = sim_recv(cl[i].s);
	if (!p)
		return NULL;

	s = ptime();

	if (s != NULL)
		printf("%s %s Принят  пакет  с  заголовком 0x%x.\n", s, ip, p->head);

	if (data_first == NULL) {
		fprintf(stderr, "data_first == NULL\n");
		return NULL;
	}

	ptr = data_first;

	while (ptr->head != p->head)
		if (ptr->next != NULL) /*{*/
			ptr = ptr->next;
		/*} else {
			fprintf(stderr, "ptr->next == NULL\n");
			return NULL;
		}*/
		else
			return p;

	if (ptr->len != p->sizeOfData) {
		fprintf(stderr, "ptr->len != p->sizeOfData\n");
		return NULL;
	}

	return p;
}

int data_send(const char *ip, const unsigned short head)
{
	int rv;
	char *s;
	struct data *ptr;
	struct SIM_UserData t;
	/*unsigned char i, j;*/
	unsigned char i;

	for (i = 0; i < CL_NUMBER; i++)
		if (!strcmp(ip, cl[i].ip))
			break;

	if (i > (CL_NUMBER - 1))
		return 1;

	if (data_first == NULL) {
		fprintf(stderr, "data_first == NULL\n");
		return -1;
	}

	ptr = data_first;

	while (ptr->head != head)
		if (ptr->next != NULL) {
			ptr = ptr->next;
		} else {
			t.head = head;
			t.pData = NULL;
			t.sizeOfData = 0;

			if (!strcmp(ip, CL_APK) || !strcmp(ip, CL_OEP) || !strcmp(ip, CL_SUGP))
				switch (solve(ip, head, cl[i].s, &t)) {
				case -1:				
					return 2;
				case 0:
					return 0;
				case 1:
					break;
				}

			if (sim_send(cl[i].s, &t) == -1)
				return -1;

			s = ptime();

			if (s != NULL)
				printf("%s %s Отправлен пакет с заголовком 0x%x.\n", s, ip, head);

			return 0;
		}

	rv = pthread_mutex_lock(&(ptr->mtx));
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	t.head = ptr->head;
	t.pData = ptr->addr;
	t.sizeOfData = ptr->len;

	/*if (ptr->head != 0xe010) {
		for (j = 0; j < t.sizeOfData; j++)
			printf("%x ", *((unsigned char *)t.pData + j));
		printf("\n");
	}*/

	if (!strcmp(ip, CL_APK) || !strcmp(ip, CL_OEP) || !strcmp(ip, CL_SUGP))
		switch (solve(ip, head, cl[i].s, &t)) {
		case -1:
			rv = pthread_mutex_unlock(&(ptr->mtx));
			if (rv) {
				assert_perror(rv);
				return -1;
			}

			return 2;
		case 0:
			rv = pthread_mutex_unlock(&(ptr->mtx));
			if (rv) {
				assert_perror(rv);
				return -1;
			}

			return 0;
		}

	if (sim_send(cl[i].s, &t) == -1) {
		rv = pthread_mutex_unlock(&(ptr->mtx));
		if (rv) {
			assert_perror(rv);
			return -1;
		}

		return -1;
	}

	rv = pthread_mutex_unlock(&(ptr->mtx));
	if (rv) {
		assert_perror(rv);
		return -1;
	}

	s = ptime();

	if (s != NULL)
		printf("%s %s Отправлен пакет с заголовком 0x%x.\n", s, ip, head);

	return 0;
}
