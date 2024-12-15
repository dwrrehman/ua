#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>


static void print_state(
	const int* pointers, 
	const int* elements, 
	const int N,
	const int r, 
	const int i, 
	const int k
) {
	if (elements) {
		printf("E(%2u){ ", N);
		for (int e = 0; e < N; e++) {
			printf("%2u ", elements[e]);

		}
		printf("}   ");
	}

	printf("P(%2u){ ", k);
	for (int e = 0; e < k; e++) {
		printf("%2u ", pointers[e]);
	}
	printf("}   ");
	printf(" N=%2u r=%2d i=%2d k=%2u    ", N, r, i, k);

	if (elements) {
		printf(" -----> { ");
		for (int e = 0; e < k; e++) {
			printf("%2u ", elements[pointers[e]]);
		}
		printf("}");
	}
	puts("");
}

static void combination(
	const int* elements, 
	const int N, 
	const int k
) {	
	if (not k) return;
	int count = 0;
	int* pointers = calloc(k, sizeof(int));	
	int r = 0, i = 0;
	while (r >= 0) {	
		if (i <= (N + (r - k))) {
			pointers[r] = i;
			if (r == k - 1) {
				print_state(pointers, elements, N, r, i, k);
				count++;
				i++;				
			} else {
				i = pointers[r] + 1;
				r++;
			}				
		} else {
			r--;
			if (r >= 0) i = pointers[r] + 1;			
		}
	}
	printf("--> n choose k = %u\n", count);
}





static void index_combination(int n, int k) {
	if (not k) return;
	int* pointers = calloc(k, sizeof(int));	
	int count = 0, r = 0, i = 0;
	while (r >= 0) {
		if (i <= (n + (r - k))) {
			pointers[r] = i;	
			if (r == k - 1) {
				print_state(pointers, NULL, n, r, i, k);
				count++;
				i++;
			} else {
				i = pointers[r] + 1;
				r++;
			}				
		} else {
			r--;
			if (r >= 0) i = pointers[r] + 1;
		}			
	}
	printf("--> n choose k = %u\n", count);
}




int main(void) {

{
	puts("6 choose 3");
	const int N = 6;
	const int k = 3;
	const int array[] = {2, 3, 5, 7, 11, 13};
	combination(array, N, k);
}
getchar();
puts("\n\n");
{
	puts("3 choose 2");
	const int N = 3;
	const int k = 2;
	const int array[] = {12, 5, 8};
	combination(array, N, k);
}

getchar();
puts("\n\n");
{
	puts("3 choose 0");
	const int N = 3;
	const int k = 0;
	const int array[] = {12, 5, 8};
	combination(array, N, k);
}

getchar();
puts("\n\n");
{
	puts("3 choose 1");
	const int N = 3;
	const int k = 1;
	const int array[] = {12, 5, 8};
	combination(array, N, k);
}

getchar();
puts("\n\n");
{
	puts("3 choose 3");
	const int N = 3;
	const int k = 3;
	const int array[] = {12, 5, 8};
	combination(array, N, k);
}




getchar();
puts("\n\n");
{
	puts("18 choose 3");
	const int N = (28 - 4) - 6;
	const int k = 3;
	index_combination(N, k);
}

getchar();
puts("\n\n");
{
	puts("18 choose 2");
	const int N = (28 - 4) - 6;
	const int k = 2;
	index_combination(N, k);
}

getchar();
puts("\n\n");
{
	puts("18 choose 4");
	const int N = (28 - 4) - 6;
	const int k = 4;
	index_combination(N, k);
}

getchar();
puts("\n\n");
{
	puts("18 choose 5");
	const int N = (28 - 4) - 6;
	const int k = 5;
	index_combination(N, k);
}


	for (int i = 0; i < 14; i++) puts("\n");
	exit(0);
}





























/* 
	combination c n p x

	get the x-th lexicographically ordered set of p elements in n
	output is in "c", and should be sizeof(int) * p 

static void combination(int* c, int n, int p, int x) {
    int r = 0, k = 0;
    for (int i = 0; i < p - 1; i++) {
        c[i] = i ? c[i - 1] : 0;
        do {
            c[i]++;
            r = choose(n - c[i], p - (i + 1));
            k = k + r;
        } while(k < x);
        k = k - r;
    }
    c[p - 1] = c[p - 2] + x - k;
}

*/



