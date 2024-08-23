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
#include <time.h>
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
	return not strcmp(buffer, "1\n");
}

static bool get_direction(void) {
	FILE* p = popen("cat /sys/class/gpio/gpio17/direction", "r");
	if (not p) { perror("get_data: popen"); exit(1); }
	char buffer[10] = {0};
	fgets(buffer, sizeof buffer, p);
	pclose(p);
	return not strcmp(buffer, "out\n");
}

static int gpio_pin_inaccessible(void) {
	return access("/sys/class/gpio/gpio17", F_OK);
}

static void sleep_1(void) {
	nanosleep((const struct timespec[]){{1, 0}}, NULL);
}

static void sleep_third(void) {
	nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
}



static void transmit_byte(int x) {

	for (int i = 0; i < 8; i++) {
		set_data_1();
		sleep_1();
		set_data_0();
		if ((x >> i) & 1) { sleep_1(); sleep_1(); sleep_1(); }
		sleep_1();
	}

	set_data_1();
	sleep_1();
	set_data_0();
	for (int i = 0; i < 10; i++) sleep_1();
}


static void transmit_packet(int address, int data) {

	sleep_1(); sleep_1(); sleep_1();
	direction_output();
	sleep_1(); sleep_1(); sleep_1();

	transmit_byte(0x00);
	transmit_byte(0x00);
	transmit_byte(0x00);

	transmit_byte(address);
	transmit_byte(address);
	transmit_byte(address);

	transmit_byte(data);
	transmit_byte(data);
	transmit_byte(data);

	transmit_byte(0xFF);
	transmit_byte(0xFF);
	transmit_byte(0xFF);

	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1();
	direction_input();
	sleep_1(); sleep_1(); sleep_1(); sleep_1();  // is this neccessary?..
}

// returns a byte.
static void receive_packet(int* address, int* data) {
	int counts[128] = {0};
	int bits[128] = {0};
	int bit_count = 0;
	int last_state = 0;

	direction_input();
	sleep_1();

	while (bit_count < 107) {

		int bit = get_data();
		// if (bit) puts("1"); else puts("0");
		
		if (not last_state and not bit) counts[bit_count]++;
		if (not last_state and     bit) { }
		if (    last_state and not bit) { printf("%u ", bit_count); fflush(stdout); bit_count++; }
		if (    last_state and     bit) { }

		sleep_third();

		last_state = bit;
	}

	printf("printing counts: bit_count = %u\n", bit_count);
	for (int i = 0; i < bit_count; i++) {
		printf("    counts[%u] = %10u\n", i, counts[i]);
	}
	puts("[counts done]");

/*
	const int threshold = counts[26] + ((counts[1] - counts[26]) / 2);
	printf("threshold = %llu\n", threshold);

	int real_bit_count = 0;

	for (int i = 0; i < bit_count; i++) {
		if (i == 0) continue;
		if (i == 1) continue;
		if (i == 26) continue;
		bits[real_bit_count++] = counts[i] >= threshold ? 1 : 0;
	}

	printf("printing bits: real_bit_count = %llu\n", real_bit_count);
	for (int i = 0; i < real_bit_count; i++) {
		printf("    bits[%u] = %10u\n", i, bits[i]);
	}
	puts("[bits done]");
	
	int data = 0;
	for (int i = 0; i < 8; i++) {
		const int a = bits[i + 0];
		const int b = bits[i + 8];
		const int c = bits[i + 16];
		
		int x = 0;
		if (a == b) x = a; 
		else if (a == c) x = a; 
		else if (b == c) x = b; 
		
		data |= x << i;
	}
	return data;

*/

	*address = 0xF0;
	*data = 0xF0;
}

 





int main(void) {
	export_pin();

	if (gpio_pin_inaccessible()) {
		puts("error: cannot access gpio 17, terminating...");
		exit(1);
	}


	for (int _ = 0; _ < 2; _++) {

	sleep_1(); sleep_1(); sleep_1();

	transmit_packet(0x61, 0xF0);

	int data_bytes[4096] = {0};
	int data_count = 0;
	
	for (int i = 0; i < 1; i++) {       // while (1)  eventually, and break when control transfer address in packet happens. 

		puts("receiving packet...");

		int address = 0, data = 0;
		receive_packet(&address, &data);
		data_bytes[data_count++] = data;
		printf("\t\treceived: address = 0x%02X , data = 0x%02X\n",address, data);
		
		if (address == 0xCA) {
			puts("control transfer successful, breaking...");
			break;
		}
	}

	printf("received: (%u) {", data_count);
	for (int d = 0; d < data_count; d++) printf("0x%02X, ", data_bytes[d]);
	puts("}");

	}

	direction_input();
	unexport_pin();
}





















// x is a unsigned byte value: 0 through 255.
/*static void transmit(int x) {
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
*/
