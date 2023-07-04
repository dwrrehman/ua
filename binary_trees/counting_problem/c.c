#include <stdio.h>
#include <stdlib.h>
int main(void) {
	char buffer[512] = {0};
	fgets(buffer, sizeof buffer, stdin);
	int m = atoi(buffer);
	fgets(buffer, sizeof buffer, stdin);
	int n = atoi(buffer);
	int* a = calloc(n + 1, sizeof(int)), i = 0;
	goto print;
loop: 	if (a[i] < m) goto _3;
	if (i < n) goto _7; else return 0;
_3:	a[i]++; i = 0; 
print:	for (int _ = 0; _ < n + 1; _++) printf("%d ", a[_]); putchar(10);
	goto loop;
_7: 	a[i] = 0; i++; goto loop;
}











