#include <stdio.h>
#include <stdlib.h>

static inline void print(const char* message, int* bowls, int plate, int count) {
	printf("%s (i=%2d) [ ", message, plate);
	for (int i = 0; i < count; i++) {
		printf("%d ", bowls[i]);
	}
	printf("]\n");
}

int main(int argc, const char** argv) {
	if (argc != 4) return 1;

	const int m = atoi(argv[1]), n = atoi(argv[2]);     // pick two natrual numbers, n and m,

	int* a = calloc((size_t) n + 1, sizeof(int));	// n + 1 bowls
	int i = 0;                              // 1 plate.

	// instead of moving "a" along, im moving an 
	// iterator, i, along. mainly because c is weird.

	int t = atoi(argv[3]);

	for (int _ = 0; _ < t; _++) {
	
		print("-------------------------\nI: ", a, i, n + 1);

		if (a[i] < m) a[i]++; else a[i] = 0;
		// print("\t\tM: ", a, i, n + 1);

		if (i + a[i] < n) i += a[i] + 1; else i = 0;
		// print("\t\tN: ", a, i, n + 1);


	}

	printf("finished running the universe.\n");
}


