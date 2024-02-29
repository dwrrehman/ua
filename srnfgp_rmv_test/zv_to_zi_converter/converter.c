#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>  
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;


/*

	this utility converts a z value to a z index. 

	0122101221433002400305122300

		convert this z value into its corresponding z index!

*/

static const byte D = 1;        // the duplication count (operation_count = 5 + D)
static const bool R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

enum operations { one, two, three, five, six, };

static const byte _ = 0;
static const byte _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};

static const byte _63R_hole_count = 9;
static const byte _63R_hole_positions[_63R_hole_count] = {3, 6, 7, 10, 11, 13, 14, 15, 19};

static const byte _36R[5 * 4] = {
	0,  1, 2, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  _, 4, _,      //  9    11
	3,  _, _, _,      // 13 14 15
	4,  0, 0, _,      //       19
};

static const byte _36R_hole_count = 9;
static const byte _36R_hole_positions[_36R_hole_count] = {3, 6, 7, 9, 11, 13, 14, 15, 19};

static const byte initial = R ? _63R_hole_count : _36R_hole_count;

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const byte hole_count = initial + 4 * D;



static byte* positions = NULL; 

static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }

static void init_graph_from_string(const char* string, byte* graph) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

static void print_graph_as_adj(byte* graph) {
	puts("graph adjacency list: ");
	puts("{");
	for (byte i = 0; i < graph_count; i += 4) {
		const byte op = graph[i + 0];
		const byte l = graph[i + 1];
		const byte g = graph[i + 2];
		const byte e = graph[i + 3];

		printf("\t#%u: ins(.op = %hhu, .lge = [", i, op);
		printf("%2u,", l);
		printf("%2u,", g);
		printf("%2u]", e);
		printf(")\n\n");
	}
	printf("}\n\n");
}

static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}




	

int main(int argc, const char** argv) {

	if (argc == 1) return puts("usage: ./run <z value>");


	positions = calloc(hole_count, 1); 
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 
	
	byte* graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	init_graph_from_string(argv[1], graph);
	printf("converting z value: ");
	print_graph_raw(graph);
	puts("");
	print_graph_as_adj(graph);

	nat zindex = 0;

	nat p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	
	printf("\n\n\t\tzindex = %llu\n\n", zindex);
}


/*
	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		graph[positions[i]] = (byte) ((range_begin / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_begin >= p) { puts("range_begin is too big!"); printf("range_begin = %llu, p = %llu\n", range_begin, p); abort(); }

*/



































