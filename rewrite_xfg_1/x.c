#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>

typedef unsigned long long nat;

// dwrr, 2208173.222343



// constants:


static const nat array_size = 4096;   	// psudo inf

static const nat max_stack_size = 15; 	// should never need more than 10, because there are only 10 unknowns in the R.

static const nat unknown = 123456789;	// some bogus value, that represents a hole. 

static const nat origin = 1; 		// start out execution at the instruction 3.

static const nat execution_limit = 100;   // untested value, tweak this



struct stack_frame {
	nat try;
	nat* options;
	nat option_count;

	nat array_state[array_size];
	nat comparator;
	nat pointer;
	nat k;

	nat ip;
	nat state;

};



static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}

static void print_graph_as_adj(nat* graph) {
	
	puts("graph adjacency list: ");
	puts("{");
	for (nat i = 0; i < 20; i += 4) {
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



static void print_stack(struct stack_frame* stack, nat stack_count) {

	printf("printing %llu stack frames:\n{\n", stack_count);
	for (nat i = 0; i < stack_count; i++) {

		printf("FRAME #%llu:   {try=%llu, k=%llu, i=%llu, *n=%llu, ip=%llu, state=%c, ", 
			i, 
			stack[i].try,
			stack[i].k,
			stack[i].pointer,
			stack[i].comparator,
			stack[i].ip,
			(char) stack[i].state
		);

		print_nats(stack[i].options, stack[i].option_count);

		puts("}");
	}

	printf("}\n[end of stack]\n");

}



static nat* generate_options(nat* option_count, nat ip, nat expected, nat comparator) {

	nat* options = malloc(5 * sizeof(nat));
	nat count = 0;

	for (nat option = 0; option < 5; option++) {
		if (ip == option) continue;
		
		if (	(option == 0 and expected == 1) or 
			(option == 1 and expected == 3) or
			(option == 4 and expected == 5) or
			(option == 2 or comparator) 

		) options[count++] = option; 
	}

	*option_count = count;
	return options;
}





static void unexecute(nat* os, nat length) {
	
	
	nat** candidates = calloc(100, sizeof(nat*));
	nat candidate_count = 0;
	
	for (nat i = 0; i < 100; i++) candidates[i] = calloc(20, sizeof(nat));
	
	nat array[array_size] = {0};
	const nat _ = unknown;
	nat 
		comparator = 0, 
		pointer = 0, 
		ip = origin
	;

	// ------------------------ 3->6  R ----------------------------

	nat _graph[20] = {
		1,  2, 1, _,
		3,  _, 3, _,
		2,  0, _, _,
		6,  0, _, _,
		5,  _, _, _,
	};



	// ------------------------ 6->3  R ----------------------------

	nat graph[20] = {
		1,  2, 3, _,
		3,  0, _, _,
		2,  0, _, _,
		6,  1, _, _,
		5,  _, _, _,
	};


	

	print_nats(graph, 20);
	print_graph_as_adj(graph);

	struct stack_frame* stack = calloc(max_stack_size, sizeof(struct stack_frame));
	nat stack_count = 0;

	nat k = 0;
	nat tried = 0;
	nat executed_count = 0;

begin:
	while (k < length and executed_count < execution_limit) {

		printf("(%llu)EXECUTING: %llu   ", executed_count, graph[4 * ip]);
		printf("expecting: %llu\n", os[k]);

		const nat I = ip * 4;
		const nat op = graph[I];

		if (op == 1) pointer++;
		else if (op == 5) pointer = 0;
		else if (op == 2) comparator++;
		else if (op == 6) comparator = 0;
		else if (op == 3) array[pointer]++;

		executed_count++;

		nat state = 0;
		if (comparator < array[pointer]) state = 1;
		if (comparator > array[pointer]) state = 2;
		if (comparator == array[pointer]) state = 3;

		if (op == 3 or op == 1 or op == 5) {
			if (op != os[k]) goto backtrack;
			k++;
			if (k >= length) goto done_graph;
		}

		if (graph[I + state] != unknown) goto set;

		nat option_count = 0;
		nat* options = generate_options(&option_count, ip, os[k], comparator);
		
		struct stack_frame frame = {
			.try = 0,
			.pointer = pointer, 
			.comparator = comparator,
			.options = options,
			.option_count = option_count,
			.k = k,
			.ip = ip,
			.state = state,
		};

		executed_count = 0;

		memcpy(frame.array_state, array, sizeof(nat) * array_size);
		stack[stack_count++] = frame;

		print_stack(stack, stack_count);

		graph[I + state] = options[0];
	set: 	ip = graph[I + state];
	} 

done_graph:
	tried++;

	print_graph_as_adj(graph);
	printf("tried so far: %llu: ", tried);

	if (
		(graph[16 + 1] != unknown or 
		graph[16 + 2] != unknown or 
		graph[16 + 3] != unknown) 
	
			and

		k == length
	) {
		memcpy(candidates[candidate_count], graph, 20 * sizeof(nat));
		candidate_count++;
	}

backtrack:

	memcpy(array, stack[stack_count - 1].array_state, array_size * sizeof(nat));
	pointer = stack[stack_count - 1].pointer;
	comparator = stack[stack_count - 1].comparator;
	k = stack[stack_count - 1].k;

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
	
	printf("tried = %llu\n", tried);
	print_graph_as_adj(graph);


	printf("[Printing the graphs that we found]: \n");

	for (nat i = 0; i < candidate_count; i++) {
		printf("CANDIDATE #%llu: ", i);
		print_graph_as_adj(candidates[i]);
	}
	printf("[end of candidates]\n");
	
	printf("\n\n[[ candidate_count = %llu ]] \n\n\n", candidate_count);

	print_nats(os, length);

	if (candidate_count == 0) {
		printf("\n\n\t\t MCAL CONTRADICTION\n\n\n");
	} else {

		printf("\n\n\t\t MCAL satisfied\n\n\n");
	}


	printf("run all candidates? (return to confirm) ");
	if (getchar() == '\n') {

		for (nat c = 0; c < candidate_count; c++) {
			
			memcpy(graph, candidates[c], sizeof(nat) * 20);

			memset(array, 0, sizeof array);
			pointer = 0; comparator = 0;

			ip = origin;

			for (nat e = 0; e < 2000; e++) {

				if (graph[4 * ip] == 1) pointer++;

				else if (graph[4 * ip] == 5) {   

					pointer = 0;  

					for (nat i = 0; i < array_size; i++) {
						if (array[i]) printf("%2llu ", array[i]);
						else { printf("   "); break;}
					}
					puts("");
				}

				else if (graph[4 * ip] == 2) comparator++;
				else if (graph[4 * ip] == 6) comparator = 0;
				else if (graph[4 * ip] == 3) array[pointer]++;


				nat state = 0;
				if (comparator < array[pointer]) state = 1;
				if (comparator > array[pointer]) state = 2;
				if (comparator == array[pointer]) state = 3;
				ip = graph[4 * ip + state]; 

				if (ip == unknown) {  puts("ENCOUNTERED HOLE, stopping"); break;  }
			}

			puts("");
			printf("[candidate #%llu/%llu]\n", c, candidate_count);
			getchar();


		}

	}

}



static void run_modified_xfg_lifetime_cand1_2() {

	nat array[array_size] = {0};

	bool modes[array_size] = {0};


	nat 
		comparator = 0, 
		pointer = 0, 
		ip = origin
	;
const nat _ = unknown;

/*
	#0: ins(.op = 1, .lge = [ 2, 3, 2])

	#4: ins(.op = 3, .lge = [ 0,  , 4])

	#8: ins(.op = 2, .lge = [ 0, 1, 0])

	#12: ins(.op = 6, .lge = [ 1,  , 2])

	#16: ins(.op = 5, .lge = [ 2,  , 3])
	
*/

	
	nat graph[20] = {
		1,  2, 3, 2,
		3,  0, 0, 4,
		2,  0, 1, 0,
		6,  1, _, 2,
		5,  2, 2, 3,
	};
	
	
	// supplying 5 -> 2 (>)     we found the hole,   for 3 [.g]!!!!!   
	// so lets find all possibilities for it!!

	//     3 -> 1, 3 -> 2,   and 3 -> 6



	/// 3 -> 1  is very good,  cant distinguish if its bad at all.... so yeah.

	// 3 -> 2    has an infinite loop between    3 and 2. very bad. 


	/// 3 -> 6        this one is awesome!    very good, cant distinguish whether or not its the xfg.   2208195.035523   roughly tried on that time





	// so yeah, i think that means that 3 -> 


	// memcpy(graph, candidates[c], sizeof(nat) * 20);

	memset(array, 0, sizeof array);
	pointer = 0; comparator = 0;

	ip = origin;

	for (nat e = 0; e < 2000000; e++) {

		// printf("{{{%llu}}}\n", graph[4 * ip]);

		if (graph[4 * ip] == 1) pointer++;

		else if (graph[4 * ip] == 5) {   

			if (pointer == 0) abort();
			pointer = 0;  

			for (nat i = 0; i < array_size; i++) {
				if (modes[i]) printf("## ");
				else { printf("   "); }
				if (not array[i]) break;
			}
			puts("");

			memset(modes, 0, sizeof modes);
		}

		else if (graph[4 * ip] == 2) comparator++;
		else if (graph[4 * ip] == 6) {   if (comparator == 0) abort();   comparator = 0; }
		else if (graph[4 * ip] == 3) { array[pointer]++; modes[pointer] = 1; }

		nat state = 0;
		if (comparator < array[pointer]) state = 1;
		if (comparator > array[pointer]) state = 2;
		if (comparator == array[pointer]) state = 3;
		ip = graph[4 * ip + state]; 

		if (ip == unknown) {  puts("ENCOUNTERED HOLE, stopping"); break;  }
	}

	puts(": ");
	//printf("[candidate #%llu/%llu]\n", c, candidate_count);
	getchar();





		// wrote on 2208195.032113




}






int main() {
	puts("this is a rewrite of the xfg search utility, that uses a backwards approach to searching for graphs.");

/*

	nat v[] = { 

		3, 1, 3, 5,  

		3, 1, 1, 3, 5, 

		1, 3,


	};
	
	unexecute(v, sizeof v / sizeof(nat));
*/
	run_modified_xfg_lifetime_cand1_2();
}









/*


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

}


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

