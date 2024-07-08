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




static void create_process(char** args) {



	pid_t pid = fork();
	if (pid < 0) { perror("fork"); getchar(); return; }


	if (not pid) {
		if (execve(*args, args, environ) < 0) { perror("execve"); exit(1); }
	} 

	int status = 0;
	if ((pid = wait(&status)) == -1) { perror("wait"); getchar(); return; }






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

static void raw_say(const char* text) { create_process((char*[]) {strdup("/usr/bin/say"), strdup(text), 0}); }

static void input_off(void) { 
	create_process((char*[]) {
		strdup("/bin/bash"), 
		strdup("-c"), 
		strdup("echo 'actions'"), 
		0
	});
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


















