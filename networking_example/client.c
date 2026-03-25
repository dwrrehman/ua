// tcp networking example: client code
// written on 1202603242.151031 by dwrr

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

static const char* server_ip_address = "::1";
static const int server_port = 32768;

int main(void) {	
	int connection = socket(AF_INET6, SOCK_STREAM, 0);
	if (connection < 0) { perror("socket"); exit(1); }

	struct sockaddr_in6 server_address = {0};
	server_address.sin6_family = AF_INET6;
	server_address.sin6_port = htons(server_port);

	int r = inet_pton(AF_INET6, server_ip_address, &server_address.sin6_addr);
	if (r <= 0) { perror("inet_pton"); exit(1); }

	r = connect(connection, (struct sockaddr *) &server_address, sizeof server_address);
	if (r < 0) { perror("connect"); exit(1); }


	char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &server_address.sin6_addr, ip, sizeof ip);
	const int port = ntohs(server_address.sin6_port);
	printf("connected to server %s:%d\n", ip, port);
	char buffer[4096] = {0};
	char running = 1;
	while (running) {
		memset(buffer, 0, sizeof buffer);
		printf("[ready]: "); fflush(stdout);
		fgets(buffer, sizeof buffer, stdin);
		printf("[%s:%u]: sending...\n", ip, port);
		send(connection, buffer, sizeof buffer - 1, 0);
		printf("[%s:%u]: sent: <<%s>>\n", ip, port, buffer);
		if (*buffer == '/') running = 0;
		printf("[%s:%u]: receiving...\n", ip, port);
		recv(connection, buffer, sizeof buffer - 1, 0);
		printf("[%s:%u]: received: <<%s>>\n", ip, port, buffer);		
	}

	close(connection);
}



































