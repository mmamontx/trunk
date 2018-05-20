/*#define NDEBUG*/
#define _GNU_SOURCE
#include <assert.h>

#include "io.h"
#include "list.h"
#include "mkio.h"
#include "rage.h"

#include <memory.h>
#include <stdlib.h>

struct list_item *level, *root;

void *list(void *arg)
{
	int rv;
	char state = 0;
	unsigned char cursor = 0, i;
	unsigned short buf;

	if (*((int *)arg)) {
		rv = modestart(*((int *)arg));

		pthread_exit(&rv);
	}

	root = list_init();
	if (root == NULL) {
		rv = -1;
		pthread_exit(&rv);
	}

	root->selected = root->item[0];

	rv = list_print(root, cursor, 1);
	if (rv == -1)
		pthread_exit(&rv);

	level = root;

	while (1) {
		buf = io_wait_push();

		switch (buf) {
		case CAI_DOWN:
			if (cursor < level->size - 1)
				cursor++;

			level->selected = level->item[cursor];

			rv = list_print(level, cursor, 0);
			if (rv == -1)
				pthread_exit(&rv);

			break;
		case CAI_UP:
			if (cursor > 0)
				cursor--;

			level->selected = level->item[cursor];

			rv = list_print(level, cursor, 0);
			if (rv == -1)
				pthread_exit(&rv);

			break;
		case CAI_LEFT:
			if (level != root) {
				level = level->parent;

				for (i = 0; i < level->size; i++)
					if (level->item[i] == level->selected)
						cursor = i;

				rv = list_print(level, cursor, 1);
				if (rv == -1)
					pthread_exit(&rv);
			}

			break;
		case CAI_ENTER:
			cursor = 0;
			level = level->selected;
			level->selected = level->item[cursor];

			if (level->mode != 0) {
				if ((level->msg == msg[23]) || (level->msg == msg[27]))
					citem = 2;
				if ((level->msg == msg[24]) || (level->msg == msg[26]))
					citem = 1;
				if ((level->msg == msg[25]) || (level->msg == msg[28]))
					citem = 0;

				rv = modestart(level->mode);

				pthread_exit(&rv);
			} else {
				rv = list_print(level, cursor, 1);
				if (rv == -1)
					pthread_exit(&rv);
			}

			break;
		case CAI_END_MODE:
			if (!state) {
				rv = io_send_msg(msg[32], 8);
				if (rv == -1)
					pthread_exit(&rv);

				rv = io_send_msg(msg[33], 40);
				if (rv == -1)
					pthread_exit(&rv);

				state = 1;
			}

			break;
		case CAI_RESET:
			if (state) {
				rv = io_send_msg(msg[37], 16);
				if (rv == -1)
					pthread_exit(&rv);

				rv = io_send_msg(msg[34], 40);
				if (rv == -1)
					pthread_exit(&rv);

				buf = 0x380;

				rv = mkio_send(BKD, R1, 1, &buf);
				if (rv == -1)
					pthread_exit(&rv);

				delay(1000, 50);

				do {
					rv = mkio_recv(CAI, REG_ST, 1, &buf);
					if (rv == -1)
						pthread_exit(&rv);

					rv = pthread_yield();
					if (rv) {
						assert_perror(rv);
						pthread_exit(&rv);
					}
				} while (buf != 0xffff);

				pthread_exit(&rv);
			}

			break;
		case ((unsigned short) -1):
			rv = -1;
			pthread_exit(&rv);
		}

		usleep(10000);
	}

	return NULL;
}

struct list_item *list_init()
{
	unsigned char i, j, k;
	struct list_item *level;

	level = (struct list_item *)malloc(sizeof(struct list_item));
	if (level == NULL)
		return NULL;

	level->msg = msg[0];
	level->size = 4;
	level->parent = NULL;
	level->item[4] = NULL;
	level->item[5] = NULL;

	for (i = 0; i < level->size; i++) {
		level->item[i] = (struct list_item *)malloc(sizeof(struct list_item));
		if (level == NULL)
			return NULL;

		level->item[i]->mode = 0;
		level->item[i]->msg = msg[i + 1];
	}

	level->item[0]->size = 4;
	level->item[0]->parent = level;
	level->selected = level->item[0];

	for (i = 0; i < level->item[0]->size; i++) {
		level->item[0]->item[i] = (struct list_item *)malloc(sizeof(struct list_item));
		if (level->item[0]->item[i] == NULL)
			return NULL;

		level->item[0]->item[i]->msg = msg[i + 5];
	}

	level->item[1]->size = 5;
	level->item[1]->parent = level;
	level->item[1]->item[5] = NULL;

	for (i = 0; i < level->item[1]->size; i++) {
		level->item[1]->item[i] = (struct list_item *)malloc(sizeof(struct list_item));
		if (level->item[1]->item[i] == NULL)
			return NULL;

		level->item[1]->item[i]->msg = msg[i + 11];
	}

	level->item[2]->size = 5;
	level->item[2]->parent = level;
	level->item[2]->item[5] = NULL;

	for (i = 0; i < level->item[2]->size; i++) {
		level->item[2]->item[i] = (struct list_item *)malloc(sizeof(struct list_item));
		if (level->item[2]->item[i] == NULL)
			return NULL;

		level->item[2]->item[i]->msg = msg[i + 16];
	}

	level->item[3]->size = 2;
	level->item[3]->parent = level;
	level->item[3]->item[2] = NULL;
	level->item[3]->item[3] = NULL;
	level->item[3]->item[4] = NULL;
	level->item[3]->item[5] = NULL;

	for (i = 0; i < level->item[3]->size; i++) {
		level->item[3]->item[i] = (struct list_item *)malloc(sizeof(struct list_item));
		if (level->item[3]->item[i] == NULL)
			return NULL;

		level->item[3]->item[i]->msg = msg[i + 21];
	}

	for (i = 0; i < level->size; i++)
		for (j = 0; j < level->item[i]->size; j++) {
			level->item[i]->item[j]->mode = 0;
			level->item[i]->item[j]->size = 3;
			level->item[i]->item[j]->parent = level->item[i];

			for (k = 0; k < 3; k++) {
				level->item[i]->item[j]->item[k] = (struct list_item *)malloc(sizeof(struct list_item));
				if (level->item[i]->item[j]->item[k] == NULL)
					return NULL;

				if (((i == 2) && (j == 0)) || ((i == 2) && (j == 3)) || ((i == 3) && (j == 1)))
					level->item[i]->item[j]->item[k]->msg=msg[k + 26];
				else
					level->item[i]->item[j]->item[k]->msg=msg[k + 23];
			}

			level->item[i]->item[j]->item[3] = NULL;
			level->item[i]->item[j]->item[4] = NULL;
			level->item[i]->item[j]->item[5] = NULL;
		}

	for (i = 0; i < 3; i++) {
		level->item[0]->item[0]->item[i]->mode = 14;
		level->item[0]->item[1]->item[i]->mode = 13;
		level->item[0]->item[2]->item[i]->mode = 15;
		level->item[0]->item[3]->item[i]->mode = 2;

		level->item[1]->item[0]->item[i]->mode = 3;
		level->item[1]->item[1]->item[i]->mode = 12;
		level->item[1]->item[2]->item[i]->mode = 18;
		level->item[1]->item[3]->item[i]->mode = 17;
		level->item[1]->item[4]->item[i]->mode = 16;

		level->item[2]->item[0]->item[i]->mode = 1;
		level->item[2]->item[1]->item[i]->mode = 21;
		level->item[2]->item[2]->item[i]->mode = 254;
		level->item[2]->item[3]->item[i]->mode = 255;
		level->item[2]->item[4]->item[i]->mode = 4;
	}

	return level;
}

char *list_frame(const struct list_item *top, const unsigned char part, const unsigned char cursor, const char reset_shift)
{
	unsigned char i, j;
	static unsigned char shift;
	char *buf;

	if (reset_shift)
		shift = 0;

	switch (cursor) {
	case 0:
		if (shift == 1)
			shift = 0;
		break;
	case 1:
		if (shift == 2)
			shift = 1;
		break;
	case 4:
		if (shift == 0)
			shift = 1;
		break;
	case 5:
		if (shift == 1)
			shift = 2;
	}

	switch (part) {
	case 1:
		buf = (char *)malloc(32);
		if (buf == NULL)
			return NULL;

		if (top == root)
			memcpy(buf, root->msg, 16);
		else if ((top == root->item[0]) || (top == root->item[1]) || (top == root->item[2]) || (top == root->item[3]))
			memcpy(buf, msg[29], 16);
		else
			memcpy(buf, msg[30], 16);

		memcpy(buf + 16, top->item[0 + shift]->msg, 16);

		if (top->item[0 + shift] == top->selected)
			buf[16] = '*';

		break;
	case 2:
		buf = (char *)malloc(48);
		if (buf == NULL)
			return NULL;

		for (i = 1; i < 4; i++) {
			if (top->item[i] != NULL)
				memcpy(buf + 16 * (i - 1), top->item[i + shift]->msg, 16);
			else
				for (j = 0; j < 16; j++)
					buf[16 * (i - 1) + j] = ' ';

			if (top->item[i + shift] == top->selected)
				buf[16 * (i - 1)] = '*';
		}

		break;
	default:
		TRACE(part);
		return NULL;
	}

	return buf;
}

int list_print(const struct list_item *top, const unsigned char cursor, const char reset_shift)
{
	char *buf;

	buf = list_frame(top, 1, cursor, reset_shift);
	if (buf == NULL)
		return -1;

	if (io_send_buf(buf, 8, 32) == -1) {
		free(buf);
		return -1;
	}

	free(buf);

	buf = list_frame(top, 2, cursor, reset_shift);
	if (buf == NULL)
		return -1;

	if (io_send_buf(buf, 24, 48) == -1) {
		free(buf);
		return -1;
	}

	free(buf);

	return 0;
}

void list_free()
{
	unsigned char i, j, k;

	for (i = 0; i < root->size; i++) {
		for (j = 0; j < root->item[i]->size; j++) {
			for (k = 0; k < 3; k++)
				free(root->item[i]->item[j]->item[k]);

			free(root->item[i]->item[j]);
		}

		free(root->item[i]);
	}

	free(root);
}
