#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

typedef uint64_t nat;


static const nat height = 100;
static const nat width  = 300;


struct row {
	char* name;
	double* points;
	nat count;
	nat color;
};

static struct row* data = NULL; 
static nat data_count = 0;

int main(int argc, const char** argv) {

	if (argc != 2) return puts("usage: ./plot <data.csv>");

	FILE* f = fopen(argv[1], "r");
	if (not f) { perror("fopen"); exit(1); }

	char number[4096] = {0};
	nat number_count = 0;

	while (1) {
		const int c = fgetc(f);
		if (c == 10 or c == ',') {
			data = realloc(data, sizeof(struct row) * (data_count + 1));
			data[data_count++] = (struct row) {.name = strdup(number), .color = 0, .points = NULL, .count = 0};
			memset(number, 0, sizeof number);
			number_count = 0;
		}
		if (c == 10) break;
		if (c == ',') continue;
		if (isspace(c)) continue;
		number[number_count++] = (char) c;
	}

	nat index = 0;

	while (1) {
		const int c = fgetc(f);
		if (c == EOF) break;

		if (c == 10 or c == ',') { 
 			double x = strtod(number, NULL);
			memset(number, 0, sizeof(number));
			number_count = 0;			

			data[index].points = realloc(data[index].points, sizeof(double) * (data[index].count + 1));
			data[index].points[data[index].count++] = x; 

			if (c == 10) { index = 0; continue; }
			if (c == ',') { index++; continue; }
		}
		if (isspace(c)) continue;
		if (isdigit(c) or c == '.' or c == '-') {
			number[number_count++] = (char) c;
			continue;
		}
	}

	const nat colors[] = {
		231, 10, 11, 9, 6, 147, 81, 
		226, 219, 191, 111, 88, 
		26, 100, 113, 200, 202, 
		222, 231, 194, 208, 75, 
		90, 46, 51, 128, 158, 
		167, 196, 94, 63
	};
	const nat color_count = sizeof colors / sizeof colors[0];
	for (nat i = 0; i < data_count; i++) 
		data[i].color = colors[i % color_count];

	double min_x =  10000000.0;
	double max_x = -10000000.0;
	for (nat i = 0; i < data->count; i++) {
		if (data->points[i] < min_x) min_x = data->points[i];
		if (data->points[i] > max_x) max_x = data->points[i];
	}
	
	
	double min_y =  10000000.0;
	double max_y = -10000000.0;
	for (nat r = 1; r < data_count; r++) {
		for (nat i = 0; i < data[r].count; i++) {
			if (data[r].points[i] < min_y) min_y = data[r].points[i];
			if (data[r].points[i] > max_y) max_y = data[r].points[i];
		}
	}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// hard code the min/max  x and y values here, if neccessary.  leave as is, for auto range detection.
	//max_y = 0.08;
	//max_x = 30000;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double y_range = max_y - min_y;
	const double x_range = max_x - min_x;
	const double y_step = y_range / (double) height;
	const double x_step = x_range / (double) width;

	const nat ts_step = data->count / width ? data->count / width : 1;
	

	double x = min_x;
	double y = max_y;

	nat ts = 0;
	puts("");
	printf("             .-"); 
	for (nat w = 0; w < width; w++) printf("-");
	puts(".");
	
	for (nat h = 0; h < height; h++) {
		printf(" %2.5e | ", y);
		for (nat w = 0; w < width; w++) {
			for (nat i = 1; i < data_count; i++) {

				if (data[i].points[ts] >= y - y_step / 2.0 and 
				    data[i].points[ts] <= y + y_step / 2.0) {
					printf("\033[38;5;%llum", data[i].color);					
					printf("#");
					printf("\033[0m");
					goto found;
				} 
			} 
			printf(" ");

		found:		
			if (data->points[ts] < x) ts += ts_step;
			x += x_step;
		}
		y -= y_step;
		puts("|");
		ts = 0;
		x = min_x;
	}

	printf("             +-"); 
	for (nat w = 0; w < width; w++) printf("-");
	puts("+");

	const nat base = 10;
	double d = log10(max_x) + 1;
	const nat digit_count = (nat) d;

	nat p = 1;
	for (nat i = 0; i < digit_count; i++) {
		x = min_x;
		printf("               ");
		ts = 0;
		for (nat w = 0; w < width; w++) {
			if (w % 2) printf("%1llu ", (nat) (data->points[ts] / p) % base);

			if (data->points[ts] < x) ts += ts_step;
			x += x_step;
		}
		puts("");
		p *= base;
	}
	puts("\n");
}





/*


//printf("info: x ranges from [%lf, ... %lf]\n", min_x, max_x);
	//printf("info: y ranges from [%lf, ... %lf]\n", min_y, max_y);





//printf("[index=%llu]: number is now: \"%s\"(%llu)\n", index, number, number_count);




// printf("debug: ts_step = %llu, because data->count = %llu, and width = %llu\n", ts_step, data->count, width);



	puts("read these labels for each column:");
	for (nat i = 0; i < data_count; i++) {
		printf("%llu: \"%s\"\n", i, data[i].name);
	}



	printf("printing a plot of (%lf-%lf=y%lf, %lf-%lf=x%lf), (w%llu, h%llu), (x+=%lf, y+=%lf)  ...\n", 
		max_y, min_y, y_range, 
		max_x, min_x, x_range,
		width, height,
		x_step, y_step
	);



				if (i == 2 and ts > data[i].count - 5) {
					printf("(%lf <= [%lf] <= %lf) ", 
						y - y_step / 2.0,
						data[i].points[ts], 
						y + y_step / 2.0
					);
				}



	puts("read this data for each column: ");
	for (nat i = 0; i < data_count; i++) {
		printf("%llu: name=\"%s\" (color=%llu): {...%llu values...}\n", i, data[i].name, data[i].color, data[i].count);
	}
	
	
	puts("data for data[2]: ");
	for (nat i = 0; i < data[2].count; i++) {
		printf("%lf\n", data[2].points[i]);
	}
	puts("[end of data]");
	// getchar();//
*/


//printf("just added @%llu, the value %lf...\n", index, x);


