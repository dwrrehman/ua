#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>
#include <stdbool.h>
typedef uint64_t nat;

static bool contains(nat first, nat second, nat* relation, nat* count) {
	for (nat i = 0; i < *count; i += 2) {
		if (relation[i + 0] == first and 
		    relation[i + 1] == second) return 1;
	}
	return 0;
}

static bool inserted(nat first, nat second, nat* relation, nat* count) {
	if (contains(first, second, relation, count)) return 0;
	relation[(*count)++] = first;
	relation[(*count)++] = second;
	return 1;
}

static nat is_consistent(
	nat* array, const nat count,
	nat* less, nat* lcount,
	nat* equal, nat* ecount
) {
	*ecount = 0; *lcount = 0;
	nat* s = array, * z = array + count;
repeat:

	for (nat i = 0; i < count; i++) {
		for (nat j = 0; j < count; j++) {

			if (contains(  i , z[j], less,  lcount)) return 1;
			if (contains(  i , s[j], less,  lcount)) return 1;
			if (contains(z[i],   j , less,  lcount)) return 1;
			if (contains(s[i],   j , less,  lcount)) return 1;

			if (contains(  i , z[j], equal, ecount)) return 1;
			if (contains(  i , s[j], equal, ecount)) return 1;
			//if (contains(z[i],   j , equal, ecount)) return 1;
			//if (contains(s[i],   j , equal, ecount)) return 1;

			if (contains(z[i], z[j], less,  lcount)) return 1;
			if (contains(z[i], s[j], less,  lcount)) return 1;
			if (contains(s[i], z[j], less,  lcount)) return 1;
			if (contains(s[i], s[j], less,  lcount)) return 1;

			if (contains(z[i], z[j], equal, ecount)) return 1;
			if (contains(z[i], s[j], equal, ecount)) return 1;
			//if (contains(s[i], z[j], equal, ecount)) return 1;
			if (contains(s[i], s[j], equal, ecount)) return 1;

			// inserted, goto repeat; // contains, return 1;
		}
	}



	/*for (nat i = 0; i < count; i++) {
		if (contains(i, i,    less, lcount)) return 1;
		if (contains(i, z[i], less, lcount)) return 3;
		if (inserted(i, s[i], less, lcount)) goto repeat;

		// duplicate:  if (contains(i, i, less, lcount)) return 1;

		if (contains(s[i], i, less, lcount)) return 2;
		if (inserted(z[i], i, less, lcount)) goto repeat;

		if (inserted(i, i,    equal, ecount)) goto repeat;
		if (contains(i, s[i], equal, ecount)) return 8;
		if (contains(i, z[i], equal, ecount)) return 7;

		if (contains(s[i], z[i], less,  lcount)) return 5;
		if (inserted(z[i], s[i], less,  lcount)) goto repeat;
		if (contains(z[i], s[i], equal, ecount)) return 10;
	}*/




		
		//if (contains(i, z[i], equal, ecount)) return 9;      // are zero resets allowed?.... 

		/*
		for (nat j = 0; j < count; j++) {
			if (contains(z[i], s[j], equal, ecount)) return 10;
			if (inserted(z[i], s[j], less,  lcount)) goto repeat;
			if (contains(z[i], z[j], less,  lcount)) return 5;
			if (inserted(z[i], z[j], equal, ecount)) goto repeat;

			if (contains(i, j, less,  lcount) and contains(j, i, less,  lcount)) return 14;			
			if (contains(i, j, less,  lcount) and contains(s[i], s[j], equal, ecount)) return 13;
			if (contains(i, j, less,  lcount) and inserted(s[i], s[j], less,  lcount)) goto repeat;
			if (contains(i, j, equal, ecount) and inserted(s[i], s[j], equal, ecount)) goto repeat;
			if (contains(i, j, equal, ecount) and contains(s[i], s[j], less,  lcount)) return 12;
		}
		*/
	//}
	return 0;
}


int main(void) {
	const nat element_count = 1;

	const nat k = element_count + 1;
	nat less_count = 0, equal_count = 0;
	nat* less = calloc(2 * k * k, sizeof(nat));
	nat* equal = calloc(2 * k * k, sizeof(nat));

	const nat modulus = element_count + 1, numerus = (element_count << 1) - 1;
	nat* array = calloc(numerus + 1, sizeof(nat));
	nat pointer = 0;
	goto init;
loop:
	if (array[pointer] < modulus) goto increment;
	if (pointer < numerus) goto reset;
	goto done;

increment:
	array[pointer]++;
	pointer = 0;
init:;

	const nat c = is_consistent(
		array, element_count, 
		less, &less_count, 
		equal, &equal_count
	);

	if (c) goto loop;

	printf("array: [ ");
	for (nat i = 0; i < numerus + 1; i++) {
		printf("%llu ", array[i]);
	}
	printf("] ---> %llu %s\n", c, not c ? "\033[32;1m[true]\033[0m" : "");


	printf("\t\t\t\t\t\tS[ ");
	for (nat i = 0; i < element_count; i++) {
		printf("incr[%llu] = %llu, ", i, array[i]);
	}
	printf("]\n");

	printf("\t\t\t\t\t\tZ[ ");
	for (nat i = 0; i < element_count; i++) {
		printf("reset[%llu] = %llu, ", i, array[i + element_count]);
	}
	printf("]\n\n");

	printf("\t\t\t\t\t\t<[ ");
	for (nat i = 0; i < less_count; i += 2) {
		printf("%llu < %llu, ", less[i], less[i + 1]);
	}
	printf("]\n\n");	

	printf("\t\t\t\t\t\t=[ ");
	for (nat i = 0; i < equal_count; i += 2) {
		printf("%llu == %llu, ", equal[i], equal[i + 1]);
	}
	printf("]\n\n\n");

	goto loop;

reset:
	array[pointer] = 0;
	pointer++;
	goto loop;

done:
	puts("done!");
}















/*




202403063.202101:

				i think we can actually make a new axiom,   maybe:
							

							reset(x) < x    is true!     becuase basically,
											you cant reset 0, so this works out!!

											wow nice 

							yay
									kinda complementary  to    x < reset(x)   is false!! nice.


								hmm

						of course, we also know that          x == reset(x)   is false     but yeah. 


										yeah, the < relation is order dependant, so there is technically a tonnn of other relations we could add    that are technicallyyyy      greater than!?!?!

								like, we would then have 


								<   =   and       >   



								axioms for all of those.   wow thats actually so cool



					yeah, in hardware,   theres a trichotomy anyways!      trichotomies are probably the right way to go, in terms of branches, actually. wow. interesting. 

		hm




	so yeah

	we'll try adding axioms for those lol 

	yay 



	














clang -g -O0 c.c -Weverything -fsanitize=address,undefined

./a.out



static nat execute(nat input, nat* function, nat count) {
	for (nat i = 0; i < count; i++) {
		if (i == input) return function[i];
	}
	printf("\033[31;1merror:\033[0m execute could not find entry for %llx\n", input);
	return 0;
}


			//if (not contains(i, j, less, lcount) and contains(s[i], s[j], less, lcount)) return 15;
			//if (not contains(i, j, equal, ecount) and contains(s[i], s[j], equal, ecount)) return 16;
			//if (not contains(i, j, equal, ecount) and contains(s[i], s[j], equal, ecount)) return 16;






---------------- the axioms that this utility uses: -------------------------




	ax0.		x : N					 			 is true.


---------------
   < axioms:
---------------
	l.ax1.		for all x : N,      x < x                                        is false.

	l.ax2.  	for all x : N,      x < _asht_(x)   				 is true.

	l.ax3.   	for all x : N,      x < _neoi_(x)   				 is false.

	l.ax4.		for all x,y : N,    _neoi_(x) < _asht_(y)  			 is true.

	l.ax5.		for all x,y : N,    _neoi_(x) < _neoi_(y)     			 is false.

	l.ax6.		for all x,y : N,    if  x < y,   then _asht_(x) < _asht_(y)      is true.


---------------
   == axioms:
---------------

	e.ax1.		for all x : N,      x == x                                       is true.

	e.ax2.  	for all x : N,      x == _asht_(x)   				 is false.

	e.ax3.   	for all x : N,      x == _neoi_(x)   				 is false.   <--- ?????

	e.ax4.		for all x,y : N,    _neoi_(x) == _asht_(y)  			 is false.

	e.ax5.		for all x,y : N,    _neoi_(x) == _neoi_(y)     			 is true.

	e.ax6.		for all x,y : N,    if  x == y,   then _asht_(x) == _asht_(y)    is true.   <---- ???




*/









