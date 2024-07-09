/*
	202407081.133041: dwrr
	a program to run on the raspberry pi 3B,
	to communicate with the riscv computer
	reliably using only a gpio single pin. 
	because the pcie controller is unreliable.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

static void export_pin(void) {
	system("echo 17 > /sys/class/gpio/export");
}

static void unexport_pin(void) {
	system("echo 17 > /sys/class/gpio/unexport");
}

static void set_data_1(void) {
	system("echo 1 > /sys/class/gpio/gpio17/value");
}

static void set_data_0(void) {
	system("echo 0 > /sys/class/gpio/gpio17/value");
}

static void direction_output(void) {
	system("echo out > /sys/class/gpio/gpio17/direction");
}

static void direction_input(void) {
	system("echo in > /sys/class/gpio/gpio17/direction");
}

static bool get_data(void) {
	FILE* p = popen("cat /sys/class/gpio/gpio17/value", "r");
	if (not p) { perror("get_data: popen"); exit(1); }
	char buffer[10] = {0};
	fgets(buffer, sizeof buffer, p);
	pclose(p);
	return not strcmp(buffer, "1");
}

static bool get_direction(void) {
	FILE* p = popen("cat /sys/class/gpio/gpio17/direction", "r");
	if (not p) { perror("get_data: popen"); exit(1); }
	char buffer[10] = {0};
	fgets(buffer, sizeof buffer, p);
	pclose(p);
	return not strcmp(buffer, "out");
}

static int gpio_pin_inaccessible(void) {
	return access("/sys/class/gpio/gpio17", F_OK);
}

static void sleep_1(void) {
	sleep(1);
}

// x is a unsigned byte value: 0 through 255.
static void transmit(int x) {
	direction_output();

	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1(); sleep_1(); sleep_1(); sleep_1();

	for (int _ = 0; _ < 3; _++) {
		for (int i = 0; i < 8; i++) {
			set_data_1();
			sleep_1();
			set_data_0();
			if ((x >> i) & 1) { sleep_1(); sleep_1(); sleep_1(); }
			sleep_1();
		}
	}

	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1();
	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1();

	direction_input();
}

int main(void) {
	export_pin();

	if (gpio_pin_inaccessible()) {
		puts("error: cannot access gpio 17, terminating...");
		exit(1);
	}

	puts("transmitting...");
	transmit(0xAC);

	unexport_pin();
	exit(0);
}


