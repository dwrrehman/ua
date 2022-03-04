#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <iso646.h>
#include <stdint.h>
// compile me with:     clang -O0 -Weverything -fsanitize=address,undefined main.c -o run


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

// example input: 00E0A0F000D0E0000000D6000021325A

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
			
			unsigned long long array[4096] = {0};
			unsigned long long m = 2048, n = 4095;    // basically n = m = infinity. (ie, xfg)
			int8_t ins = 0x0E;
			
			while (ins) {
				if (ins == 1) {
					array[0]++; ins = graph[ins][0]; 
					printf("[1]"); 
				} else if (ins == 2) { 
					array[n]++; ins = graph[ins][0]; 
					printf("[2]"); 
				} else if (ins == 3) { 
					array[array[0]]++; ins = graph[ins][0]; 
					printf("[3]");
				} else if (ins == 4) { 
					/* nop */ ins = graph[ins][0]; 
					printf("[4]");
				} else if (ins == 5) { 
					array[0] = 0; ins = graph[ins][0]; 
					printf("[5]"); 
				} else if (ins == 6) { 
					array[n] = 0; ins = graph[ins][0]; 
					printf("[6]"); 
				} else if (ins == 7) { 
					array[array[0]] = 0; ins = graph[ins][0]; 
					printf("[7]"); 
				} else if (ins == 8) { 
					ins = graph[ins][1]; 
					printf("[8][T]"); 
				} else if (ins == 9) {
					ins = (array[0] < n ? graph[ins][1] : graph[ins][0]); 
					printf("[9][%d]", array[0] < n);
				} else if (ins == 0xA) {
					ins = (array[n] < array[0] ? graph[ins][1] : graph[ins][0]); 
					printf("[A][%d]", array[n] < array[0]); 
				} else if (ins == 0xB) {
					ins = (array[array[0]] < m ? graph[ins][1] : graph[ins][0]); 
					printf("[B][%d]", array[array[0]] < m);
				} else if (ins == 0xC) {
					printf("[C]"); /* do nothing */ 
				} else if (ins == 0xD) {
					ins = (array[0] < array[n] ? graph[ins][1] : graph[ins][0]);
					printf("[D][%d]", array[0] < array[n]); 
 				} else if (ins == 0xE) {
					ins = (array[n] < array[array[0]] ? graph[ins][1] : graph[ins][0]); 
					printf("[E][%d]", array[n] < array[array[0]]);  
				} else if (ins == 0xF) {
					ins = (array[array[0]] < array[n] ? graph[ins][1] : graph[ins][0]); 
					printf("[F][%d]", array[array[0]] < array[n]);
				}

				const unsigned long long display_limit = 10;
				// display:
				printf("\n    *0:[ %3llu ] ", array[0]);
				for (unsigned long long i = 1; i < display_limit; i++) { // how much do we print of the thing?...
					printf(" [ %3llu ] ", array[i]);
				} 
				printf("    ...  *n:[ %3llu ]\n", array[n]);
				
				// put marker note:
				printf("           ");
				for (unsigned long long i = 0; i < array[0] and i < display_limit; i++) {
					printf("         ");
				}
				printf("^\n. ");
				if (getchar() == 'q') break;
			}
			printf("[HALT]\n");

		} else if (not strcmp(buffer, "init")) {
			printf("hex[32] string: ");
			fgets(buffer, sizeof buffer, stdin);
			initialize_graph_from_string(buffer);

		} else if (not strcmp(buffer, "dump")) {
			char string[33] = {0};
			stringify_graph(string);
			printf("graph:  %s \n", string);

		} else if (not strcmp(buffer, "")) {
		
		} else {
			printf("error: unknown command: %s\n", buffer);
		}
	}

	return 0;
}









///  tried graphs:

// 00E0A0F000D0E0000000D6000021325A

// 00E0A0F000D0E000000016000021325A


/* 

*/