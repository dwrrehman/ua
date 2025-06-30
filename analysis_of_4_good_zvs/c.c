#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iso646.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

#define D 2

#define execution_limit 10000000000LLU
#define array_size 1000000LLU

enum analyses {
	none, s0xw, mv_hg, bl_hg, wic_hg, erp_hg, bl_csv, 
	print_array,
	print_lifetime,
	print_array_over_time,
};

static nat analysis = print_lifetime;

static const bool should_generate_xw_csv = false;

enum operations { one, two, three, five, six };

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }




static nat execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	nat bout_length_tallys[100000] = {0};
	nat walk_ia_count_tallys[100000] = {0};
	nat er_position_tallys[100000] = {0};

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;
	nat 	xw = 0,  pointer = 0,  bout_length = 0, walk_ia_counter = 0, er_count = 0;
	byte ip = origin;
	byte last_mcal_op = 255;
	//nat did_ier_at = (nat)~0;


	for (nat e = 0; e < execution_limit; e++) {

		if (not (e & 0x3FFFFFFF)) { printf("e = %llu\n", e); }

		const byte I = ip * 4, op = graph[I];

		if (analysis == print_lifetime) usleep(100);

		if (op == one) {
			if (pointer == n) { 
				puts("FEA condition violated by a z value: "); 
				print_graph_raw(graph); 
				puts(""); 
				abort(); 
			}
  
			bout_length++;
			pointer++;

			if (analysis == print_lifetime)  {   if (last_mcal_op == three) putchar('#'); else putchar(' '); fflush(stdout); } 

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {

			if (should_generate_xw_csv) printf("%llu, %llu\n", er_count, xw);
			walk_ia_count_tallys[walk_ia_counter]++;
			er_position_tallys[pointer]++;

			walk_ia_counter = 0;
			//did_ier_at = pointer;
			pointer = 0;
			er_count++;

			if (analysis == print_lifetime) { puts("#"); fflush(stdout); } 
		}

		else if (op == two) {
			array[n]++;
		}

		else if (op == six) {  
			array[n] = 0;
		}
		else if (op == three) {
			walk_ia_counter++;

			bout_length_tallys[bout_length]++;

			bout_length = 0;
			
			array[pointer]++;
		}
		if (op == three or op == one or op == five) last_mcal_op = op;
		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];

		if (analysis == print_array_over_time) {

			const nat max_cell_index = 80;
			const nat max_cell_value = 70;

			printf("\033[H\033[2J");
			
			printf("bout_length = ");
			for (nat i = 0; i < bout_length; i++) {
				printf("# ");
			}
			puts("\n");
			printf("walk_counter = ");
			for (nat i = 0; i < walk_ia_counter; i++) {
				printf("# ");
			}
			puts("\n");
		
			printf("         ");

			for (nat i = 0; i < pointer; i++) {
				printf("      ");
			}
			printf("   V  ");
			puts("");

			printf(" [N]     ");

			for (nat i = 0; i < max_cell_index; i++) {
				printf(" [%03llu]", i);
			}
			puts("");
			
 			for (nat m = 0; m < max_cell_value; m++) {

				if (array[n] > m) printf("  #      "); else printf("         ");

				for (nat i = 0; i < max_cell_index; i++) {
					if (array[i] > m) printf("   #  "); else printf("      ");
				}
				puts("");
			}

			usleep(1000);
		}


	}
	if (analysis == print_lifetime) { puts(""); fflush(stdout); } 
	

	if (analysis == s0xw) {

		printf("xw = %llu\n", xw);
		printf("*0 = %llu\n", array[0]);
		printf("xw / *0 = %10.10lf\n", ((double) xw) / ((double) array[0]));
		printf("*0 / xw = %10.10lf\n", ((double) array[0]) / ((double) xw));

		printf("array: { ");
		for (nat i = 0; i < xw + 5; i++) 
			printf("%llu ", array[i]);		
		puts("}");
	}


	if (analysis == mv_hg) {
		nat max_modnat_value = 0;
		for (nat i = 0; i < n; i++) {
			if (max_modnat_value < array[i])
			    max_modnat_value = array[i];
		}

		nat* tallys = calloc(max_modnat_value + 1, sizeof(nat));
		for (nat i = 0; i < n; i++) {
			if (not array[i]) break;
			tallys[array[i]]++;
		}

		for (nat i = 0; i < max_modnat_value + 1; i++) {
			const nat value = tallys[i];
			printf("%5llu : %5llu : ", i, value);
			for (nat _ = 0; _ < value; _++) putchar('#');
			puts("");
		}
		puts("");


		free(tallys);
	}



	if (analysis == bl_hg) {

		const nat scale = 30;

		for (nat i = 0; i < 1000; i++) {
			const nat value = bout_length_tallys[i];
			//if (not value) continue;
			printf("%5llu : %5llu : ", i, value);
			for (nat _ = 0; _ < value / scale; _++) putchar('#');
			puts("");
		}
		puts("");
	}


	if (analysis == wic_hg) {

		const nat scale = 20000;

		for (nat i = 0; i < 60; i++) {
			const nat value = walk_ia_count_tallys[i];
			printf("%5llu : %5llu : ", i, value);
			for (nat _ = 0; _ < value / scale + !!value; _++) putchar('#');
			puts("");
		}
		puts("");
	}


	if (analysis == erp_hg) {

		const nat scale = 100;

		for (nat i = 0; i < 3000; i++) {
			const nat value = er_position_tallys[i];
			// if (not value) continue;
			printf("%5llu : %5llu : ", i, value);
			for (nat _ = 0; _ < value / scale + !!value; _++) putchar('#');
			puts("");
		}
		puts("");
	}

	if (analysis == print_array) {
		puts("array state:");

		const nat scale = 10;

		for (nat i = 0; i < n; i++) {			
			const nat value = array[i];
			if (not value) break;

			printf("%5llu : %5llu : ", i, value);
			for (nat _ = 0; _ < value / scale + !!value; _++) putchar('#'); 
			puts("");
		}
	}

	return 0;
}


static void init_graph_from_string(byte* graph, const char* string) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

int main(void) {
	nat* array = calloc(array_size + 1, sizeof(nat));
	byte graph[graph_count] = {0};

	byte origin = 0;
	const char* zv_string = NULL;



	zv_string = "0464152021303144424601641213"; 
	origin = 1;




	//if (zv_index == 1) { abort(); zv_string = "0524165121303142424110001521"; origin = 2; } 
	//if (zv_index == 2) { zv_string = "0464152021303144424601641213"; origin = 1; }
	//if (zv_index == 3) { abort(); zv_string = "0112102625303161414204152414"; origin = 1; }

	init_graph_from_string(graph, zv_string);
	if (should_generate_xw_csv) printf("timestep, xw\n");
	execute_graph_starting_at(origin, graph, array);

	if (not should_generate_xw_csv) {
		puts("");
		print_graph_raw(graph); puts("");
		printf("execution_limit = %llu\n", execution_limit);
		printf("array_size = %llu\n", array_size);
	}

}

























/*
		0524165121303142424110001521 2 0
		0464152021303144424601641213 1 7
		0112102625303161414204152414 1 8
*/


/*


2412146.160835:

looking at the ratio of "xw / *0"  and finding out the fact that its always very nearly equal to 2.

	xw / *0 == 2  ish





014415352131354442420020

	xw = 2519
	*0 = 1260
	2 * *0 = 2520
	xw / *0 = 1.9992063492
	014415352131354442420020
	EL = 1000000000

	xw = 11697
	*0 = 5848
	2 * *0 = 11696
	xw / *0 = 2.0001709986

	014415352131354442420020
	EL = 100000000000




014415352131354542420020


	xw = 2521
	*0 = 1258
	2 * *0 = 2516
	xw / *0 = 2.0039745628

	014415352131354542420020
	EL = 1000000000


012115252033300442420040

	xw = 2517
	*0 = 1259
	2 * *0 = 2518
	xw / *0 = 1.9992057188

	012115252033300442420040
	EL = 1000000000




014415252133310442420021


	xw = 2503
	*0 = 1268
	2 * *0 = 2536
	xw / *0 = 1.9739747634

	014415252133310442420021
	EL = 1000000000





*/




/*

				014415352131354442420020 2 dt
				014415352131354542420020 2 dt
				012115252033300442420040 2 dt
				014415252133310442420021 2 dt

*/







	//const nat zv_index = 0;
	//const char* zv_string = NULL;
	//if (zv_index == 1) zv_string = "014415352131354442420020";
	//if (zv_index == 2) zv_string = "014415352131354542420020";
	//if (zv_index == 3) zv_string = "012115252033300442420040";
	//if (zv_index == 4) zv_string = "014415252133310442420021";

	
//	zv_string = "0114102521363141424204140140";
//		     0114102521363141424204140140
//static const nat analysis = print_lifetime; //erp_hg;



