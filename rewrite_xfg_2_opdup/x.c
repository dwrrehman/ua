//  original rewrite on dwrr, 2208173.222343
//   copied, and revised, to allow for operation duplication!
//   revised on 2209084.234537,  dwrr

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>




// 2space = total candidates = 9536595


// new pruning metrics:

	//    NDI :    double modnat incrs      (ban!)


	//     FEA constraints!!!!!   (make sure it works at the fea, if it segfaults, then discard it.  (finite array, but xfg graph!!)


	//    i dont think we need to do empirical reducing to the R...?

	//    look at the lifetimes of the candidates, and see what the candidates look like!    (most important!)

	// 



typedef unsigned long long nat;


// constants:

static const nat max_stack_size = 128; 			// maximum number of holes we can fill in the partial graph simultaneously.

static const nat array_size = 400;   			// effectively infinity.  (xfg uses an infinite array)


static const nat FEA = 300;				// artificial the finite end of the array, used for testing 
							// if a graph segfaults by accessing memory outside of the 
							// this imposed bound on the "infinite" array (whos total 
							// real memory size is given by array_size)
							// note: this value must be less than array_size.
							// note: this is the value of n, being simulated- but without the branch 9.
							// also the location of *n, of course.

static const nat execution_limit = 3000;   		// untested value, tweak this
							// the number of instructions to execute while trying to fill in holes in the graph.
static const nat unknown = 123456789;			// some bogus value, that represents a hole. 
static const nat deadstop = unknown;			// same as above. used to mark the unknown as impossible to specify.

static const nat required_er_count = 6;			// the required number of ER's that need to happen during exec, to be good.

static const nat required_le_width = 5; 		// the required index of the Leading Edge in order to not classify the XP as bad.
							// (looking at what is the width of the expansion pattern of the graph)


							// expansion pattern as a short_expansion.

static const nat display_rate = 13;			// inverse logarithmically related to how fast you print out the status info
							// while searching.




static const nat max_operation_count = 10;              // maximum number of instructions in the graph. 


static const nat _ = unknown;
static const nat X = deadstop;



// used for partially specifying the mcal further, based on "subtractive" (negated) specification.
// static const nat NOT_5 = 131313131;
// static const nat NOT_1 = 353353535;
// static const nat NOT_3 = 151515151;


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
	nat k;
	nat ip;
	nat state;
	nat zero_reset_happened;
	nat er_count;
	nat ndi_happened;
	nat last_mcal_op;

	nat options[max_operation_count];
	nat array_state[array_size];
};


static inline void clear_screen() { printf("\033[2J\033[H"); }


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
			stack[i].k,
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
	nat* os, 
	nat k, 
	nat os_length,
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
			k >= os_length or                                               // if we ran outof mcal, do anything.
			graph[4 * option] == os[k]                                      // dont allow wrong mcal coi.

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




static nat determine_expansion_type(nat* array, nat n) { 

	if (array[0] == 1) return firstone_expansion;

	nat first = 0, last = n;
	for (;last--;)
		if (array[last]) break; 
	last++;
	for (;first < last; first++)
		if (not array[first]) break; 

	if (last != first) return hole_expansion;
		
	if (last < required_le_width) return short_expansion;

	return good_expansion;
}



static void print_zs(nat graph_count, nat operation_count, nat candidate_count, nat* candidates) {
	for (nat c = 0; c < candidate_count; c++) {
		print_graph_z_value(candidates + graph_count * c, operation_count);
	}
}





static nat search(nat origin, nat* os, const nat os_length, nat* graph, nat graph_count, nat operation_count) {
	
	nat candidate_count = 0, candidate_capacity = 0;
	nat* candidates = NULL;

	nat* array = calloc(array_size, sizeof(nat));
	nat 
		pointer = 0, 
		ip = origin
	;

	struct stack_frame* stack = calloc(max_stack_size, sizeof(struct stack_frame));
	nat stack_count = 0;

	nat k = 0;

	nat last_mcal_op = 0;
	nat er_count = 0;
	nat tried = 0;
	nat executed_count = 0;	

begin:
	while (executed_count < execution_limit) {

		const nat I = ip * 4;
		const nat op = graph[I];

		if (op == 1) {
			pointer++;
			if (pointer > FEA) goto backtrack;        // FEA
		}
		else if (op == 5) {
			if (last_mcal_op != 3) goto backtrack;     // PCO
			if (not pointer) goto backtrack;           // ZR-5
			pointer = 0;
			er_count++;
		}

		else if (op == 2) {
			array[FEA]++;
		}

		else if (op == 6) {  
			if (not array[FEA]) goto backtrack;       //  ZR-6
			array[FEA] = 0;   
		}

		else if (op == 3) {
			if (last_mcal_op == 3) goto backtrack;    // NDI
			array[pointer]++;
		}

		executed_count++;

		nat state = 0;
		if (array[FEA] < array[pointer]) state = 1;
		if (array[FEA] > array[pointer]) state = 2;
		if (array[FEA] == array[pointer]) state = 3;

		if (op == 3 or op == 1 or op == 5) {
			if (k < os_length) {
				if (op != os[k]) goto backtrack;
				k++;
			}
			last_mcal_op = op;
		}

		if (graph[I + state] != unknown) goto next_ins;

		nat option_count = 0; 
		generate_options(stack[stack_count].options, &option_count, ip, os, k, os_length, array[FEA], graph, operation_count);
		
		stack[stack_count].try = 0;
		stack[stack_count].option_count = option_count;

		stack[stack_count].k = k;
		stack[stack_count].ip = ip;
		stack[stack_count].state = state;

		stack[stack_count].er_count = er_count;
		stack[stack_count].last_mcal_op = last_mcal_op;

		stack[stack_count].pointer = pointer;
		memcpy(stack[stack_count].array_state, array, sizeof(nat) * array_size);
		
		stack_count++;

		graph[I + state] = stack[stack_count - 1].options[0];
		executed_count = 0;

	next_ins:   ip = graph[I + state];

	}

	tried++;

	const nat type = determine_expansion_type(array, array_size - 1);
	const bool complete = is_complete(graph, operation_count);
	const bool all = uses_all_operations(graph, operation_count);

	if (	er_count >= required_er_count and
		complete and all and
		type == good_expansion and  
		k == os_length
	) {

		if (graph_count * (candidate_count + 1) > candidate_capacity) {
			candidate_capacity = 4 * (candidate_capacity + graph_count);
			candidates = realloc(candidates, sizeof(nat) * candidate_capacity);
		}

		memcpy(candidates + graph_count * candidate_count, graph, graph_count * sizeof(nat));
		candidate_count++;
	}


	if (not (tried & ((1 << display_rate) - 1))) {
		clear_screen();

		printf("\n\t");
		print_graph_z_value(graph, operation_count);
		printf("\n");
		printf("----> tried [c=%llu] / t=%llu control flow graphs.\n", candidate_count, tried);

		printf("k == os_length? %s\n", 	k == os_length ? "true" : "false");
		printf("good_expansion? %s\n", 	type == good_expansion ? "true" : "false" );
		printf("complete? %s\n", 	complete ? "true" : "false");
		printf("all? %s\n", 		all ? "true" : "false" );
		printf("er >= req? %s\n", 	er_count >= required_er_count ? "true" : "false" );
		
		print_graph_as_adj(graph, operation_count);
		printf("searching: [origin = %llu, limit = %llu]\n", origin, execution_limit);
		print_stack(stack, stack_count);

		fflush(stdout);

		// usleep(10000);
	}

backtrack:

	memcpy(array, stack[stack_count - 1].array_state, array_size * sizeof(nat));
	pointer = stack[stack_count - 1].pointer;
	k = stack[stack_count - 1].k;
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

	// print_zs(graph_count, operation_count, candidate_count, candidates);
	print_nats(os, os_length); puts("");
	
	printf("tried = %llu\n", tried);
	print_graph_as_adj(graph, operation_count);
	printf("\n\n[[ candidate_count = %llu ]] \n\n\n", candidate_count);

	free(array);
	free(stack);
	free(candidates);
	return candidate_count;
}




static bool is_reset_statement(nat op) { return op == 5 or op == 6; }

static bool have_tried(nat tried_count, nat* tried, nat op1, nat op2) {

	for (nat i = 0; i < tried_count; i++) {
		if (
			tried[2 * i + 0] == op1 and tried[2 * i + 1] == op2    or 

			tried[2 * i + 0] == op2 and tried[2 * i + 1] == op1

		) {
			return true;
		}
	}
	return false;
}




// 2209294.100710   implementing:   fea, ndi, pco       constraints, now!

/*


*/




int main() {

	puts("this is a rewrite of the xfg search utility, "
		"that uses a backwards approach to searching for graphs.");

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



			//  if (is_reset_statement(op1) /*and is_reset_statement(op2)*/) continue;

			




			

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



		/*
			if (op1 == 3)  {

				total += search(5, mcal, mcal_length, graph, graph_count, operation_count);    

				sleep(2);

			}

			*/
		}
	}

	printf("\n\n\t\ttotal candidates = %llu\n\n", total);
}























// for the mcal,     when we are past it, 
			// we can keep track of the previous mcal operation that we said, and if 
			// we see the sequence 15 in it at all, then we know its bad. even if we are OUTSIDE the mcal. 
			//   ie, if you say 5, you better have nottttt said an mcal op =   1   prior, becaues thats not using your pointer very well.
				// this is implementing the pointer coi opt constraint. 

































/*


	---------- 1-SPACE -------------

		you cant duplicate a reset operation, because then 

	

			1 2 3 5 6    +    5/6
			I I I R R          R


				x3 I's      and x3 R's,     and    because  # R's >= # I's        then no candidates.




	---------- 2-SPACE -------------

		you cant duplicate  two operations     which are both reset operations.

	

			1 2 3 5 6    +    5/6    5/6
			I I I R R          R      R


				x3 I's      and x3 R's,     and    because  # R's >= # I's        then no candidates.







	... i think.... i'm not exactly sure why this invariant/constraint holds... but it seems like empirically it has so far?

			not sure what the exact equation/constraint really is though 


						or why it is working that way 






*/







































































































	// printf("[Printing the graphs that we found]: \n");
	// for (nat i = 0; i < candidate_count; i++) {
		// printf("CANDIDATE #%llu: ", i);
		// print_graph_as_adj(candidates[i], operation_count);
	// }
	// printf("[end of candidates]\n");




/*
	// memcpy(graph, , sizeof(nat) * operation_count * 4);
	// printf("[candidate #%llu/%llu]\n", c, candidate_count);
	printf("run xfg lifetime of graph? (y to confirm) ");
	if (getchar() == 'y') run_xfg_lifetime(graph, 20000, 1);
	getchar();
	*/



// ------------------------ 3->6  R ----------------------------

	/*nat _graph[20] = {
		1,  2, 1, _,
		3,  _, 3, _,
		2,  0, _, _,
		6,  0, _, _,
		5,  _, _, _,
	};*/



	

	// ------------------------ 6->3  R ----------------------------

	/*nat graph[20] = {
		1,  2, 3, _,
		3,  0, _, _,
		2,  0, _, _,
		6,  1, _, _,
		5,  _, _, _,
	};*/








/*(graph[4 * 4 + 1] != unknown or 
		graph[4 * 4 + 2] != unknown or 
		graph[4 * 4 + 3] != unknown) */



	/*
	if (candidate_count == 0) {
		printf("\n\n\t\t MCAL CONTRADICTION\n\n\n");
	} else {
		printf("\n\n\t\t MCAL satisfied\n\n\n");
	}
	*/




/*

static const nat operation_count = 5;


static void find_extenstions_of_candidate() {


	// ------------------------ partial xfg candidate MCAL313531 ----------------------------
	// wrote on 2208291.101021

	const nat _ = unknown;
	nat graph[20] = {
		1,  2, 3, 2,
		3,  0, _, 4,
		2,  0, 1, 1,
		6,  1, _, _,
		5,  2, 2, 3,
	};

	2208291.102649     so i found two viable hole options for this graph, for the graph[15] hole (ie, 6(=).)

			they are:           1       and        2          ie,   the instructions   3    and    2


			they both look good,    however one took longer to find the other holel(graph[6]) than the other one. 


				so yeah

			lets look at hole 6, that they both found,       and find all options for it 

		
			only valid values for  hole15, are    1   and   2

			i am allowing valid values of:  {0, 1, 2, 3, 4}  for hole6.  so anything. 

		

	heres the combinations:

	2208291.103522

		[15] = 1   [6] = 0       bad lifetime ish    [CONFIRMED:  very bad lifetime.]

x		[15] = 1   [6] = 1         ZERO RESET     [CONF]

x		[15] = 1   [6] = 2       never said 5 ever again   [CONF]

***		[15] = 1   [6] = 3       very good lifetime! very varied   2208291.103950      [BAD!! lots of lines.]

x		[15] = 1   [6] = 4       zero reset. not good.    [CONF]


	
		

		[15] = 2   [6] = 0        bad lifetime ish...   [CONF:  lots of lines in the lifetime!]

x		[15] = 2   [6] = 1       zero reset   [c]

x		[15] = 2   [6] = 2       never 5 said 5 again    [c]

***		[15] = 2   [6] = 3       decent lifetime, minimal clumping, better ish    [CONF:  very very bad lifetime.]

x		[15] = 2   [6] = 4      zero reset


		

		

			so it seems like the relevant candidates would be 


					[15,6] =  1, 3     (best candidate)

					[15,6] =  2, 3       (also okay)


	
			





	//holes: 6 15 
	
	for (nat i = 1; i <= 2; i++) {
		graph[15] = i;
		run_xfg_lifetime(graph, 2000, 1);
		print_graph_as_adj(graph, operation_count);
		printf("trying %llu option for [15]: continue? (y) ", i);
		if (getchar() != 'y') break;
		getchar();

		for (nat j = 0; j < 5; j++) {
			graph[6] = j;
			run_xfg_lifetime(graph, 1000000, 1);
			print_graph_as_adj(graph, operation_count);
			printf("trying %llu option for [6]: continue? (y) ", j);
			if (getchar() != 'y') break;
			getchar();
		}

		printf("finished all combinations for [6]!\n");

		graph[6] = _;
	}

	graph[15] = _;

}




static void find_ext_4_var() {

	const nat _ = unknown;
	nat graph[20] = {
		1,  2, 3, 2,
		3,  0, _, 4,
		2,  0, 1, 1,
		6,  1, _, _,
		5,  _, _, 3,
	};


	// holes:    6, 15, 17, 18


	nat total = 0, cand = 0;
	
	for (nat i15 = 0; i15 < 5; i15++) {
		for (nat i6 = 0; i6 < 5; i6++) {
			for (nat i17 = 0; i17 < 5; i17++) {
				for (nat i18 = 0; i18 < 5; i18++) {

					total++;
					puts("****************************");

					graph[15] = i15;
					graph[6] = i6;
					graph[17] = i17;
					graph[18] = i18;

					if (run_xfg_lifetime(graph, 10000, 0)) continue;

					run_xfg_lifetime(graph, 10000, 1);

					print_graph_as_adj(graph, operation_count);

					printf("c=%llu,t=%llu: [15:%llu,6:%llu,17:%llu,18:%llu]: continue? (y) ", 
							cand, total, i15,i6,i17,i18);

					if (getchar() != 'y') break;
					getchar();

					cand++;
				}
				graph[15] = _;
			}
			graph[15] = _;
		}
		graph[15] = _;
	}
	graph[15] = _;


	printf("finally: total cand graphs: c=%llu : t=%llu\n", cand, total);
}







static void display_hole6_options() {

	const nat _ = unknown;
	nat graph[20] = {
		1,  2, 3, 2,
		3,  0, _, 4,  // hole 6
		2,  0, 1, 1,
		6,  1, _, 1,  
		5,  2, 3, 3,
	};

	// holes:    6 only 
	
	// trying 6 possibilities  on 2208291.114217

	for (nat i6 = 0; i6 < 5; i6++) {
		puts("****************************");
		graph[6] = i6;
		run_xfg_lifetime(graph, 1000000, 1);
		print_graph_as_adj(graph, operation_count);
		printf("showing: [15:1,6:%llu,17:2,18:3]: continue? (y) ", i6);
		if (getchar() != 'y') break;
		getchar();		
	}


}



*/




	// 2208291.115729
	/*
		i think operation duplication is required to look at now!

			we looked at all 5 options for hole6,   and none of them were good. they all had lots of lines. 

			so i think we need to rewrite the utility to handle operation duplcation now!
	
			so that will be fun. 

			
	*/





/*

	we are searching over this partial xfg           which is MCAL313531    +     2(=) --> 3



	nat graph[20] = {
		1,  2, 3, 2,
		3,  0, _, 4, // hole6      3(>)
		2,  0, 1, 1,
		6,  1, _, _,    // hole 15      6(=)
		5,  _, _, 3,    // holes 17 and 18,       5(<)    5(>) 
	};






2208291.111816

 found a potential candidate    with a good lifetime:    [15:1,6:0,17:2,18:3]:

2208291.112010
 
	found another thats interesting:    [15:1,6:1,17:2,18:3]:

2208291.112337

	found another decent one!       [15:1,6:2,17:2,18:3]: 


2208291.112629

	found a (kindaaa) okay one.. nothing amazing...     [15:1,6:3,17:2,18:2]
	oh wait we already went over this possibility already!?
		so lets ditch this one. 


2208291.112737

	found another good one!!!          [15:1,6:3,17:2,18:3]:

		sensing a pattern?.... lol


2208291.112954

	andddd found another, probably the last one:     [15:1,6:4,17:2,18:3]:






*/






/*
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1, 0])

	#12: ins(.op = 6, .lge = [ 1,  , 2])

	#16: ins(.op = 5, .lge = [ 2,  , 3])
	







 1, 3, 5, 

		1, 3,

// wrote on 2208195.032113





	-------------------- assuming the       3 -> 6 R  ------------------------



// using mcal:       31 35 31 

CANDIDATE #2: printing graph as adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 1, 2])

	#1: ins(.op = 3, .lge = [ 2, 3, 4])

	#2: ins(.op = 2, .lge = [ 0, 1, 3])

	#3: ins(.op = 6, .lge = [ 0,  ,  ])

	#4: ins(.op = 5, .lge = [  ,  , 1])

}
	only candidate, i think 






---------------------------- assuming      6 -> 3  R --------------------


we got these two!!

	using    mcal of    31 35 31 





CANDIDATE #0: graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1,  ])

	#12: ins(.op = 6, .lge = [ 1,  ,  ])

	#16: ins(.op = 5, .lge = [  ,  , 1])

}
		this one is bad,  bad lifetime.





CANDIDATE #1: graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1,  ])

	#12: ins(.op = 6, .lge = [ 1,  ,  ])

	#16: ins(.op = 5, .lge = [  ,  , 3])

}   *******************************************************************************************


		this one found a hole, 






now,   supplying     31 35 31 1 35


	gives us instead,     (using  63R of course)



1 candidate:



CANDIDATE #0: graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1, 0])

	#12: ins(.op = 6, .lge = [ 1,  , 2])

	#16: ins(.op = 5, .lge = [  ,  , 3])

}


	this one has a good so far lifetime,   found a hole,  after w#2.







if i now give what i think is the beginning of walk 3,  so 


	mcal = 3, 1, 3, 5,  

		3, 1, 1, 3, 5, 

		1, 3,





	
then we get: 2 candidates

CANDIDATE #0: graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1, 0])

	#12: ins(.op = 6, .lge = [ 1,  , 2])

	#16: ins(.op = 5, .lge = [ 0,  , 3])

}

CANDIDATE #1: graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1, 0])

	#12: ins(.op = 6, .lge = [ 1,  , 2])

	#16: ins(.op = 5, .lge = [ 2,  , 3])

}



	very interesting!!!



oh wait 




		oh my god


		the first one


				its actually very interesting 



					it has the largest     good looking   lifetime ive ever seen



									ever

					basically 





					its very good looking, even expands logarithimically 



						has a couple holes, but never seems to say them... which is probably a bug
 lol

				like the     3 [.g]        that seems like a hole that shoulddddd be filled?.. kinda?



		idk 



	but the other one,    CANDIDATE #1/2    actually just immediately hit a hole sometime after walk number 4


		


				found both of them,     at     2208195.011730



				







	okay, so i am going to actually try supplying     5 -> 1    on greater than, 


	because thats the hole it encounters!


		and,   5 ->1 on >  seems like the obvious connection to make given the fact that its semanically the "Would-ER" case,   and thus, saying 1 is actually a good thing, to garentee 5 coi!


			so yeah, im going code up that modified graph now,   and try running its lifetime 






CANDIDATE #1: graph adjacency list: 
{
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1, 0])

	#12: ins(.op = 6, .lge = [ 1,  , 2])

	#16: ins(.op = 5, .lge = [ 2,  , 3])

}
heres the graph    ^^^

on 2208195.030705











// supplying 5 -> 2 (>)     we found the hole,   for 3 [.g]!!!!!   
	// so lets find all possibilities for it!!

	//     3 -> 1, 3 -> 2,   and 3 -> 6
	/// 3 -> 1  is very good,  cant distinguish if its bad at all.... so yeah.
	// 3 -> 2    has an infinite loop between    3 and 2. very bad. 
	/// 3 -> 6        this one is awesome!    very good, cant distinguish whether or not its the xfg.   2208195.035523   roughly tried on that time





	// so yeah, i think that means that 3 -> 


	// memcpy(graph, candidates[c], sizeof(nat) * 20);








*/


























































/*     --------------------------------------------------------------------------------------------------------








		//1. execute graph ip .op
		//2. look at state of *n and *i,
		//3. find corresp. Tern_Br case, 
		//3.1.  if not hole,  then goto,   and repeat from top.
		//3.2.  if hole,  then generate option list (based on   NZ(*n), os[k],   
		//					and anything else that could help prune our options for this hole.
		//3.2.(cont).      then, sub in opt [0],  and goto it,   and resume execution. 
		// when there are no more holes, 
		//	wait no 
		//
		//			when    k == length           
		//	then we finished validating the entire modnat coi abs lifetime (MCAL)    
		//		and we have a finished graph, whatever it is. 




struct option {
	nat address;
	bool says_5;   // if this is true, then we do this whole process again immed after doing this process for Br(t), but instead for the dest of 5.

			// if says_5 is only set when a   Br(t)   option   newly creates 5,   (ie it wasnt in the graph, prior)


	
			// if says_5,     then we need to add the    operation 5    instruction  into the graph. 
							//     graph[graph_count++] = (instruction){.op=5, ....etc};

	   // this is destination for Br(t) (" optimized away split edge")    or     5(f)      (if says_5=true)

		    /// this might also, at some point,   (probably right after the says_5=true thing)   be the address for where 5 goes. 

};















	os,l       is the behaviourrrr that we actually want to happen, 
				(at least, in terms of 3's,  1's, and 5's)


	
	we are executing the R-extended graph, starting from a fresh R,

		and we are TRYING TO GET  the    os,l   behaviour,    for   1/3/5


		CASE 1:  if we ever dont get it,     :::


				thennnn we need to add a branch.

		CASE 2:  if we ever do a *n zero reset,  :::

				then we know that we need to try a 
				different option for where the outcomes of a 
				branch go, or we need to do a branch. 



		


note:  
	we arent assuming  that    "X"   in the R  is any particular branch. 

	it is abstract, just like any other branch that we would add. 




		

			


				this is literally an unknown that we need to search over, before we even start running the graph. 



  // the    Pure_X   branch      present already in the R





		// we can just assume the combined branch operation model!! and that simplifies the problem drastically!!!















		l := less than 

		g :=  greater than 

		e :=  equal to 

		u :=   unconditional         (used for operations)

	












			NOTE:    this utility assumes that in the R graph:

							3 ---> 6 

	
						not the other way around. 


		




	this function takes a 3-1-5 OS  and finds all R-extension PBA graphs that are consistent with that OS

	[	OS := operation sequence
		PBA := partially branch abstract    	]



	


	this function assumes the rrxfg, which is:

			

	      6          1
		o-----> o <-----+
		^       |       |
		|       v       |
		o <--t-(X)-t--> o
              3                   2











































	nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));
	nat pointer = 0;

	byte i = origin;

	nat zero_reset_happened = false;
	nat er_count = 0;
	

	byte parent = 0;   
	bool parent_side = false;
	
	struct stack_frame* stack = calloc(max_stack_size, sizeof(struct stack_frame));
	nat stack_count = 0;
	nat executed_count = 0;

begin:
	while (executed_count < execution_limit) {
	
		
		if (i == 0) {
			struct options options = generate(parent, parent_side);
			
			struct stack_frame frame;
			frame.try = 0; 
			frame.source = parent;      // keep track of the origin/parent of the hole.
			frame.side = parent_side;
			frame.options = options;

			memcpy(frame.array_state, array, sizeof(nat) * array_size);
			frame.pointer_state = pointer;
			frame.zero_reset_happened = zero_reset_happened;
			frame.er_count = er_count;

			stack[stack_count++] = frame;
			if (stack_count == max_stack_size) { printf("error: stack overflow\n"); abort(); }
			
			
			executed_count = 0;
			graph[2 * parent + parent_side] = options.options[0];
			i = options.options[0];

		} else {

			executed_count++;
			parent = i;
			parent_side = false;
			
		
			if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[pointer])];
				parent_side = (array[n] < array[pointer]);

			} else if (i == 0xC) {
				i = graph[i * 2 + (array[n] != array[pointer])];
				parent_side = (array[n] != array[pointer]);

			} else if (i == 0xF) {
				i = graph[i * 2 + (array[n] > array[pointer])];
				parent_side = (array[n] > array[pointer]);
			}


			else if (i == 1) { pointer++; 				i = graph[i * 2];  }
			else if (i == 2) { array[n]++; 				i = graph[i * 2];  }
			else if (i == 3) { array[pointer]++; 			i = graph[i * 2];  }

			else if (i == 5) { 
				if (not pointer) zero_reset_happened = true;
				er_count++;
				pointer = 0; 
				i = graph[i * 2];

			} else if (i == 6) { 
				if (not array[n]) zero_reset_happened = true;
				array[n] = 0;
				i = graph[i * 2];
			}
			
			else abort();
		} // else 
	} // while()
	

	if (new.is_complete and new.expansion_type == good_expansion) {
		candidates[candidate_count++] = new;
	}
	

backtrack:

	if (stack_count == 0) {
		printf("error: no hole was found in the partial graph, after %llu instructions executed.\n", executed_count);
		return;
	}

	
	memcpy(array, stack[stack_count - 1].array_state, array_size * sizeof(nat));
	pointer = stack[stack_count - 1].pointer_state;
	zero_reset_happened = stack[stack_count - 1].zero_reset_happened;
	er_count = stack[stack_count - 1].er_count;

	if (stack[stack_count - 1].try < stack[stack_count - 1].options.count - 1) {
		
		stack[stack_count - 1].try++;
				
		graph[2 * stack[stack_count - 1].source + stack[stack_count - 1].side]  //   ".t" of the added br.
			= stack[stack_count - 1].options.options[stack[stack_count - 1].try];

		i = stack[stack_count - 1].options.options[stack[stack_count - 1].try];
		executed_count = 0;
		goto begin;
	} else {		
		graph[2 * stack[stack_count - 1].source + stack[stack_count - 1].side] = 0;
		if (stack_count <= 1) goto done;
		stack_count--; 
		goto backtrack;
	}























//one of  ≥ ≤ < > ≠ =        fund:    < = > 

enum check {  

	check__	    // 0          // what if we dont want to say the slit edge at all?  
					this is useful, if we want to actually have Branches everywhere!  and then just reduce certain edges, (by determining their correct branch!) so that the reduced edges become goto's ie, regular control flow paths. 


	check_l,    // 1          // this means that in order to say the SLIT EDGE, we need to have *n < *i


	check_g,    // 2
	check_e,    // 3
	check_lg,   // 4
	check_le,   // 5
	check_ge,   // 6

	check_lge,  // 7           //    this one will literally never be useful. 

				// it would mean that ANY STATE OF THE SYSTEM results in us taking       Br(t).  ie,  the split edge.

					// this not good, because it means that an edge of R   is being deleted. 
};



















		const nat op = os[k];



	
		if (op_Br == graph[ip].op) {

			///   if (check) {}     todo:   determine how we evaluate the abstract checks for this branch,
			//				to tell what side of exec we go to.


			prev = ip;
			ip = graph[ip].f;
			
			

		} else if (op_2 == graph[ip].op) {
			comparator++;
			prev = ip;
			ip = graph[ip].u;





		} else if (op_6 == graph[ip].op) {  // detect a zero reset:

			

			if (comparator == 0) {  // 		zero-reset  hole-trigger

				////////////////////// HOLE /////////////////////////////

				// add a branch to split this edge, to prevent to zero reset:


				//	prev        ip
				//	3	    6

				//	3 -> Br         Br -(t/f)>  6

				//	todo:   edit     3     to go to graph_count!



				struct option* options = generate_option_list();

				
				 ///   the false side always remains in the R.
				// where does this go!?!  this will be set to option_list[0] to begin with.





				graph[graph_count++] = (struct ins){
					.op = op_Br, 
					.l = XXXXXXXX 
					.t = YYYYYYYY 
					.e = ZZZZZZZZ
					
				}
				



				



				////////////////////// END-HOLE /////////////////////////////

			} else {
				comparator = 0;
				prev = ip;
				ip = graph[ip].f;
			}


 
		} else if (op != graph[ip].op) {   // 3, 1, 5            // 	invalid-lifetime  hole-trigger
			
			//////////////////////// HOLE /////////////////////////////

			// generate an abstract branch:

			////////////////////// END-HOLE /////////////////////////////
			
			
			prev = ip;
			ip = graph[ip].f;



		} else if (op == graph[ip].op) {


			if (op_1 == graph[ip].op) {
				pointer++;
				prev = ip;
				ip = graph[ip].f;
				recent_op = 1;

				// do a split!
			}
		
			if (op_3 == graph[ip].op) {
				(array[pointer])++;
				prev = ip;
				ip = graph[ip].f;
				recent_op = 3;

				// do a split!

				
			}

			if (op_5 == graph[ip].op) {
				pointer = 0;
				prev = ip;
				ip = graph[ip].f;
				recent_op = 5;

				// do a split!
				// if we just did a 5, though, 
				// 	then generate 3 options     for the ternary branch that 5 goes to!


			
			}	
		}
	}














	Q:   when is a graph flly spec?

			


			well,     there are two outcomes:




				1. we ran the graph,    trying out options, 


						and we finished going through the op_seq!!!

									which means 



								its good,   it is consistent with the lifetime 


						so we output this graph's Z value 


							and then we continue trying other optiosn (ie, backtrack!!)




				ORRRRRR


				2. 

					we actually could not get this graph to have the right behaviour(os),  even after ADDING   100   BRANCHES!!

				and so we have to backtrack,  (ie, try different options for a given branch, popping off the stackk, and trying diff opt for some other br!


				


		eventually, 


		we could get into a situation where we have tried all possible opt for all br

					(AND all checks?.... not sure)


		and we still cant get the rihght behvaiour or      non-zero-reset-ness


						for this   OS





			which means          we output:      CONTRADICTION!

							saying that the LIFETIME   is actually invalid, 

					i think 























		// 



// this records which side pureX goes on,   (the branch in the R!)     

	//	if it goes on  >   then the largescale inv is    'g'  :=    greater than
	//	if it goes on  <    its   'l' := less than 

		
	// char largescale_invariant = 'g'; 		//  'l', 'g',

nat recent_op = 0;
		
















enum operation {
	op_HALT,

	op_1, 
	op_2, 
	op_3, 

	op_NOP,

	op_5, 
	op_6,
};


//  abort(); // ! ? ! ? ! ? ! ? !
				// this should never happen, by design.	




// only 5 instructions,  
	// using a Combined Operation-Ternary-Branch model.


// note, at this point,    if the graph has no specified destinations, for 5,   
	// ie,    .l, .g and .e   of graph[4]  are all "unknown",  then we know this graph is a dud. 
	// we need to NOT save it. 






// at this point, we have a "full"    (as full as it can be, given the MCAL, "os") specified graph.
	// print it out for now,





//puts("DEBUG: ");
		//printf("\t i = %llu\n", pointer);
		//printf("\t *n = %llu\n", comparator);
		//printf("\t *i = %llu\n", array[pointer]);

-----------------------------------------------------------------------------------

*/









	// option   is in units of      address space 
	// me       is in op   space        allowed values :     1, 2, 3, 5, 6
	// expected    also in op space       a v       1, 3, 5





// printf("in generate options! generate(me=%llu, exp=%llu)\n", me, expected);


// printf("skipping over self: ip=%llu  (me=%llu)\n", ip, me);



// mcal, zr, er, complete, used_all,
	// const nat counts_size = 2 * 2 * 2 * 2 * 2 * expansion_type_count;
	// nat counts[counts_size] = {0}




// static const nat max_operation_count = 10;              // maximum size of the graph. (number of instructions: fused op with ternary_br)

