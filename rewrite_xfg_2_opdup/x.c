//  original rewrite on dwrr, 2208173.222343
//   copied, and revised, to allow for operation duplication!
//   revised on 2209084.234537,  dwrr

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h> 
#include <termios.h>
#include <math.h>
#include <errno.h>

typedef unsigned long long nat;
/*



x	1. make the graphing utility for showing the relationship between   fea and execlimit and    candidatecount


x	2. merge the 1space, 2space, and 0space functions, to all be the same function, parameterized on   .duplication_count  parameter


	3. look at previous iternary





*/

// -------- constants: --------

static const nat max_stack_size = 128; 			// maximum number of holes we can fill in the partial graph simultaneously.

static const nat max_array_size = 2048;   			// effectively infinity.  (xfg uses an infinite array)


static const nat max_mcal_length = 16;                  // maximum mcal length you can supply.


static const nat unknown = 123456789;			// some bogus value, that represents a hole. 
static const nat deadstop = unknown;			// same as above. used to mark the unknown as impossible to specify.

static const nat _ = unknown;
static const nat X = deadstop;

static const nat max_operation_count = 16;              // maximum number of instructions in the graph. 


static const nat operations[] = {6, 5, 3, 1, 2};
static const nat unique_count = sizeof operations / sizeof(nat);





struct parameters {
	nat FEA;
	nat execution_limit;
	nat required_er_count;
	nat required_le_width;

	nat window_width;
	nat scale;
	nat scratch;
	nat step;

	nat should_print;
	nat display_rate;
	nat combination_delay;
	nat frame_delay;

	nat mcal_length;
	nat mcal[max_mcal_length];

	nat duplication_count; 
	nat operation_count;
	nat graph_count;
	nat graph[4 * max_operation_count];
};


enum expansion_type  {
	no_expansion,
	firstone_expansion,
	short_expansion,
	hole_expansion,
	good_expansion,
	expansion_type_count,
};


struct stack_frame {
	nat try;
	nat option_count;
	nat pointer;
	nat mcal_index;
	nat ip;
	nat state;
	nat er_count;
	nat last_mcal_op;

	nat options[max_operation_count];
	nat array_state[max_array_size];
};

// static bool is_reset_statement(nat op) { return op == 5 or op == 6; }

static inline void clear_screen(void) { printf("\033[2J\033[H"); }

static void debug_pause() {
	printf("continue? ");
	fflush(stdout);
	getchar();
}




static nat exponentiate(const nat a, const nat b) {
	nat c = 1;
	for (nat i = 0; i < b; i++) {
		c *= a;
	}
	return c;
}



static bool is(const char* c[8], const char* _long, const char* _short) {
	return c[0] and (not strcmp(c[0], _long) or not strcmp(c[0], _short));
}

static void get_datetime(char datetime[16]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 15, "%y%m%d%u.%H%M%S", tm_info);
}

static void print_datetime() {
	char dt[16] = {0};
	get_datetime(dt);
	printf("%s\n", dt);
}


static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}

static void print_graph_as_adj(nat* graph, nat operation_count) {
	
	puts("graph adjacency list: ");
	puts("{");
	for (nat i = 0; i < operation_count * 4; i += 4) {
		const nat op = graph[i + 0];
		const nat l = graph[i + 1];
		const nat g = graph[i + 2];
		const nat e = graph[i + 3];

		printf("\t#%llu: ins(.op = %llu, .lge = [", i, op);
		if (l != unknown)	 printf("%2llu,", l); else printf("  ,");
		if (g != unknown)	 printf("%2llu,", g); else printf("  ,");
		if (e != unknown)	 printf("%2llu]", e); else printf("  ]");

		printf(")\n\n");
	}
	printf("}\n\n");
}



static void print_graph_z_value(nat* graph, nat operation_count) {
		
	for (nat i = 0; i < operation_count * 4; i += 4) {
		const nat op = graph[i + 0];
		const nat l = graph[i + 1];
		const nat g = graph[i + 2];
		const nat e = graph[i + 3];

		printf("%llu", op);
		if (l != unknown) printf("%llu", l); else printf("_");
		if (g != unknown) printf("%llu", g); else printf("_");
		if (e != unknown) printf("%llu", e); else printf("_");
	}

	printf("\n");
}


static void print_stack(struct stack_frame* stack, nat stack_count) {

	printf("printing %llu stack frames:\n{\n", stack_count);
	for (nat i = 0; i < stack_count; i++) {

		printf("FRAME #%llu:   {try=%llu, k=%llu, i=%llu, ip=%llu, state=%c, ", 
			i, 
			stack[i].try,
			stack[i].mcal_index,
			stack[i].pointer,
			stack[i].ip,
			(char) stack[i].state
		);

		print_nats(stack[i].options, stack[i].option_count);

		puts("}");
	}

	printf("}\n[end of stack]\n");

}


static nat* generate_options(
	nat* options,
	nat* option_count, 
	nat ip, 
	nat* mcal, 
	nat mcal_index, 
	nat mcal_length,
	nat comparator, 
	nat* graph, 
	nat operation_count
) {
	nat count = 0;

	for (nat option = 0; option < operation_count; option++) {
		if (ip == option) continue;
		
		if (	
			(graph[4 * option] == 6 and comparator) or        		//   dont allow 6 zero resets.
			graph[4 * option] == 2 or                                       // allow for comparator incr, at anytime.
			mcal_index >= mcal_length or                                               // if we ran outof mcal, do anything.
			graph[4 * option] == mcal[mcal_index]                                      // dont allow wrong mcal coi.

		) options[count++] = option; 
		
	}

	*option_count = count;
	return options;
}


/*


static bool run_xfg_lifetime(nat* graph, const nat timesteps, bool visualize) {

	nat array[array_size] = {0};
	bool modes[array_size] = {0};
	nat 
		comparator = 0, 
		pointer = 0, 
		ip = origin
	;

	// const nat _ = unknown;

	nat graph[20] = {
		1,  2, 3, 2,
		3,  0, 0, 4,
		2,  0, 1, 0,
		6,  1, _, 2,
		5,  2, 2, 3,
	};
	
	
	memset(array, 0, sizeof array);
	pointer = 0; comparator = 0;

	ip = origin;

	for (nat e = 0; e < timesteps; e++) {

		// printf("{{{%llu}}}\n", graph[4 * ip]);

		if (graph[4 * ip] == 1) pointer++;

		else if (graph[4 * ip] == 5) {

			if (pointer == 0) { 
				puts("ABORT: ZERO RESET!"); 
				return 1;
			}
			pointer = 0;  

			for (nat i = 0; i < array_size; i++) {
				if (visualize) {
					if (modes[i]) printf("## ");
					else { printf("   "); }
				}
				if (not array[i]) break;
			}
			if (visualize) puts("");

			memset(modes, 0, sizeof modes);
		}

		else if (graph[4 * ip] == 2) comparator++;

		else if (graph[4 * ip] == 6) {  
			if (comparator == 0) { 
				puts("ABORT: ZERO RESET!"); 
				return 1;
			}
			comparator = 0; 
		}
		else if (graph[4 * ip] == 3) { array[pointer]++; modes[pointer] = 1; }

		nat state = 0;
		if (comparator < array[pointer]) state = 1;
		if (comparator > array[pointer]) state = 2;
		if (comparator == array[pointer]) state = 3;
		nat temp = graph[4 * ip + state]; 

		if (temp == unknown) {  
			printf("ENCOUNTERED HOLE: value=%llu: %llu(4*%llu+%llu), stopping\n", 
				temp, 4 * ip + state, ip, state); 
			return 1;
		}
		
		ip = temp;
	}


	
	nat i = 0;
	for (; i < array_size; i++) {
		if (not array[i]) break;
	}
	if (i < 4) return 1;

	return 0;
}

*/



static inline bool is_complete(nat* graph, nat operation_count) {


	for (nat i = 0; i < operation_count * 4; i += 4) {
		if (graph[i + 0] == 6) {

			if (graph[i + 1] == unknown) return false;
			// <<deadstop>>
			if (graph[i + 3] == unknown) return false;

		} else {
			if (graph[i + 1] == unknown) return false;
			if (graph[i + 2] == unknown) return false;
			if (graph[i + 3] == unknown) return false;
		}
	}

	return true;
}



static inline bool uses_all_operations(nat* graph, nat operation_count) {
	// tallys up how many times a given operation is used as a dest in the graph.

	nat ops[7] = {0};

/*
ins:	  1    2     3      5      6
	------------------------------
occ	[ 0    1     0      5      0  ]
*/

	for (nat i = 0; i < operation_count * 4; i += 4) {

		if (graph[i + 1] != unknown)
			ops[graph[4 * graph[i + 1] + 0]]++;

		if (graph[i + 2] != unknown)
			ops[graph[4 * graph[i + 2] + 0]]++;

		if (graph[i + 3] != unknown)
			ops[graph[4 * graph[i + 3] + 0]]++;

	}

	if (	not ops[1] or 
		not ops[2] or
		not ops[3] or
		not ops[5] or
		not ops[6]

	) return false; else return true;
}




static nat determine_expansion_type(nat* array, nat n, nat required_le_width) { 

	if (array[0] == 1) return firstone_expansion;

	nat first = 0, last = n;
	for (;last--;)
		if (array[last]) break;
	last++;
	for (;first < last; first++)
		if (not array[first]) break; 

	if (last != first) abort();                 // holes should be impossible now!  because of the no-skip-over-zero-modnat's principle.
			//  return hole_expansion; 
	
	if (last < required_le_width) return short_expansion;

	return good_expansion;
}


/*

static void print_zs(nat graph_count, nat operation_count, nat candidate_count, nat* candidates) {
	for (nat c = 0; c < candidate_count; c++) {
		print_graph_z_value(candidates + graph_count * c, operation_count);
	}
}

*/


static nat search(struct parameters p, nat origin) {

	nat candidate_count = 0, candidate_capacity = 0;
	nat* candidates = NULL;

	nat array[max_array_size] = {0};

	nat 
		pointer = 0, 
		ip = origin
	;
	
	nat* graph = calloc(p.graph_count, sizeof(nat));
	memcpy(graph, p.graph, p.graph_count * sizeof(nat));

	const nat n = p.FEA;

	struct stack_frame* stack = calloc(max_stack_size, sizeof(struct stack_frame));
	nat stack_count = 0;

	nat mcal_index = 0;
	nat last_mcal_op = 0;
	nat er_count = 0;
	nat tried = 0;
	nat executed_count = 0;	

begin:
	while (executed_count < p.execution_limit) {

		const nat I = ip * 4;
		const nat op = graph[I];

		if (op == 1) {
			if (pointer == n) goto backtrack;        // FEA
			if (not array[pointer]) goto backtrack;  // No-Skip-Over-Zero-Modnat
			pointer++;
		}

		else if (op == 5) {
			if (last_mcal_op != 3) goto backtrack;     // PCO
			if (not pointer) goto backtrack;           // ZR-5
			pointer = 0;
			er_count++;
		}

		else if (op == 2) {
			array[n]++;
		}

		else if (op == 6) {  
			if (not array[n]) goto backtrack;       //  ZR-6
			array[n] = 0;   
		}

		else if (op == 3) {
			if (last_mcal_op == 3) goto backtrack;    // NDI
			array[pointer]++;
		}

		executed_count++;

		nat state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;

		if (op == 3 or op == 1 or op == 5) {
			if (mcal_index < p.mcal_length) {
				if (op != p.mcal[mcal_index]) goto backtrack;
				mcal_index++;
			}
			last_mcal_op = op;
		}

		if (graph[I + state] != unknown) goto next_ins;

		nat option_count = 0; 
		generate_options(
			stack[stack_count].options, &option_count, 
			ip, p.mcal, mcal_index, p.mcal_length, array[n], 
			graph, p.operation_count
		);
		
		stack[stack_count].try = 0;
		stack[stack_count].option_count = option_count;

		stack[stack_count].mcal_index = mcal_index;
		stack[stack_count].ip = ip;
		stack[stack_count].state = state;

		stack[stack_count].er_count = er_count;
		stack[stack_count].last_mcal_op = last_mcal_op;

		stack[stack_count].pointer = pointer;



		// save:
		memcpy(stack[stack_count].array_state, array, sizeof(nat) * 



				//	(n + 1)            // CORRECT   and efficient  (? untested.)




					max_array_size            // ALSO CORRECT   but wasteful/inefficient.




				);





		
		stack_count++;

		graph[I + state] = stack[stack_count - 1].options[0];
		executed_count = 0;

	next_ins:   ip = graph[I + state];

	}

	tried++;

	const nat type = 


			determine_expansion_type(array, 





				n,             // CORRECT

				
					/* ----- was exchanged with ------*/
	

			//	max_array_size, // INCORRECT






			p.required_le_width);








	const bool complete = is_complete(graph, p.operation_count);
	const bool all = uses_all_operations(graph, p.operation_count);

	if (	er_count >= p.required_er_count and
		complete and all and
		type == good_expansion and  
		mcal_index == p.mcal_length
	) {

		if (p.graph_count * (candidate_count + 1) > candidate_capacity) {
			candidate_capacity = 4 * (candidate_capacity + p.graph_count);
			candidates = realloc(candidates, sizeof(nat) * candidate_capacity);
		}

		memcpy(candidates + p.graph_count * candidate_count, graph, p.graph_count * sizeof(nat));
		candidate_count++;
	}


	if (not (tried & ((1 << p.display_rate) - 1))    and p.should_print) {
		clear_screen();

		printf("\n\t");
		print_graph_z_value(graph, p.operation_count);
		printf("\n");
		printf("----> tried [c=%llu] / t=%llu control flow graphs.\n", candidate_count, tried);

		printf("mcal_index == mcal_length? %s\n", 	mcal_index == p.mcal_length ? "true" : "false");
		printf("good_expansion? %s\n", 			type == good_expansion ? "true" : "false" );
		printf("complete? %s\n", 			complete ? "true" : "false");
		printf("all? %s\n", 				all ? "true" : "false" );
		printf("er >= req? %s\n", 			er_count >= p.required_er_count ? "true" : "false" );
		
		print_graph_as_adj(graph, p.operation_count);
		printf("searching: [origin = %llu, limit = %llu, n = %llu]\n", origin, p.execution_limit, n);
		print_stack(stack, stack_count);

		fflush(stdout);
		if (p.frame_delay) usleep((unsigned) p.frame_delay);
	}

backtrack:


	if (not stack_count) {
			// NOTE:  execution_limit must be zero!!!

			goto done;    // just finish like normal.

	}

	// revert:
	memcpy(

		array, 


			stack[

			stack_count - 1


			].

				array_state, 


			sizeof(nat)  * 

				//	(n + 1)                // <---------- we should have this. (not n!!)
								 // CORRECT   and efficient  (? untested.)

					max_array_size           // but this one is a safe alternative.
							         // ALSO CORRECT   but wasteful/inefficient.

				);



	pointer = stack[stack_count - 1].pointer;
	mcal_index = stack[stack_count - 1].mcal_index;
	er_count = stack[stack_count - 1].er_count;
	last_mcal_op = stack[stack_count - 1].last_mcal_op;

	if (stack[stack_count - 1].try < stack[stack_count - 1].option_count - 1) {
		stack[stack_count - 1].try++;
		const struct stack_frame T = stack[stack_count - 1];
		graph[4 * T.ip + T.state] = T.options[T.try];
		ip = T.options[T.try];
		executed_count = 0;
		goto begin;

	} else {		
		graph[4 * stack[stack_count - 1].ip + stack[stack_count - 1].state] = unknown;
		if (stack_count == 0) abort();
		stack_count--;
		if (stack_count == 0) goto done;
		goto backtrack;
	}
done:

	// print_zs(p.graph_count, p.operation_count, candidate_count, candidates);

	if (p.should_print) printf("tried = %llu\n", tried);
	if (p.should_print) printf("\n\n[[ candidate_count = %llu ]] \n\n\n", candidate_count);

	free(stack);
	free(candidates);
	free(graph);

	return candidate_count;
}


static bool is_unique(nat* stack_operations, const nat tried_count, nat* tried, const nat D) {

	for (nat i = 0; i < tried_count; i++) {

		nat so_used_op[7] = {0}; 
		nat t_used_op[7] = {0}; 

		for (nat e = 0; e < D; e++) so_used_op[stack_operations[e]]++;
		for (nat e = 0; e < D; e++) t_used_op[tried[D * i + e]]++;

		for (nat o = 0; o < 7; o++) 
			if (t_used_op[o] != so_used_op[o]) goto _continue;

		return false;

		_continue:;

	}
	return true;
}


//   6   5   3   1   2 
//   T   F   T   F   T




static void print_combinations(nat* tried, nat tried_count, const nat D) {

	printf("printing all combinations we are trying: \n");

	for (nat i = 0; i < tried_count; i++) {
		printf("\t%llu: ", i); 
		print_nats(tried + D * i, D); puts("");
	}

	printf("[end of combinations]\n");
}






static nat any_space_search(struct parameters p) {
	
	const nat D = p.duplication_count;
	const nat n = D - 1;
	const nat U = exponentiate(unique_count, D);
	const nat m = unique_count - 1;

	nat total = 0;
	nat tried_count = 0;
	nat* tried = calloc(U * D, sizeof(nat));

	nat entry = 0;


	if (D == 0) goto execute;
	
	nat* indicies = calloc(D, sizeof(nat));
	nat* stack_operations = calloc(D, sizeof(nat));

loop:; 	nat pointer = 0;

	for (nat op = 0; op < D; op++) stack_operations[op] = operations[indicies[op]];
	

	if (is_unique(stack_operations, tried_count, tried, D)) {
		for (nat offset = 0; offset < D; offset++) 
			tried[D * tried_count + offset] = stack_operations[offset];
		tried_count++;
	}

backtrack: 
	if (indicies[pointer] >= m) {
		indicies[pointer] = 0;
		if (pointer == n) goto done;
		pointer++;
		goto backtrack;
	}
	indicies[pointer]++;
	goto loop;

done:;

	if (p.should_print) {
		print_combinations(tried, tried_count, D);
		debug_pause();
	}
	
	for (; entry < tried_count; entry++) {

		for (nat offset = 0; offset < D; offset++) {
			p.graph[20 + 4 * offset] = tried[D * entry + offset];
		}

	execute:
		for (nat origin = 0; origin < p.operation_count; origin++) {

			if (p.graph[4 * origin] == 3) {

				total += search(p, origin);

				if (p.should_print) {
					printf("[origin = %llu]\n", origin);
					print_graph_as_adj(p.graph, p.operation_count);
					print_nats(p.mcal, p.mcal_length); 
					puts("\n");

					if (p.combination_delay == 1) debug_pause();
					if (p.combination_delay) usleep((unsigned) p.combination_delay);
				}
			}
		}
	}

	if (p.should_print) printf("\n\t[total candidates = %llu]\n\n\n", total);

	free(tried);
	free(indicies);
	free(stack_operations);
	return total;

}

static bool have_tried(nat tried_count, nat* tried, nat op1, nat op2, nat D) {      // legacy:   delete me

	if (D != 2) abort();

	for (nat i = 0; i < tried_count; i++) {

		if (	tried[D * i + 0] == op1 and tried[D * i + 1] == op2    or 
			tried[D * i + 0] == op2 and tried[D * i + 1] == op1
		) return true;
	}
	return false;
}


static nat two_space_search(struct parameters p) {
	
	const nat D = p.duplication_count;

	nat tried_count = 0;
	nat* tried = calloc(unique_count * unique_count * p.duplication_count, sizeof(nat));

	for (nat i1 = 0; i1 < unique_count; i1++) {
		for (nat i2 = 0; i2 < unique_count; i2++) {
			
			const nat op1 = operations[i1];
			const nat op2 = operations[i2];

			if (have_tried(tried_count, tried, op1, op2, D)) continue;

			tried[2 * tried_count + 0] = op1;
			tried[2 * tried_count + 1] = op2;
			tried_count++;
		}
	}

	if (p.should_print) {
		print_combinations(tried, tried_count, p.duplication_count);
		debug_pause();
	}
	
	nat total = 0;
	
	for (nat i = 0; i < tried_count; i++) {

		p.graph[20] = tried[2 * i + 0];
		p.graph[24] = tried[2 * i + 1];

		for (nat origin = 0; origin < p.operation_count; origin++) {

			if (p.graph[4 * origin] == 3) {

				total += search(p, origin);


				if (p.should_print) {
					printf("[origin = %llu]\n", origin);
					print_graph_as_adj(p.graph, p.operation_count);
					print_nats(p.mcal, p.mcal_length); 
					puts("\n");

					if (p.combination_delay == 1) debug_pause();
					if (p.combination_delay) usleep((unsigned) p.combination_delay);
				}

			}
		}
	}

	if (p.should_print) printf("\n\t[total candidates = %llu]\n\n\n", total);

	free(tried);
	
	return total;
}




static nat one_space_search(struct parameters p) {
	

	const nat D = p.duplication_count;

	nat tried_count = 0;
	nat* tried = calloc(unique_count * unique_count * D, sizeof(nat));

	for (nat i1 = 0; i1 < unique_count; i1++) {
		tried[D * tried_count] = operations[i1];
		tried_count++;
	}

	if (p.should_print)  {
		print_combinations(tried, tried_count, D);
		debug_pause();
	}
	
	nat total = 0;
	
	for (nat i = 0; i < tried_count; i++) {

		p.graph[20] = tried[D * i];

		for (nat origin = 0; origin < p.operation_count; origin++) {

			if (p.graph[4 * origin] == 3) {

				total += search(p, origin);

				if (p.should_print) {
					printf("[origin = %llu]\n", origin);
					print_graph_as_adj(p.graph, p.operation_count);
					print_nats(p.mcal, p.mcal_length); 
					puts("\n");

					if (p.combination_delay == 1) debug_pause();
					if (p.combination_delay) usleep( (unsigned) p.combination_delay); 
				}
			}
		}
	}

	if (p.should_print) printf("\n\t[total candidates = %llu]\n\n\n", total);

	free(tried);
	return total;
}


static void zero_space_search(struct parameters p) {

	nat total = search(p, 1);

	if (p.should_print) {
		printf("[origin = %llu]\n", 1ULL);
		print_graph_as_adj(p.graph, p.operation_count);
		print_nats(p.mcal, p.mcal_length); 
		puts("\n");
		printf("\n\t[total candidates = %llu]\n\n\n", total);
	}
}





static void print_help_menu() {

printf("available commands:\n"

"\n"
	"\t- dump(d) : dump the current graph as a hex string.\n"
	"\t- list(ls) : display the current partial graph as an adjacency list. \n"	
"\n"
	"\t- edit(e) : edit the parameters. \n"
	"\t- print(p)  : print the parameters. \n"
"\n"	
	"\t- search2(2) : search over all possible extensions of the current partial graph in 2-space.\n"
	"\t- search1(1) : search over all possible extensions of the current partial graph in 1-space.\n"
	"\t- search0(0) : search over all possible extensions of the current partial graph in 0-space.\n"
	"\t- searchA(A) : search over all possible extensions of the current partial graph in any (variable) duplication_count space.\n"

	"\t- plot_el(pel) : plot the relationship between execution_limit, and candidate_count, with fea held constant.\n"
	"\t- plot_fea(pfea) : plot the relationship between fea, and candidate_count, with execution_limit held constant.\n"
"\n"
	"\t- visualize(v) <file> <width> <height>: ....\n"
	"\t- generate(g) <z_list_file> <dest_image_dir> <begin_cell> <end_cell> <begin_ts> <end_ts> <maximum_whitepoint>: ....\n"
"\n"
	"\t- datetime(dt) : print the current time and date.\n"
	"\t- clear(o) : clear the screen.\n"
	"\t- help(?) : this help menu.\n"
	"\t- quit(q) : quit the XFG utility.\n"

"\n");
}


static void parse_command(const char* arguments[8], char* buffer) {
	nat length = strlen(buffer) - 1;
	buffer[length] = 0;

	for (nat i = 0, a = 0; a < 8; a++) {
		while (buffer[i] == ' ' and i < length) i++;
		arguments[a] = buffer + i;
		while (buffer[i] != ' ' and i < length) i++; 
		buffer[i++] = 0;
	}
}


static void print_parameter(const char** command, struct parameters p) {

	if (not strcmp(command[1], "all")) {
		
		printf(

			"printing all parameters values:" "\n\t"
		"\n\t"
			"FEA = %llu" "\n\t"
			"execution_limit = %llu" "\n\t"
			"required_er_count = %llu" "\n\t"
			"required_le_width = %llu" "\n\t"
		"\n\t"
			"window_width = %llu" "\n\t"
			"scale = %llu" "\n\t"
			"scratch = %llu" "\n\t"
			"step = %llu" "\n\t"
		"\n\t"
			"should_print = %llu" "\n\t"
			"display_rate = %llu" "\n\t"
			"combination_delay = %llu" "\n\t"
			"frame_delay = %llu" "\n\t"
		"\n\t"
			"mcal_length = %llu" "\n\t"
		"\n\t"
			"duplication_count = %llu" "\n\t"
			"operation_count = %llu" "\n\t"
			"graph_count = %llu" "\n\t"
		"\n\t", 
			p.FEA, p.execution_limit, p.required_er_count, p.required_le_width, 
			p.window_width, p.scale, p.scratch, p.step, 
			p.should_print, p.display_rate, p.combination_delay, p.frame_delay, 
			p.mcal_length,
			p.duplication_count, p.operation_count, p.graph_count
		);

		printf("mcal = ");
		print_nats(p.mcal, p.mcal_length); 
		puts("\n");

		print_graph_as_adj(p.graph, p.operation_count);
		puts("");

	}


	// ...
	
}

static void edit_parameter(const char** command, struct parameters* p) {
	// format:    "edit <PARAMETER-NAME> <VALUE>"

	if (not strcmp(command[1], "FEA")) p->FEA = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "execution_limit")) p->execution_limit = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "required_er_count")) p->required_er_count = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "required_le_width")) p->required_le_width = (nat) atoi(command[2]); 	

	else if (not strcmp(command[1], "window_width")) p->window_width = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "scale")) p->scale = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "scratch")) p->scratch = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "step")) p->step = (nat) atoi(command[2]); 

	else if (not strcmp(command[1], "should_print")) p->should_print = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "display_rate")) p->display_rate = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "combination_delay")) p->combination_delay = (nat) atoi(command[2]); 
	else if (not strcmp(command[1], "frame_delay")) p->frame_delay = (nat) atoi(command[2]); 

	else if (not strcmp(command[1], "mcal_length")) p->mcal_length = (nat) atoi(command[2]); 

	else if (not strcmp(command[1], "duplication_count")) {
		p->duplication_count = (nat) atoi(command[2]); 
		p->operation_count = 5 + p->duplication_count;
		p->graph_count = 4 * (5 + p->duplication_count);

	} else printf("error: unknown parameter: %s\n", command[1]);
}











//  set   p.scratch    to the minimum el,   set    p.execution_limit to  the maximum el,    
//  and set p.step to the stride width/size, to go through el possibilities at.
//  set p.window_width to the maximum number of characters to print per line, 
// set  p.scale   to the  maximum total candidate_count   that you expect to see.

// note: the plot/graph is printed downwards, and sideways. 

static void plot_el(struct parameters p) {
	
	const nat max_el = p.execution_limit;

	printf("plotting EL v.s. candcount : [maxl_el = %llu, fea = %llu]\n", max_el, p.FEA);

	for (nat el = p.scratch; el < max_el; el += p.step) {

		p.execution_limit = el;
		p.should_print = false;

		const nat total = one_space_search(p);
		const double fraction =  ((double)total / (double) p.scale);
		const nat space_count = (nat)(fraction * (double) p.window_width);
		printf("%10llu : ", total);
		printf("%10llu : ", el);
		for (nat i = 0; i < space_count; i++) putchar(' '); puts("@");
	}

	puts("[done]");
}







//  set   p.scratch    to the minimum fea,   set    p.FEA to  the maximum fea,    
//  and set p.step to the stride width/size, to go through fea possibilities at.
//  set p.window_width to the maximum number of characters to print per line, 
// set  p.scale   to the  maximum total candidate_count   that you expect to see.

// note: the plot/graph is printed downwards, and sideways. 

static void plot_fea(struct parameters p) {
	
	const nat max_fea = p.FEA;

	printf("plotting FEA v.s. candcount : [max_fea = %llu, EL = %llu]\n", max_fea, p.execution_limit);

	for (nat fea = p.scratch; fea < max_fea; fea += p.step) {

		p.FEA = fea;
		p.should_print = false;

		const nat total = one_space_search(p);
		const double fraction =  ((double)total / (double) p.scale);
		const nat space_count = (nat)(fraction * (double) p.window_width);
		printf("%10llu : ", total);
		printf("%10llu : ", fea);
		for (nat i = 0; i < space_count; i++) putchar(' '); puts("@");
	}

	puts("[done]");
}








int main() {

	static struct parameters p = {

		.FEA = 300, 
		.execution_limit = 3000, 

		.required_er_count = 6, 
		.required_le_width = 5,

		.window_width = 100,
		.scale = 7000,
		.scratch = 0,
		.step = 1,
	
		.should_print = 1,
		.display_rate = 15,
		.combination_delay = 1,
		.frame_delay = 0,

		.mcal_length = 6,
		.mcal = { 3, 1,  3, 5,  3, 1 },

		.duplication_count = 1,
		.operation_count = 5 + 1,
		.graph_count = 4 * (5 + 1),

		.graph = { 
			1,  2, 3, _,
			3,  0, _, _,
			2,  0, _, _,
			6,  1, X, _,
			5,  _, _, _,

			0,  _, _, _,
			0,  _, _, _,
			0,  _, _, _,
			0,  _, _, _,
			0,  _, _, _,
		},
	};

	puts("this is a rewrite of the xfg search utility, "
		"that uses a backwards approach to searching for graphs.");

	char buffer[4096] = {0};
	
	
loop: 	printf(":: ");
	fgets(buffer, sizeof buffer, stdin);
	const char* command[8] = {0};
	parse_command(command, buffer);

	if (is(command, "", "")) {}
	else if (is(command, "quit", "q")) goto done;

	else if (is(command, "debug_command", "dc")) {
		printf("\n{ \n\t");
		for (nat a = 0; a < 8; a++) {
			printf("[%llu]:\"%s\", ", a, command[a]);
		}
		printf("\n}\n\n");
	}
	else if (is(command, "help", "?")) print_help_menu();
	else if (is(command, "clear", "o")) clear_screen();
	else if (is(command, "datetime", "dt")) print_datetime();
	else if (is(command, "dump", "d")) print_graph_z_value(p.graph, p.operation_count);
	else if (is(command, "list", "ls"))  print_graph_as_adj(p.graph, p.operation_count);

	else if (is(command, "print", "p")) print_parameter(command, p);
	else if (is(command, "edit", "e")) edit_parameter(command, &p);

	else if (is(command, "search2", "2")) two_space_search(p);
	else if (is(command, "search1", "1")) one_space_search(p);
	else if (is(command, "search0", "0")) zero_space_search(p);
	else if (is(command, "searchA", "A")) any_space_search(p);

	else if (is(command, "plot_el", "pel")) plot_el(p);
	else if (is(command, "plot_fea", "pfea")) plot_fea(p);

	else printf("error: unknown command.\n");




	/*

	else if (is(command, "visualize", "v")) 
		visualize_set(
			command[1], 
			graph, origin, 
			(nat)atoi(command[2]), (nat)atoi(command[3])
		);

	else if (is(command, "generate", "g")) 
		generate_lifetime_images(
			command[1], command[2], 
			graph, origin, 
			(nat)atoi(command[3]), (nat)atoi(command[4]), 
			(nat)atoi(command[5]), (nat)atoi(command[6]),
			(nat)atoi(command[7])
		);

	*/


	goto loop; done:;
}








































































































































































/*






void two_space() {
	

		nat total = 0;
	nat tried[25 * 2] = {0};
	nat tried_count = 0;

	const nat duplication_count = 2;
	const nat operations[] = {6, 5, 3, 1, 2};	

	for (nat i1 = 0; i1 < 5; i1++) {
		for (nat i2 = 0; i2 < 5; i2++) {

			
			const nat op1 = operations[i1];
			const nat op2 = operations[i2];

			if (have_tried(tried_count, tried, op1, op2)) continue;


			tried[2 * tried_count + 0] = op1;
			tried[2 * tried_count + 1] = op2;
			tried_count++;



			//  if (is_reset_statement(op1) and is_reset_statement(op2)) continue;

			




			

			nat mcal[] = { 3, 1,  3, 5,  3, 1};
			const nat mcal_length =  sizeof mcal / sizeof(nat);

			const nat operation_count = 5 + duplication_count;
			const nat graph_count = 4 * operation_count;





			nat graph[graph_count] = { 
				1,  2, 3, _,
				3,  0, _, _,
				2,  0, _, _,
				6,  1, X, _,
				5,  _, _, _,

				op1,  _, _, _,
				op2,  _, _, _,
			};





			total += search(1, mcal, mcal_length, graph, graph_count, operation_count);

			sleep(2);



		
			if (op1 == 3)  {

				total += search(5, mcal, mcal_length, graph, graph_count, operation_count);    

				sleep(2);

			}

		
		}
	}

	printf("\n\n\t\ttotal candidates = %llu\n\n", total);


}




*/









/* todo:   2210016.173927  iternary


	try different params for   fea  arrray sizee,  exec limt, etc.  

	look at the lifetimes!   print all 784 cand for 1space. 

*/

// 2space = total candidates = 9536595


// new pruning metrics:
	//    NDI :    double modnat incrs      (ban!)
	//     FEA constraints!!!!!   (make sure it works at the fea, if it segfaults, then discard it.  (finite array, but xfg graph!!)
	//    i dont think we need to do empirical reducing to the R...?
	//    look at the lifetimes of the candidates, and see what the candidates look like!    (most important!)









/*


			nat graph[graph_count] = { 
				1,  2, 3, _,
				3,  0, 0, 4,
				2,  0, 1, 1,
				6,  1, X, _,
				5,  _, 6, _,

				2,  0, 1, 1,
				3,  0, 0, 4,
			};










// --------  global parameters:--------- 

static nat mcal_length = 6;
static nat mcal[max_mcal_length] = { 3, 1,  3, 5,  3, 1};



static  nat FEA = 300;				// artificial the finite end of the array, used for testing 
							// if a graph segfaults by accessing memory outside of the 
							// this imposed bound on the "infinite" array (whos total 
							// real memory size is given by array_size)
							// note: this value must be less than array_size.
							// note: this is the value of n, being simulated- but without the branch 9.
							// also the location of *n, of course.

static  nat execution_limit = 3000;   		// untested value, tweak this
							// the number of instructions to execute while trying to fill in holes in the graph.



static  nat required_er_count = 6;			// the required number of ER's that need to happen during exec, to be good.

static  nat required_le_width = 5; 		// the required index of the Leading Edge in order to not classify the XP as bad.
							// (looking at what is the width of the expansion pattern of the graph)


							// expansion pattern as a short_expansion.







static  nat display_rate = 13;			// inverse logarithmically related to how fast you print out the status info
							// while searching.







*/







// 2209294.100710   implementing:   fea, ndi, pco       constraints, now!

/*





:: p all 
printing all parameters values:
	
	FEA = 300
	execution_limit = 10000000
	required_er_count = 6
	required_le_width = 5
	
	window_width = 150
	scale = 7000
	scratch = 1000000
	step = 1000000
	
	should_print = 0
	display_rate = 2
	combination_delay = 1
	frame_delay = 100
	
	mcal_length = 6
	
	duplication_count = 1
	operation_count = 6
	graph_count = 24
	
	mcal = (6)[ 3 1 3 5 3 1 ]

graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3,  ])

	#4: ins(.op = 3, .lge = [ 0,  ,  ])

	#8: ins(.op = 2, .lge = [ 0,  ,  ])

	#12: ins(.op = 6, .lge = [ 1,  ,  ])

	#16: ins(.op = 5, .lge = [  ,  ,  ])

	#20: ins(.op = 0, .lge = [  ,  ,  ])

}


:: pel
plotting EL v.s. candcount : [maxl_el = 10000000, fea = 300]
      3009 :    1000000 :                                                                 @
      2727 :    2000000 :                                                           @
      2708 :    3000000 :                                                           @
      2685 :    4000000 :                                                          @
      2683 :    5000000 :                                                          @
      2684 :    6000000 :                                                          @
      2684 :    7000000 :                                                          @
      2684 :    8000000 :                                                          @
      2684 :    9000000 :                                                          @
[done]
:: 





			2210086.141358
 
						the size of   1space, according to this utility,  seems to be around   


										the 2684 mark!!





							very interesting!


							now, to look at   making the   fea_plotter!


					yay







*/
































/*
		if (
				tried[2 * i + 0] == op1 and tried[2 * i + 1] == op2    or 

			tried[2 * i + 0] == op2 and tried[2 * i + 1] == op1

		) return false;
	*/




