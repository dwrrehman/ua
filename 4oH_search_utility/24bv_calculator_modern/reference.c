#include <stdio.h>   // calcuating the edge directions for the 4D orthoplex hypothesis, 
#include <stdlib.h>   // written on 1202507067.024237 by dwrr
#include <stdint.h>
#include <iso646.h>   //  update: turns out there is no control flow graph valid edge direction setting  that corresponds to the hexdecachrono, and thus we need to abondon the 4D orthoplex hypoethesis! welpppppppp 
#include <stdbool.h>

typedef uint8_t byte;

static void print_binary(uint32_t x) {
	for (byte i = 0; i < 32; i++) {
		if ((x >> i) & 1) printf("1"); else printf("0");
	}
}



static void count_outputs(byte* nodes, uint32_t bits) {

// ---------------bottom layer---------------------
	
	if (bits & 0x000001) { nodes[0]++; nodes[1]--; } // 1000 0000 0000  0000 0000 0000   X
	if (bits & 0x000002) { nodes[0]++; nodes[2]--; } // 0100 0000 0000  0000 0000 0000
	if (bits & 0x000004) { nodes[0]++; nodes[3]--; } // 0010 0000 0000  0000 0000 0000

	if (bits & 0x000008) { nodes[2]++; nodes[1]--; } // 0001 0000 0000  0000 0000 0000
	if (bits & 0x000010) { nodes[3]++; nodes[2]--; } // 0000 1000 0000  0000 0000 0000
	if (bits & 0x000020) { nodes[1]++; nodes[3]--; } // 0000 0100 0000  0000 0000 0000
	
	if (bits & 0x000040) { nodes[0]++; nodes[4]--; } // 0000 0010 0000  0000 0000 0000
	if (bits & 0x000080) { nodes[0]++; nodes[5]--; } // 0000 0001 0000  0000 0000 0000

	if (bits & 0x000100) { nodes[0]++; nodes[6]--; } // 0000 0000 1000  0000 0000 0000   X

// ------------- middle layer -----------------------

	if (bits & 0x000200) { nodes[4]++; nodes[6]--; } // 0000 0000 0100  0000 0000 0000
	if (bits & 0x000400) { nodes[5]++; nodes[4]--; } // 0000 0000 0010  0000 0000 0000
	if (bits & 0x000800) { nodes[6]++; nodes[5]--; } // 0000 0000 0001  0000 0000 0000


	if (bits & 0x001000) { nodes[4]++; nodes[1]--; } // 0000 0000 0000  1000 0000 0000
	if (bits & 0x002000) { nodes[6]++; nodes[1]--; } // 0000 0000 0000  0100 0000 0000

	if (bits & 0x004000) { nodes[4]++; nodes[2]--; } // 0000 0000 0000  0010 0000 0000
	if (bits & 0x008000) { nodes[5]++; nodes[2]--; } // 0000 0000 0000  0001 0000 0000

	if (bits & 0x010000) { nodes[5]++; nodes[3]--; } // 0000 0000 0000  0000 1000 0000 X
	if (bits & 0x020000) { nodes[6]++; nodes[3]--; } // 0000 0000 0000  0000 0100 0000


// ------------ top layer ------------------------

	if (bits & 0x040000) { nodes[7]++; nodes[1]--; } // 0000 0000 0000  0000 0010 0000
	if (bits & 0x080000) { nodes[7]++; nodes[4]--; } // 0000 0000 0000  0000 0001 0000
	if (bits & 0x100000) { nodes[7]++; nodes[2]--; } // 0000 0000 0000  0000 0000 1000

	if (bits & 0x200000) { nodes[7]++; nodes[5]--; } // 0000 0000 0000  0000 0000 0100
	if (bits & 0x400000) { nodes[7]++; nodes[6]--; } // 0000 0000 0000  0000 0000 0010
	if (bits & 0x800000) { nodes[7]++; nodes[3]--; } // 0000 0000 0000  0000 0000 0001
}










static void count_inputs(byte* nodes, uint32_t bits) {

// ---------------bottom layer---------------------
	
	if (bits & 0x000001) { nodes[0]--; nodes[1]++; } // 1000 0000 0000  0000 0000 0000   X
	if (bits & 0x000002) { nodes[0]--; nodes[2]++; } // 0100 0000 0000  0000 0000 0000
	if (bits & 0x000004) { nodes[0]--; nodes[3]++; } // 0010 0000 0000  0000 0000 0000

	if (bits & 0x000008) { nodes[2]--; nodes[1]++; } // 0001 0000 0000  0000 0000 0000
	if (bits & 0x000010) { nodes[3]--; nodes[2]++; } // 0000 1000 0000  0000 0000 0000
	if (bits & 0x000020) { nodes[1]--; nodes[3]++; } // 0000 0100 0000  0000 0000 0000
	
	if (bits & 0x000040) { nodes[0]--; nodes[4]++; } // 0000 0010 0000  0000 0000 0000
	if (bits & 0x000080) { nodes[0]--; nodes[5]++; } // 0000 0001 0000  0000 0000 0000

	if (bits & 0x000100) { nodes[0]--; nodes[6]++; } // 0000 0000 1000  0000 0000 0000   X

// ------------- middle layer -----------------------

	if (bits & 0x000200) { nodes[4]--; nodes[6]++; } // 0000 0000 0100  0000 0000 0000
	if (bits & 0x000400) { nodes[5]--; nodes[4]++; } // 0000 0000 0010  0000 0000 0000
	if (bits & 0x000800) { nodes[6]--; nodes[5]++; } // 0000 0000 0001  0000 0000 0000


	if (bits & 0x001000) { nodes[4]--; nodes[1]++; } // 0000 0000 0000  1000 0000 0000
	if (bits & 0x002000) { nodes[6]--; nodes[1]++; } // 0000 0000 0000  0100 0000 0000

	if (bits & 0x004000) { nodes[4]--; nodes[2]++; } // 0000 0000 0000  0010 0000 0000
	if (bits & 0x008000) { nodes[5]--; nodes[2]++; } // 0000 0000 0000  0001 0000 0000

	if (bits & 0x010000) { nodes[5]--; nodes[3]++; } // 0000 0000 0000  0000 1000 0000 X
	if (bits & 0x020000) { nodes[6]--; nodes[3]++; } // 0000 0000 0000  0000 0100 0000


// ------------ top layer ------------------------

	if (bits & 0x040000) { nodes[7]--; nodes[1]++; } // 0000 0000 0000  0000 0010 0000
	if (bits & 0x080000) { nodes[7]--; nodes[4]++; } // 0000 0000 0000  0000 0001 0000
	if (bits & 0x100000) { nodes[7]--; nodes[2]++; } // 0000 0000 0000  0000 0000 1000

	if (bits & 0x200000) { nodes[7]--; nodes[5]++; } // 0000 0000 0000  0000 0000 0100
	if (bits & 0x400000) { nodes[7]--; nodes[6]++; } // 0000 0000 0000  0000 0000 0010
	if (bits & 0x800000) { nodes[7]--; nodes[3]++; } // 0000 0000 0000  0000 0000 0001
}


int main(void) {

	uint32_t good_count = 0, bad_count = 0;

	const uint32_t max_count = (1 << 24);
	for (uint32_t bits = 0; bits < max_count; bits++) {

		byte outputs[8] = { 0, 5, 5, 5,  3, 3, 3, 0 };
		count_outputs(outputs, bits);
		for (byte i = 0; i < 8; i++) if (outputs[i] != 3) goto bad;

		byte inputs[8] = { 6, 1, 1, 1,  3, 3, 3, 6 };
		count_inputs(inputs, bits);
		for (byte i = 0; i < 8; i++) if (inputs[i] != 3) goto bad;

		print_binary(bits); printf(" : GOOD\n"); 
		good_count++; continue;
	bad: 	bad_count++; continue;

	}
	printf("good_count = %u, bad_count = %u\n", good_count, bad_count);
}

/* 


1202507126.221429

// 11100100000010000011100000000000 : GOOD
// 11100100011110000011100000000000 : GOOD
// 11100100010001000011100000000000 : GOOD
// 01100100100011000011100000000000 : GOOD



good_count 6936
bad_count 16770280


6936 + 16770280 = 16,777,216
2 ^ 24 = 16,777,216





before : just output constraints: 

	good_count = 6936, bad_count = 16770280


after : both input and output constraints:

	good_count = 6936, bad_count = 16770280







edge direction configurations =    6936      (one dir setting per job!!!)


node permutations =    8! = 40,320

trichotomy colorings per node =     2 * (6 ^ 7) =   559,872
trichotomy colorings per node =     6 ^ 8 =   1,679,616       (not accounting for 6g)





--->   40,320 * 6936 * 559,872    =     156,573,534,781,440 





--->	40,320 * 6936 * 1,679,616    =     469,720,604,344,320      		469 trillion   







469,720,604,344,320  / (1,000,000 * 3600 * 24 * 365) = 14 years ish

processing each combination, once per microsecond




469,720,604,344,320  / (10 * 100,000,000 * 3600 * 24 * 365) = 0.01489474265 years

469,720,604,344,320  / (10 * 100,000,000 * 3600 * 24) = 5.4365810688 days















1202507126.225403


processing each combination, one zv per microsecond

	156,573,534,781,440  / (1,000,000 * 3600 * 24 * 365) = 4.9649142181 years



using 10 cores: (one zv per microsecond)

	156,573,534,781,440  / (10 * 1,000,000 * 3600 * 24 * 365) = 0.4964914218 years

	156,573,534,781,440  / (10 * 1,000,000 * 3600 * 24) = 181.21936896 days



using  64 + 10 cores: (one zv per 0.1 microseconds)

	156,573,534,781,440  / (74 * 10,000,000 * 3600 * 24) = 2.4489103914 days

	156,573,534,781,440  / (84 * 10,000,000 * 3600 * 24) = 2.15737344 days





*/
































/*


		byte inputs[8] = { 6, 3, 6, 8, 5, 3, 2, 6, 8};
		count_inputs(inputs, bits);
		for (byte i = 0; i < 8; i++) if (inputs[i] != 3) goto bad;


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





printf("node %u : { A:%hhu, B:%hhu, C:%hhu, D:%hhu }\n", 
				node, 
				result[0][0], 
				result[0][1],
				result[1][0],
				result[1][1]
			);
			const byte number_of_outputs = result[1][0] + result[0][1];
			const byte number_of_inputs = result[1][1] + result[0][0];
			printf(" ----> outputs = %hhu : inputs : %hhu\n", 
				number_of_outputs, number_of_inputs
			);







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




*/



















