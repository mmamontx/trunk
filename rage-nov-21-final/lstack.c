#include "lstack.h"
#include "rage.h"

#include <stdlib.h>

unsigned char lsize = 0;
unsigned char *lhead = NULL;

void lstack_destroy()
{
	if (lhead != NULL) {
		free(lhead);

		lsize = 0;
	}
}

unsigned char lstack_pop()
{
	unsigned char temp = lhead[lsize - 1];

	if (lsize - 1 > 0) {
		lhead = realloc(lhead, --lsize);
		if (lhead == NULL) {
			fprintf(stderr, "realloc() == NULL\n");
			exit(-1);
		}
	} else {
		free(lhead);

		lhead = NULL;
		lsize = 0;
	}

	return temp;
}

int lstack_release()
{
	unsigned char temp1, temp2, temp3;

	if (lhead != NULL)
		while (lsize > 0) {
			temp3 = lstack_pop();
			temp2 = lstack_pop();
			temp1 = lstack_pop();

			if (setrej(temp1, temp2, temp3) == -1)
				return -1;

			lsize -= 3;
		}

	return 0;
}

int lstack_push(const unsigned char byte)
{
	lhead = realloc(lhead, ++lsize);
	if (lhead == NULL) {
		fprintf(stderr, "realloc() == NULL\n");
		return -1;
	}

	lhead[lsize - 1] = byte;

	return 0;
}