/*




	./run 0 10883911679

range markers:
	0
	1813985280
	3627970560
	5441955840
	7255941120
	9069926400



output:

range_generate: run these calls: 

	./run 0 1813985279

	./run 1813985280 3627970559

	./run 3627970560 5441955839

	./run 5441955840 7255941119

	./run 7255941120 9069926399

	./run 9069926400 10883911679



*/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef uint64_t nat;

int main(int argc, const char** argv) {
	if (argc != 3) return puts("usage: ./run total parallelization_factor"); 

	char* total_invalid = NULL, * parallelization_factor_invalid = NULL;

	const nat total  			= strtoull(argv[1], &total_invalid, 10);
	const nat parallelization_factor    	= strtoull(argv[2], &parallelization_factor_invalid, 10);

	if (*total_invalid) {
		printf("ERROR: error parsing total near \"%s\" aborting...\n", total_invalid);
		abort();
	}
	if (*parallelization_factor_invalid) {
		printf("ERROR: error parsing parallelization_factor near \"%s\" aborting...\n", parallelization_factor_invalid);
		abort();
	}

	printf("using: [total=%llu, parallelization_factor=%llu]\n", total, parallelization_factor);

	const nat width = total / parallelization_factor;
	printf("range_generate: run these calls: (%llu,%llu)\n\n", total, parallelization_factor);

	nat begin = 0;
	for (nat i = 0; i < parallelization_factor; i++) {
		printf("\t./run %llu %llu\n\n", begin, i < parallelization_factor - 1 ? begin + width - 1 : total - 1);
		begin += width;
	}
}













