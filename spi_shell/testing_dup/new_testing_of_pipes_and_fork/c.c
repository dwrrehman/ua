#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <iso646.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <sys/wait.h> 
#include <stdio.h>   // 202402191.234834: a modal text editor written by dwrr. 
#include <stdlib.h>  // new modal editor: rewritten on 202406075.165239
#include <string.h>  // to be more stable, and display text with less bugs. 
#include <iso646.h>  // also redo all the keybinds and change semantics of 
#include <unistd.h>  // many commands.
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <sys/wait.h> 
#include <stdint.h>
#include <signal.h>
#include <stdnoreturn.h>



static char status[4096] = {0};
static char filename[4096] = {0};
static char autosavename[4096] = {0};
static volatile struct winsize window = {0};
static struct termios terminal = {0};

extern char** environ;




typedef uint64_t nat;

/*
int main(void) {

	int fd[2];
	pipe(fd);

	int id = fork();

	if (not id) {
		close(fd[0]);
		int x = 0;
		printf("input from child: ");
		scanf("%d", &x);
		write(fd[1], &x, sizeof(int));
		close(fd[1]);

	} else {
		close(fd[1]);
		int y = 0;
		read(fd[0], &y, sizeof(int));
		close(fd[0]);	
		printf("got from child processo: %d\n", y);
	}
}
*/


static void window_resized(int _) {if(_){} }//ioctl(0, TIOCGWINSZ, &window); }
static noreturn void interrupted(int _) {if(_){} 
	write(1, "\033[?25h", 6);
	//tcsetattr(0, TCSANOW, &terminal);
	//save(); 
	puts(""); 
	exit(0); 
}












static void change_directory(const char* d) {
	if (chdir(d) < 0) {
		printf("crp:err\""); printf("%s\n", d); printf("\"chdir\""); 
		printf("%s\n", strerror(errno)); 
		return;
	}
	//print("changed directories\n");
}

static void create_process(char** args) {
	pid_t pid = fork();
	if (pid < 0) { 
		printf("crp:*:fork\""); printf("%s\n", strerror(errno)); 
		return;
	}
	if (not pid) {
		if (execve(args[0], args, environ) < 0) { perror("execve"); exit(1); }
	} 
	int s = 0;
	if ((pid = wait(&s)) == -1) { 
		printf("crp:*:wait\""); printf("%s\n", strerror(errno)); 
		return;
	}
	char dt[32] = {0};
	struct timeval t = {0};
	gettimeofday(&t, NULL);
	struct tm* tm = localtime(&t.tv_sec);
	strftime(dt, 32, "1%Y%m%d%u.%H%M%S", tm);
	if (WIFEXITED(s)) 	printf("[%s:(%d) exited with code %d]\n", dt, pid, WEXITSTATUS(s));
	else if (WIFSIGNALED(s))printf("[%s:(%d) was terminated by signal %s]\n", dt, pid, strsignal(WTERMSIG(s)));
	else if (WIFSTOPPED(s)) printf("[%s:(%d) was stopped by signal %s]\n", 	dt, pid, strsignal(WSTOPSIG(s)));
	else 			printf("[%s:(%d) terminated for an unknown reason]\n", dt, pid);
	fflush(stdout);
	getchar();
}

static void execute(char* command) {
	if (not strlen(command)) return;
	//save();
	const char delimiter = command[0];
	const char* string = command + 1;
	const size_t length = strlen(command + 1);

	char** arguments = NULL;
	size_t argument_count = 0;

	size_t start = 0, argument_length = 0;
	for (size_t index = 0; index < length; index++) {
		if (string[index] != delimiter) {
			if (not argument_length) start = index;
			argument_length++;

		} else if (string[index] == delimiter) {
		push:	arguments = realloc(arguments, sizeof(char*) * (argument_count + 1));
			arguments[argument_count++] = strndup(string + start, argument_length);
			start = index;
			argument_length = 0; 
		}
	}
	if (argument_length) goto push;

	arguments = realloc(arguments, sizeof(char*) * (argument_count + 1));
	arguments[argument_count] = NULL;

	write(1, "\033[?25h", 6);
	//tcsetattr(0, TCSANOW, &terminal);

	for (nat i = 0; i < (nat) (window.ws_row * 2); i++) puts("");
	printf("\033[H"); fflush(stdout);

	create_process(arguments);
	//struct termios terminal_copy = terminal; 
	//terminal_copy.c_lflag &= ~((size_t) ECHO | ICANON);
	//tcsetattr(0, TCSANOW, &terminal_copy);
	write(1, "\033[?25l", 6);
	free(arguments);
}


static void signal_handler_sigpipe(int event) {
	printf("WARNING: %d: program received a SIGPIPE signal.\n", event);
	puts("ignoring this signal.");
	fflush(stdout);
}

int main(void) {

	signal(SIGPIPE, SIG_IGN);

	struct sigaction action = {.sa_handler = window_resized}; 
	sigaction(SIGWINCH, &action, NULL);

	struct sigaction action2 = {.sa_handler = interrupted}; 
	sigaction(SIGINT, &action2, NULL);

	//struct sigaction action3 = {.sa_handler = signal_handler_sigpipe}; 
	//sigaction(SIGPIPE, &action3, NULL);


	int fd[2];
	pipe(fd);

	int rfd[2];
	pipe(rfd);

	if (fork() == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);

		close(rfd[1]);
		dup2(rfd[0], 0);
		close(rfd[0]);

		execlp("./my_program", "./my_program", 0);

	} else {

		close(fd[1]);
		//dup2(fd[0], 0);
		//close(fd[0]);

		close(rfd[0]);
		//dup2(rfd[1], 1);
		//close(rfd[1]);

			/*	char buffer[1280] = {0};


					ssize_t nbytes = read(fd[0], buffer, sizeof buffer);
					printf("n = %ld\n", nbytes);
					if (nbytes <= 0) {
						printf("CHILD ERROR read(). \n");
						printf("error: %s\n", strerror(errno));
					} else {
						printf("child says: \"");
						fwrite(buffer, 1, (size_t) nbytes, stdout);
						puts("\"");
					}

					fflush(stdout);


				sleep(4);



				puts("sending ACK to child...");
				nbytes = write(rfd[1], "ACK\n", 4);
				printf("n = %ld\n", nbytes);
				if (nbytes <= 0) {
					printf("CHILD ERROR read(). \n");
					printf("error: %s\n", strerror(errno));
				} else {
					puts("successfully sent ack.");
				}

				fflush(stdout);



				sleep(4);


					nbytes = read(fd[0], buffer, sizeof buffer);
					printf("n = %ld\n", nbytes);
					if (nbytes <= 0) {
						printf("CHILD ERROR read(). \n");
						printf("error: %s\n", strerror(errno));
					} else {
						printf("child says: \"");
						fwrite(buffer, 1, (size_t) nbytes, stdout);
						puts("\"");
					}
					fflush(stdout);
*/



	
		int quit = 0;
		while (quit == 0) {
			char input[4096] = {0};
			printf(":: ");
			fflush(stdout);
			ssize_t n = read(0, input, sizeof input);
			if (n <= 0) {
				quit = 1;
				puts("ERROR: read(0) could not return, command mode failed.");
				printf("error: %s\n", strerror(errno));
			}

			if (input[0] == 'r') {

				char buffer[128] = {0};

				puts("reading from process..");

				if (poll(&(struct pollfd){ 
				.fd = fd[0], .events = POLLIN }, 1, 0) == 1) {
			
					ssize_t nbytes = read(fd[0], buffer, sizeof buffer);
					printf("n = %ld\n", nbytes);
					if (nbytes <= 0) {
						printf("CHILD ERROR read(). \n");
						printf("error: %s\n", strerror(errno));
					} else {
						printf("child says: \"");
						fwrite(buffer, 1, (size_t) nbytes, stdout);
						puts("\"");
					}

				} else {
					printf("nothing to read at the moment.\n");
				}
				fflush(stdout);

			} else if (input[0] == 'w') {

				puts("sending ACK to child...");
				ssize_t nbytes = write(rfd[1], "ACK\n", 4);
				printf("n = %ld\n", nbytes);
				if (nbytes <= 0) {
					printf("CHILD ERROR read(). \n");
					printf("error: %s\n", strerror(errno));
				} else {
					puts("successfully sent ack.");
				}

				fflush(stdout);

			} else if (input[0] == 'q') {
				puts("note: quit command issued...");
				quit = 1;

			} else {
				printf("error: command not found: %d...\n", input[0]);
			}
			usleep(1000000);
		}

		


		puts("closing file descriptors now.");
		fflush(stdout);

		close(fd[0]);
		close(rfd[1]);
	}

	puts("exiting program now.");
	fflush(stdout);
	exit(0);
}









/*

do 
/usr/bin/clang
-Weverything
-Wno-declaration-after-statement
c.c

do  ./a.out


*/






/*
		puts("reading from process..");
		ssize_t nbytes = read(fd[0], buffer, sizeof buffer);
		printf("read --> %ld\n", nbytes);
		fflush(stdout);
		printf("child says: \"");
		fwrite(buffer, 1, (size_t) nbytes, stdout);
		puts("\"");
		fflush(stdout);
		puts("sending ACK to child..");
		write(rfd[1], "ACK\n", 4);
		puts("sent ack.");

		fflush(stdout);
		puts("2reading from process..");
		nbytes = read(fd[0], buffer, sizeof buffer);
		printf("read --> %ld\n", nbytes);
		fflush(stdout);
		printf("child says: \"");
		fwrite(buffer, 1, (size_t) nbytes, stdout);
		puts("\"");
		fflush(stdout);
		puts("sending ACK to child..");
		write(rfd[1], "ACK\n", 4);
		puts("sent ack.");

		fflush(stdout);

*/


