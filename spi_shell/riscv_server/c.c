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
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>




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
	return not strcmp(buffer, "1\n");
}

static bool get_direction(void) {
	FILE* p = popen("cat /sys/class/gpio/gpio505/direction", "r");
	if (not p) { perror("get_data: popen"); exit(1); }
	char buffer[10] = {0};
	fgets(buffer, sizeof buffer, p);
	pclose(p);
	return not strcmp(buffer, "out\n");
}

static int gpio_pin_inaccessible(void) {
	return access("/sys/class/gpio/gpio505", F_OK);
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
	sleep_1(); sleep_1(); sleep_1(); sleep_1();
}


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
		printf("error: program does not have access to gpio505 (gpio pin 25)\n");
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


























/*





	for (int i = 0; i < 5; i++) {
		int data_bytes[4096] = {0};
		int data_count = 0;
	
		while (1) {
			puts("receiving pair of bytes..");

			const int control = receive();
			const int data = receive();

			data_bytes[data_count++] = data;
	
			printf("\n\n\t\treceived: data = 0x%02X, control = 0x%02X\n\n", data, control);
		
			if (control == 0xCA) {
				puts("control transfer successful, breaking...");
				break;
			}
		}
		
		printf("received: (%u) {", data_count);
		for (int d = 0; d < data_count; d++) printf("0x%02X, ", data_bytes[d]);
		puts("}");

		sleep_1(); sleep_1(); sleep_1();

		transmit(0x00); transmit(0xDE);
		transmit(0x00); transmit(0xAD);
		transmit(0x00); transmit(0xFA);
		transmit(0xCA); transmit(0xCE);
	}

	direction_input();
	unexport_pin();





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


static int receive(void) {
	int counts[128] = {0};
	int bits[128] = {0};
	int bit_count = 0;
	int last_state = 0;

	direction_input();
	sleep(1);

	while (bit_count < 27) {

		int bit = get_data();
		// if (bit) puts("1"); else puts("0");
		
		if (not last_state and not bit) counts[bit_count]++;
		if (not last_state and     bit) { }
		if (    last_state and not bit) { printf("%u ", bit_count); fflush(stdout); bit_count++; }
		if (    last_state and     bit) { }

		nanosleep((const struct timespec[]){{0, 300000000L}}, NULL);

		last_state = bit;
	}

	printf("printing counts: bit_count = %llu\n", bit_count);
	for (int i = 0; i < bit_count; i++) {
		printf("    counts[%u] = %10u\n", i, counts[i]);
	}
	puts("[counts done]");

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
}
*/






/*	direction_output();

	for (int i = 0; i < 8; i++) {
		set_data_1();
		sleep(1);
		set_data_0();
		sleep(1);

	}



this program produced this output:


1
...
0
1
1
1
0
printing counts: bit_count = %llu
    counts[0] =          0
    counts[1] =         12
    counts[2] =          2
    counts[3] =          2
    counts[4] =         12
    counts[5] =         12
    counts[6] =          2
    counts[7] =         12
    counts[8] =          2
    counts[9] =         12
    counts[10] =          2
    counts[11] =          2
    counts[12] =         12
    counts[13] =         12
    counts[14] =          2
    counts[15] =         12
    counts[16] =          2
    counts[17] =         12
    counts[18] =          2
    counts[19] =          3
    counts[20] =         12
    counts[21] =         12
    counts[22] =          2
    counts[23] =         12
    counts[24] =          3
    counts[25] =         12
[done]




now this output:




printing counts: bit_count = %llu
    counts[0] =          0
    counts[1] =         12
    counts[2] =          2
    counts[3] =          3
    counts[4] =         12
    counts[5] =         12
    counts[6] =          2
    counts[7] =         12
    counts[8] =          3
    counts[9] =         12
    counts[10] =          3
    counts[11] =          2
    counts[12] =         12
    counts[13] =         12
    counts[14] =          2
    counts[15] =         13
    counts[16] =          2
    counts[17] =         12
    counts[18] =          2
    counts[19] =          2
    counts[20] =         12
    counts[21] =         12
    counts[22] =          3
    counts[23] =         12
    counts[24] =          2
    counts[25] =         12
    counts[26] =          2
[done]


27 bits,   the first two are actually not important, always just   0 1, basically, 


	then the last bit is always a 0 bit, i think?

		theres supposed to be two though.. hm... idk. 


				hmm ill see if we can increase the bit count to 28 and see if it still terminates now 


		hm


1202407081.184739

nopee turns out 27's the max lol. it hangs if i do 28,  which means its expecting another falling edge lol. so yeah, i think 27 is good, 

	the 0th bit is always 0,   the first bit is a done,  and the last bit is always a zero.  thats just it will work, i think, 


	cool beans


*/


/*


current output:


printing counts: bit_count = %llu
    counts[0] =          0
    counts[1] =         12
    counts[2] =          3
    counts[3] =          2
    counts[4] =         12
    counts[5] =         12
    counts[6] =          2
    counts[7] =         13
    counts[8] =          2
    counts[9] =         12
    counts[10] =          2
    counts[11] =          2
    counts[12] =         12
    counts[13] =         12
    counts[14] =          3
    counts[15] =         12
    counts[16] =          2
    counts[17] =         12
    counts[18] =          2
    counts[19] =          2
    counts[20] =         12
    counts[21] =         12
    counts[22] =          2
    counts[23] =         12
    counts[24] =          3
    counts[25] =         12
    counts[26] =          3
[done]





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







static void create_process(char** args) {


/*
	pid_t pid = fork();
	if (pid < 0) { perror("fork"); getchar(); return; }


	if (not pid) {
		if (execve(*args, args, environ) < 0) { perror("execve"); exit(1); }
	} 

	int status = 0;
	if ((pid = wait(&status)) == -1) { perror("wait"); getchar(); return; }

*/




//    ---------------------------  do this at process termination, saying the status and pid and dt. ----------------------------------
	//char dt[32] = {0};
	//struct timeval t = {0};
	//gettimeofday(&t, NULL);
	//struct tm* tm = localtime(&t.tv_sec);
	//strftime(dt, 32, "1%Y%m%d%u.%H%M%S", tm);
	//if (WIFEXITED(status)) 		{}//printf("[%s:(%d) exited with code %d]\n", dt, pid, WEXITSTATUS(status));
	//else if (WIFSIGNALED(status)) 	{}//printf("[%s:(%d) was terminated by signal %s]\n", dt, pid, strsignal(WTERMSIG(status)));
	//else if (WIFSTOPPED(status)) 	{}//printf("[%s:(%d) was stopped by signal %s]\n", 	dt, pid, strsignal(WSTOPSIG(status)));
	//else 				{}//printf("[%s:(%d) terminated for an unknown reason]\n", dt, pid);
	//fflush(stdout);
	// return status;

// -------------------------------------------------------------------------------------------------------------------------------------


}

// static void raw_say(const char* text) { create_process((char*[]) {strdup("/usr/bin/say"), strdup(text), 0}); }

/* static void input_off(void) { 
	create_process((char*[]) {
		strdup("/bin/bash"), 
		strdup("-c"), 
		strdup("echo 'actions'"), 
		0
	});
}*/



















 


/*


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





*/


















