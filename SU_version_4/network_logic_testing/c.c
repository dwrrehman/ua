// code for testing the network logic, in 
// the SU version 4, which uses the networked 
// peer-to-peer job distribution system!
// 1202606173.125913 by dwrr

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iso646.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <time.h>
#include <unistd.h>

typedef uint64_t nat;

static const nat server_port = 32768;    //   server_port + mi     that   machine  mi      puts their server on to listen!

static const char* addresses[10] = {
	"::1",
	"::1",
	"::1",
	"::1",
	"::1",

	"::1",
	"::1",
	"::1",
	"::1",
	"::1",
};

static void print_array(nat* array, nat count) {
	printf("{\n\t");
	for (nat i = 0; i < count; i++) {
		if (not (i % 4)) puts("");
		//printf("%02x(%c) ", array[i], array[i]); 
		printf("%lld ", array[i]);
	}
	puts("\n}");
}

static nat ACTUAL_job_counts[10] = {
	560, 450, 1680, 390, 500,
	570, 480, 400, 520, 510,
};






// 560 + 450 + 1680  prior      2,690

// after:      896 +  898  +  896         = 2,690




int main(int argc, const char** argv) {
	srand((unsigned) time(NULL));
	if (argc == 1) return puts("usage error, give the mi");
	const nat mi = (nat) atoi(argv[1]);

	nat job_count = ACTUAL_job_counts[mi];

	printf("info: running with mi = %llu, job_count = %llu\n", mi, job_count);

	nat global_job_counts[10];	
	memset(global_job_counts, 0xff, sizeof global_job_counts);

	// set up the server
	
	int server = socket(AF_INET6, SOCK_STREAM, 0);
	if (server < 0) { perror("socket"); exit(1); }
	int opt = 1;
	int r = setsockopt(server, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof opt);
	if (r) { perror("setsockopt(SO_REUSEPORT)"); exit(1); }
	struct sockaddr_in6 client_address = {0};
	client_address.sin6_family = AF_INET6;
	client_address.sin6_addr = in6addr_any;
	client_address.sin6_port = htons((int) (server_port + mi));
	client_address.sin6_flowinfo = 0;
	client_address.sin6_scope_id = 0;
	r = bind(server, (struct sockaddr *) &client_address, sizeof client_address);
	if (r < 0) { perror("bind"); exit(1); }
	r = listen(server, 10);
	if (r < 0) { perror("listen"); exit(1); }
	printf("[listening on %llu]\n", server_port + mi);
	struct sockaddr_in6 client = client_address;
	int length = sizeof client_address;





	if (fcntl(server, F_SETFL, O_NONBLOCK) < 0) { perror("fcntl(O_NONBLOCK)"); exit(1); } 





	printf("[server thread running on port %llu]\n", server_port + mi);

mainloop:;

	puts("\n\n\n\n");


	global_job_counts[mi] = job_count;

	puts("bar graph: ");
	for (nat i = 0; i < 3; i++) {
		if (global_job_counts[i] == (nat) -1) continue;
		printf("%5llu : ", global_job_counts[i]); 
		const nat amount = global_job_counts[i] / 80;
		for (nat _ = 0; _ < amount; _++) putchar('#'); puts("");
	}

	puts("");

	puts("current idea of what the global job counts are:");
	print_array(global_job_counts, 10);
	puts("");

	nat transfer_amount = 0;

	// compute transfer,   set to zero, if no transfer is neccessary
		
	nat 
		min_index = (nat) -1,
		min_value = (nat) -1,
		max_index = (nat) -1,
		max_value = 0;

	for (nat i = 0; i < 10; i++) {
		if (global_job_counts[i] == (nat) -1) continue;
		if (min_value > global_job_counts[i]) {
			min_value = global_job_counts[i];
			min_index = i;
		}
	}

	for (nat i = 0; i < 10; i++) {
		if (global_job_counts[i] == (nat) -1) continue;

		if (max_value < global_job_counts[i]) {
			max_value = global_job_counts[i];
			max_index = i;
		}
	}				

	if (	max_value > 40 and 
		max_value > min_value and 
		max_value - min_value > 30 and 
		mi == min_index
	) {
		const nat ideal = (max_value + min_value) / 2;
		const nat diff = max_value - ideal;
		if ((int64_t) diff < 0) abort();
		transfer_amount = diff;
		if (transfer_amount > 64) transfer_amount = 64;
		printf("WARING: computed non-zero transfer amount of %llu, initiating transfer!\n", transfer_amount);
	} else transfer_amount = 0; 


	accept_again:; { int connection = accept(server, (struct sockaddr *) &client, (socklen_t*) &length);
	if (connection < 0 and errno == EWOULDBLOCK) { 
		printf("...no clients to connect to at the moment...\n");
	} else if (connection < 0) { perror("accept"); exit(1); }
	else {
		char ip[INET6_ADDRSTRLEN] = {0};
        	inet_ntop(AF_INET6, &client.sin6_addr, ip, sizeof ip);
		int port = ntohs(client.sin6_port);
		printf("connected to client [%s:%d]\n", ip, port);

		nat bytes[4] = {0};
		ssize_t n = read(connection, bytes, sizeof bytes);
		if (n <= 0) { puts("---------------------"); perror("read"); puts("---------------------"); } printf("[n = %ld]\n", n);

		const nat command = bytes[0];
		const nat whotheyare = bytes[1];
		const nat their_job_count = bytes[2];
		const nat requested_transfer_amount = bytes[3];

		if (command == 0x1005) {
			printf("their_job_count = %llu\n", their_job_count);
			global_job_counts[whotheyare] = their_job_count;
			printf("they requested %llu jobs be sent!!!\n", requested_transfer_amount);

			if (requested_transfer_amount) { 
				nat buffer[64] = {0};
				for (nat i = 0; i < requested_transfer_amount; i++) {
					job_count--;
					buffer[i] = job_count;
				}	
				n = write(connection, buffer, sizeof buffer);
				if (n <= 0) { puts("---------------------"); perror("write"); puts("---------------------"); } printf("[n = %ld]\n", n);


				sleep(1);

			loop_it:;

				printf("trying to receive ack...\n");
				uint8_t ack = 255;
				n = read(connection, &ack, 1);
				if (n <= 0) { puts("---------------------"); perror("read"); puts("---------------------"); } printf("[n = %ld]\n", n);
				if (ack != 1) { 
					printf("error: [%hhu]: did not receive acknowledgement "
						"to transfer_request, received byte [%hhu] instead....", 
						ack, ack
					); 

					printf("looping the ack statement....");
					sleep(3);

					goto loop_it;
				} else 
					puts("--->    SUCCESSFULLY RECEIVED ACK FOR TRANSFER_REQUEST!!!!\n");				
				
			}
		
		} else {
			printf("error: unknown command received: %llu\n", command);
			abort();
		}
		close(connection);
		goto accept_again;
	}}

	for (nat i = 0; i < 3; i++) {
		if (i == mi) continue;	
		int connection = socket(AF_INET6, SOCK_STREAM, 0);
		if (connection < 0) { perror("socket"); exit(1); }

		struct sockaddr_in6 server_address = {0};	
		memset(&server_address, 0, sizeof server_address);
		server_address.sin6_family = AF_INET6;
		server_address.sin6_port = htons((int) (server_port + i));

		int rr = inet_pton(AF_INET6, addresses[i], &server_address.sin6_addr);
		if (rr <= 0) { perror("inet_pton"); exit(1); }
		printf("client: [trying to connect to %s:%u...]\n", addresses[i], (int) (server_port + i));
		int r2 = connect(connection, (struct sockaddr *) &server_address, sizeof server_address); 
		if (r2 < 0) perror("connect");
		else { 
			puts("[connection server was successful!]");
			char ip[INET6_ADDRSTRLEN];
	        	inet_ntop(AF_INET6, &server_address.sin6_addr, ip, sizeof ip);
			const int port = ntohs(server_address.sin6_port);
			printf("connection to machine [i = %llu]: [connected to server %s:%d]\n", i, ip, port);

			const nat amount = i == max_index ? transfer_amount : 0;
			nat command[4] = { 0x1005, mi, job_count, amount};

			ssize_t n = write(connection, command, sizeof command);
			if (n <= 0) { puts("---------------------"); perror("write"); puts("---------------------"); } printf("[n = %ld]\n", n);

			printf("broadcasted our job_count to machine, sent %llu\n", job_count);
			printf("requested transfer of %llu zv_jobs from them(%llu) to us(%llu)\n", amount, i, mi);
			
			if (amount) {
				nat jobs_to_push[64] = {0};

				n = read(connection, jobs_to_push, sizeof jobs_to_push);
				if (n <= 0) { puts("---------------------"); perror("read"); puts("---------------------"); } printf("[n = %ld]\n", n);

				printf("received these jobs, to push, as a result of the transfer request: \n");
				printf("\t { ");
				for (nat j = 0; j < amount; j++) {
					printf("%llu ", jobs_to_push[j]);
				} printf(" } (count=%llu)\n", amount);
				
				job_count += amount;

				printf("just sent ack to the one that we sent the request to...\n");
				uint8_t ack = 1;
				n = write(connection, &ack, 1);
				if (n <= 0) { puts("---------------------"); perror("write"); puts("---------------------"); } printf("[n = %ld]\n", n);
			}
		}
		close(connection);
	}
			
	sleep(2);

	//job_count -= (nat) (rand() % 5);

	goto mainloop;
}





















/*
	pthread_t server_thread;
	pthread_create(&server_thread, NULL, server_function, NULL);
	printf("[server thread running on port %llu]\n", server_port + mi);
mainloop:;
	puts("current idea of what the global job counts are:");
	print_array(global_job_counts, 10);
	if (false) goto done;
	sleep(1);
	goto mainloop;

done:
	pthread_join(server_thread, NULL);








static void* server_function(void* arg) { 
	while (1) {
		puts("server looping!");

		// ...accept() here...
		// talk with that client for a bit
		// close connection

		sleep(1);
	}
	pthread_exit(arg);
}


*/





// if (fcntl(socket, F_SETFL, O_NONBLOCK) < 0) { perror("fcntl(O_NONBLOCK)"); exit(1); } 








































/*



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
				puts("(newline)");
				command = calloc(512, 1);
				command[0] = 10;
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
				
				const int this = connections[i];
				send_command(this, c);
				send_string(this, command, command_length);
				//print_string("sent shell command", command, command_length);
				//printf("[%s:%u] awaiting response...\n", addresses[i], ports[i]);
				nat response_length = 0;
				char* response = get_string(this, &response_length);
				print_string("", response, response_length);
			}

		} else if (not strcmp(buffer, "read")) {

			char message[512] = "<read>";
			const nat message_length = strlen(message);

			for (nat i = 0; i < connection_count; i++) {
				const int this = connections[i];
				send_command(this, 'R');
				send_string(this, message, message_length);
				//print_string("sent ", message, message_length);
				//printf("[%s:%u] awaiting response...\n", addresses[i], ports[i]);
				nat output_length = 0;
				char* output = get_string(this, &output_length);
				print_string("", output, output_length);
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

int main(void) {
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
				for (nat i = 0; i < command_length; i++) if (command[i] == '#') command[i] = '0' + machine_index;
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



































static void send_string(int con, char* string, nat length) {
	send(con, &length, sizeof(nat), 0);
	//printf("SEND_STRING: info: send(8 bytes) call sent a string length of %llu\n", length);
	nat packet_count = (length / 512) + 1;	
	//printf("SEND_STRING: info: sending %llu packets...\n", packet_count);
	nat pointer = 0;
	for (nat p = 0; p < packet_count; p++) {
		send(con, string + pointer, 512, 0);
		pointer += 512;
	}
	
}

static char* get_string(int con, nat* output_length) {
	recv(con, output_length, sizeof(nat), 0);
	//printf("GET_STRING: info: recv(8 bytes) call received a string length of %llu\n", *output_length);
	nat packet_count = (*output_length / 512) + 1;
	//printf("GET_STRING: info: receiving %llu packets...\n", packet_count);
	char* output = calloc(packet_count, 512);
	nat pointer = 0;
	for (nat p = 0; p < packet_count; p++) {
		recv(con, output + pointer, 512, 0);
		pointer += 512;
	}
	return output;
}



*/













