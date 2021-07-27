#include <stdio.h>// 2104143.14, dwrr
#include <stdlib.h>  // testing the latest ua control flow graph code.. in th xfcfg search.
				// doesnt work, it seems.



 // build with :    clang main.c -o out -Weverything -Wno-poison-system-directories


// XF: m = n = inf
static const int pseudo_n = 20;    // obviously we cant allow n to actually be infinite... "unfrortunately".... lol



static inline void debug(int* a, int i) {
	printf(" i = %3d    |   { ", i);
	for (int k = 0; k < pseudo_n; k++) {
		if (a[k]) printf("%3d ", a[k]);
		else printf("    ");
	}
	printf("}\n");
	getchar();
}

int main() {

	int i = 0;
	int* a = calloc(pseudo_n, sizeof(int));	

_0: 	
	if (a[0] < a[i]) goto _5;
	else goto _4;

_1: 	
	if (a[0] == a[i]) goto _2;
	else goto _3;

_2: 	
	i++; 
	debug(a, i); 
	goto _0;

_3:  	
	i = 0; 
	debug(a, i);
	goto _5;

_4: 	
	(a[i])++; 
	debug(a, i);
	goto _1;

_5:  	
	(a[0])++; 
	debug(a, i);
	goto _2;

}

