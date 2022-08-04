#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <iso646.h>
#include <time.h>


// run me with:       clang -g -O0 -Weverything -fsanitize=address,undefined c.c -o run


struct pair { int first, second; };



static inline void clear_screen() { printf("\033[2J\033[H"); }


static int execute(int input, struct pair* function, int count) {
	for (int i = 0; i < count; i++) {
		if (function[i].first == input) return function[i].second;
	}
	printf("error: execute could not find entry for %x\n", input);
	return 0;
}

static bool is_in(struct pair this, struct pair* less, int less_count) {
	for (int i = 0; i < less_count; i++) {
		if (less[i].first == this.first and 
		    less[i].second == this.second) return true;
	}
	return false;
}


static bool unique_insert(struct pair this, struct pair* less, int* less_count) {
	if (is_in(this, less, *less_count) or not this.first or not this.second) return true;
	less[(*less_count)++] = this;
	return false;
}


static bool satisfies_all_axioms(int* elements, int element_count, 
				struct pair* increment, int increment_count, 
				struct pair* reset, int reset_count, 
				struct pair* less, int* less_count) {

	*less_count = 0;


	// ax0.
	if (not element_count) {
		puts("axoim 0: contradiction: no elements");
		return false;
	}


repeat:


	for (int xi = 0; xi < element_count; xi++) {


		int x = elements[xi];
		
		// ax1.
		if (is_in((struct pair){x, x}, less, *less_count)) {
			printf("axiom 1: contradiction: %x < %x is true\n", x, x);
			return false;
		}
		
		// ax2.
		if (not unique_insert((struct pair){x, execute(x, increment, increment_count)}, less, less_count)) {
			printf("axiom 2: added: %x < increment(%x) : [%x < %x] \n", x, x, x, execute(x, increment, increment_count));
			goto repeat;
		}

		// ax3.
		if (is_in((struct pair){x, execute(x, reset, reset_count)}, less, *less_count)) {
			printf("axiom 3: contradiction: %x < reset(%x) : [%x < %x] is true\n", x, x, x, execute(x, reset, reset_count));
			return false;
		}

		for (int yi = 0; yi < element_count; yi++) {

			int y = elements[yi];

			// ax4.
			if (not unique_insert((struct pair){
					execute(x, reset, reset_count),
					execute(y, increment, increment_count)
				}, less, less_count)) {

				printf("axiom 4: added: reset(%x) < increment(%x) : [%x < %x] \n", 
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

				printf("axiom 5: contradiction: reset(%x) < reset(%x) : [%x < %x] is true\n", 
					x, y, execute(x, reset, reset_count), execute(y, reset, reset_count));
				return false;
			}

			// ax6.
			if (is_in((struct pair){x,y}, less, *less_count) and 
			    not unique_insert((struct pair){
					execute(x, increment, increment_count),
					execute(y, increment, increment_count)
				}, less, less_count)) {

				printf("axiom 6: added: increment(%x) < increment(%x) : [%x < %x] \n", 
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



/*


	ax0.			x : N					 			 is true.


	ax1.			for all x : N,      x < x                                        is false.

	ax2.  			for all x : N,      x < _asht_(x)   				 is true.

	ax3.   			for all x : N,      x < _neoi_(x)   				 is false.

	ax4.			for all x,y : N,    _neoi_(x) < _asht_(y)  			 is true.

	ax5.			for all x,y : N,    _neoi_(x) < _neoi_(y)     			 is false.

	ax6.			for all x,y : N,    if  x < y,   then _asht_(x) < _asht_(y)      is true.











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



static void print_pairs(const char* message, struct pair* pairs, int count, const char* un_named_thingy) {

	printf("%s = {\n", message);
	for (int i = 0; i < count; i++) {
		printf("\t%x%s%x\n", pairs[i].first, un_named_thingy, pairs[i].second);
	}
	printf("}\n");
}



static void print_ints(const char* message, int* elements, int count) {

	printf("%s = { ", message);
	for (int i = 0; i < count; i++) {
		printf("%x ", elements[i]);
	}
	printf("}\n");
}





static inline int unreduce(int* in, int length, int radix) {
	int s = 0;

	return s;
}


static inline void reduce(int* out, int length, int radix, int s) {
	for (int i = 0; ) out[i] = s % 10
}







typedef unsigned long long nat;




static inline nat power(nat m, nat n) {
	return pow(m, n);
}

static inline void do_this_thing() {
	printf("DO: doing the thing!\n");

	for (nat length = 0; length < 5; length++) {

		const nat radix = length;
		const nat Z = power(length, radix);

		for (nat z = 0; z < Z; z++) {
			print("[%llu/%llu] = ");
			print(ints
		}
	}
}









int main() {

	srand((unsigned)time(0));
	
#define max_element_count 2048

	int elements[max_element_count] = {0};
	int element_count = 0;
	
	struct pair less[max_element_count] = {0};
	int less_count = 0;

	struct pair increment[max_element_count] = {0};
	int increment_count = 0;

	struct pair reset[max_element_count] = {0};
	int reset_count = 0;

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

		print_ints("elements", elements, element_count);

		print_pairs("less", less, less_count, " < ");
		print_pairs("increment", increment, increment_count, " ---> ");
		print_pairs("reset", reset, reset_count, " ---> ");


	} else if (not strcmp(buffer, "check") or not strcmp(buffer, "c")) {
		bool b = satisfies_all_axioms(elements, element_count, 
						increment, increment_count, 
						reset, reset_count,
						less, &less_count);
		printf("result = %s\n", b ? "consistent" : "contradiction");


	} else if (not strcmp(buffer, "bubbles") or not strcmp(buffer, "bubbles")) {
		do_this_thing();
	}
	
	else if (not strcmp(buffer, "add increment")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		int input = (int) strtoul(buffer, 0, 16);
		printf("enter increment(%x) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		int output = (int) strtoul(buffer, 0, 16);

		printf("adding increment(%x) ---> %x\n", input, output);
		increment[increment_count++] = (struct pair) {input, output};
	}

	else if (not strcmp(buffer, "add reset")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		int input = (int) strtoul(buffer, 0, 16);
		printf("enter reset(%x) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		int output = (int) strtoul(buffer, 0, 16);

		printf("adding reset(%x) ---> %x\n", input, output);
		reset[reset_count++] = (struct pair) {input, output};
	}


	else if (not strcmp(buffer, "edit increment")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		int input = (int) strtoul(buffer, 0, 16);
		printf("enter increment(%x) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		int output = (int) strtoul(buffer, 0, 16);

		bool found = false;
		for (int i = 0; i < increment_count; i++) {
			if (increment[i].first == input) {
				printf("editing increment(%x) to return %x instead of %x\n", input, output, increment[i].second);
				increment[i].second = output;
				found = true;
				break;
			}
		}
		if (not found) printf("error: increment function doesnt take that input yet, consider using add instead.\n");
	}

	else if (not strcmp(buffer, "edit reset")) {
		printf("enter input UID = ");
		fgets(buffer, sizeof buffer, stdin);
		int input = (int) strtoul(buffer, 0, 16);
		printf("enter reset(%x) = ", input);
		fgets(buffer, sizeof buffer, stdin);
		int output = (int) strtoul(buffer, 0, 16);

		bool found = false;
		for (int i = 0; i < reset_count; i++) {
			if (reset[i].first == input) {
				printf("editing reset(%x) to return %x instead of %x\n", input, output, reset[i].second);
				reset[i].second = output;
				found = true;
				break;
			}
		}
		if (not found) printf("error: reset function doesnt take that input yet, consider using add instead.\n");
	}

	else if (not strcmp(buffer, "new")) {
		int new = rand();
		printf("generated new element %x, adding...\n", new);
		elements[element_count++] = new;
	}

	else if (not strcmp(buffer, "delete last")) {
		printf("removing the last element added...\n");
		if (element_count) element_count--; else puts("error: no elements to remove");
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
			"\tcheck(c) : check if the function behaviour and elements satisfies the axioms.\n"
			"\tlist(ls) : list the current state of the system.\n"
			"\thelp(?) : print this menu.\n"
			"\tquit(q) : quit the utility.\n"
			);
	}	

	goto loop;
done: 	puts("exiting...");

}






/*




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


