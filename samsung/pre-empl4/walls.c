#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int knapsack(int W, int *unused, int n)
{
	int i, j;
	int value[W + 1][n + 1];
	//printf("knapsack()\n");
	//printf("W = %d n = %d\n", W, n);
	memset(value, 0, sizeof(value));
	int w[n + 1];
	w[0] = 0;
	memcpy(&(w[1]),	unused, sizeof(int) * n);

	for (i = 1; i < n + 1; i++) {
		for (j = 1; j < W + 1; j++) {
			value[j][i] = value[j][i - 1];
			if (w[i] <= j) {
				int val = value[j - w[i]][i - 1] + w[i];
				if (value[j][i] < val)
					value[j][i] = val;
			}
		}
	}

	/*for (i = 0; i < n + 1; i++) {
		for (j = 0; j < W + 1; j++)
			printf("%d ", value[j][i]);
		printf("\n");
	}*/

	return value[W][n];
}

int comp(const void *a, const void *b)
{
	return *((int *)b) - *((int *)a);
}

void solve()
{
	int n, i;

	scanf("%d", &n);
	int sticks[n];

	int maxlen = 0;
	//printf("The sticks are:");
	for (i = 0; i < n; i++) {
		scanf("%d", &(sticks[i]));
		//printf(" %d", sticks[i]);
		maxlen += sticks[i];
	}
	//printf("\n");
	maxlen /= 2;

	qsort(sticks, n, sizeof(int), comp);

	/*printf("The sorted sticks are:");
	for (i = 0; i < n; i++)
		printf(" %d", sticks[i]);
	printf("\n");*/

	int h1 = 0, h2 = 0;
	int w1[n], w2[n];
	int unused[n];
	int size1 = 0, size2 = 0, usize = 0;
	for (i = 0; i < n; i++) {
		if (h1 <= h2) {
			if (h1 + sticks[i] < maxlen) {
				h1 += sticks[i];
				w1[size1] = sticks[i];
				size1++;
			} else {
				unused[usize] = sticks[i];
				usize++;
				continue;
			}
		} else {
			if (h2 + sticks[i] < maxlen) {
				h2 += sticks[i];
				w2[size2] = sticks[i];
				size2++;
			} else {
				unused[usize] = sticks[i];
				usize++;
				continue;
			}
		}
	}

	/*printf("The first wall:");
	for (i = 0; i < size1; i++)
		printf(" %d", w1[i]);
	printf("\n");

	printf("The first wall:");
	for (i = 0; i < size2; i++)
		printf(" %d", w2[i]);
	printf("\n");

	printf("The unused sticks:");
	for (i = 0; i < usize; i++)
		printf(" %d", unused[i]);
	printf("\n");

	printf("The heights are %d and %d.\n", h1, h2);*/

	if (h1 == h2) {
		printf("%d\n", h1);
	} else while (h1 != h2) {
		if (h1 > h2) {
			unused[usize] = w1[size1 - 1];
			h1 -= w1[size1 - 1];
			usize++;
			size1--;

			//printf("h1 = %d h2 = %d usize = %d\n", h1, h2, usize);
			int dt = h2 > h1 ? h2 - h1 : h1 - h2;
			int rv = knapsack(dt, unused, usize);
			//printf("knapsack() returned %d\n", rv);
			if (rv == dt) {
				//printf("Achtung!\n");
				if (h1 > h2)
					h2 += rv;
				else
					h1 += rv;
			}
		} else {
			unused[usize] = w2[size2 - 1];
			h2 -= w2[size2 - 1];
			usize++;
			size2--;

			//printf("h1 = %d h2 = %d usize = %d\n", h1, h2, usize);
			int dt = h2 > h1 ? h2 - h1 : h1 - h2;
			int rv = knapsack(dt, unused, usize);
			//printf("knapsack() returned %d\n", rv);
			if (rv == dt) {
				//printf("Achtung!\n");
				if (h1 > h2)
					h2 += rv;
				else
					h1 += rv;
			}
		}
	}
	printf("%d\n", h1);
}

int main()
{
	int ntests, i;

	scanf("%d", &ntests);

	for (i = 0; i < ntests; i++) {
		printf("#%d ", i + 1);
		solve();
	}

	return 0;
}

