#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t byte;

int main(void) {

	byte graph_data[] = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  2, 3, 3, 4,  2, 3, 3, 4};

	byte end_data[]   = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  2, 3, 3, 4,  2, 3, 3, 4};
	//                                                                   ^

	size_t graph_count = sizeof(graph_data);
	byte operation_count = sizeof(graph_data) / 4;

	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1,   graph_count + (8 - (graph_count % 8)) % 8);

	printf("graph_count = %lu\n", graph_count + (8 - (graph_count % 8)) % 8);
	printf("operation_count = %hhu\n", operation_count);
	printf("(operation_count & 1) + (operation_count >> 1) = %u\n", (operation_count & 1) + (operation_count >> 1));

	memcpy(raw_graph, graph_data, graph_count);
	memcpy(raw_end, end_data, graph_count);

	//byte* graph = raw_graph;
	//byte* end = raw_end;

	uint64_t* graph_64 = raw_graph;
	uint64_t* end_64 = raw_end;
	
	for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
		printf("processing #%hhd of loop: comparing:  %016llx <=> %016llx     (  (%lld) <=> (%lld)  ) \n", 
				i, 
				graph_64[i], end_64[i], 
				graph_64[i], end_64[i]
		);
		if (graph_64[i] <  end_64[i]) { puts("g < e"); goto continue_; } 
		if (graph_64[i] >  end_64[i]) { puts("g > e"); break; }
		if (graph_64[i] == end_64[i]) { puts("g == e"); continue; }
	}
	puts("before done");
	goto done;

continue_:
	puts("at continue_!  processing next z value!");
	return 0;
	
	done:	puts("DONE! finished srnfgp call.");
	return 1;
}

/*





, 			 0 1 2 0 1 0 4 4 2 1 4 0 3 4 0 0 4 0 0 3 2 3 3 4











				// end state check   that will work with the zskip opt:
	////////////////////////////////////////////////////////////////////////////////////////// test seperately in its own file plz.
	for (byte i = 1 + (operation_count >> 1); i--;) {
		if (graph_64[i] <  end_64[i]) goto continue_;
		if (graph_64[i] >  end_64[i]) break;
		if (graph_64[i] == end_64[i]) continue;
	}
	goto done;
continue_:
	//////////////////////////////////////////////////////////////////////////////////////////


*/

