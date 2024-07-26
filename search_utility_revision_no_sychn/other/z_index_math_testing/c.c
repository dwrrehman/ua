// a program to test out the division and multiplication of large zi 
// using multi precision integer arith. written on 202407265.111210: dwrr

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t nat;
typedef uint64_t chunk;
#define chunk_count 8

static nat adc_raw(nat* d, nat s) {
	*d += s;
	return *d < s;
}

static nat adc_chunk(nat* d, nat s, nat cin) {
	nat cout = adc_raw(d, s) | adc_raw(d, cin);
	return cout;
}

static nat adc(chunk* d, chunk* s, nat cin) {
	nat c = cin;
	for (nat i = 0; i < chunk_count; i++) {
		c = adc_chunk(d + i, s[i], c);
	}
	return c;
}

/*static void mul_small(chunk* d, nat s) {

	nat copy[chunk_count] = {0};
	memcpy(copy, d, chunk_count * sizeof(nat));

	for (nat i = 0; i < s - 1; i++) 
		adc(d, copy, 0);
}*/

static nat shift_left(chunk* d) {
	nat c = 0;
	for (nat i = 0; i < chunk_count; i++) {
		const nat save_c = !!(d[i] & 0x8000000000000000);
		d[i] = (d[i] << 1) | c;
		c = save_c;
	}
	return c;
}

static void bitwise_invert(chunk* d) {
	for (nat i = 0; i < chunk_count; i++) {
		d[i] = ~(d[i]);
	}
}

static nat get_bit(chunk* a, nat i) {
	const nat chunk = i / 64LLU;
	const nat bit_in_chunk = i % 64LLU;
	return (a[chunk] >> bit_in_chunk) & 1LLU;
}

static void set_bit(chunk* a, nat i) {
	const nat chunk = i / 64LLU;
	const nat bit_in_chunk = i % 64LLU;
	a[chunk] |= 1LLU << bit_in_chunk;
}

static void print_binary64(nat x) {
	for (nat i = 0; i < 64; i++) {
		printf("%llu", (x >> i) & 1);
	}
}

static void debug_zi(const char* s, chunk* number) {
	printf("%s = {\n", s);
	for (nat i = 0; i < chunk_count; i++) {
		printf("[%llu]: ", i);
		print_binary64(number[i]); 
		printf("  (dec %llu)\n", number[i]);
	}
	puts("} ");
}

static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

static nat ge_r_s(nat* r, nat* s) {
	nat copy[chunk_count] = {0};
	memcpy(copy, s, chunk_count * sizeof(nat));
	bitwise_invert(copy);
	return adc(copy, r, 1);
}

int main(void) {
	
	const nat D = 5;
	nat space_size_64 = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	printf("64-bit space size = %llu\n", space_size_64);

	const nat K = expn(5 + D, 3);
	nat total[chunk_count] = {0};
	total[0] = expn(5 + D, 9);

	for (nat _ = 0; _ < D; _++) {
		nat copy[chunk_count] = {0};
		memcpy(copy, total, chunk_count * sizeof(nat));
		for (nat i = 0; i < 5 - 1; i++)  adc(total, copy, 0);
		memcpy(copy, total, chunk_count * sizeof(nat));
		for (nat i = 0; i < K - 1; i++) adc(total, copy, 0);
	}
	
	debug_zi("final_space_size", total);

	nat a[chunk_count] = {0};
	nat b[chunk_count] = {0};
	nat r[chunk_count] = {0};
	nat q[chunk_count] = {0};

	memcpy(a, total, chunk_count * sizeof(nat)); 
	*b = 10 * 5;

	nat negative_b[chunk_count] = {0};
	memcpy(negative_b, b, chunk_count * sizeof(nat)); 
	bitwise_invert(negative_b);
	adc(negative_b, r, 1);

	debug_zi("b", b);	

	for (nat i = 64 * chunk_count; i--; ) {
		shift_left(r);
		*r |= get_bit(a, i);
		if (ge_r_s(r, b)) {  
			adc(r, negative_b, 0);
			set_bit(q, i);
		}
	}

	debug_zi("q", q);
	debug_zi("r", r);
}




 // memcpy(q, total, chunk_count * sizeof(nat)); 


















       //        a = 029483502938450928350698235615      b = 3
// memcpy(q, total, chunk_count * sizeof(nat));




/**



nat q = 0, r = 0;

for (nat i = 63; i--; {
  R <<= 1;
  R += (a >> i) & 1;
  if (r >= b) {
    r −= b;
    q |= 1 << i;
  }
}




*/














/*


	nat copy[chunk_count] = {0};
	memcpy(copy, total, chunk_count * sizeof(nat));
	// bitwise_invert(copy);

	nat n1[chunk_count] = {0};

	bitwise_invert(n1);
	adc(total, n1, 0);

	debug_zi("total", total);
	printf("%llu\n", *total);

	debug_zi("copy", copy);
	printf("%llu\n", *copy);
	
	nat condition = ge_r_s(copy, total);
	printf("total - total -    greater than or equal to? = %llu\n", condition);


*/


/*
nat copy[chunk_count] = {0};
	memcpy(copy, total, chunk_count * sizeof(nat));

	bitwise_invert(copy);
	
	debug_zi("zero", total);
	printf("%llu\n", *total);





*/
















/*

	x *= 2;

	x += x

	x *= 1     ---->   x = x * 1



*/
























/*



nat cin = 1;
	nat r = 0xFFFFFFFFFFFFFFF0;//-1;//1;//;
	nat s = 0xFF;//1;//2;//;
	nat d = r;

	nat c = adc_chunk(&d, s, cin);
	printf("%lld + %lld ( + carry=%llu )  ==> %lld (carry = %llu) \n", r, s, cin, d, c);



// adc(total, copy, 1);



*/









		//debug_zi("5_space_size", total);
		//printf("%llu\n", *total);


	//debug_zi("init_space_size", total);
	//printf("%llu\n", *total);
	
		//mul_small(total, total, 5); 


//debug_zi("5_space_size", total);
			//printf("%llu\n", *total);



//mul_small(total, total, K);
//debug_zi("5_space_size", total);
			//printf("%llu\n", *total);


/*
	int a = 19;
	int b = 5;

	//  a / b

	int q = 0, r = 0;
	while (a >= b) {
		a -= b;
		q++;
	}
	r = a; 
	printf("q = %u, r = %u\n", q, r);
*/





202407265.112133:


781 250 000 000 000 000
 q   T   b   m   t   h


times 4 billion        is   roughlyyy

the size of 5 space. 



17096717051798806126

times 
4000

is the size of   4 space!



wow









