#include <stdio.h>   // calcuating the edge directions for the 4D orthoplex hypothesis, 
#include <stdlib.h>   // written on 1202507067.024237 by dwrr
#include <stdint.h>
#include <iso646.h>   //  update: turns out there is no control flow graph valid edge direction setting  that corresponds to the hexdecachrono, and thus we need to abondon the 4D orthoplex hypoethesis! welpppppppp 
#include <stdbool.h>

static const bool the_instruction_6_has_5_connections = false;  
static const bool q = the_instruction_6_has_5_connections;

//	false :  then pli 0   is a 6.
//	true :  then pli 3   is a 6. 

typedef uint8_t byte;

static const byte total_edge_count[8] = {
	6,  // 0
	6,  // 1
	6,  // 2
	5,  // 3
	6,  // 4
	6,  // 5
	6,  // 6
	5   // 7
};

static const byte graph[8 * 6] = {
	0, 1, 2, 6, 7, 8,  	// 0 
	0, 3, 5, 13, 12, 18,  	// 1 
	3, 4, 1, 14, 15, 20,  // 2 
	2, 4, 5, 17, 16, 255,  // 3 
	6, 9, 10, 12, 14, 19,  // 4 
	7, 16, 11, 10, 15, 21,  // 5 
	8, 17, 11, 9, 13, 22,  // 6 
	18, 19, 20, 21, 22, 255,  // 7
};

static const byte direction_is_output[8 * 6] = {
	0, 0, 0, 0, 0, 0,  	// 0 
	1, 1, 0, 1, 1, 1,  	// 1 
	0, 1, 1, 1, 1, 1,  	// 2 
	1, 0, 1, 1, 1, 255,  	// 3 
	1, 0, 1, 1, 1, 0,  	// 4 
	1, 0, 1, 0, 0, 1,  	// 5 
	1, 0, 0, 1, 0, 1,  	// 6 
	0, 0, 0, 0, 0, 255,  	// 7
};

static void count_edges(
	byte** result,
	const byte node, 
	const uint32_t bits
) {	
	for (byte i = 0; i < 6; i++) {
		if (graph[node * 6 + i] == 255) continue;

		const bool edge_is_complemented = !!(bits & (1 << graph[node * 6 + i]));
		const bool is_output = direction_is_output[node * 6 + i];
		result[is_output][edge_is_complemented]++;
	}	
}

static void print_binary(uint32_t x) {
	for (byte i = 0; i < 32; i++) {
		if ((x >> i) & 1) printf("1"); else printf("0");
	}
}

int main(void) {
	
	const uint32_t max_count =(1 << 23);
	for (uint32_t bits = 0; bits < max_count; bits++) {

		print_binary(bits);
		printf(" result: \n");
		for (byte node = 0; node < 8; node++) {
			byte** result = calloc(2, sizeof(byte*));
			result[0] = calloc(2, 1);
			result[1] = calloc(2, 1);
			count_edges(result, node, bits);		
			/*printf("node %u : { A:%hhu, B:%hhu, C:%hhu, D:%hhu }\n", 
				node, 
				result[0][0], 
				result[0][1],
				result[1][0],
				result[1][1]
			);*/
			const byte number_of_outputs = result[1][0] + result[0][1];
			const byte number_of_inputs = result[1][1] + result[0][0];
			/*printf(" ----> outputs = %hhu : inputs : %hhu\n", 
				number_of_outputs, number_of_inputs
			);*/

			if (not q) {
				if (node == 0 and number_of_outputs != 2) goto bad;
				else if (node != 0 and number_of_outputs != 3) goto bad;

				if (node == 0 and number_of_inputs != total_edge_count[node] - 2) goto bad;
				else if (node != 0 and number_of_inputs != total_edge_count[node] - 3) goto bad;
			} else {
				if (node == 3 and number_of_outputs != 2) goto bad;
				else if (node != 3 and number_of_outputs != 3) goto bad;

				if (node == 3 and number_of_inputs != total_edge_count[node] - 2) goto bad;
				else if (node != 3 and number_of_inputs != total_edge_count[node] - 3) goto bad;
			}
		}
 
		//printf("\n\n");
		printf("GOOD\n"); getchar(); 
		bad: continue;
	}
	puts("");
}






























		/*printf("count_edges(node: 0, bits: ");
		print_binary(bits);
		printf(") = %hhu  -- ", count);
		for (byte i = 0; i < count; i++) {
			putchar('#');
		}
		if (count == 6) printf(" <----- 6 connections!");
		puts("");*/
























