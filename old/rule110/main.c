#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iso646.h>
#include <string.h>


int main(int argc, const char** argv) {

	if (argc <= 3) exit(puts("usage: ./rule110 <cells> <timesteps> <delay>"));

	int S = atoi(argv[1]);
	int T = atoi(argv[2]);
	int delay = atoi(argv[3]);

	int* array = calloc((size_t) S, sizeof(int));
	int* copy = calloc((size_t) S, sizeof(int));
	array[S - 1] = 1;

	for (int t = 0; t < T; t++) {

		memcpy(copy, array, (size_t) S * sizeof(int));

		for (int s = 0; s < S; s++) {
			if (copy[s]) printf("██ "); else printf("-- ");
		}
		printf("\n");

		for (int s = 0; s < S; s++) {

			const int me = s;
			const int left = s == 0 ? S - 1 : s - 1 ;
			const int right = s == S - 1 ? 0 : s + 1; 

			if (copy[left] == 1 and copy[me] == 1 and copy[right] == 1) array[me] = 0;
			else if (copy[left] == 0 and copy[me] == 0 and copy[right] == 1) array[me] = 1;
			else if (copy[left] == 1 and copy[me] == 0 and copy[right] == 1) array[me] = 1;
		}

		if (delay) usleep((unsigned)delay);
		else getchar();
	}

	free(array); free(copy);
}
