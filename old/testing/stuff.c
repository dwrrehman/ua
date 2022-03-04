#include <stdio.h>              // 2104143.150249, dwrr
#include <stdlib.h>		// a utility to try to find different control flow graphs (cfg's) that use 
#include <unistd.h>             // a particular instruction set, that are candidate ua graphs, 
#include <string.h>              // when formulating the ua as in the m=inf,n=inf expansion pattern formulation. (xf)
#include <stdint.h>
#include <math.h>
#include <iso646.h>


 // build with :    clang stuff.c -o out -Weverything -Wno-poison-system-directories



typedef uint64_t number;







// static inline void debug(int* a, int i) {
// 	printf(" i = %3d    |   { ", i);
// 	for (int k = 0; k < pseudo_n; k++) {
// 		if (a[k]) printf("%3d ", a[k]);
// 		else printf("    ");
// 	}
// 	printf("}\n");
// 	getchar();
// }



static inline number index_into(number* array, number size, number index, number* error) {
	return index < size ? array[index] : (*error = 1);
}





/*


	the standard form of "code" is going to be 

	an array of pairs of ints.     ie, just two vectors!
	
	the index of the pair represnets the instruction, and is always found in the following order:

		0: blt [l,k]
		1: bne [l,k]
		2: i = 0 [l]
		3: i++ [l]
		4: *0 = 0 [l]
		5: *0++ [l]
		6: *i++ [l]
		

	false is always of length two. 
		because there are two branches, 


	and true is always of length 7 		i guess

	and actually, we can just put false inside true, at the end, really. 

	so thats cool.


	so basically, every number can actually only ever be in the lrs of 7

				because there are 7 ins, 

		but there will be 9 numbers. 
			so yeah, thats easy. 

	7 * 7 * 7 * 7 * 7 * 7 * 7     *   7 * 7


	okay whattttt             thats 40 million           40353607       to be precise



	okay, so we have to be alittle bit more methodical than that lol




		well, i mean, i guess its actually okay, 

	because, like, we can just ween down the search space, using known constaints, 

		like, the vlaue at [8] is differnt from [0], etc. 


				no instruction shoudl have more than 3p arents, etc. 


			there are tons of constraints, that we have on the xfcfg,
			which we can use to totally reduce down the graph space. 

				but yeah, the raw graph space, at least, naively, is 40 million.


			totally doable on my llaptop honestly, 


					so yay



	


*/



/*

	data is sized    size
	code is sized    9 ints long 
			where each int is a value 0, through 6  inclusive on both.
	and size is less than 255
*/

static inline void execute(number* code, number* data, number size) {
	memset(data, 0, sizeof(number) * size);

	
}








number unreduce(number* in, number radix, number length) {
    number graph = 0;
    for (number i = 0, p = 1; i < length; i++, p *= radix)
        s += p * in[i];
    return s;
}

void reduce(number* out, number s, number radix, number length) {
    for (number i = 0, p = 1; i < length; i++, p *= radix)
        out[i] = (s / p) % radix;
}


int main() {

	const double d = pow((double)7, (double)9); // length 9 array, each element in lrs(7).
	const number total_graphs = (graph) d;


	const number max_found = 1000;
	number* found = malloc(sizeof(number) * max_found);
	number found_count = 0;


	const number size = 20;

	number* code = malloc(sizeof(number) * 9);
	number* data = malloc(sizeof(number) * size);

	for (number index = 0; index < total_graphs; index++) {
		reduce(code, index, 7, 9);
		
		if (is_valid(code) and execute(code, data, size)) {
			found[found_count++] = index
		}
	}






}





// 	int i = 0;
// 	int* a = calloc(pseudo_n, sizeof(int));	

// _0: 	
// 	if (a[0] < a[i]) goto _5;
// 	else goto _4;

// _1: 	
// 	if (a[0] == a[i]) goto _2;
// 	else goto _3;

// _2: 	
// 	i++; 
// 	debug(a, i); 
// 	goto _0;

// _3:  	
// 	i = 0; 
// 	debug(a, i);
// 	goto _5;

// _4: 	
// 	(a[i])++; 
// 	debug(a, i);
// 	goto _1;

// _5:  	
// 	(a[0])++; 
// 	debug(a, i);
// 	goto _2;


