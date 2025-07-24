/// node permutation calculator 
// written on 1202507233.201626 by dwrr


/*
1202507233.211940



we obtain the pre_graph    via the  find() function. we write find()

static const byte graph[8 * 6] = {    //TODO: verify the contents of this array!!!!

	0, 1, 2, 6, 7, 8,  	  // 0  (verified)
	0, 3, 5, 13, 12, 18,  	  // 1 
	3, 4, 1, 14, 15, 20,      // 2 

	2, 4, 5, 17, 16, 23,     // 3 

	6, 9, 10, 12, 14, 19,     // 4 
	7, 16, 11, 10, 15, 21,    // 5 
	8, 17, 11, 9, 13, 22,     // 6 

	18, 19, 20, 21, 22, 23,  // 7
};

static const byte direction_is_output[8 * 6] = {
	0, 0, 0, 0, 0, 0,  	// 0 
	1, 1, 0, 1, 1, 1,  	// 1 
	0, 1, 1, 1, 1, 1,  	// 2 
	1, 0, 1, 1, 1, 1,  	// 3 

	1, 0, 1, 1, 1, 0,  	// 4       WRONGGGGG  <----------------- should total to only 3 outputs. FIX THIS

	1, 0, 1, 0, 0, 1,  	// 5 
	1, 0, 0, 1, 0, 1,  	// 6 
	0, 0, 0, 0, 0, 0,  	// 7
};





static const byte sources[8 * 6] = {

	1, 2, 3, 4, 5, 6,  	// 0  	for edges  :  { 0, 1, 2, 6, 7, 8 } 

	0, 0, 0, 0, 0, 0,  	// 1

	0, 0, 0, 0, 0, 0,  	// 2
 
	0, 0, 0, 0, 0, 0,  	// 3

	0, 0, 0, 0, 0, 0,  	// 4

	0, 0, 0, 0, 0, 0,  	// 5

	0, 0, 0, 0, 0, 0,  	// 6

	1, 4, 2, 5, 6, 3,  	// 7    for edges :  { 18, 19, 20, 21, 22, 23 } 
};



static u32 find_given_plis_output_list(
	const byte node, 

	const u32 bits     // this is a 24 bitvector.
) {	
	for (byte i = 0; i < 6; i++) {
		const bool edge_is_complemented = !!(bits & (1 << graph[node * 6 + i]));
		const bool is_output = direction_is_output[node * 6 + i];
		if (is_output xor edge_is_complemented) {
			push(result, sources[node * 6 + i]);
		}
	}
	return result;
}


	



trash:

static void count_edges(
	const byte node, 
	const uint32_t bits
) {	for (byte i = 0; i < 6; i++) {
		if (graph[node * 6 + i] == 255) continue;
		const bool edge_is_complemented = !!(bits & (1 << graph[node * 6 + i]));
		const bool is_output = direction_is_output[node * 6 + i];
		result[is_output][edge_is_complemented]++;}	
}













pre_graph:
-------------------------------

	opi : {{ pli, pli, pli }}                    
	opi : {{ pli, pli, pli }}                    
	opi : {{ pli, pli, pli }}                    
	opi : {{ pli, pli, pli }}                    

	opi : {{ pli, pli, pli }}                   
	opi : {{ pli, pli, pli }}                    
	opi : {{ pli, pli, pli }}                    
	opi : {{ pli, pli, pli }}                    {{...}} denotes an unordered set
	




we use the trich coloring to turn a pre_graph into a graph!






















/*

1202507233.210533

we found 5040 total permutations for    DOL[2] = three; (opi 2)


	5040 * 6936 * (2 * 6^7) = 19,571,691,847,680


we also found 3360 total perms for   DOL[2] = one; (opi 0)


	3360 * 6036 * (2 * 6^7) = 11,354,741,637,120







*/





#include <stdio.h>
#include <stdlib.h>
#include <iso646.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t byte;


static u32 permutations[65536] = {0};
static u32 permutation_count = 0;

static void print_bytes(byte* a, byte c) {
	printf("(%hhu):{", c);
	for (byte i = 0; i < c; i++) 
		printf("%hhu, ", a[i]);
	printf("}\n");
}


static void generate_all_permutations(byte dol2) {
	byte nodes[8] =   {0, 1, 2, 3, 4, 0, 1, dol2};
	byte moduli[8] =  {7, 6, 5, 4, 3, 2, 1, 0};
	byte pointer = 0;
	byte placements[8] = {0};
	byte array[8] = {0};

	goto init;

loop:
	if (array[pointer] < moduli[pointer]) goto increment;
	if (pointer < 7) goto reset_;
	goto done;
increment:
	array[pointer]++;
init:  	pointer = 0;

	memset(placements, 255, 8);
	for (byte i = 0; i < 8; i++) {
		const byte skip_count = array[i];
		byte s = 0;
		bool placed = false;
		for (byte p = 0; p < 8; p++) {
			if (placements[p] != 255) continue;
			if (placements[p] == 255 and s < skip_count) { s++; continue; } 
			if (placements[p] == 255 and s >= skip_count) {
				placements[p] = nodes[i];
				placed = true; break;
			}
		}
		if (not placed) abort();
		
	}


	const u32 data = 
		((u32) placements[0] << (3U * 0U)) | 
		((u32) placements[1] << (3U * 1U)) | 
		((u32) placements[2] << (3U * 2U)) | 
		((u32) placements[3] << (3U * 3U)) | 
		((u32) placements[4] << (3U * 4U)) | 
		((u32) placements[5] << (3U * 5U)) | 
		((u32) placements[6] << (3U * 6U)) | 
		((u32) placements[7] << (3U * 7U));

	for (u32 i = 0; i < permutation_count; i++) 
		if (permutations[i] == data) goto loop;

	permutations[permutation_count++] = x;
	printf("0x%08x : ", data); 
	print_bytes(placements, 8);
	goto loop;
reset_:
	array[pointer] = 0; 
	pointer++;
	goto loop;
done:
	printf("permutation_count = %u\n", permutation_count);
	return;
}

int main(void) {

	generate_all_permutations(0); 

	// DOL[2] = 0 means   duplicating a one;
	// DOL[2] = 2 means   duplicating a three;

	printf("all permutations computed:");

	for (u32 i = 0; i < permutation_count; i++) {
		if (i % 4 == 0) puts("");
		printf(" %08x ", permutations[i]);
	}
	puts("");
}



























