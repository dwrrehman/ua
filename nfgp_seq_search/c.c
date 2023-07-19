#include <stdio.h>
#include <stdlib.h>    // rewrite of the generate algorithm, called "nfgp". 
#include <string.h>
#include <iso646.h>    // written on 2307031.171603 by dwrr
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
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


typedef uint64_t nat;
static const nat unknown  = 11223344556677;
static const nat deadstop = 90909090909090;
static const nat _ = unknown;
static const nat X = deadstop;

static const nat unique_operation_count = 5;
static const nat unique_operations[unique_operation_count] = {1, 2, 3, 5, 6};


enum pruning_metrics {
	PM_fea, PM_ns0, PM_pco, 
	PM_zr5, PM_zr6, PM_ndi, 
	PM_rer, PM_oer, PM_r0i, 
	PM_h,   PM_f1e, PM_erc,
	PM_ric, PM_nsvl,

	PM_count
};

static const char* pm_spelling[] = {
	"PM_fea", "PM_ns0", "PM_pco", 
	"PM_zr5", "PM_zr6", "PM_ndi", 
	"PM_rer", "PM_oer", "PM_r0i", 
	"PM_h",   "PM_f1e", "PM_erc", 
	"PM_ric", "PM_nsvl", 
};



struct parameters {
	nat execution_limit;
	nat array_size;
	nat rer_count;
	nat oer_count;
	nat required_er_count;
	nat max_acceptable_consecutive_incr;
	nat max_acceptable_run_length;
	nat expansion_check_timestep;
};

struct list {
	nat count;
	nat* z;
	char* dt;
};

struct option {
	nat option;
	nat position;
};

static void print_graph(nat* graph, nat graph_count) {
	for (nat i = 0; i < graph_count; i++) {
		if (graph[i] == X) printf("_");
		else if (graph[i] == _) printf("HOLE");
		else printf("%llu", graph[i]);
	}
	putchar(10);
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

// static bool is_increment(nat a) { return a < 3; }
// static bool is_reset(nat a) { return a >= 3; }

static bool graph_analysis(nat* graph, nat operation_count) {
	for (nat index = 0; index < operation_count; index++) {

		if (	graph[4 * index + 0] == 4 and 
			graph[4 * index + 2]
		) return true;
	}
	return false;
}



static nat* array = NULL;
static nat* modes = NULL;

static nat counts[PM_count] = {0};

static bool execute_graph_starting_at(nat origin, nat* graph, struct parameters p, bool should_print_pm) {

	const nat n = p.array_size;

	memset(array, 0, (n + 1) * sizeof(nat));
	memset(modes, 0, (n + 1) * sizeof(nat));

	nat pointer = 0, ip = origin;
	nat a = PM_count;
	nat 	last_mcal_op = 0,    er_count = 0, 
		RER_er_at = 0, RER_counter = 0, 
	    	OER_er_at = 0, OER_counter = 0, 
		R0I_counter = 0,     H_counter = 0;
	
	nat e = 0;
	for (; e < p.execution_limit; e++) {

		if (e >= p.expansion_check_timestep) {   
			if (array[0] == 1) 			{ a = PM_f1e; goto bad; }
			if (er_count < p.required_er_count) 	{ a = PM_erc; goto bad; }
		}

		const nat I = ip * 4;
		const nat op = unique_operations[graph[I]];

		if (op == 1) {
			if (pointer == n) 	{ a = PM_fea; goto bad; } 
			if (not array[pointer]) { a = PM_ns0; goto bad; } 
			pointer++;
		}

		else if (op == 5) {
			if (last_mcal_op != 3) 	{ a = PM_pco; goto bad; } 
			if (not pointer) 	{ a = PM_zr5; goto bad; } 

			// rer:
			if (RER_er_at == pointer) RER_counter++; else { RER_er_at = pointer; RER_counter = 0; }
			if (RER_counter == p.rer_count) { a = PM_rer; goto bad; }

			// oer:
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter == p.oer_count) { a = PM_oer; goto bad; }

			
			if (*modes) R0I_counter++; else R0I_counter = 0;
			if (R0I_counter > p.max_acceptable_consecutive_incr) { a = PM_r0i; goto bad; }

			/// {NSVLPM HERE}

			memset(modes, 0, (n + 1) * sizeof(nat));
			pointer = 0;
			er_count++;
		}

		else if (op == 2) {
			array[n]++;
		}

		else if (op == 6) {  
			if (not array[n]) 	{ a = PM_zr6; goto bad; }
			array[n] = 0;   
		}

		else if (op == 3) {
			if (last_mcal_op == 3) 	{ a = PM_ndi; goto bad; }

			// h
			if (pointer and modes[pointer - 1]) H_counter++; else H_counter = 0;
			if (H_counter > p.max_acceptable_run_length) { a = PM_h; goto bad; }

			array[pointer]++;
			modes[pointer] = 1;

			/// {NSVLPM HERE}
		}
		
		if (op == 3 or op == 1 or op == 5) last_mcal_op = op;

		nat state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}

	return 0; // graph is good!
	
bad: 	
	counts[a]++;
	if (should_print_pm) printf("%7s ( on e=%8llu )\n", pm_spelling[a], e);
	return 1;
}

static bool execute_graph(nat* graph, nat operation_count, struct parameters p, bool b) {
	for (nat o = 0; o < operation_count; o++) {
		if (unique_operations[graph[4 * o]] != 3) continue;
		if (not execute_graph_starting_at(o, graph, p, b)) return false;
	}
	return true;
}





/*
static void write_graph(nat* g, nat oc, char dt[32]) {

	nat candidate_count = 0, candidate_capacity = 0, candidate_timestamp_capacity = 0;
	nat* candidates = NULL;
	char* candidate_timestamps = NULL;


	if (p.graph_count * (candidate_count + 1) > candidate_capacity) {
		candidate_capacity = 4 * (candidate_capacity + p.graph_count);
		candidates = realloc(candidates, sizeof(nat) * candidate_capacity);
	}
	memcpy(candidates + p.graph_count * candidate_count, graph, p.graph_count * sizeof(nat));
	
	if (16 * (candidate_count + 1) > candidate_timestamp_capacity) {
		candidate_timestamp_capacity = 4 * (candidate_timestamp_capacity + 16);
		candidate_timestamps = realloc(candidate_timestamps, sizeof(char) * candidate_timestamp_capacity);
	}
	memcpy(candidate_timestamps + 16 * candidate_count, dt, 16);
}

*/






static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], counts[i]);
	}
	puts("");
	puts("[done]");
}


int main(void) {

	const nat D = 1;

	const nat display_rate = 18;

	struct parameters p = {
		.execution_limit = 100000,
		.array_size = 1000,
		.rer_count = 40,
		.oer_count = 80,
		.required_er_count = 6,
		.max_acceptable_consecutive_incr = 50,
		.max_acceptable_run_length = 7,
		.expansion_check_timestep = 10000,
	};

	array = calloc(p.array_size + 1, sizeof(nat));
	modes = calloc(p.array_size + 1, sizeof(nat));

	const nat max_operation_count = 5 + D;	
	const nat max_graph_count = 4 * max_operation_count;

	nat operation_count = 5;
	nat graph_count = 4 * operation_count;
	nat* graph = calloc(max_graph_count, sizeof(nat));

	const nat initial_hole_count = 9;
	const nat _63RRXFG[5 * 4] = {
		0,  1, 4, _,      // 3
		1,  0, _, _,      // 6 7 
		2,  0, _, _,      // 10 11
		3,  _, _, _,      // 13 14 15
		4,  2, 0, _,      // 19                    // note, 6.> is 0 by force, 
					///ie deadstop, is never executed, doesnt matter what it is!
	};

	memcpy(graph, _63RRXFG, graph_count * sizeof(nat));

	nat m1_pointer = 0;
	nat m1_array_count = initial_hole_count;
        struct option* m1_array = calloc(3 * max_operation_count, sizeof(struct option));
	
	m1_array[0].position = 3;
	m1_array[1].position = 6; 
	m1_array[2].position = 7;
	m1_array[3].position = 10;
	m1_array[4].position = 11;
	m1_array[5].position = 13; 
	m1_array[6].position = 14;
	m1_array[7].position = 15;
	m1_array[8].position = 19;

	for (nat i = 0; i < m1_array_count; i++) 
		graph[m1_array[i].position] = m1_array[i].option;
	
	nat m2_pointer = 0;
	nat m2_array_count = 0;
	struct option* m2_array = calloc(D + 1, sizeof(struct option));

	nat raw_counter = 0, maybe_counter = 0, good_counter = 0;
	goto m1_pointer_reset;

loop:
	if (m1_array[m1_pointer].option < operation_count - 1) goto m1_increment;
	if (m1_pointer < m1_array_count - 1) goto m1_reset;
loop2:
	if (not m2_array_count) goto check_if_done;
	if (m2_array[m2_pointer].option < unique_operation_count - 1) goto m2_increment;
	if (m2_pointer < m2_array_count - 1) goto m2_reset;
	check_if_done: 

	printf("\n[finished %llu space]: looked at [%llu:%llu:%llu] poss!\n", operation_count - 5, raw_counter, maybe_counter, good_counter);
	print_counts();

	if (m2_array_count == D) goto done;
	

	m2_array[m2_array_count++] = (struct option) {.option = 0, .position = graph_count + 0};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 1};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 2};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 3};
	operation_count++;
	graph_count = 4 * operation_count;
	m2_array[m2_pointer].option = 0; 
	goto thing_blah;

m2_increment:
	m2_array[m2_pointer].option++;
	graph[m2_array[m2_pointer].position] = m2_array[m2_pointer].option;
	thing_blah: m2_pointer = 0;
	m1_array[m1_pointer].option = 0;
	goto m1_pointer_reset;

m2_reset:
	m2_array[m2_pointer].option = 0; 
	m2_pointer++;
	goto loop2;

m1_increment:
	m1_array[m1_pointer].option++;
	graph[m1_array[m1_pointer].position] = m1_array[m1_pointer].option;

	m1_pointer_reset: m1_pointer = 0; 
	

	if (not (raw_counter & ((1 << display_rate) - 1))) {
		for (nat i = 0; i < m1_array_count; i++) printf("%llu ", m1_array[i].option); 
		printf("  -  ");
		for (nat i = 0; i < m2_array_count; i++) printf("%llu ", m2_array[i].option); 
		printf("  ---->  ");

		//print_graph(graph, graph_count);
	}

	//char dt[32] = {0}; get_datetime(dt);

	raw_counter++;

	if (graph_analysis(graph, operation_count)) {
		puts("#");
		goto loop;
	}

	maybe_counter++;

	if (execute_graph(graph, operation_count, p, not (raw_counter & ((1 << display_rate) - 1)))) {
		goto loop;
	}

	good_counter++;

	// write_graph(graph, operation_count);

	//if (not (raw_counter & ((1 << display_rate) - 1))) {
		for (nat i = 0; i < m1_array_count; i++) printf("%llu ", m1_array[i].option); 
		printf("  -  ");
		for (nat i = 0; i < m2_array_count; i++) printf("%llu ", m2_array[i].option); 
		printf("  ---->  z = ");
		print_graph(graph, graph_count);
	//}

        goto loop;
m1_reset:
	m1_array[m1_pointer].option = 0; 
	m1_pointer++; 
	goto loop;

done:	printf("\n [DONE] looked at [%llu:%llu:%llu] poss!\n", raw_counter, maybe_counter, good_counter);
	print_counts();
}
























/*
202307182.184300:

	hiii!

		so the running of the nfgp 0space call just finished! the output is:



				looked at [1953125:1953125:130] poss!
			printing pm counts:
			PM_fea:        0		PM_ns0:    31801		PM_pco:   505885		PM_zr5:     3389		
			PM_zr6:   813299		PM_ndi:    62631		PM_rer:     3900		PM_oer:        0		
			PM_r0i:      800		  PM_h:   421915		PM_f1e:   109375		PM_erc:        0		
			PM_ric:        0		PM_nsvl:        0		
			[done]





			the first couple numbers,   1953125     thats the total size of the search space,    raw    with no pruning, 
						(graph analysis or pms)


			the second number,             130           is the number of good possibilities!



					ie, graphs that passsed all of our pruning metrics, 


								...except for    nsvlpm  (and ric)       because those arent implemented yet!


					so yeah, 0  space really isnt that big,  just takes a long time to go over lol. 



				so yeah, the next step woudld be to the implement the parellel version of this, 


						..after we integrate nsvlpm. 


					which i think i am going to do now?
							yeah


			but then after that, 

			we coulddddd try to run  1space! using this nfgp  thingy 




			but more likely, i think we should code up     the parellelizeddddddd   0space  version        

				and run that, 

							(which will use opencl of course)


				and then once thats coded up, code up 1space parellelized version, 


				and then run that too!


					and it shouldnt take as much of an eternity     as the nfgp would.. 

			so yeah 



	cool
				probably going to do that now


yay





			




*/






































// {execute instructions using array of size 3 here!! (SFEA)}     <----------- do this one last!!!

























// printf("continue to %llu-space? (ENTER) ", operation_count - 5);
	// if (getchar() == ' ') goto done;   // for debug for now







/*


202307031.153108: 


turns out there are   exactly        1,953,125   (ie, 5^9) possibilities       in raw 0 space!

					thats right.           ZERO space. 


									not 1 or 2 space. 0.



			soooooooo

	
yeah 
				the search spaces



							are 
								somewhatttt bigger..


		lol

	









202307031.163338:

CRAP

uhhhh

		uhhhhhhhhhhhhh

				soooo 1-space search space size 

					was just calculated to be 


			

			         10,885,864,805



		which, 
			if you want to calculate the raw size, 



			it will be:





		0sp:	5^9 = 1,953,125	
			
		1sp:	5 * (6^12) = 10,883,911,680

		
		
		0+1sp:		= 1,953,125 + 10,883,911,680

				=  10,885,864,805


		






hypotheticallyyy 

	2 space will end up being around:


	5^2 * (7^15) = 118,689,037,748,575  ie  118 trillion z values. 

						in the raw space.


			...

	i don't think we are ever going to actually go through that lol.

	so.. 

			WELLLLLLLLL        AT LEAST  WITH    a single threaded application.


		soooooo basicallyyyyyyy the only way we have of actually going over 2sp 

	is to PARELLELIZE THIS to the EXTREME. 

	unless we want to wait like MONTHSSS for it to finish. 



so yeah.
thats fun lol.


0sp  ->  1sp   ->   2sp
     A          B


A =  x5000  increase

B =  x10000  increase

....


so yeah basically 3 space is going to be 

	somewhat large.

just slightly 

just 3sp would be:

	(5^3) * (8^18)  =  2,251,799,813,685,248,000
		
so about about 2 quintillion

ish




yeahhhhhhh hopefully it doesnt come to that


lol







202307031.170434:


	okay, so implementing the graph an. deadstop rule, 


		ie, enforcing that 6.> == 0    always. 



		takes the origin 1sp  size of 10,885,864,805



			and ticks it down to 8709082469


		ie, 

			8,709,082,469      we pruned about 2 billion z values.

						niceeeee


			okay, thats progress i guesss





































	

*/














// push new elements to m1a and m2a here!!!!
	// until we reach the max number of elements in m1/m2



/*
 o#   :=      {          the sequential values  of     an M_2  variable/hole.

		0   ->	mean "i++",    (1)
		1   ->	means "(*n)++" (2)
		2   ->	means "(*i)++" (3)
		3   ->	means "i = 0"  (5)
		4   ->	means "*n = 0" (6)

	}
*/


/*

	the partial graph we are going to test with:


	the 63R:     (RRXFG ("R")    with   6->3         as opposed to 3->6     (which is an alternate valid formulation of the R)

		{

		op   *n<*i  *n>*i *n==*i
		=============================

			1,  2, 3, _,
			3,  0, _, _,
			2,  0, _, _,
			6,  1, X, _,
			5,  _, _, _,

		}




	0	1,  2, 3, _,
	1	3,  0, _, _,
	2	2,  0, _, _,
	3	6,  1, X, _,
	4	5,  _, _, _,


*/






