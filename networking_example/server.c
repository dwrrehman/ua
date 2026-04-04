// tcp networking example in c!
// this is the server
// written on 1202603242.145705 by dwrr

/*



./start_server                        ("./attach")

write mkdir search_calls
write mkdir search_calls/0/
write cd search_calls/0/

send c.c

write ls
read

write mv 1202604035.211652_0193501842.txt c.c
read

send build

write ls
read

write mv 1202604035.211652_10928405972.txt build
read

write ./build release
read

write screen ./run #
read

write
read

write <control-A + 'd'>
read

disconnect

*/

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>

typedef uint64_t nat;

static const int server_port = 32768;
static const nat required_client_count = 10;

static void clear_screen(void) { printf("\033[H\033[2J"); } 

#include "common.c"

int main(void) {
	srand((unsigned) time(NULL));
	int server = socket(AF_INET6, SOCK_STREAM, 0);
	if (server < 0) { perror("socket"); exit(1); }

	int opt = 1;
	int r = setsockopt(server, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof opt);
	if (r) { perror("setsockopt(SO_REUSEPORT)"); exit(1); }

	struct sockaddr_in6 client_address = {0};
	client_address.sin6_family = AF_INET6;
	client_address.sin6_addr = in6addr_any;
	client_address.sin6_port = htons(server_port);
	client_address.sin6_flowinfo = 0;
	client_address.sin6_scope_id = 0;

	r = bind(server, (struct sockaddr *) &client_address, sizeof client_address);
	if (r < 0) { perror("bind"); exit(1); }

	r = listen(server, 10);
	if (r < 0) { perror("listen"); exit(1); }

	printf("[listening on %u]\n", server_port);

	int connections[16] = {0};
	struct sockaddr_in6 clients[16] = {0};
	char* addresses[16] = {0};
	int ports[16] = {0};
	nat connection_count = 0;

	while (connection_count < required_client_count) {
		clients[connection_count] = client_address;
		int length = sizeof client_address;
		connections[connection_count] = accept(server, (struct sockaddr *) (clients + connection_count), (socklen_t*) &length);
		if (connections[connection_count] < 0) { perror("accept"); exit(1); }

		char ip[INET6_ADDRSTRLEN] = {0};
        	inet_ntop(AF_INET6, &clients[connection_count].sin6_addr, ip, sizeof ip);
		int port = ntohs(clients[connection_count].sin6_port);
		printf("connected to client [%s:%d]\n", ip, port);

		addresses[connection_count] = strdup(ip);
		ports[connection_count] = port;
		connection_count++;
	}

	char running = 1;
	const nat buffer_size = 65536 * 4;
	char* buffer = calloc(buffer_size, 1);

	while (running) {

		memset(buffer, 0, buffer_size);
		printf(":: "); fflush(stdout);
		fgets(buffer, buffer_size, stdin);

		if (strlen(buffer)) buffer[strlen(buffer) - 1] = 0;

		if (not strcmp(buffer, "")) {} 
		else if (not strcmp(buffer, "clear")) clear_screen();

		else if (not strcmp(buffer, "list")) {
			printf("connected clients:\n");
			for (nat i = 0; i < connection_count; i++) {
				printf("%llu: %s:%u\n", i, addresses[i], ports[i]);
			}
			puts("");

		} else if (
			not strncmp(buffer, "write ", strlen("write ")) or not strcmp(buffer, "write") or 
			not strncmp(buffer, "send ", strlen("send "))
		) { 
			char* command = NULL;
			nat command_length = 0;
			char c = 0;

			if (not strcmp(buffer, "write")) {
				command = strdup("\n");
				command_length = 1;
				c = 'W';

			} else if (buffer[0] == 'w') {
				command = buffer + strlen("write ");
				command_length = strlen(buffer) - strlen("write ");
				c = 'W';

			} else {
				command = load_file(buffer + strlen("send "), &command_length);
				memcpy(buffer, command, command_length);
				command = buffer;
				c = 'S';
			}

			for (nat i = 0; i < connection_count; i++) {
				printf("sending/writing shell command %s (%llu chars) "
					"to %s:%u client...\n", 
					command, command_length, 
					addresses[i], ports[i]
				);
				const int this = connections[i];
				send_command(this, c);
				send_string(this, command, command_length);
				print_string("sent shell command", command, command_length);
				printf("[%s:%u] awaiting response...\n", addresses[i], ports[i]);
				nat response_length = 0;
				char* response = get_string(this, &response_length);
				print_string("received response", response, response_length);
			}

		} else if (not strcmp(buffer, "read")) {

			char message[512] = "<read>";
			const nat message_length = strlen(message);

			for (nat i = 0; i < connection_count; i++) {
				printf("reading output from shell commands %s (%llu chars) "
					"from %s:%u client...\n", 
					message, message_length, 
					addresses[i], ports[i]
				);
				const int this = connections[i];
				send_command(this, 'R');
				send_string(this, message, message_length);
				print_string("sent ", message, message_length);
				printf("[%s:%u] awaiting response...\n", addresses[i], ports[i]);
				nat output_length = 0;
				char* output = get_string(this, &output_length);
				print_string("received shell command output", output, output_length);
			}

		} else if (not strcmp(buffer, "disconnect") or not strcmp(buffer, "exit")) {
			for (nat i = 0; i < connection_count; i++) {
				printf("sending disconnect command to %s:%u client...\n", addresses[i], ports[i]);
				send_command(connections[i], 'D');
			}
			running = 0;

		} else if (not strcmp(buffer, "terminate")) {
			for (nat i = 0; i < connection_count; i++) {
				printf("sending disconnect command to %s:%u client...\n", addresses[i], ports[i]);
				send_command(connections[i], 'K');
			}
			running = 0;

		} else printf("error: unknown comand: %s\n", buffer);

	}
	close(server);
}




















































			// for each client, 
			//    send "EXECUTE[...command...]"  packet to all clients, with a payload of the users command. 





	/*


			else if (not strncmp(buffer, "disconnect", strlen("disconnect")) {

			printf("disconnecting from all %llu clients...\n", N);

			for (nat i = 0; i < connection_count; i++) close(connections[i]);


		






		} else if (not strncmp(buffer, "send_file ", strlen("send_file ")) { 

			printf("sending file to all %llu clients...\n", N);

			// read file off of local disk, then


			// for each client:

			//    send "CREATE_FILE[name, contents]" packet to client
			//        which creates the file on each client, with that name and file contents.





		} else if (not strncmp(buffer, "retreive_file ", strlen("retreive_file ")) { 

			printf("retreiving a file from all %llu clients...\n", N);
			

			// for each client: 

			//   send "GET_FILE[name]" packet

			//   receive CREATE_FILE[name, contents] packet from all clients

			//   
 


		} else if (not strncmp(buffer, "gather ", strlen("gather ")) { 



			// the argument given is the directory you want to store the zvs to.

			// loop over each client, 

			//      send packet "GATHER_Z_LISTS" 

			//      receive   multiple   "CREATE_FILE[name, contents]"    packet from that client!

			//      create the file based on that packet data. 


		} 
		*/

















/*

//printf("[%s:%u]: receiving...\n", ip, port);

		recv(connection, buffer, sizeof buffer - 1, 0);

		//printf("[%s:%u]: received: <<%s>>\n", ip, port, buffer);


		//if (*buffer == '/') running = 0;

		//memset(buffer, 0, sizeof buffer);
		//printf("[ready]: "); fflush(stdout);
		//fgets(buffer, sizeof buffer, stdin);

		//printf("[%s:%u]: sending...\n", ip, port);

		send(connection, buffer, sizeof buffer - 1, 0);


		//printf("[%s:%u]: sent message: <<%s>>\n", ip, port, buffer);
*/























