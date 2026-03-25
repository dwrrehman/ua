// tcp networking example in c!
// written on 1202603242.145705 by dwrr

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>

typedef uint64_t nat;

static const int server_port = 32768;

static const nat required_client_count = 1; // 10

static void clear_screen(void) { printf("\033[H\033[2J"); } 

static void print_binary(nat x) {
	for (nat i = 0; i < 64; i++) {
		if (not (i & 3)) putchar('_');
		printf("%llu", (x >> i) & 1);
	}
}



#define get_ack     char ack = 0; printf("[%s:%u] awaiting ack...\n", addresses[i], ports[i]); recv(this, &ack, 1, 0);



int main(void) {

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
	char buffer[4096] = {0};
	const nat N = connection_count;
	while (running) {
		memset(buffer, 0, sizeof buffer);
		printf(":: "); fflush(stdout);
		fgets(buffer, sizeof buffer, stdin);

		if (strlen(buffer)) buffer[strlen(buffer) - 1] = 0;

		if (not strcmp(buffer, "")) {} 
		else if (not strcmp(buffer, "clear")) clear_screen();

		else if (not strcmp(buffer, "list")) {
			printf("connected clients:\n");
			for (nat i = 0; i < connection_count; i++) {
				printf("%llu: %s:%u\n", i, addresses[i], ports[i]);
			}
			puts("");


		} else if (	not strncmp(buffer, "execute ", strlen("execute ")) or 
			not strcmp(buffer, "execute")
		) { 
			char* command = buffer + strlen("execute ");
			const nat command_length = strlen(buffer) - strlen("execute ");
			for (nat i = 0; i < connection_count; i++) {
				printf("sending/executing shell command %s (%llu chars) to %s:%u client...\n", command, command_length, addresses[i], ports[i]);
				const int this = connections[i];
				char operation[16] = {0};
				operation[0] = 'E';
				operation[1] = command_length & 0xff;
				operation[2] = (command_length >> 8) & 0xff;				
				send(this, operation, 16, 0);
				send(this, command, command_length, 0);				

				nat output_length = 0; 
				printf("[%s:%u] awaiting shell output...\n", addresses[i], ports[i]);
				recv(this, &output_length, sizeof output_length , 0);

				const nat packet_count = output_length / 512;
				char* output = calloc(output_length + 1, 1);
				nat length = 0;
				for (nat p = 0; p < packet_count; p++) {
					char packet_buffer[512] = {0};
					recv(this, packet_buffer, 512, 0);
					memcpy(output + length, packet_buffer, 512);
					length += 512;
				}
				get_ack;

				printf("received shell command output: (%llu bytes) <<<%s>>>\n", output_length, output);
			}

		} else if (not strcmp(buffer, "disconnect")) {
			for (nat i = 0; i < connection_count; i++) {
				printf("sending disconnect command to %s:%u client...\n", addresses[i], ports[i]);
				const int this = connections[i];
				char operation[16] = {0};
				operation[0] = 'D';
				send(this, operation, 16, 0);
				get_ack;
			}
			running = 0;

		} else if (not strcmp(buffer, "terminate")) {
			for (nat i = 0; i < connection_count; i++) {
				printf("sending disconnect command to %s:%u client...\n", addresses[i], ports[i]);
				const int this = connections[i];
				char operation[16] = {0};
				operation[0] = 'K';
				send(this, operation, 16, 0);
				get_ack;
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























