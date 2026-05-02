
/*

1202605015.221746

size of 2sp = 7^12 * (5 * (7^3))^2


z_is_good:

       165
       145
       152
       106
       231
       147
       171
       199
       111
       210
---------------------
      1637   <---- sum




size of 3sp = 8^12 * (5 * (8^3))^3


z_is_good:

   4872746
   3923316
   3317637
   2776360
   3499478
   3720487
   4099371
   3414289
   3471357
   3895916
---------------------
  36990957   <---- sum



*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

int main(int argc, const char** argv) {	
	if (argc == 1) return puts("usage error");
	FILE* file = fopen(argv[1], "r");
	if (not file) { perror("fopen"); exit(1); }
	fseek(file, 0, SEEK_END);
	size_t length = (size_t) ftell(file);
	char* buffer = calloc(1, length + 1);
	fseek(file, 0, SEEK_SET);
	fread(buffer, 1, length, file);
	printf("read file: length = %lu\n", length);
	size_t line_start = 0;
	size_t line_length = 0;

	size_t sum = 0;

	for (size_t i = 0; i < length; i++) {
		if (buffer[i] == 10) {
			line_start += strlen("z_is_good: ");
			//printf("line: <<<%.*s>>>\n", (int) line_length, buffer + line_start);		
			size_t number_length = 0;
			for (size_t j = 0; j < line_length; j++) {
				if (buffer[line_start + j] == 9) break;
				number_length++;
			}
			size_t n = (size_t) atoi(strndup(buffer + line_start, number_length));
			//if (not n) printf("added count of 0 to sum...\n");
			printf("%10lu\n", n);
			sum += n;
			line_length = 0;
			line_start = i + 1;
		} else {
			line_length++;
		}
	}
	puts("---------------------");
	printf("%10lu   <---- sum\n", sum);
}





































