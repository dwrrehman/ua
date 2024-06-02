// a simple prime number program to test how you
// can profile c code using clang.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint64_t nat;

int main(void) {
	const nat n = 500000;
	for (nat i = 0; i < n; i++) {
		for (nat j = 2; j < i; j++) {
			if (i % j == 0) goto composite;
		}
		printf("%llu\n", i);
		composite:;
	}
	exit(0);
}


