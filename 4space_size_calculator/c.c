#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint64_t nat;


/*
	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);

	total = (5 + D) ^ 9;
	nat k = (5 + D) ^ 3;
	for (D) {
		total *= 5;
		total *= K;
	}
*/




static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

static nat adc64(nat* d, nat s, nat cin) {
	nat sum = 0;
	nat c = cin;
	for (nat i = 0; i < 64; i++) {
		const nat a = (s >> i) & 1;
		const nat b = (*d >> i) & 1;
		sum |= ((a ^ b ^ c) << i);
		c = (a & c) | ((a ^ c) & b);
	}
	*d = sum;
	return c;
}



//   d = d + s + cin       in 128bit math.
static nat adc128(nat* dl, nat* dh, nat sl, nat sh, nat cin) {
	nat cl = adc64(dl, sl, cin);
	nat c = adc64(dh, sh, cl);
	return c;
}


//   d = r * s   in 128bit math.
static void mul128(nat* dl, nat* dh, const nat rl, const nat rh, const nat sl) {
	for (nat i = 0; i < sl; i++) {
		adc128(dl, dh, rl, rh, 0);
	}
}

static void print_binary(nat x) {
	for (nat i = 0; i < 64; i++) {
		printf("%llu", (x >> i) & 1);
	}
}

static void print128(const char* s, nat l, nat h) {
	printf("%s = ", s);
	print_binary(l); 
	putchar(' '); 
	print_binary(h); 
	puts("");
}

/*

static void print128_decimal_msb(const char* s, nat l, nat h) {
	printf("%s = ", s);
	puts("");

	struct number {
		nat l;
		nat h;
	};

	struct number given = {.l = l, .h = h};

	4349875 / 1000000

	4349875 / 100000

	4349875 / 10000

	4349875 / 1000

	4349875 / 100

	4349875 / 10

	4349875 / 1
	
	struct number total = given;
	struct number array[digit_count] = {0};

	for (i < digit_count) {
		array[i] = total
		total /= 10;
	}

	for digit_count {
		print out array[i] % 10;
	}
}
*/


int main(void) {
	const nat D = 0;

	nat space_size_64 = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	printf("64-bit space size = %llu\n", space_size_64);

	const nat K = expn(5 + D, 3);
	nat total_l = expn(5 + D, 9);
	nat total_h = 0;
	
	for (nat _ = 0; _ < D; _++) {
		mul128(   &total_l, &total_h,    total_l,total_h,    5);
		mul128(   &total_l, &total_h,    total_l,total_h,    K);
	}
	
	print128("space_size", total_l, total_h);
}





















/*


	nat dh = 4;
	nat dl = (nat) ~0;

	nat sh = 0;
	nat sl = 5;






	// mul128(&dl, &dh, sl, sh);




//print128("s", sl, sh);
	//print128("d", dl, dh);
	// mul128(&dl, &dh, sl, sh);




	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);

	total = (5 + D) ^ 9;
	nat k = (5 + D) ^ 3;
	for (D) {
		total *= 5;
		total *= K;
	}
*/

	// if (c) puts("CARRY");



/*
     0000
*    0000
--------------
0000 0000


 	  0000 0000 
	  0000 0000
---------------------
0000 0000 [0000 0000]



 	  0000 0000 
	  0000 0000
---------------------
0000 0000 [0000 0000]


*/


/*
c = 1;
for A {
	c *= B;
}
*/
/*
//   d = r ^ s   in 128bit math.
static void exp128(nat* dl, nat* dh, const nat rl, const nat rh, nat sl) {


	for 
	mul128(&cl, &ch, )

	if (sl == 0) { *dl = 0; *dh = 0; return; }
	for (nat i = 0; i < sl; i++) {
		mul128(dl, dh, rl, rh);
	}
}
*/

