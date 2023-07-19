// an RXFG utility that runs the graph using random numbers and get statistics about it. 
// by dwrr , written on 2207306.003228

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

typedef unsigned long long nat;


static nat power(nat b, nat e) {
	nat c = 1;

	for (nat i = 0; i < e; i++) {
		c *= b;
	}

	return c;
}


static nat print_hex_digit(nat digit, nat value) {
	return ((value / power(16, digit)) % 16);
}


/*
static void print_array_raw(nat* array, nat count) {
	printf("{ ");
	for (nat i = 0; i < count; i++) {
		printf("%0 ", array[i]);
	}
	printf("}\n");
}*/


static void clear_screen() { printf("\033[2J\033[H"); }


int main() {

	srand((unsigned) time(0));

	printf("this is a utility program to test out the behaviour of RXFG graphs, \n"
		"  on an effectively-infinite sequence of random-valued modnats.\n");
	

// constants:


	const nat count = 380;			// just the screen width, effectively. 

	const nat modnat_range = 10000000000000;		// effectively  M, the modulus.

	const nat pointer = count - 10;		// this is where the pointer is.   just for visual effect. 

	const nat bucket_count = 105;		// the number of bout length histogram buckets. 
	
	const nat bucket_width = 1600;		// the number of values considered as equivalent to count towards a single bucket. 

	const nat hist_scaler = 9;		// a number to stretch out the bout histogram plot to make it more obvious whats happening. 

	const nat delay = 0;			// the microseconds to delay after each iteration. 

	const nat display_rate = 19;		// the log2 of the number of ticks to wait before printing another frame of display.



// init variables:

	nat comparator = 0;

	bool* modes = calloc(count, sizeof(bool));
	

	nat* array = malloc(count * sizeof(nat));

	for (nat i = 0; i < count; i++) 
		array[i] = (((nat) rand()) | ((nat)rand() << 32)) % modnat_range + 1; 			// non zero modnats only. 
	

	nat total_count = 0;

	nat Xet_count = 0;
	nat Xef_count = 0;
	//nat Ret_count = 0;
	//nat Ref_count = 0;


	nat tick = 0;
	
	nat current_bout_length = 0;
	nat bout_count = 0;
	nat* buckets = calloc(bucket_count, sizeof(nat));



// main loop:

	while (1) {

		/*

			current rrxfg graph, wrote on 2207306.024804


		_E: 	if (sn < *i) goto _2;
		 	++*i;
			sn = 0;
			goto _1;
		_2:	sn++;
		_1: 	i++;
			goto _E;


		adj list representation:

			21 36 61 1E Ef3 Et2


		*/



		
// perform RXFG graph:

		if (comparator < array[pointer]) { 	// Xe(t)
			comparator++; // 2
			Xet_count++;

		} else { 			// Xe(f)
			array[pointer]++; // 3
			comparator = 0;    // 6

			modes[pointer] = true;
			// update bout length buckets:
			const nat c = current_bout_length;
			for (nat i = 0; i < bucket_count; i++) {
				if (c < (1 + i) * bucket_width) {
					buckets[i]++;
					break;
				}
			}
			bout_count++;
			current_bout_length = 0;
			Xef_count++;
		}

		// we do an implicit 1 via shifting the array, 
		// later on, and then loop back to the top. 


		current_bout_length++;
		total_count++;

		







		/*
		if (comparator < array[pointer]) {      // Re(t)
			// do nothing, 
			Ret_count++;
		} else {			// Re(f)
			comparator = 0; 
			Ref_count++;
		}
		*/

		








// display the array, pointer, and IA's:

	if (not (tick & ((1 << display_rate) - 1))) {
		tick = 0;


		clear_screen();

		printf("*n = %llx     m = %llx\n", comparator, modnat_range);
		printf("*i = %llx\n", array[pointer]);

		for (nat i = 0; i < pointer; i++) 
			printf(" ");
		printf("*\n");


		for (nat i = 0; i < count; i++) 
			printf("%c", modes[i] ? 'I' : ' ');
		printf("\n");


		for (nat p = 0; p < 7; p++) { // log base 16 of range
			for (nat i = 0; i < count; i++) {
				printf("%llx", print_hex_digit(p, array[i]));
			}
			printf("\n");
		}




		printf("\n\t%lld: xt:%lld(%lf), xf:%lld(%lf)\n",        // Rt:%lld(%lf), Rf:%lld(%lf)\n",  
			total_count, 
			Xet_count, (double) Xet_count / (double) total_count,
			Xef_count, (double) Xef_count / (double) total_count
			//Ret_count, (double) Ret_count / (double) total_count,
			//Ref_count, (double) Ref_count / (double) total_count
		);

		printf("\ngraph hist: \n");
{
		double f = 0;
		nat c = 0;

		const nat w = count / 2;

		printf("Xt: ");
		f = (double) Xet_count / (double) total_count;
		c = (nat) (f * (double) w);
		for (nat i = 0; i < w; i++) printf("%c", i < c ? '#' : '`'); 
		printf("\n");

		printf("Xf: ");
		f = (double) Xef_count / (double) total_count;
		c = (nat) (f * (double) w);
		for (nat i = 0; i < w; i++) printf("%c", i < c ? '#' : '`'); 
		printf("\n");
/*
		printf("Rt: ");
		f = (double) Ret_count / (double) total_count;
		c = (nat) (f * (double) w);
		for (nat i = 0; i < w; i++) printf("%c", i < c ? '#' : '`'); 
		printf("\n");

		printf("Rf: ");
		f = (double) Ref_count / (double) total_count;
		c = (nat) (f * (double) w);
		for (nat i = 0; i < w; i++) printf("%c", i < c ? '#' : '`'); 
		printf("\n");


*/
}			

		if (bout_count) {
			printf("\nhistogram: (bw=%llu,bc=%llu,hs=%llu) \n", bucket_width, bucket_count, hist_scaler);
			for (nat b = 0; b < bucket_count; b++) {
				printf("\t%lld: ",b);    //" %lld / %lld (%lf)\n", b, buckets[b], bout_count, (double) buckets[b] / (double) bout_count);

				double f = (double) buckets[b] / (double) bout_count;
				nat c = (nat) (f * (double) count * hist_scaler);
				for (nat i = 0; i < c; i++) printf("#"); 
				printf("\n");
			}
		}

		fflush(stdout);

	} //if(tick)

	


// instead of advancing the pointer, we instead move the array in the opposite direction. 
// then fill in the last value with something new. 


		memmove(array, array + 1, sizeof(nat) * (count - 1));
		array[count - 1] = (nat) rand() % modnat_range + 1;

		memmove(modes, modes + 1, sizeof(bool) * (count - 1));
		modes[count - 1] = false;

		if (delay) usleep(delay);
		tick++;
	} // while







/*
	char buffer[128] = {0};
	printf(": ");
	fgets(buffer, sizeof buffer, stdin);

	const int value = atoi(buffer); 
	printf("for reference: %d = 0x%x\n", value, value);
	for (int i = 0; i < 5; i++) {
		printf("%d(%d) = %x\n", value, i, print_hex_digit(i, value));
	}
*/

}





