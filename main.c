#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <ctype.h>



// a test framework to visualize the possibility space and search it manually, but make it easier to see what the graphs do, ie their computation, and whether that edge is sensical. searching the graph possible edges space.

typedef unsigned long long nat;
typedef int8_t byte;

static const nat display_limit = 10;

static inline void display_state(nat* array, nat n) {
	printf("\n    *0:[ %3llu ] ", array[0]);
	for (nat i = 1; i < display_limit; i++) { 
		printf(" [ %3llu ] ", array[i]);
	} 
	printf("    ...  *n:[ %3llu ]\n", array[n]);
			
	printf("           ");
	for (nat i = 0; i < array[0] and i < display_limit; i++) {
		printf("         ");
	}
	printf("^\n. \n");
}


static inline char hex(byte i) {
	return i < 10 ? '0' + i : i - 10 + 'A';
}

static inline char nonzero_hex(byte i) {
	if (not i) return ' '; else return hex(i);
}


static inline char* hex_string(byte* graph) {

	static char string[33] = {0};
	byte s = 0;

	for (byte i = 0; i < 16; i++) {
		for (byte j = 0; j < 2; j++) {
			const byte c = graph[i * 2 + j];
			string[s++] = hex(c);
		}
	}
	string[s++] = 0;
	return string;
}

static inline void run(nat m, nat n, byte* graph, byte start, bool should_display) {
	
	nat* array = calloc(n + 1, sizeof(nat));
	byte i = start;

	if (should_display) display_state(array, n);

	while (i) {
		
		if (should_display) printf("[%c]", hex(i));
		
		if (i == 9) {
			i = graph[i * 2 + (array[0] < n)];
			if (should_display) printf("[%c]\n", array[0] < n ? 't' : 'f');

		} else if (i == 0xA) {
			i = graph[i * 2 + (array[n] < array[0])]; 
			if (should_display) printf("[%c]\n", array[n] < array[0] ? 't' : 'f'); 

		} else if (i == 0xB) {
			i = graph[i * 2 + (array[array[0]] < m)]; 
			if (should_display) printf("[%c]\n", array[array[0]] < m ? 't' : 'f');
		
		} else if (i == 0xD) {
			i = graph[i * 2 + (array[0] < array[n])]; 
			if (should_display) printf("[%c]\n", array[0] < array[n] ? 't' : 'f'); 

		} else if (i == 0xE) {
			i = graph[i * 2 + (array[n] < array[array[0]])];
			if (should_display) printf("[%c]\n", array[n] < array[array[0]] ? 't' : 'f');

		} else if (i == 0xF) {
			i = graph[i * 2 + (array[array[0]] < array[n])];
			if (should_display) printf("[%c]\n", array[array[0]] < array[n] ? 't' : 'f');
		}

		else if (i == 1) { array[0]++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 2) { array[n]++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 3) { array[array[0]]++; 		i = graph[i * 2];  printf("\n"); }
		else if (i == 4) { 		 		i = graph[i * 2];  printf("\n"); }
		else if (i == 5) { array[0] = 0; 		i = graph[i * 2];  printf("\n"); }
		else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  printf("\n"); }
		else if (i == 7) { array[array[0]] = 0; 	i = graph[i * 2];  printf("\n"); }
		else if (i == 8) { 		 		i = graph[i * 2];  printf("\n"); }

		if (should_display) display_state(array, n);

		if (should_display) { printf("> "); if (getchar() == 'q') break; }
	}

	printf("[HALT]\n");

}

static inline void print_as_adjacency_list(byte* graph) {

	printf("graph:\n");

	for (byte i = 0; i < 8; i++) {
		if (i == 4 or i == 0) { printf("\n"); continue; }
		printf("\t%c: %c, %c    ", hex(i), nonzero_hex(graph[i * 2 + 0]), nonzero_hex(graph[i * 2 + 1]));
		printf("%c: %c, %c\n", hex(i + 8), nonzero_hex(graph[(i + 8) * 2 + 0]), nonzero_hex(graph[(i + 8) * 2 + 1]));
	}
}

static inline void initialize_graph_from_string(byte* graph, char* string) {
	int s = 0;
	for (byte i = 0; i < 16; i++) {
		for (byte j = 0; j < 2; j++) {
			int c = string[s++];
			int g = isalpha(c) ? toupper(c) - 'A' + 10 : c - '0';
			graph[i * 2 + j] = (byte) g;
		}
	}
}



int main() {

	printf("this is a program to help with finding the XFG, in the UA theory.\ntype help for more info.\n");

	byte* graph = calloc(32, 1);
	
	char buffer[128] = {0};
	while (1) {

		printf("::> ");
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;
		
		if (not strcmp(buffer, "quit") or not strcmp(buffer, "q")) {
			printf("quitting...\n");
			break;
		} else if (not strcmp(buffer, "")) {} 
		else if (not strcmp(buffer, "clear") or not strcmp(buffer, "l")) printf("\033[2J\033[H");
		


		else if (not strcmp(buffer, "help")) {
			printf("available commands:\n\t- quit\n\t- help\n\t- show\n\t- edit\n\t- run\n\t- init\n\t- dump\n\t \n");

		} else if (not strcmp(buffer, "edit")) {  // [format: {source}{false}{true} ]

			printf("edge: ");
			fgets(buffer, sizeof buffer, stdin);

			int s_c = buffer[0];
			int f_c = buffer[1];
			int t_c = buffer[2];

			int source = isalpha(s_c) ? toupper(s_c) - 'A' + 10 : s_c - '0';
			int false_destination = isalpha(f_c) ? toupper(f_c) - 'A' + 10 : f_c - '0';
			int true_destination = isalpha(t_c) ? toupper(t_c) - 'A' + 10 : t_c - '0';
			
			if (source < 0 or source >= 16) { printf("error: source out of bounds\n"); continue; }

			graph[source * 2 + 0] = (byte) false_destination;
			graph[source * 2 + 1] = (byte) true_destination;

		} else if (not strcmp(buffer, "init")) {
			printf("hex[32] string: ");
			fgets(buffer, sizeof buffer, stdin);
			initialize_graph_from_string(graph, buffer);

		} else if (not strcmp(buffer, "show") or not strcmp(buffer, "ls")) print_as_adjacency_list(graph);
		else if (not strcmp(buffer, "run")) run(2048, 4096, graph, 0xE, true);
		else if (not strcmp(buffer, "dump")) printf("graph: %s\n", hex_string(graph));
		else printf("error: unknown command.\n");
	}
}


/// mtrc:

// 000000F000D000000000000000203050




// try

// 00E0A0F000D0A0000000DE0000213056


