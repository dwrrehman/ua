//a grpah idea that we are testing out /202501131.012014: dwrr
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


typedef uint64_t nat;

#define execution_limit 100000
#define array_size 100000

int main(void) { 

	nat array[array_size] = {0};
	nat pointer = 0;
	nat comparator = 0;

	nat t = 0;
	goto init;

skip0:  if (t >= execution_limit) goto done; else t++; pointer++;
	if (comparator < array[pointer]) { comparator++; putchar(' '); goto skip1; }
	if (comparator > array[pointer]) { comparator = 0; init: array[pointer]++; putchar('#'); goto skip1; }
	array[pointer]++; pointer = 0; puts("#"); comparator++;
	if (comparator < array[pointer]) { putchar(' '); goto skip1; }
	if (comparator >= array[pointer]) { comparator = 0; array[pointer]++; putchar('#'); goto skip1; }
	putchar(' '); goto skip0;

skip1:  if (t >= execution_limit) goto done; else t++; pointer++;
	if (comparator < array[pointer]) { comparator++; putchar(' '); goto skip0; }
	if (comparator > array[pointer]) { comparator = 0; array[pointer]++; putchar('#'); goto skip0; }
	array[pointer]++; pointer = 0; puts("#"); comparator++;
	if (comparator < array[pointer]) { putchar(' '); goto skip0; }
	if (comparator > array[pointer]) { comparator = 0; array[pointer]++; putchar('#'); goto skip0; }
	putchar(' '); goto skip1;

done:; puts("");
}


/*
*n:    0


   [0] [1] [2]
------------------------
    1   1   0 

    ^
*/




















