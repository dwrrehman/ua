#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <iso646.h>
#include <time.h>
/*

	example usage:

		help

		new

		new

		list

		add increment
		<hexstring0>
		<hexstring1>

		add increment
		<hexstring1>
		5

		add reset
		<hexstring0>
		<hexstring0>

		add reset
		<hexstring1>
		<hexstring0>

		list

		check
	
		list
		

*/
typedef uint64_t nat;

static const nat max_element_count = 4096;

struct pair { nat first, second; };

static inline void clear_screen(void) { printf("\033[2J\033[H"); }

static nat execute(nat input, struct pair* function, nat count) {
	for (nat i = 0; i < count; i++) {
		if (function[i].first == input) return function[i].second;
	}
	printf("\033[31;1merror:\033[0m execute could not find entry for %llx\n", input);
	return 0;
}

static bool is_in(struct pair this, struct pair* less, nat less_count) {
	for (nat i = 0; i < less_count; i++) {
		if (less[i].first == this.first and 
		    less[i].second == this.second) return true;
	}
	return false;
}

static bool unique_insert(struct pair this, struct pair* less, nat* less_count) {
	if (is_in(this, less, *less_count) or not this.first or not this.second) return true;
	less[(*less_count)++] = this;
	return false;
}


static bool satisfies_all_axioms(nat* elements, nat element_count, 
				struct pair* increment, nat increment_count, 
				struct pair* reset, nat reset_count, 
				struct pair* less, nat* less_count) {

	*less_count = 0;

	// ax0.
	if (not element_count) {
		puts("axoim 0: contradiction: no elements");
		return false;
	}

repeat:
	for (nat xi = 0; xi < element_count; xi++) {


		nat x = elements[xi];
		
		// ax1.
		if (is_in((struct pair){x, x}, less, *less_count)) {
			printf("axiom 1: contradiction: %llx < %llx is true\n", x, x);
			return false;
		}
		
		// ax2.
		if (not unique_insert((struct pair){x, execute(x, increment, increment_count)}, less, less_count)) {
			printf("axiom 2: added: %llx < increment(%llx) : [%llx < %llx] \n", x, x, x, execute(x, increment, increment_count));
			goto repeat;
		}

		// ax3.
		if (is_in((struct pair){x, execute(x, reset, reset_count)}, less, *less_count)) {
			printf("axiom 3: contradiction: %llx < reset(%llx) : [%llx < %llx] is true\n", x, x, x, execute(x, reset, reset_count));
			return false;
		}

		for (nat yi = 0; yi < element_count; yi++) {

			nat y = elements[yi];

			// ax4.
			if (not unique_insert((struct pair){
					execute(x, reset, reset_count),
					execute(y, increment, increment_count)
				}, less, less_count)) {

				printf("axiom 4: added: reset(%llx) < increment(%llx) : [%llx < %llx] \n", 
						x, y, 
						execute(x, reset, reset_count),
						execute(y, increment, increment_count));
				goto repeat;
			}

			// ax5.
			if (is_in((struct pair){
					execute(x, reset, reset_count), 
					execute(y, reset, reset_count)
				}, less, *less_count)) {

				printf("axiom 5: contradiction: reset(%llx) < reset(%llx) : [%llx < %llx] is true\n", 
					x, y, execute(x, reset, reset_count), execute(y, reset, reset_count));
				return false;
			}

			// ax6.
			if (is_in((struct pair){x,y}, less, *less_count) and 
			    not unique_insert((struct pair){
					execute(x, increment, increment_count),
					execute(y, increment, increment_count)
				}, less, less_count)) {

				printf("axiom 6: added: increment(%llx) < increment(%llx) : [%llx < %llx] \n", 
						x, y, 
						execute(x, increment, increment_count),
						execute(y, increment, increment_count));
				goto repeat;
			}
		}
	}
	
	printf("success: no contradictions found for any axiom with any possible elements.\n");
	return true;
}

static void print_pairs(const char* message, struct pair* pairs, nat count, const char* relation) {

	printf("%s = {\n", message);
	for (nat i = 0; i < count; i++) {
		printf("\t%llx%s%llx\n", pairs[i].first, relation, pairs[i].second);
	}
	printf("}\n");
}

static void print_nats(const char* message, nat* elements, nat count) {

	printf("%s = { ", message);
	for (nat i = 0; i < count; i++) {
		printf("%llx ", elements[i]);
	}
	printf("}\n");
}

static inline void search_definition_space(void) {

	//use the nf twice in a row(?)   to search over all  possible    reset   and increment  defintions you could give!

		// note   there are n + 1 elementst for the output   for the increment function,  but n inputs. ie, you have the option to use an element which does not appear in the input list. only for the increment though. lol. 

		// you use indicies into the elements array to refer to things though. at least, you store this version of the def's, and then convert it to the version that check_consistent uses. 
}

int main(void) {

	srand((unsigned)time(0));
	
	nat elements[max_element_count] = {0};
	nat element_count = 0;
	
	struct pair less[max_element_count] = {0};
	nat less_count = 0;

	struct pair increment[max_element_count] = {0};
	nat increment_count = 0;

	struct pair reset[max_element_count] = {0};
	nat reset_count = 0;

	puts("axiom-based theorem prover by dwrr.\ntype \"?\" for more info.");

loop:;
	char buffer[128] = {0};

	printf(": ");

	fgets(buffer, sizeof buffer, stdin);
	buffer[strlen(buffer) - 1] = 0;


	if (not strcmp(buffer, "quit") or not strcmp(buffer, "q")) goto done;
	else if (not strcmp(buffer, "")) {} 
	else if (not strcmp(buffer, "clear") or not strcmp(buffer, "o")) clear_screen();

	else if (not strcmp(buffer, "list") or not strcmp(buffer, "ls")) {

		print_nats("elements", elements, element_count);

		print_pairs("less", less, less_count, " < ");
		print_pairs("increment", increment, increment_count, " ---> ");
		print_pairs("reset", reset, reset_count, " ---> ");


	} else if (not strcmp(buffer, "check") or not strcmp(buffer, "c")) {
		bool b = satisfies_all_axioms(	elements, element_count, 
						increment, increment_count, 
						reset, reset_count,
						less, &less_count);
		printf("result = %s\n", b ? "consistent" : "contradiction");


	} else if (not strcmp(buffer, "search") or not strcmp(buffer, "s")) {
		search_definition_space();
	}
	
	else if (not strcmp(buffer, "add increment")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		nat input = (nat) strtoul(buffer, 0, 16);
		printf("enter increment(%llx) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		nat output = (nat) strtoul(buffer, 0, 16);

		printf("adding increment(%llx) ---> %llx\n", input, output);
		increment[increment_count++] = (struct pair) {input, output};
	}

	else if (not strcmp(buffer, "add reset")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		nat input = (nat) strtoul(buffer, 0, 16);
		printf("enter reset(%llx) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		nat output = (nat) strtoul(buffer, 0, 16);

		printf("adding reset(%llx) ---> %llx\n", input, output);
		reset[reset_count++] = (struct pair) {input, output};
	}


	else if (not strcmp(buffer, "edit increment")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		nat input = (nat) strtoul(buffer, 0, 16);
		printf("enter increment(%llx) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		nat output = (nat) strtoul(buffer, 0, 16);

		bool found = false;
		for (nat i = 0; i < increment_count; i++) {
			if (increment[i].first == input) {
				printf("editing increment(%llx) to return %llx instead of %llx\n", input, output, increment[i].second);
				increment[i].second = output;
				found = true;
				break;
			}
		}
		if (not found) printf("\033[31;1merror:\033[0m increment function doesnt take that input yet, consider using add instead.\n");
	}

	else if (not strcmp(buffer, "edit reset")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		nat input = (nat) strtoul(buffer, 0, 16);
		printf("enter reset(%llx) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		nat output = (nat) strtoul(buffer, 0, 16);

		bool found = false;
		for (nat i = 0; i < reset_count; i++) {
			if (reset[i].first == input) {
				printf("editing reset(%llx) to return %llx instead of %llx\n", input, output, reset[i].second);
				reset[i].second = output;
				found = true;
				break;
			}
		}
		if (not found) printf("\033[31;1merror:\033[0m reset function doesnt take that input yet, consider using add instead.\n");
	}

	else if (not strcmp(buffer, "new")) {
		const nat new = ((nat) rand()) | (nat) ((nat) rand() << (nat) 32);
		printf("generated new element %llx, adding...\n", new);
		elements[element_count++] = new;
	}

	else if (not strcmp(buffer, "delete last")) {
		printf("removing the last element added...\n");
		if (element_count) element_count--; else puts("\033[31;1merror:\033[0m no elements to remove");
	}


	else if (not strcmp(buffer, "reset less")) {
		printf("reseting know less relations...\n");
		less_count = 0;
	}

	else if (not strcmp(buffer, "help") or not strcmp(buffer, "?")) {
		printf(	"list of available commands:\n"
			"\tnew : generate a new unique element.\n"
			"\tdelete last : remove the last element.\n"
			"\treset less : remove all known less relations.\n"
			"\tadd reset : add to the behaviour of the reset function.\n"
			"\tadd increment : add to the behaviour of the increment function.\n"
			"\tedit reset : edit the behaviour of the reset function.\n"
			"\tedit increment : edit the behaviour of the increment function.\n"
			"\tcheck(c) : check if the function behaviour and elements satisfies the axioms.\n"
			"\tlist(ls) : list the current state of the system.\n"
			"\tspace(s) : search the space of all possible definitions.\n"
			"\thelp(?) : print this menu.\n"
			"\tquit(q) : quit the utility.\n"
			);
	} else {
		printf("\033[31;1merror:\033[0m unknown command: %s\n", buffer);
	}

	goto loop;
done: 	puts("exiting...");

}


















/*




static inline nat power(nat base, nat exponent) {
	nat c = 1;
	for (nat i = 0; i < exponent; i++) {
		c *= base;
	}
	return c;
}






	ax0.			x : N					 			 is true.


<
	l.ax1.			for all x : N,      x < x                                        is false.

	l.ax2.  		for all x : N,      x < _asht_(x)   				 is true.

	l.ax3.   		for all x : N,      x < _neoi_(x)   				 is false.

	l.ax4.			for all x,y : N,    _neoi_(x) < _asht_(y)  			 is true.

	l.ax5.			for all x,y : N,    _neoi_(x) < _neoi_(y)     			 is false.

	l.ax6.			for all x,y : N,    if  x < y,   then _asht_(x) < _asht_(y)      is true.




==
	e.ax1.			for all x : N,      x == x                                       is true.

	e.ax2.  		for all x : N,      x == _asht_(x)   				 is false.

	e.ax3.   		for all x : N,      x == _neoi_(x)   				 is false.   <------ ???????

	e.ax4.			for all x,y : N,    _neoi_(x) == _asht_(y)  			 is false.

	e.ax5.			for all x,y : N,    _neoi_(x) == _neoi_(y)     			 is true.

	e.ax6.			for all x,y : N,    if  x == y,   then _asht_(x) == _asht_(y)    is true.      <----------- ??????




202402224.041454:

	NOTE:

		we actually cannot give a value for   _neoi_(zero)    ie, we cannot do a zero reset! at least according to the above set of equality-axioms.
			  which is interesting.... 

						apparently    _neoi_(0) = infinity     essentially 


				ie, some element that doesnt appear in the output lol. soooo yeah thats interesting 

	lol 
						hmmm
				




	also
						can these set of 13 axioms be reduced?.....




								welllllllll





					the goal of this program is to see what the minimal set of axioms are, 

								by searching over all possible definitions of _neoi and _asht_


										and seeing what defintiosn are valid given this set of axioms

									and then 
										we remove an axiom        and see if we get the same set of defs that are consistent

								and ideallyyyy those defs (or just one, if theres only one)   will be the actual definition of the natural numbers that we are looking for. like the actual nats



								so yeah




									thats the main idea of this program



								we are looking to see if these axioms are sufficient   or can be further minimized



									





				

	













old----------------











more compactly said:

	-x<x x<Sx -x<Zx Zx<Sy -Zx<Zy x<y->Sx<Sy
	

also







			not 	x < x

				x < S(x)

			not	x < Z(x)

				Z(x) < S(y)

			not 	Z(x) < Z(y)

				x < y --> S(x) < S(y)




*/









/*









//static inline nat unreduce(nat* in, nat length, nat radix) {
//	nat s = 0;
//	return s;
//}

//static inline void reduce(nat* out, nat length, nat radix, nat s) {
//	//for (nat i = 0; ) out[i] = s % 10
//}








my current way of defining the computational constructs/things neccesary for the ua:                [N, <, ++, =0]
-------------------------------------------------------------------------------------


N		define N 				and
<		define _<_(x : N, y : N) : Boolean   	and
++		define  _asht_(x : N) : N      		and 
=0		define  _neoi_(y : N) : N      		such that 


	ax0.			x : N					 			 is true.
	
	ax1.			for all x : N,      x < x                                        is false.

	ax2.  			for all x : N,      x < _asht_(x)   				 is true.

	ax3.   			for all x : N,      x < _neoi_(x)   				 is false.

	ax4.			for all x,y : N,    _neoi_(x) < _asht_(y)  			 is true.

	ax5.			for all x,y : N,    _neoi_(x) < _neoi_(y)     			 is false.

	ax6.			for all x,y : N,    if  x < y,   then _asht_(x) < _asht_(y)      is true.






x:	ax7.  			for all x,y,z : N   if x < y and y < z,  then    x < z           is true.              // is this neccessary?


---------------------------------------------------------------------------------------------------

now im experimenting with what possible realizations are valid for asht and neoi using some toy figurines lol. 
so far i have logically necc:



functions behavior:
--------------------------
				
_asht_(tiger) == man
_asht_(man) == tiger

_neoi_(tiger) == tiger
_neoi_(man) == man



relation behavior:
--------------------------

tiger < man
man < tiger


not   tiger < tiger
not   man < man



*/


