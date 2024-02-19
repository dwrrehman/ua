#include <time.h>
#include <stdio.h>

int main(void) {


	clock_t t = clock();

	printf("%lf\n", (double) t);

}