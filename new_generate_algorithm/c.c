#include <stdio.h>
#include <stdlib.h>    // rewrite of the generate algorithm, called "nfgp". 
#include <string.h>
#include <iso646.h>    // written on 2307031.171603 by dwrr
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t nat;
static const nat unknown  = 11223344556677;
static const nat deadstop = 90909090909090;
static const nat _ = unknown;
static const nat X = deadstop;

static const nat unique_operation_count = 5;
static const nat unique_operations[unique_operation_count] = {1, 2, 3, 5, 6};

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

static bool is_increment(nat a) { return a < 3; }
static bool is_reset(nat a) { return a >= 3; }

static bool graph_analysis(nat* graph, nat operation_count) {
	for (nat index = 0; index < operation_count; index++) {

		if (	graph[4 * index + 0] == 4 and 
			graph[4 * index + 2]
		) return true;
	}
	return false;
}

int main(void) {

	const nat D = 1;      //    1-space

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

	const nat display_rate = 25;
	nat raw_counter = 0, maybe_counter = 0, good_counter = 0;

	goto print;

loop:
	if (m1_array[m1_pointer].option < operation_count - 1) goto m1_increment;
	if (m1_pointer < m1_array_count - 1) goto m1_reset;
loop2:
	if (not m2_array_count) goto check_if_done;
	if (m2_array[m2_pointer].option < unique_operation_count - 1) goto m2_increment;
	if (m2_pointer < m2_array_count - 1) goto m2_reset;
	check_if_done: if (m2_array_count == D) goto done;

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
	goto print;

m2_reset:
	m2_array[m2_pointer].option = 0; 
	m2_pointer++;
	goto loop2;

m1_increment:
	m1_array[m1_pointer].option++;
	graph[m1_array[m1_pointer].position] = m1_array[m1_pointer].option;
	print: 	m1_pointer = 0; 

	if (not (raw_counter & ((1 << display_rate) - 1))) {
		for (nat i = 0; i < m1_array_count; i++) printf("%llu ", m1_array[i].option); 
		printf("  -  ");
		for (nat i = 0; i < m2_array_count; i++) printf("%llu ", m2_array[i].option); 

		printf("  ---->  z = ");
		print_graph(graph, graph_count);
	}

	raw_counter++;
	if (graph_analysis(graph, operation_count)) goto loop;
	maybe_counter++;
	// {execute instructions using array of size 3 here!! (SFEA)}
	// {execute instructions using the graph here!}
	// if (bad) goto loop;
	good_counter++;
	// write the good z value to a candidates_list!
        goto loop;
m1_reset:
	m1_array[m1_pointer].option = 0; 
	m1_pointer++; 
	goto loop;

done:	printf("looked at [%llu:%llu:%llu] poss!\n", raw_counter, maybe_counter, good_counter);
}































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






