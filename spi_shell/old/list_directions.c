#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>

static char* get_direction(int gpio) {
	
}

int main(void) {
	int base[3] = {416, 448, 480}; 
	for (int bank = 0; bank < 3; bank++) {
		for (int gpio = 26; gpio < 32; gpio++) {
			const int number = base[bank] + gpio;
			printf("checking direction for gpio %u : %s\n", 
				number, get_direction);
		}
	}
}







