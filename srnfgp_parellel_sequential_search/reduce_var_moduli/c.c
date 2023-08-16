#include <stdio.h>





int main(void) {

	const int count = 7;

	int s = 56439;

	int out[count]    = { 0,  0,  0,  0,  0,  0,  0};
	int moduli[count] = {10, 10, 11, 10, 10, 10, 10};


	int p = 1;
	for (int i = 0; i < count; i++) {
		out[i] = (s / p) % moduli[i];
		p *= moduli[i];
	}

	printf("(%u)[ ", count);
	for (int i = 0; i < count; i++) printf("%5u ", out[i]);
	printf("]\n");


}




/*

mod  2 4 3 2 3 

  0     1     2     1     0    ->    42


0 * (1) + 
1 * (2) + 
2 * (2 * 4) + 
1 * (2 * 4 * 3) + 
0 * (2 * 4 * 3 * 2)








{2, 3, 2, 2, 2};


0     1     0     1     0       ->    14 


0 * (1) + 
1 * (2) +
0 * (2 * 3) +
1 * (2 * 3 * 2) + 
0 * (2 * 3 * 2 * 2)







*/

