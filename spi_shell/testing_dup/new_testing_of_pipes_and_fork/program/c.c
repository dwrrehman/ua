#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {
	write(1, "hello there!\n", 13);

	char buffer[10] = {0};
	read(0, buffer, 4);

	printf("received: %s\n", buffer);
	fflush(stdout);

	exit(42);
}

