// a program to test reading and 
// writing to file descr. of other
// processes that the program spawns.
// useful for making a shell, mainly. 

// an example of pipes, and using dup 
// and dup2 and fork and exec to look 
// at commands output and use them 
// for other things.

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iso646.h>
#include <poll.h>


int main(void) {

	puts("creating channel to communicate with process...");
	int channel[4];

	int r = pipe(channel);
	if (r < 0) { perror("pipe"); exit(1); }

	r = pipe(channel + 2);
	if (r < 0) { perror("pipe"); exit(1); }

	puts("calling fork...");
	pid_t pid = fork();
	if (pid < 0) { perror("fork"); exit(1); }

	if (pid == 0) {

		// puts("inside child process...");

		// puts("making fd[0/1] be standard io...");

		close(0);
		close(1);
		dup2(channel[2], 0);
		dup2(channel[1], 1);

		// puts("closing the pipe on the child end.");
		close(channel[0]);
		close(channel[3]);
		close(channel[1]);
		close(channel[2]);

		// puts("execing the program list stuff...");

		execlp(	"./program/list_stuff", 
			"./program/list_stuff", 0);

		fprintf(stderr, "Failed to execute \"command\"\n");
		perror("execve");

		exit(1);

	} else {
		puts("closing unused pipe ends now...");
		close(channel[1]);
		close(channel[2]);
		int process_in = channel[3];
		int process_out = channel[0];

		puts("parent process: opening repl for interaction.");
		char buffer[4096] = {0};

		while (1) {

			printf(":#ready#: "); fflush(stdout);
			fgets(buffer, sizeof buffer, stdin);

			if (not strcmp(buffer, "#wait\n")) break;

			else if (not strcmp(buffer, "#read\n")) {

				puts("[reading from child]");

				ssize_t n = 0;
				do {
					if (poll(&(struct pollfd){ 
						.fd = process_out, 
						.events = POLLIN 
					}, 1, 0) == 1) {

						char c = 0;
						n = read(process_out, &c, 1);
						if (c) putchar(c);
   					} else break;
				} while (n >= 1);

			} else {
				puts("writing buffer to child...");
				write(process_in, buffer, strlen(buffer));
				printf("[%lu bytes written]\n", strlen(buffer));
			}
		}

		puts("waiting for process to terminate...");

		int status = 0;
		if ((pid = wait(&status)) == -1) { perror("wait"); exit(1); }

		char dt[32] = {0};
		struct timeval t = {0};
		gettimeofday(&t, NULL);
		struct tm* tm = localtime(&t.tv_sec);
		strftime(dt, 32, "1%Y%m%d%u.%H%M%S", tm);

		if (WIFEXITED(status))
			printf("[%s:(%d) exited %d]\n", dt, pid, WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			printf("[%s:(%d) terminated %s]\n", dt, pid, strsignal(WTERMSIG(status)));
		else if (WIFSTOPPED(status)) 
			printf("[%s:(%d) stopped %s]\n", 	dt, pid, strsignal(WSTOPSIG(status)));
		else 	printf("[%s:(%d) terminated for an unknown reason]\n", dt, pid);
	}
	puts("terminating...");
	exit(0);
}











/*



		puts("writing newline to child.");
		write(process_in, "\n", 1);

		puts("writing insert one to child.");
		write(process_in, "insert_one\n", strlen("insert_one\n"));

		puts("child said:");
		do {
			char c = 0;
			n = read(process_out, &c, 1);
			if (c) putchar(c);
		} while (n >= 1);

		puts("writing Ansert one to child.");
		write(process_in, "Ansert_one\n", strlen("Ansert_one\n"));

		puts("child said:");
		do {
			char c = 0;
			n = read(process_out, &c, 1);
			if (c) putchar(c);
		} while (n >= 1);


*/




		// waitpid(pid, &status, 0);






		// pid = fork();

		// if (pid == 0) {

		// 	dup2(fd[0], 0);
		// 	close(fd[1]);
		// 	close(fd[0]);
		// 	execlp("wc", "wc", "-l", 0);
		// 	fprintf(stderr, "Failed to execute \"%s\"\n", "wc");
		// 	exit(1);
		// } else {
