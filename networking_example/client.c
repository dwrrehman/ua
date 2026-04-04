// tcp networking example: client code
// this is the client code.
// written on 1202603242.151031 by dwrr


#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <sys/wait.h> 
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <sys/wait.h> 
#include <stdint.h>
#include <signal.h>
#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iso646.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>

typedef uint64_t nat;

static const char* server_ip_address = "::1";
static const int server_port = 32768;
static const int try_connect_delay = 5;

//static struct winsize window = {0};
//static struct termios terminal = {0};

extern char** environ;

#include "common.c"

static char** parse_arguments(const char* string, const nat length) {

	const char delimiter = ' ';
	char** arguments = NULL;
	size_t argument_count = 0;
	size_t start = 0, argument_length = 0;

	for (size_t index = 0; index < length; index++) {
		if (string[index] != delimiter) {
			if (not argument_length) start = index;
			argument_length++;

		} else {
		push:	arguments = realloc(arguments, sizeof(char*) * (argument_count + 1));
			arguments[argument_count++] = strndup(string + start, argument_length);
			start = index;
			argument_length = 0; 
		}
	}
	if (argument_length) goto push;
	arguments = realloc(arguments, sizeof(char*) * (argument_count + 1));
	arguments[argument_count] = NULL;
	return arguments;
}


static void debug_argument_list(char** arguments) {
	printf("debugging argument list:\n");
	for (nat i = 0; arguments[i]; i++) {
		printf("\targuments[%llu] = \"%s\"\n", i, arguments[i]);
	}
	puts("[done]");
}


static char* path_lookup(const char* executable_name) {
	const nat executable_length = (nat) strlen(executable_name);
	if (not executable_length) return NULL;
	const char* path = getenv("PATH");
	const nat length = (nat) strlen(path);
	puts(path);
	size_t start = 0, argument_length = 0;
	for (size_t index = 0; index < length; index++) {
		if (path[index] != ':') {
			if (not argument_length) start = index;
			argument_length++;
		} else {
		push:;	
			char* file = calloc(argument_length + 1 + executable_length + 1, 1);
			memcpy(file, path + start, argument_length);
			file[argument_length] = '/';
			memcpy(file + argument_length + 1, executable_name, executable_length);
			printf("testing: %s  ---> ", file);
			if (not access(file, X_OK)) {
				printf("\033[32mGOOD\033[0m\n");
				return file;
			} else {
				printf("\033[31mERROR\033[0m");
				free(file);
			}
			puts("");
			start = index;
			argument_length = 0; 
		}
	}
	if (argument_length) goto push;
	return NULL;
}

struct job {
	char** arguments;
	char* output;
	nat status;
	nat length;
	int fd[2];
	int rfd[2];
	int fdm[2];
	int padding;
	pid_t pid;
};


static char* execute_shell_command(
	struct job* this, 
	const char* command, 
	const nat command_length
) {

	char message[4096] = {0};



	if (this->status) {
		ssize_t nbytes = write(this->rfd[1], command, command_length);
		if (nbytes <= 0) {
			printf("CHILD ERROR write(). \n");
			printf("error: %s\n", strerror(errno));
			fflush(stdout);
		}
		this->output = realloc(this->output, this->length + (size_t) nbytes);
		memcpy(this->output + this->length, command, (size_t) nbytes);
		this->length += (size_t) nbytes;
		goto finish_execution;
	}


	
	char** arguments = parse_arguments(command, command_length);
	debug_argument_list(arguments);

	if (not strcmp(arguments[0], "cd")) {
		const char* path = arguments[1] ? arguments[1] : getenv("HOME");
		if (chdir(path) < 0) {
			snprintf(message, sizeof message, "chdir: [path=%s]: %s\n", path, strerror(errno));
			goto finish_execution;
		} else {
			snprintf(message, sizeof message, "chdir: successfully changed directories to \"%s/\"\n", path);
			goto finish_execution;
		}
	} 

	if (arguments[0][0] != '.' and arguments[0][0] != '/') {
		char* path = path_lookup(arguments[0]);
		if (not path) {
			snprintf(message, sizeof message, 
				"error: shell executable not found: \"%s\".", 
				arguments[0]
			);
			goto finish_execution;
		}
		free(arguments[0]);
		arguments[0] = path;
	}

	pipe(this->fd);
	pipe(this->rfd);
	pipe(this->fdm);

	const pid_t pid = fork();
	if (pid < 0) { 
		printf("crp:*:fork\""); printf("%s\n", strerror(errno)); 
		fflush(stdout);
		sleep(4);

	} else if (not pid) {
		close(this->fd[0]);
		dup2(this->fd[1], 1);
		close(this->fd[1]);
		close(this->fdm[0]);
		dup2(this->fdm[1], 2);
		close(this->fdm[1]);
		close(this->rfd[1]);
		dup2(this->rfd[0], 0);
		close(this->rfd[0]);
		execve(arguments[0], arguments, environ);
		printf("error: could not execute \"%s\": %s", 
			arguments[0], strerror(errno)
		);
		fflush(stdout);
		sleep(4);
		exit(1);

	} else {
		close(this->fd[1]);
		close(this->fdm[1]);
		close(this->rfd[0]);
		this->status = 1;
		this->arguments = arguments;
		this->pid = pid;
	}
finish_execution:
	return strdup(message);
}

static char* read_shell_command_output(struct job* this, nat* output_length) {

	char message[4096] = {0};
	if (this->status != 1) goto done_with_waiting;

	char buffer[65536] = {0};
	if (poll(&(struct pollfd){ .fd = this->fd[0], .events = POLLIN }, 1, 0) == 1) {
		ssize_t nbytes = read(this->fd[0], buffer, sizeof buffer);
		if (nbytes <= 0) {
			printf("CHILD ERROR stdout read(). \n");
			printf("error: %s\n", strerror(errno));
		} else {
			this->output = realloc(this->output, this->length + (size_t) nbytes);
			memcpy(this->output + this->length, buffer, (size_t) nbytes);
			this->length += (size_t) nbytes;
		}
	}
	if (poll(&(struct pollfd){ .fd = this->fdm[0], .events = POLLIN }, 1, 0) == 1) {
		ssize_t nbytes = read(this->fdm[0], buffer, sizeof buffer);
		if (nbytes <= 0) {
			printf("CHILD ERROR stderr read(). \n");
			printf("error: %s\n", strerror(errno));
		} else {
			this->output = realloc(this->output, this->length + (size_t) nbytes);
			memcpy(this->output + this->length, buffer, (size_t) nbytes);
			this->length += (size_t) nbytes;
		}
	}

	int s = 0;
	pid_t pid = this->pid;
	int r = waitpid(pid, &s, WNOHANG);

	if (r == -1) { 
		printf("crp:*:wait\""); printf("%s\n", strerror(errno)); 
	} else if (r == 0) goto done_with_waiting;

	char dt[32] = {0};
	struct timeval t = {0};
	gettimeofday(&t, NULL);
	struct tm* tm = localtime(&t.tv_sec);
	strftime(dt, 32, "1%Y%m%d%u.%H%M%S", tm);
	if (WIFEXITED(s)) 
		snprintf(message, sizeof message, 
			"[%s:(%d) exited with code %d]\n", 
			dt, pid, WEXITSTATUS(s)
		);
	else if (WIFSIGNALED(s)) 
		snprintf(message, sizeof message, 
			"[%s:(%d) was terminated by signal %s]\n", 
			dt, pid, strsignal(WTERMSIG(s))
		);
	else if (WIFSTOPPED(s)) 
		snprintf(message, sizeof message, 
			"[%s:(%d) was stopped by signal %s]\n", 	
			dt, pid, strsignal(WSTOPSIG(s))
		);
	else snprintf(message, sizeof message, 
		"[%s:(%d) terminated for an unknown reason]\n", 
		dt, pid
	);
	this->status = 0;
	this->arguments = NULL;

done_with_waiting:
	this->output = realloc(this->output, this->length + strlen(message));
	memcpy(this->output + this->length, message, strlen(message));
	this->length += strlen(message);

	*output_length = this->length;
	return this->output;
}


static char* get_command_output(const char* input_command) {
	char command[4096] = {0};
	snprintf(command, sizeof command, "%s 2>&1", input_command);
	FILE* f = popen(command, "r");
	if (not f) { perror("popen"); abort(); }
	char* string = NULL;
	size_t length = 0;
	char line[2048] = {0};
	while (fgets(line, sizeof line, f)) {
		size_t l = strlen(line);
		string = realloc(string, length + l);
		memcpy(string + length, line, l);
		length += l;
	}
	pclose(f);
	return strndup(string, length);
}

static nat translate(char* s) {
	if (not strcmp(s, "dwrr-mini.local\n")) return 0;
	if (not strcmp(s, "dwrr-mini1.local\n")) return 1;
	if (not strcmp(s, "dwrr-mini2.local\n")) return 2;
	if (not strcmp(s, "dwrr-mini3.local\n")) return 3;
	if (not strcmp(s, "dwrr-mini4.local\n")) return 4;
	if (not strcmp(s, "dwrr-mini5.local\n")) return 5;
	if (not strcmp(s, "dwrr-mini6.local\n")) return 6;
	if (not strcmp(s, "dwrr-mini7.local\n")) return 7;
	if (not strcmp(s, "dwrr-mini8.local\n")) return 8;
	if (not strcmp(s, "dwrr-mini9.local\n")) return 9;
	if (not strcmp(s, "dwrr.local\n")) return 0;
	puts(s); abort();
}

int main(void) {
	const nat machine_index = translate(get_command_output("hostname"));
	printf("client: running shell with machine_index set to %llu\n", machine_index);

	signal(SIGPIPE, SIG_IGN);

	char killed = 0;
	while (not killed) { 
		int r = 0, connection = 0;
		struct sockaddr_in6 server_address = {0};
		do {
			connection = socket(AF_INET6, SOCK_STREAM, 0);
			if (connection < 0) { perror("socket"); exit(1); }
			memset(&server_address, 0, sizeof server_address);
			server_address.sin6_family = AF_INET6;
			server_address.sin6_port = htons(server_port);
			int rr = inet_pton(AF_INET6, server_ip_address, &server_address.sin6_addr);
			if (rr <= 0) { perror("inet_pton"); exit(1); }
			printf("client: [trying to connect to %s:%u...]\n", server_ip_address, server_port);
			r = connect(connection, (struct sockaddr *) &server_address, sizeof server_address); 
			if (r < 0) { perror("connect"); sleep(try_connect_delay); } 
		} while (r < 0);

		puts("connection successful!");

		char ip[INET6_ADDRSTRLEN];
	        inet_ntop(AF_INET6, &server_address.sin6_addr, ip, sizeof ip);
		const int port = ntohs(server_address.sin6_port);
		printf("[connected to server %s:%d]\n", ip, port);

		struct job current_job = {0};
		
		char running = 1;
		while (running) {
			printf("[%s:%u]: receiving...\n", ip, port);

			char operation[16] = {0};
			recv(connection, operation, 16, 0);

			printf("[%s:%u]: received: %c", ip, port, operation[0]);
			print_array(operation, 16);

			char op = operation[0];

			if (op == 'W') {

				nat command_length = 0;
				char* command = get_string(connection, &command_length);
				print_string("received shell command", command, command_length);

				char* message = execute_shell_command(&current_job, command, command_length);
				char response[4096] = {0};
				memcpy(response, message, strlen(message));
				send_string(connection, response, strlen(response));
				print_string("sent response", response, strlen(response));
			
			} else if (op == 'S') {

				nat command_length = 0;
				char* command = get_string(connection, &command_length);
				print_string("received shell command", command, command_length);

				print_string("generating file with file contents", command, command_length);
				write_to_file(command, command_length);
				char message[1024] = "file wrote to client local disk.";
				nat message_length = strlen(message);
				send_string(connection, message, message_length);
				print_string("write confirmation", message, message_length);			

			} else if (op == 'R') {
				nat message_length = 0;
				char* message = get_string(connection, &message_length);
				print_string("received message", message, message_length);
				nat output_length = 0;
				char* output = read_shell_command_output(&current_job, &output_length);

				nat packet_count = (output_length / 512) + 1;
				char* response = calloc(packet_count, 512);
				memcpy(response, output, output_length);
				send_string(connection, response, output_length);
				print_string("sent shell command output", response, output_length);
				free(response);
				current_job.length = 0;
	
			} else if (op == 'D') {					
				puts("client: received DISCONNECT COMMAND");
				running = 0;

			} else if (op == 'K') {
				puts("client: received KILL COMMAND");
				running = 0;
				killed = 1;

			} else {
				printf("client received erroneous command %u", op);
				running = 0;
			}
		}

		close(connection);
		sleep(1);
	}	
}

































































/*

send(connection, buffer, sizeof buffer - 1, 0);
			printf("[%s:%u]: sent: <<%s>>\n", ip, port, buffer);

	

			printf("[%s:%u]: receiving...\n", ip, port);
			recv(connection, buffer, sizeof buffer - 1, 0);
			printf("[%s:%u]: received: <<%s>>\n", ip, port, buffer);		

*/



























