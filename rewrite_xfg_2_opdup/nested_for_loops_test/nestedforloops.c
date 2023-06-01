#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>


typedef unsigned long long nat;

	



static void print(nat* array, nat count) {
	printf("array(%2llu): [ ", count);
	for (nat i = 0; i < count; i++) {
		printf("%2llu ", array[i]);
	}
	printf("]\n");
}





static void NF(const nat n, const nat m) {
	nat* stack = calloc(n + 1, sizeof(nat));
loop:; 	nat tos = 0;

	print(stack, n + 1);
	usleep(10000);

label:	if (stack[tos] >= m) {
		stack[tos] = 0;
		if (tos == n) goto done;
		tos++;
		goto label;
	} 
	stack[tos]++;
	goto loop;
done:;
}







static nat exponentiate(const nat a, const nat b) {
	nat c = 1;
	for (int i = 0; i < b; i++) {
		c *= a;
	}
	return c;
}




int main() {
	puts("testing an algorithm to compute iterators of nested for loops (in general) without actually using nested for loops.");


	printf("0^0 = %llu\n", exponentiate(0, 0));
	printf("1^0 = %llu\n", exponentiate(1, 0));
	printf("0^1 = %llu\n", exponentiate(0, 1));
	printf("2^3 = %llu\n", exponentiate(2, 3));
	printf("3^4 = %llu\n", exponentiate(3, 4));
	printf("4^7 = %llu\n", exponentiate(4, 7));

	// NF(0, 5);
}

































// nat tried_count = 0;
	// nat* tried = calloc(5 * 5 * D, sizeof(nat));










//	for (nat i1 = 0; i1 < 5; i1++) {
//		for (nat i2 = 0; i2 < 5; i2++) {




/*

	
			
		0 0 0 0 0 0 0 1 1
		                ^   
	
	


*/






//			const nat op1 = operations[i1];
//			const nat op2 = operations[i2];

//			if (have_tried(tried_count, tried, op1, op2)) continue;




//			for (nat o = 0; o < D; o++) {
//				tried[D * tried_count + offset] = op[o];
//			}
//			tried_count++;
//		}
//	}



	

/*

	if (p.should_print) {
		print_combinations(tried, tried_count);
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












2210156.174119




static void NF(const nat n, const nat m) {

	nat pointer = 0;
	nat* array = calloc(n + 1, sizeof(nat));




_B9:	if (array[pointer] < m) goto _3;                // B
	if (pointer == n) exit(0);                      // 9
	goto _7;



_7:	array[pointer] = 0;                             // 7
	pointer++; goto _B9;                            // 1



_3:	array[pointer]++;                               // 3
 	pointer = 0; goto _B9;                          // 5













2210241.021342


		turns out you can actually switch the 9 and 7 instructions,    and you can get the nf algorithm to drastically simplify to simply be 

					a rank 1 control graph, which looks similar to the R,   except its still finite, and it doesnt have the comparator,   and it puts 7 with 1, in a loop,       when   *i >= m,     and it puts   5 with 3,  (which is present still in the xfg!)

										when  *i < m





					very very very very very very very interesitng!!!


							i really really like this graph. 

								i will keep it in mind, strongly, when going forward with the xfg stuff 

	
										yay






















}



			///   this list of instructions       is literally all possible instructions in the FUA 
					
					EXCEPT      the ones that involve (in any way)      the comparator.

								so thats interesting.



its missing:


	array[n]++; goto __;                           			// 2

	array[n] = 0; goto __;                         			// 6

  	
 	if (array[n] < array[pointer]) goto __; else goto __; 		// E

	if (array[n] > array[pointer]) goto __; else goto __; 		// F

	if (array[n] != array[pointer]) goto __; else goto __;     	// C








	
				the graph edges that it is actually telling us are:


					(SANSS   the comparator ins's!    very important.    its removing all of those ins that involve the comparaotor. and this is what we are left with, possibly)





				5B              Bt3             Bf7             35                79             9t1


				 9f0              1B



	 




			
	








what if n and m were infinite?




_0: 	pointer = 0;                                   // 5
_1:	if (array[pointer] < m) goto _2;               // B
	array[pointer] = 0;                            // 7
	pointer++; goto _1;                            // 1
_2:	array[pointer]++; goto _0;                     // 3




				this is the algorithm   ^ ^ ^ ^      if      n = inf   but m is finite 







_0: 	pointer = 0;                                   // 5
	array[pointer]++; goto _0;                     // 3



								the algorithm for both n = m = inf 


										very simple 





		so, as we can see, there is definitely alot of zero reseting happening with the pointer, 

		but,   it definitely isnt a c=2 xfg type of lifetime, at all...    its more of a c=1 xfg lifetime.



							

				with alot of zero reseting
















































_B9:	if (array[pointer] < m) goto _3;                // B
	if (pointer == n) exit(0);                      // 9
	goto _7;



_7:	array[pointer] = 0;                             // 7
	pointer++; goto _B9;                            // 1



_3:	array[pointer]++;                               // 3
 	pointer = 0; goto _B9;                          // 5











*/
