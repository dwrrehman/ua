#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <stdbool.h>
#include <iso646.h>
#include <stdint.h>

// compile me with     clang -O0 -Weverything -fsanitize=address,undefined main.c -o run


static int8_t graph[16][2];

static inline void stringify_graph(char* string) {
	int s = 0;
	for (int8_t i = 0; i < 16; i++) {
		for (int8_t j = 0; j < 2; j++) {
			int8_t c = graph[i][j];
			if (c < 10) string[s++] = '0' + c;
			else string[s++] = c - 10 + 'A';
		}
	}
	string[s++] = 0;
}

// example input: 000000F000D000000000000700E223A5

static inline void initialize_graph_from_string(char* string) {
	int s = 0;
	for (int8_t i = 0; i < 16; i++) {
		for (int8_t j = 0; j < 2; j++) {
			int c = string[s++];
			int g = isalpha(c) ? toupper(c) - 'A' + 10 : c - '0';
			graph[i][j] = (int8_t) g;
		}
	}
}

int main() {
	printf("this is a program to help with finding the XFG, in the UA theory.\ntype help for more info.\n");
	char buffer[128] = {0};
	memset(graph, 0, 32);

	while (1) {

		printf("::> ");
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;
		
		if (not strcmp(buffer, "quit")) {
			printf("quitting...\n");
			break;
		
		} else if (not strcmp(buffer, "help")) {
			printf("available commands:\n\t- quit\n\t- help\n\t- show\n\t- edit\n\t- run\n\t- init\n\t- dump\n\t \n");

		} else if (not strcmp(buffer, "show")) {

			printf("current graph:\n");

			for (int8_t i = 0; i < 8; i++) {
				printf("\t%hhX: %hhX, %hhX    %hhX: %hhX, %hhX\n", 
					i, graph[i][0], graph[i][1], (int8_t) (i + 8), graph[i + 8][0], graph[i + 8][1]);
			}

		} else if (not strcmp(buffer, "edit")) {
			printf("[format: {source}{false}{true} ]\nedge string: ");
			
			fgets(buffer, sizeof buffer, stdin);
			int s_c = buffer[0];
			int f_c = buffer[1];
			int t_c = buffer[2];

			int source = isalpha(s_c) ? toupper(s_c) - 'A' + 10 : s_c - '0';
			int false_destination = isalpha(f_c) ? toupper(f_c) - 'A' + 10 : f_c - '0';
			int true_destination = isalpha(t_c) ? toupper(t_c) - 'A' + 10 : t_c - '0';
			
			if (source < 0 or source >= 16) { printf("error: source out of bounds\n"); continue; }

			graph[source][0] = (int8_t) false_destination;
			graph[source][1] = (int8_t) true_destination;
	
		} else if (not strcmp(buffer, "run")) {

			char string[33] = {0};
			stringify_graph(string);
			printf("running graph: %s\n", string);
			printf("execution done.\n");

		} else if (not strcmp(buffer, "init")) {
		
			printf("hex[32] string: ");
			fgets(buffer, sizeof buffer, stdin);
			initialize_graph_from_string(buffer);

		} else if (not strcmp(buffer, "dump")) {
			
			char string[33] = {0};
			stringify_graph(string);
			printf("graph:  %s \n", string);
			
		} else {
			printf("error: unknown command: %s\n", buffer);
		}
	}

	return 0;
}

