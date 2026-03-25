// tcp networking example: client code
// written on 1202603242.151031 by dwrr

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <errno.h>

typedef uint64_t nat;

static const char* server_ip_address = "::1";
static const int server_port = 32768;
static const int try_connect_delay = 5;

#define send_ack     operation[0] = 1; send(connection, operation, 1, 0); puts("[SENT ACK]");


static void change_directory(const char* d) {
	if (chdir(d) < 0) {
		printf("crp:err\""); printf("%s\n", d); printf("\"chdir\""); 
		printf("%s\n", strerror(errno)); 
		return;
	}	
}



static char** parse_arguments(const char* string) {
	const char delimiter = 27;
	const size_t length = strlen(string);
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


static char* path_lookup(const char* executable_name) {
	const nat executable_length = (nat) strlen(executable_name);
	if (not executable_length) return NULL;

	const char* path = getenv("PATH");
	const nat length = (nat) strlen(path);
	//puts(path);
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
			//printf("testing: %s  ---> ", file);
			if (not access(file, X_OK)) {
				//printf("\033[32mGOOD\033[0m");
				return file;
			} else {
				//printf("\033[31mERROR\033[0m");
				free(file);
			}
			//puts("");

			start = index;
			argument_length = 0; 
		}
	}
	if (argument_length) goto push;
	return NULL;
}


int main(void) {

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
		
		char running = 1;
		while (running) {
			char operation[16] = {0};
			printf("[%s:%u]: receiving...\n", ip, port);
			recv(connection, operation, 16, 0);
			printf("[%s:%u]: received: ", ip, port);
			for (nat i = 0; i < 16; i++) printf("%02x(%c) ", operation[i], operation[i]); puts("");
			char op = operation[0];
			if (op == 'E') {
				nat length = operation[1] | (operation[1] << 8);
				char* command = calloc(length + 1, 1);
				recv(connection, command, length, 0);

				printf("info: received shell command (sized %llu), <<<%s>>>\n", length, command);


				// execute the shell command, and get the output from it!


				char command_output[1024] = "hello there from space!";
				nat output_length = 1024; 


				send(connection, &output_length, sizeof output_length , 0);
				const nat packet_count = output_length / 512;
				nat output_pointer = 0;
				for (nat p = 0; p < packet_count; p++) {
					char packet_buffer[512] = {0};
					memcpy(packet_buffer, command_output + output_pointer, 512);
					send(connection, packet_buffer, 512, 0);
					output_pointer += 512;
				}		
				printf("sent shell command output: (%llu bytes) <<<%s>>>\n", output_length, command_output);
				send_ack;

			} else if (op == 'D') {					
				puts("client: received DISCONNECT COMMAND");
				send_ack;
				running = 0;

			} else if (op == 'K') {
				puts("client: received KILL COMMAND");
				send_ack;
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



























