#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <ctype.h>

/// 	XFG  search and run  utility.  used for finding the UA.
/// 		written by daniel rehman on 2110317.093242 .

// description: a test framework to visualize the possibility space and search it manually, 
// but make it easier to see what the graphs do, ie their computation, 
// and whether that edge is sensical. searching the graph possible edges space.


typedef unsigned long long nat;
typedef int8_t byte;


// -------------------- constants --------------------


static const nat array_display_limit = 20;			// how many cells you print, when displaying the arrray. must be ≤ array size.

static const nat array_size = 2048; 				// finite size of the arrray, used to simulate n = infinity.    make this reasonably big.

static const nat max_stack_size = 1024; 			// how many stack frames at max. 

static const nat execution_limit = 100; 		// how many ins to be exec'd before calling it quits for an option.





static inline char hex(byte i) { return i < 10 ? '0' + i : i - 10 + 'A'; }
static inline char nonzero_hex(byte i) { if (not i) return ' '; else return hex(i); }

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
	int s = 0;
	for (byte i = 0; i < 16; i++) {
		for (byte j = 0; j < 2; j++) {
			int c = string[s++];
			int g = isalpha(c) ? toupper(c) - 'A' + 10 : c - '0';
			graph[i * 2 + j] = (byte) g;
		}
	}
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
			
			printf("generating edge:  %hhX%c --(%hhX)--> %hhX \n", source, side ? 't' : 'f', op, dest);
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

	printf("INSTANTIATE: instantiating option try #%llu / %llu  at  %hhX%c...\n", frame.try, frame.options.count, frame.source, frame.side ? 't':'f');

	

	printf("[info: was previously using option %hhX%c %hhX ...]\n", frame.source, frame.side ?'t':'f', graph[2 * frame.source + frame.side]);

	struct option option = frame.options.options[frame.try];
	graph[2 * frame.source + frame.side] = option.operation;
	graph[2 * option.operation] = option.destination;

	printf("---> using option %hhX%c %hhX %hhX now.\n", frame.source, frame.side ?'t':'f', option.operation, option.destination);
}




// starting with   000000F0000000000000000000003000     although we dont need to technically.



static inline void print_stack(struct stack_frame* stack, nat stack_count) {
	printf("printing stack: [sf count=%llu]\n{\n", stack_count);

	for (nat i = 0; i < stack_count; i++) {
		printf("\tframe #%llu:  [%hhX%c]  {.try=%llu, .source=%d, .side=%d, .options.count=%llu} :: ", 
		i, stack[i].source, stack[i].side ? 't':'f', 
		stack[i].try, stack[i].source, stack[i].side, stack[i].options.count);
		display_state_compact(stack[i].array_state, 2047);
	}
	printf("}\n");
}








// the problem is that we are getting options that look like    5f and tuff, because of the fact that we are reseting the dest and op after exhasuting a stack frame of options,
// 	when really, what we want is to anilihate the op only if we owned it. we need to look in the sf, and see if we are the owner, and then aniliate it,  = 0 ,  if we are the owner. 


// i think. i shouldlook into it more to know that thats what hppening, but i think it is is. 



// the aniliation code, ie, clenaing up for the next sf,   is superimportant. 



struct candidate {
	byte graph[32];
	nat score;
	nat rating;
};






static inline void evaluate(byte* graph, nat* array, nat n) {


	printf("---> tried:  %s:   ", hex_string(graph));
	display_state_compact(array, n);
	
	

}



/*

	okay, so i am at the point where i kind of need to sort out all the graphs that i am generating, and seeing if i am actually generating sensible ones at all.
	
	first of all, i dont really know for certain that the algorithm is doing the correct thing. i mean, i feel like it might be.. but... proably not. 
*/




static inline void search(byte* graph, byte start) {
	
	// test_generate(graph);

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
	

		printf("info: [instruction pointer = %hhX]\n", i);

		if (i == 0) {

			printf("found hole at %hhX%c (parent=%hhX, parent_side=%d) generating a list of options to fill it...\n", parent, parent_side ? 't' : 'f', parent, parent_side);

			struct options options = generate(graph, parent, parent_side);

			print_options(options, parent, parent_side); // debug.
			
			struct stack_frame frame;
			frame.try = 0; // start with the first option.
			frame.source = parent;      // keep track of the origin/parent of the hole.
			frame.side = parent_side; 
			frame.options = options;
			memcpy(frame.array_state, array, sizeof(nat) * array_size);
		
			stack[stack_count++] = frame;

			printf("pushed new stack frame [sf count=%llu]: {.try=%llu, .source=%d, .side=%d, .options.count=%llu}\n", 
				stack_count, frame.try, frame.source, frame.side, frame.options.count);

			printf("state of array before push:\n"); 
			display_state(array, n);

	

			print_stack(stack, stack_count);


			printf("[info: executed %llu / %llu ins before generate() call.]\n", executed_count, execution_limit);
			executed_count = 0;


			instantiate_option_try(graph, frame);
			printf("current graph is now: \n");
			print_as_adjacency_list(graph);


			printf("reverting instruction pointer to go back before we had executed the hole, %hhX...\n", parent);
			i = parent;

			// printf("generate: continue? > "); 
			// if (getchar() == 'q') return;
			

		} else {

			printf("[%llu / %llu] executing %hhX...\n", executed_count, execution_limit, i);

			executed_count++;
			parent = i;
			parent_side = false; // overwritten by branch instructions, not overwritten by operations.

			// execute instruction.
	
			printf("[%c]", hex(i));
	

			if (i == 9) {
				// i = graph[i * 2 + (array[0] < n)];
				// printf("[%c]\n", array[0] < n ? 't' : 'f');
				abort();


			} else if (i == 0xA) {
				i = graph[i * 2 + (array[n] < array[0])]; 
				parent_side = (array[n] < array[0]);
				printf("[%c]\n", array[n] < array[0] ? 't' : 'f'); 


			} else if (i == 0xB) { 
				//i = graph[i * 2 + (array[array[0]] < m)]; 
				//printf("[%c]\n", array[array[0]] < m ? 't' : 'f');
				abort();
			

			} else if (i == 0xD) {
				i = graph[i * 2 + (array[0] < array[n])]; 
				parent_side = (array[0] < array[n]);
				printf("[%c]\n", array[0] < array[n] ? 't' : 'f'); 

			} else if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[array[0]])];
				parent_side = (array[n] < array[array[0]]);
				printf("[%c]\n", array[n] < array[array[0]] ? 't' : 'f');

			} else if (i == 0xF) {
				i = graph[i * 2 + (array[array[0]] < array[n])];
				parent_side = (array[array[0]] < array[n]);
				printf("[%c]\n", array[array[0]] < array[n] ? 't' : 'f');
			}

			else if (i == 1) { array[0]++; 			i = graph[i * 2];  printf("\n"); }
			else if (i == 2) { array[n]++; 			i = graph[i * 2];  printf("\n"); }
			else if (i == 3) { array[array[0]]++; 		i = graph[i * 2];  printf("\n"); }
			else if (i == 4) { 		 		i = graph[i * 2];  printf("\n"); }
			else if (i == 5) { array[0] = 0; 		i = graph[i * 2];  printf("\n"); }
			else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  printf("\n"); }

			else if (i == 7) abort(); 	// { array[array[0]] = 0; i = graph[i * 2];  printf("\n"); }
			else if (i == 8) abort(); 	// { 	    		  i = graph[i * 2];  printf("\n"); }

			printf("search: printing array state:\n");
			display_state(array, n);

			// printf("execute: continue? > "); 
			// if (getchar() == 'q') return;
		} // else 
		
	} // while()
	
	

	// printf("[unimplemented.]\n");
	// now we need to backtrack! so.... we need to check to see if   try == options.count - 1,     if so, then we need to pop TOS.
	// if less than, then we need to incr try while the option is inconsistent. (easy, ie, skip over inconsistent ones.)

	
	// first, uninstatnate the current option.

	// look at TOS to find where it is, and then, remove it, incr the try, and then inst again.

	
	printf("tried candidate graph: \n");
	print_as_adjacency_list(graph);
	puts("");

	
	// &candidates, &candidate_count, 
	evaluate(graph, array, n);
	if (getchar() == 'q') return;


backtrack:

	printf("backtracking!...\n");
	print_stack(stack, stack_count);

	memcpy(array, stack[stack_count - 1].array_state, array_size * sizeof(nat));
	printf("reverted state of the array back to:\n");
	display_state(array, n);

	if (stack[stack_count - 1].try < stack[stack_count - 1].options.count - 1) {
		
		stack[stack_count - 1].try++;
		printf("incremented stack[%llu].try to be %llu... instantiating..\n", stack_count - 1, stack[stack_count - 1].try);
		instantiate_option_try(graph, stack[stack_count - 1]);
		printf("current graph is now: \n");
		print_as_adjacency_list(graph);

		printf("trying this option now... starting from [i = %hhX]...\n", stack[stack_count - 1].source);
		i = stack[stack_count - 1].source;

		printf("reseting the executed count (which was %llu...\n", executed_count);
		executed_count = 0;

		// printf("backtrack: continue? > "); 
		// if (getchar() == 'q') return;

		goto begin;
	} else {

		// delete the last option that we tried for this stack frame, from the graph. (so it doesnt fill up with gunk over time)
		// "uninstantiate"()
		struct stack_frame top = stack[stack_count - 1];
		struct option last = top.options.options[top.try];

		// wipe out the connection from the source to the op. 
		// now, if we were the owner, then now the op will be dangling- ie no one can get to it. so it should be deleted. 

		graph[2 * top.source + top.side] = 0;//(0 := unknown)   		  // always done no matter what. always applicable.

		bool owns_operation = true;
		// we set this to true, if and only if there is no one who can get to the operation at all- not even us. (because we wiped out our connection to it)
		// ie, we go through the true and false of all other branches, and see if they take you to the oepration,   if they do, then we dont own this. 
		for (nat j = 0xA; j < 16; j++) {
			if (j == 0xB or j == 0xC) continue;
			if (graph[2 * j + 0] == last.operation or graph[2 * j + 1] == last.operation) owns_operation = false;
		}

		if (owns_operation) graph[2 * last.operation] = 0; 
	
			// delete the destination. /// should we even do this!?
			// are we the ones who came up with this operation, or not!?
			// is it even harmful to wipe this all the time?

			//DELETE ME:   tood: we should be checking the stack of decisions that we made, and checking to see if our operation is in another decisions data. if so, we arent responsible for it. only when no one else below us (in the stack) has claimed this operation, are we allowed to delete it ourselves, because then we know its ours. 


		if (stack_count <= 1) goto done;
		else {
			stack_count--;
			goto backtrack;
		}

	}

done:

	printf("success: [exited search function successfully: exhausted all graph extension possibilities.]\n");
}


int main() {

	printf("this is a program to help with finding the XFG, in the UA theory.\ntype help for more info.\n");

	byte* graph = calloc(32, 1);
	

	// initialize the graph with just the basic edge that we think we know from the mtrc. just so its a nonzero graph.   
	// not technically required to do this. 

	graph[2 * 0xE] = 0x3;
	graph[2 * 0x3] = 0xF;
	
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


