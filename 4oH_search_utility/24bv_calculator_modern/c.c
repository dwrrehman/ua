// cleaned up 24bv calc, written 1202507233.220109 by dwrr
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
typedef uint8_t byte;
static void print_binary(uint32_t x) {
	for (byte i = 0; i < 32; i++) {
		if ((x >> i) & 1) printf("1"); else printf("0");
	}
}

static byte edgedir_sources[24] = {
	0, 0, 0, 2, 3, 1, 0, 0,
	0, 4, 5, 6, 4, 6, 4, 5,
	5, 6, 7, 7, 7, 7, 7, 7,
};

static byte edgedir_destinations[24] = {
	1, 2, 3, 1, 2, 3, 4, 5, 
	6, 6, 4, 5, 1, 1, 2, 2,
	3, 3, 1, 4, 2, 5, 6, 3,
};

static void edgedir_count_outputs(byte* nodes, uint32_t bits) {
	for (byte i = 0; i < 24; i++) {
		if (bits & (1 << i)) { 
			nodes[edgedir_sources[i]]++; 
			nodes[edgedir_destinations[i]]--;
		}
	}
}

static void edgedir_count_inputs(byte* nodes, uint32_t bits) {
	for (byte i = 0; i < 24; i++) {
		if (bits & (1 << i)) { 
			nodes[edgedir_sources[i]]--; 
			nodes[edgedir_destinations[i]]++;
		}
	}
}

int main(void) {
	uint32_t good_count = 0, bad_count = 0;
	const uint32_t max_count = (1 << 24);
	for (uint32_t bits = 0; bits < max_count; bits++) {
		byte outputs[8] = { 0, 5, 5, 5,  3, 3, 3, 0 };
		edgedir_count_outputs(outputs, bits);
		for (byte i = 0; i < 8; i++) if (outputs[i] != 3) goto bad;
		byte inputs[8] = { 6, 1, 1, 1,  3, 3, 3, 6 };
		edgedir_count_inputs(inputs, bits);
		for (byte i = 0; i < 8; i++) if (inputs[i] != 3) goto bad;
		print_binary(bits); printf(" : GOOD\n"); 
		good_count++; continue;
	bad: 	bad_count++; continue;
	}
	printf("good_count = %u, bad_count = %u\n", good_count, bad_count);
}






































