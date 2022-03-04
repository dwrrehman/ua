#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <ctype.h>

/// 	XFG  search and run  utility.  used for finding the XFG, a special case of the UA.
/// 		written by Daniel W. R. Rehman on 2110317.093242 .
///                               edited on 2111147.004706

// description: a test framework to brute force search the possibility space and search it manually, 
// and make it easier to see what the graphs do, ie their computation/lifetime after some number of ins.






typedef unsigned long long nat;
typedef int8_t byte;





static const nat array_display_limit = 20;			// how many cells you print, when displaying the arrray. must be ≤ array size.

static const nat array_size = 4096; 				// finite size of the arrray, used to simulate n = infinity.    make this reasonably big.

static const nat max_stack_size = 128; 			// how many stack frames at max. 

static const nat execution_limit = 800; 		// how many ins to be exec'd before calling it quits for an option.
							// we seem to bottom out around between 600 and 800, so im doing 800 just to be safe, to get the whole search space, basically.









struct option {
	// (source is implied.)
	byte operation;  //ISA ins
	byte destination; //ISA ins
};

struct options {
	struct option* options;
	nat count;
};


struct stack_frame {
	struct options options;

	nat array_state[array_size];

	nat try;

	int padding2;

	byte source;
	byte side;

	byte padding0;
	byte padding1;

};


enum expansion_type {
	no_expansion,            // eg,    x 0 0 0 0 0 0 0 0 0 0 0 .. y   ie, no expansion at all.
	constant_expansion,      // eg,    x 1 1 1 1 1 1 1 1 1 0 0 .. y   ie, no internal structure, just expansion and never ER.
	good_expansion,          // eg,    x 4 2 6 1 5 2 1 1 0 0 0 .. y   ie, what we are looking for. does ER, and has structure.
	hole_expansion,          // eg,    x 3 4 2 0 1 1 0 0 0 0 0 .. y   ie, has a hole, which means that it double skipped on LE. bad.
};

static const char* expansion_type_spelling[] = {
	"no",
	"constant",
	"good",
	"hole",
};

struct candidate {
	nat expansion_type;
	byte graph[32];
	nat lifetime[64];
};





static inline char hex(byte i) { return i < 10 ? '0' + i : i - 10 + 'A'; }
static inline char nonzero_hex(byte i) { if (not i) return ' '; else return hex(i); }

static inline char* hex_string(byte* graph) {
	static char string[33] = {0};
	nat s = 0;
	for (nat i = 0; i < 32; i++) string[s++] = hex(graph[i]);
	string[s] = 0;
	return string;
}

static inline char* lifetime_string(nat* array) {

	static char string[4096] = {0};
	// memset(string, 0, 4096);
	int length = 0;
	
	length += sprintf(string + length, "%2llu  ", array[0]);
	for (nat i = 1; i < 63; i++) {
		length += sprintf(string + length, "%2llu ", array[i]);
	}
	length += sprintf(string + length, ".. %2llu", array[63]);


	// printf("printing string that we generated: \n");
	
	// for (int i = 0; i < length + 10; i++) {
	// 	printf("\"%c\" = %d\n", string[i], string[i]);
	// }
	// printf("done.\n");
		
	return string;
}

static inline void display_state(nat* array, nat n) {
	printf("\n   *0:[ %2llu ] ", array[0]);
	for (nat i = 1; i < array_display_limit; i++) { 
		printf(" [ %2llu ] ", array[i]);
	} 
	printf("   ...  *n:[ %2llu ]\n", array[n]);
	printf("         ");
	for (nat i = 0; i < array[0] and i < array_display_limit; i++) {
		printf("        ");
	}
	printf("^\n. \n");
}

static inline void display_state_compact(nat* array, nat n) {
	printf("%2llu ", array[0]);
	for (nat i = 1; i < array_display_limit; i++) { 
		printf("%2llu ", array[i]);
	} 
	printf(".. %2llu\n", array[n]);
}

static inline void print_as_adjacency_list(byte* graph) {
	printf("graph:\n\n");
	for (byte i = 1; i < 8; i++) {
		printf("\t%c: %c, %c    ", hex(i), nonzero_hex(graph[i * 2 + 0]), nonzero_hex(graph[i * 2 + 1]));
		printf("%c: %c, %c\n", hex(i + 8), nonzero_hex(graph[(i + 8) * 2 + 0]), nonzero_hex(graph[(i + 8) * 2 + 1]));
	}
	printf("\n");
}

static inline void initialize_graph_from_string(byte* graph, char* string) {
	nat s = 0;
	for (nat i = 0; i < 32; i++) {
		char c = string[s++];
		int g = isalpha(c) ? toupper(c) - 'A' + 10 : c - '0';
		graph[i] = (byte) g;
	}
}


static inline void write_candidates_to_file(const char* filename, struct candidate* candidates, nat count) {
	FILE* file = fopen(filename, "w+");
	if (not file) { perror("fopen"); return; }
	
	for (nat i = 0; i < count; i++) {
		fprintf(file, "%s  :  ", hex_string(candidates[i].graph));
		fprintf(file, "%s  :  ", expansion_type_spelling[candidates[i].expansion_type]);
		fprintf(file, "%s\n", lifetime_string(candidates[i].lifetime));
	}

	fclose(file);
}








static inline void read_graph_edit(byte* graph, char* buffer) {  // [format: {source}{false}{true} ]

	printf("edge: ");
	fgets(buffer, sizeof buffer, stdin);

	int s = buffer[0];
	int f = buffer[1];
	int t = buffer[2];

	int source = isalpha(s) ? toupper(s) - 'A' + 10 : s - '0';
	int false_destination = isalpha(f) ? toupper(f) - 'A' + 10 : f - '0';
	int true_destination = isalpha(t) ? toupper(t) - 'A' + 10 : t - '0';
	
	if (source < 0 or source >= 16) { printf("error: source out of bounds\n"); return; }
	if (false_destination < 0 or false_destination >= 16) { printf("error: false_destination out of bounds\n"); return; }
	if (true_destination < 0 or true_destination >= 16) { printf("error: true_destination out of bounds\n"); return; }

	graph[source * 2 + 0] = (byte) false_destination;
	graph[source * 2 + 1] = (byte) true_destination;

}

static inline void run(const nat m, const nat n, byte* graph, const byte start) {
	
	nat* array = calloc(n + 1, sizeof(nat));
	byte i = start;

	display_state(array, n);

	while (i) {
		
		printf("[%c]", hex(i));
		
		if (i == 9) {
			i = graph[i * 2 + (array[0] < n)];
			printf("[%c]\n", array[0] < n ? 't' : 'f');

		} else if (i == 0xA) {
			i = graph[i * 2 + (array[n] < array[0])]; 
			printf("[%c]\n", array[n] < array[0] ? 't' : 'f'); 

		} else if (i == 0xB) {
			i = graph[i * 2 + (array[array[0]] < m)]; 
			printf("[%c]\n", array[array[0]] < m ? 't' : 'f');
		
		} else if (i == 0xD) {
			i = graph[i * 2 + (array[0] < array[n])]; 
			printf("[%c]\n", array[0] < array[n] ? 't' : 'f'); 

		} else if (i == 0xE) {
			i = graph[i * 2 + (array[n] < array[array[0]])];
			printf("[%c]\n", array[n] < array[array[0]] ? 't' : 'f');

		} else if (i == 0xF) {
			i = graph[i * 2 + (array[array[0]] < array[n])];
			printf("[%c]\n", array[array[0]] < array[n] ? 't' : 'f');
		}

		else if (i == 1) { array[0]++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 2) { array[n]++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 3) { array[array[0]]++; 		i = graph[i * 2];  printf("\n"); }
		else if (i == 4) { 		 		i = graph[i * 2];  printf("\n"); }
		else if (i == 5) { array[0] = 0; 		i = graph[i * 2];  printf("\n"); }
		else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  printf("\n"); }
		else if (i == 7) { array[array[0]] = 0; 	i = graph[i * 2];  printf("\n"); }
		else if (i == 8) { 		 		i = graph[i * 2];  printf("\n"); }

		display_state(array, n);
		printf("> "); 
		if (getchar() == 'q') break;
	}

	printf("[HALT]\n");

}



static inline bool is_consistent(byte* graph, struct option option) {
	if (not graph[2 * option.operation]) return true; // this operation hasnt been chosen yet. ie, its fresh and we can do whatever with it. 

	// if it has already been chosen, then make sure that our option is 
	// consistent with what destination has already been chosen for this operation.
	return graph[2 * option.operation] == option.destination; 
}


static inline struct options generate(byte*  graph, byte source, bool side) {
	// for now, just generate every single option possible.

	struct option* options = calloc(6 * 4, sizeof(struct option)); // there can only be 24=6*4 options at most, i think..?
	nat count = 0;

	for (byte op = 1; op < 7; op++) { // 6 options for the operation. 		1, 2, 3, 4(NOP, ie direct), 5, 6, 

		// if (op == 4) continue; // skip over the null ins.
		// actually dont skip over the null instruction! becuase we need this inorder to allow for our direct brnaches!! we are using a NOP to signfify a direct branch. because we literally need to have at least one direct branch. so yeah. dont skip over 4. 

		for (byte dest = 10; dest < 16; dest++) {	// 4 options for the destination. (actually 3, technically, because the source takes up one of them.)
								// // A: 10,  D: 13,  E: 14,  F: 15
			if (dest == source or dest == 11 or dest == 12) continue;
			
			// printf("generating edge:  %hhX%c --(%hhX)--> %hhX \n", source, side ? 't' : 'f', op, dest);
			options[count].operation = op;
			options[count].destination = dest;
			if (is_consistent(graph, options[count])) count++;

		}
	}

	struct options result = {0};
	result.options = options;
	result.count = count;
	return result;
}

static inline void print_options(struct options o, byte source, byte side) {

	printf("printing out all %llu options:\n{\n", o.count);

	for (nat i = 0; i < o.count; i++) {
		printf("\toption #%llu:\t\t  %hhX%c %hhX %hhX \n", i, source, side ? 't' : 'f', o.options[i].operation, o.options[i].destination);
	}

	printf("}\n\n");
}

static inline void test_generate(byte* graph) {

	printf("calling generate()... testing...\n");
	struct options o = generate(graph, 0xE, false);
	print_options(o, 0xE, false);

	printf("printing consistency result for each option: \n");
	for (nat i = 0; i < o.count; i++) {
		printf("\toption #%llu:\t\t  is_consistent( Ef %hhx %hhX ) = %s\n", i, o.options[i].operation, o.options[i].destination, is_consistent(graph, o.options[i]) ? "true" : "false");
	}
}


static inline void instantiate_option_try(byte* graph, struct stack_frame frame) {

	// printf("INSTANTIATE: instantiating option try #%llu / %llu  at  %hhX%c...\n", frame.try, frame.options.count, frame.source, frame.side ? 't':'f');

	

	// printf("[info: was previously using option %hhX%c %hhX ...]\n", frame.source, frame.side ?'t':'f', graph[2 * frame.source + frame.side]);

	struct option option = frame.options.options[frame.try];
	graph[2 * frame.source + frame.side] = option.operation;
	graph[2 * option.operation] = option.destination;

	// printf("---> using option %hhX%c %hhX %hhX now.\n", frame.source, frame.side ?'t':'f', option.operation, option.destination);
}




// starting with   000000F0000000000000000000003000     although we dont need to technically.



static inline void print_stack(struct stack_frame* stack, nat stack_count) {
	printf("printing stack: [sf count=%llu]\n{\n", stack_count);

	for (nat i = 0; i < stack_count; i++) {
		printf("\tframe #%llu:  [%hhX%c]  {.try=%llu, .source=%d, .side=%d, .options.count=%llu} :: ", 
		i, stack[i].source, stack[i].side ? 't':'f', 
		stack[i].try, stack[i].source, stack[i].side, stack[i].options.count);
		display_state_compact(stack[i].array_state, array_size - 1);
	}
	printf("}\n");
}








// the problem is that we are getting options that look like    5f and tuff, because of the fact that we are reseting the dest and op after exhasuting a stack frame of options,
// 	when really, what we want is to anilihate the op only if we owned it. we need to look in the sf, and see if we are the owner, and then aniliate it,  = 0 ,  if we are the owner. 


// i think. i shouldlook into it more to know that thats what hppening, but i think it is is. 



// the aniliation code, ie, clenaing up for the next sf,   is superimportant. 









static inline nat determine_expansion_type(nat* lifetime) {


	// ------------ no expansion test: -------------                    0 0 0 0 0 0 0 0 0 0 0 0 0 0 

	bool is_no_expansion = true;
	for (nat i = 1; i < 63; i++) {
		if (lifetime[i]) { is_no_expansion = false; break; }
	}
	if (is_no_expansion) return no_expansion;


	// --------------- hole expansion ----------------                  1 2 4 2 0 3 2 3 0 0 0 0 0 
	//									    ^
	nat first = 1, last = 63;
	for (;last--;) 
		if (lifetime[last]) break;     // go from the last modnat, to the first, looking for where the first nonzero cell is.

	// we are now looking at the first positive cell. move forwards back to where the most recent zero cell is.
	last++;
	
	for (;first < last; first++)
		if (not lifetime[first]) break; // go from the first modnat to the last, looking for where the first zero cell is.
	
	// if the left most zero cell (ie, last) is the same cell as the first zero cell that we found going forwards, then we say its good. if not, then there must be a hole.
	if (last != first) return hole_expansion;
	

	// ------------------ constant expansion -------------------      1 1 1 1 1 1 1 1 1 1 0 0 0 0 0

	bool is_constant_expansion = true;
	for (nat i = 1; i < last; i++) {
		if (lifetime[i] != 1) { is_constant_expansion = false; break; }
	}
	
	if (is_constant_expansion) return constant_expansion;





	// everything else:

	return good_expansion;
}


static inline struct candidate evaluate(byte* graph, nat* array, nat n) {


	// printf("---> tried:  %s:   ", hex_string(graph));
	// display_state_compact(array, n);


	struct candidate candidate = {no_expansion,{0},{0}};	
	memcpy(candidate.graph, graph, 32);
	
	for (nat i = 0; i < 63; i++) {
		candidate.lifetime[i] = array[i];
	}
	candidate.lifetime[63] = array[array_size - 1];

	/// generate verdict for each graph, based on the xp of the arrray lifetime state. 
	candidate.expansion_type = determine_expansion_type(candidate.lifetime);
	

	//testing out what the file lines will look like..
	printf("%s  :  ", hex_string(candidate.graph));
	printf("%s  :  ", expansion_type_spelling[candidate.expansion_type]);
	printf("%s\n", lifetime_string(candidate.lifetime));


	
	return candidate;




	// printf("%2llu ", array[0]);
	// for (nat i = 1; i < array_display_limit; i++) { 
	// 	printf("%2llu ", array[i]);
	// } 
	// printf(".. %2llu\n", array[n]);

}



/*

	okay, so i am at the point where i kind of need to sort out all the graphs that i am generating, and seeing if i am actually generating sensible ones at all.
	
	first of all, i dont really know for certain that the algorithm is doing the correct thing. i mean, i feel like it might be.. but... proably not. 
*/




static inline void search(byte* graph, byte start) {
	
	// test_generate(graph);

	nat tried = 0, good_count = 0, constant_count = 0, none_count = 0, hole_count = 0;

	struct candidate* candidates = NULL;
	nat candidate_count = 0;
	


	
	const nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));

	byte i = start;     // instruction pointer.
	byte parent = 0;      // previous iteration's value of the instruction pointer. ("parent")		
	bool parent_side = false;
	
	struct stack_frame* stack = calloc(max_stack_size, sizeof(struct stack_frame));
	nat stack_count = 0;
	nat executed_count = 0;

begin:
	while (executed_count < execution_limit) {
	

		// printf("info: [instruction pointer = %hhX]\n", i);

		if (i == 0) {

			// printf("found hole at %hhX%c (parent=%hhX, parent_side=%d) generating a list of options to fill it...\n", parent, parent_side ? 't' : 'f', parent, parent_side);

			struct options options = generate(graph, parent, parent_side);

			// print_options(options, parent, parent_side); // debug.
			
			struct stack_frame frame;
			frame.try = 0; // start with the first option.
			frame.source = parent;      // keep track of the origin/parent of the hole.
			frame.side = parent_side; 
			frame.options = options;
			memcpy(frame.array_state, array, sizeof(nat) * array_size);
		
			stack[stack_count++] = frame;

			// printf("pushed new stack frame [sf count=%llu]: {.try=%llu, .source=%d, .side=%d, .options.count=%llu}\n", 
				// stack_count, frame.try, frame.source, frame.side, frame.options.count);

			// printf("state of array before push:\n"); 
			// display_state(array, n);

	

			// print_stack(stack, stack_count);
// 

			// printf("[info: executed %llu / %llu ins before generate() call.]\n", executed_count, execution_limit);
			executed_count = 0;


			instantiate_option_try(graph, frame);
			// printf("current graph is now: \n");
			// print_as_adjacency_list(graph);


			// printf("reverting instruction pointer to go back before we had executed the hole, %hhX...\n", parent);
			i = parent;

			// printf("generate: continue? > "); 
			// if (getchar() == 'q') return;
			

		} else {

			// printf("[%llu / %llu] executing %hhX...\n", executed_count, execution_limit, i);

			executed_count++;
			parent = i;
			parent_side = false; // overwritten by branch instructions, not overwritten by operations.

			// execute instruction.
	
			// printf("[%c]", hex(i));
	

			if (i == 9) {
				// i = graph[i * 2 + (array[0] < n)];
				// printf("[%c]\n", array[0] < n ? 't' : 'f');
				abort();


			} else if (i == 0xA) {
				i = graph[i * 2 + (array[n] < array[0])]; 
				parent_side = (array[n] < array[0]);
				// printf("[%c]\n", array[n] < array[0] ? 't' : 'f'); 


			} else if (i == 0xB) { 
				//i = graph[i * 2 + (array[array[0]] < m)]; 
				//printf("[%c]\n", array[array[0]] < m ? 't' : 'f');
				abort();
			

			} else if (i == 0xD) {
				i = graph[i * 2 + (array[0] < array[n])]; 
				parent_side = (array[0] < array[n]);
				// printf("[%c]\n", array[0] < array[n] ? 't' : 'f'); 

			} else if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[array[0]])];
				parent_side = (array[n] < array[array[0]]);
				// printf("[%c]\n", array[n] < array[array[0]] ? 't' : 'f');

			} else if (i == 0xF) {
				i = graph[i * 2 + (array[array[0]] < array[n])];
				parent_side = (array[array[0]] < array[n]);
				// printf("[%c]\n", array[array[0]] < array[n] ? 't' : 'f');
			}

			else if (i == 1) { array[0]++; 			i = graph[i * 2];  }//printf("\n"); }
			else if (i == 2) { array[n]++; 			i = graph[i * 2];  }//printf("\n"); }
			else if (i == 3) { array[array[0]]++; 		i = graph[i * 2];  }//printf("\n"); }
			else if (i == 4) { 		 		i = graph[i * 2];  }//printf("\n"); }
			else if (i == 5) { array[0] = 0; 		i = graph[i * 2];  }//printf("\n"); }
			else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  }//printf("\n"); }

			else if (i == 7) abort(); 	// { array[array[0]] = 0; i = graph[i * 2];  printf("\n"); }
			else if (i == 8) abort(); 	// { 	    		  i = graph[i * 2];  printf("\n"); }

			// printf("search: printing array state:\n");
			// display_state(array, n);

			// printf("execute: continue? > "); 
			// if (getchar() == 'q') return;
		} // else 
		
	} // while()
	
	// printf("[unimplemented.]\n");
	// now we need to backtrack! so.... we need to check to see if   try == options.count - 1,     if so, then we need to pop TOS.
	// if less than, then we need to incr try while the option is inconsistent. (easy, ie, skip over inconsistent ones.)
	
	// first, uninstatnate the current option.

	// look at TOS to find where it is, and then, remove it, incr the try, and then inst again.

	// printf("state of the stack: \n");
	// print_stack(stack, stack_count);
	
	// printf("tried candidate graph: \n");
	// print_as_adjacency_list(graph);
	// puts("");

	// &candidates, &candidate_count, 
	struct candidate new = evaluate(graph, array, n);
	// candidates = realloc(candidates, sizeof(struct candidate) * (candidate_count + 1));
	// candidates[candidate_count++] = new;
	
	tried++;
	if (new.expansion_type == good_expansion) good_count++;
	if (new.expansion_type == no_expansion) none_count++;
	if (new.expansion_type == constant_expansion) constant_count++;
	if (new.expansion_type == hole_expansion) hole_count++;
	// if (getchar() == 'q') {
	// 	printf("info: ending graph search early...\n");
	// 	return;
	// }

backtrack:

	for (byte op = 1; op < 7; op++) {       // {1, 2, 3, 4, 5, 6}

		bool operation_is_used = false;

		for (nat each = 0; each < 32; each++) { // loop over every single connection in the graph so far:

			if (graph[each] == op) {
				operation_is_used = true;
			}
		}

		if (not operation_is_used) {
			// printf("debug: the operation %d isnt being used, deleting now...\n", op);
			graph[2 * op] = 0; 
		}
	}


	
	if (stack_count == 0) {
		printf("error: no hole was found in the partial graph, after %llu instructions executed.\n", executed_count);
		return;
	}

	// printf("backtracking!...\n");
	// print_stack(stack, stack_count);

	memcpy(array, stack[stack_count - 1].array_state, array_size * sizeof(nat));
	// printf("reverted state of the array back to:\n");
	// display_state(array, n);

	if (stack[stack_count - 1].try < stack[stack_count - 1].options.count - 1) {
		
		stack[stack_count - 1].try++;
		// printf("incremented stack[%llu].try to be %llu... instantiating..\n", stack_count - 1, stack[stack_count - 1].try);
		instantiate_option_try(graph, stack[stack_count - 1]);
		// printf("current graph is now: \n");
		// print_as_adjacency_list(graph);

		// printf("trying this option now... starting from [i = %hhX]...\n", stack[stack_count - 1].source);
		i = stack[stack_count - 1].source;

		// printf("reseting the executed count (which was %llu...\n", executed_count);
		executed_count = 0;
		// printf("backtrack: continue? > "); 
		// if (getchar() == 'q') return;
		goto begin;
	} else {
		// delete the last option that we tried for this stack frame, from the graph. (so it doesnt fill up with gunk over time)
		// "uninstantiate"()
		struct stack_frame top = stack[stack_count - 1];
		// struct option last = top.options.options[top.try];

		// wipe out the connection from the source to the op. 
		// now, if we were the owner, then now the op will be dangling- ie no one can get to it. so it should be deleted. 
		// printf("debug: reseting the options source-operation edge...\n");
		graph[2 * top.source + top.side] = 0;//(0 := unknown)   		 
		// always done no matter what. always applicable.
	
		/*       
				[source] --> [operation] --> [destination] 

					  ^               ^
					  |               |
				this edge is           this edge is only removed 
				always removed.        if NO ONE is going to this 
						       operation anymore. 


			cool, i think this makes sense. 

			alrighty lets code it up! 
		*/


		for (byte op = 1; op < 7; op++) {       // {1, 2, 3, 4, 5, 6}

			bool operation_is_used = false;

			for (nat each = 0; each < 32; each++) { // loop over every single connection in the graph so far:

				if (graph[each] == op) {
					operation_is_used = true;
				}
			}

			if (not operation_is_used) {
				// printf("debug: the operation %d isnt being used, deleting now...\n", op);
				graph[2 * op] = 0; 
			}
		}

	
		if (stack_count <= 1) goto done;
		else {
			// printf("popping...\n");
			stack_count--;
			goto backtrack;
		}

	}

done:

	printf("success: [exited search function successfully: exhausted all graph extension possibilities.]\n");


	printf("----> tried %llu control flow graphs.\n", tried);
	printf("----> statistics:\n \t %llu good\n \t %llu constant\n \t %llu none\n \t %llu hole\n  \n", 
			good_count, constant_count, none_count, hole_count);


	printf("----> percentages:\n \t %lf good\n \t %lf constant\n \t %lf none\n \t %lf hole\n  \n", 
			(double)good_count / (double)tried * 100.0,
			(double)constant_count / (double)tried * 100.0,
			(double)none_count / (double)tried * 100.0,
			(double)hole_count / (double)tried * 100.0
		); 
}



		// bool owns_operation = true;
		// // we set this to true, if and only if there is no one who can get to the operation at all- not even us. (because we wiped out our connection to it)
		// // ie, we go through the true and false of all other branches, and see if they take you to the oepration,   if they do, then we dont own this. 
		// for (nat j = 0xA; j < 16; j++) {
		// 	if (j == 0xB or j == 0xC) continue;
		// 	if (graph[2 * j + 0] == last.operation or graph[2 * j + 1] == last.operation) owns_operation = false;
		// }

		// if (owns_operation) {
		// 	printf("debug: this option owns the operation, so we are reseting the op too...\n");
		// 	graph[2 * last.operation] = 0; 
		// }

			// delete the destination. /// should we even do this!?
			// are we the ones who came up with this operation, or not!?
			// is it even harmful to wipe this all the time?

			//DELETE ME:   tood: we should be checking the stack of decisions that we made, and checking to see if our operation is in another decisions data. if so, we arent responsible for it. only when no one else below us (in the stack) has claimed this operation, are we allowed to delete it ourselves, because then we know its ours. 



int main() {

	printf("this is a program to help with finding the XFG, in the UA theory.\ntype help for more info.\n");

	byte* graph = calloc(32, 1);
	

	// initialize the graph with just the basic edge that we think we know from the mtrc. just so its a nonzero graph.   
	// not technically required to do this. 

	// graph[2 * 0xE] = 0x3;
	// graph[2 * 0x3] = 0xF;
	
	char buffer[4096] = {0};

	while (1) {

		printf("::> ");
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;
		
		if (not strcmp(buffer, "quit") or not strcmp(buffer, "q")) {
			printf("quitting...\n");
			break;

		} else if (not strcmp(buffer, "help")) {
			printf("available commands:\n\t- quit\n\t- help\n\t- show\n\t- edit\n\t- run\n\t- init\n\t- search\n\t- x\n\t- x\n\t- dump\n\t- clear\n\t\n");

		} else if (not strcmp(buffer, "init")) {
			printf("hex[32] string: ");
			fgets(buffer, sizeof buffer, stdin);
			initialize_graph_from_string(graph, buffer);
		} 
		else if (not strcmp(buffer, "")) {} 
		else if (not strcmp(buffer, "clear") or not strcmp(buffer, "l")) printf("\033[2J\033[H");
		else if (not strcmp(buffer, "edit")) read_graph_edit(graph, buffer);
		else if (not strcmp(buffer, "show") or not strcmp(buffer, "ls")) print_as_adjacency_list(graph);
		else if (not strcmp(buffer, "run")) run(2048, 4096, graph, 0xE);

		else if (not strcmp(buffer, "search") or not strcmp(buffer, "s")) {
			printf("searching the graph space...\n");
			printf("using the partial graph: %s\n", hex_string(graph));
			printf("   (0's are unknowns.)\n");

			search(graph, 0xE);

			printf("main: finished call to search.\n");
		}
		
		else if (not strcmp(buffer, "dump")) printf("graph: %s\n", hex_string(graph));
		else printf("error: unknown command.\n");
	}
}


















/*
an in variant that i think i should state-
because of the fact that operations can never be the soruce supplied to the generate function, (in other words, we always sandwhich operations between two branchs, (because 1,5) and thus we never have just a naked bare  operation)

	that means that we essentially can look at whether the operations are taken just by looking at that position in the array, namely:

	graph[2 * source + side] = ...;    that will get either the false or true side of the given source, based on what side is. 

	so basically, we can look at     g[2 * 1], g[2 * 2], g[2 * 3], g[2 * 5], g[2 * 6],

		to basically just see if an operation is already taken up. 

		because like, basically, the problem that i am seeing is that 
		if we try to do an option, then like,    what if 



	no okay stop 



	this isnt actually a problem lol.     because basically, we dont need to know the take operations- we dont care=
		we should be able to use them again- its just 

			then, when we go through the options and start using them, we need to make sure the given option we want to try, is      "consistent" meaning, that if it says for 5 to go to D, already existing in the graph,     then like the option shouldnt make 5 go to A or something, instead, as what the option is.    remember, we can approach an operaion from many different branches, but an operation can itself only have a single child. (place that it goes to, after saying the op)


	so yeah, basically, that consisnency check is really all we need to add, and it isnt done in the generate function at all lol. it is basically just a "yes or no, do we keep this option?" question when we are trying the option.   we ask,  "is it consistent" and there should be some function that we can call, called 
			
	
	
	
						"	is_consistent( options[try] )    "     im giving an example call here- it returns a bool. 







	so yeah. im pretty sure thats it for that part. 




*/









/// mtrc:

// 000000F000D000000000000000203050




// try

// 00E0A0F000D0A0000000DE0000213056










// ------------------------- dead code ------------------------------------------------------------






// static inline bool all_constraints_hold(byte* g) {
// 	// verifies that all static constraints that we have hold, such as nerdp, operation branch interactions, etc.

// 	if (g[6] == 0xD or g[6] == 0xF) return false; // nerd principle

	

// 	return true;

// }







/*


	2111066.150948:

			fun fact:



	the computer generated this graph, just now!



	1: A,      9:  ,  
	2: D,      A:  , 2
	3: F,      B:  ,  

	5:  ,      D: 1,  
	6:  ,      E: 3,  
	7:  ,      F: 1,  



	
		its crap, but i mean, still!	



				i havent implemented backtracking yet, but ive implemented the rest of the forward logic! i think it works lol. yay. thats cool.


				

*/


