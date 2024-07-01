#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint64_t nat;



static void mul128(nat* dl, nat* dh, nat sl, nat sh) {




}


static void exp128(nat* dl, nat* dh, nat sl, nat sh) {


}



static void print_binary(nat x) {
	for (nat i = 0; i < 64; i++) {
		printf("%d", (x >> i) & 1);
	}
}

int main() {


	nat dh = 1;
	nat dl = 0;

	nat sh = 0;
	nat sl = 2;


	mul128(&dl, &dh, sl, sh);
	
	printf("d = ");
	print_binary(dl); putchar(' '); 
	print_binary(dh); puts("");

}