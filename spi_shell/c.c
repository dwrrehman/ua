// this program will run on the riscv computer, as the main operating shell of the device.

// a program to emulate a shell, over a spi connection. 
// used for allowing me to interface with the risc-v cpu,
// using a raspberry pi connected to it over a spi connection. 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

/*

to control the gpio pin 25, our only pin we have, we use this sequence of commands:


	sudo -i        //  done upfront, when we execute the shell 
			from .rclocal, i think. idk. something like that. we must be root to run this. 

	echo 505 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio505/direction
	echo 1 > /sys/class/gpio/gpio505/value
	echo 0 > /sys/class/gpio/gpio505/value
	
	echo in > /sys/class/gpio/gpio505/direction
	cat /sys/class/gpio/gpio505/value

	echo 505 > /sys/class/gpio/unexport

*/

static void export_pin(void) {
	system("echo 505 > /sys/class/gpio/export");
}

static void unexport_pin(void) {
	system("echo 505 > /sys/class/gpio/unexport");
}

static void set_data_1(void) {
	system("echo 1 > /sys/class/gpio/gpio505/value");
}

static void set_data_0(void) {
	system("echo 0 > /sys/class/gpio/gpio505/value");
}

static void direction_output(void) {
	system("echo out > /sys/class/gpio/gpio505/direction");
}

static void direction_input(void) {
	system("echo in > /sys/class/gpio/gpio505/direction");
}

static bool get_data(void) {
	FILE* p = popen("cat /sys/class/gpio/gpio505/value", "r");
	if (not p) { perror("get_data: popen"); exit(1); }
	char buffer[10] = {0};
	fgets(buffer, sizeof buffer, p);
	pclose(p);
	return not strcmp(buffer, "1");
}

static bool get_direction(void) {
	FILE* p = popen("cat /sys/class/gpio/gpio505/direction", "r");
	if (not p) { perror("get_data: popen"); exit(1); }
	char buffer[10] = {0};
	fgets(buffer, sizeof buffer, p);
	pclose(p);
	return not strcmp(buffer, "out");
}

static int gpio_pin_inaccessible(void) {
	return access("/sys/class/gpio/gpio505", F_OK);
}









int main(void) {
	export_pin();

	if (gpio_pin_inaccessible()) {
		printf("error: program does not have access to gpio505 (gpio pin 25)\n");
		exit(1);
	}

	direction_output();

	for (int i = 0; i < 8; i++) {
		set_data_1();
		sleep(1);
		set_data_0();
		sleep(1);

	}

	unexport_pin();
}


















