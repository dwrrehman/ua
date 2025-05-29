// deduplication utility
// written on 1202505283.190727 by dwrr 
// to get down the 100k down to something manageable without using a ton of memory lol 

#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <iso646.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdnoreturn.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <ctype.h>    
#include <errno.h>    
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

#define D 2
#define execution_limit 10000000LLU
#define array_size 100000LLU

enum operations { one, two, three, five, six };

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

struct zvalue {
	byte* graph;
	byte origin;
	byte unused_1;
	byte unused_2;
	byte unused_3;
	uint32_t unused_4;
	const char* dt;
};

struct zlist {
	struct zvalue* values;
	nat count;
};

static void init_graph_from_string(byte* graph, const char* string) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }

//static void print_graph(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

static void print_zlist(struct zlist list) {

	printf("list of %llu values {\n", list.count);
	for (nat i = 0; i < list.count; i++) {

		printf("	#%5llu: { z = ", i); 
		print_graph_raw(list.values[i].graph);
		printf(", o = %hhu, dt = %s } \n", 
			list.values[i].origin, list.values[i].dt
		);

	}
	printf("}\n");
}


/*static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%lld ", v[i]);
	}
	printf("]");
}

static void print_bytes(byte* v, nat l) {
	printf("(%llu) { ", l);
	for (nat i = 0; i < l; i++) printf("%02hhx ", v[i]);
	printf("}\n");
}*/

static nat compare_lifetimes(
	byte* graph1,  byte origin1,  nat* array1,
	byte* graph2,  byte origin2,  nat* array2
) {


	//printf("comparing:    { z1 = "); 
	//print_graph_raw(graph1);
	//printf(", o1 = %hhu }, ", origin1);
	//printf("and  { z2 = "); 
	//print_graph_raw(graph2);
	//printf(", o2 = %hhu } \n", origin2);
	//getchar();


	nat n = array_size;

	nat pointer1 = 0; byte pc1 = origin1;
	nat pointer2 = 0; byte pc2 = origin2;
	nat xw1 = 0;  nat xw2 = 0; 

	memset(array1, 0, (array_size + 1) * sizeof(nat));
	memset(array2, 0, (array_size + 1) * sizeof(nat));

	for (nat e = 0; e < execution_limit; e++) {

		const byte la1 = pc1 * 4, op1 = graph1[la1];
		const byte la2 = pc2 * 4, op2 = graph2[la2];

	
		if (op1 == one) {
			if (pointer1 == n) { puts("fea pointer overflow"); abort(); } 
			pointer1++; 
			if (pointer1 > xw1) xw1 = pointer1;
		} else if (op1 == five) {

			//printf("[e = %llu]\n", e); 
			//printf("array1: "); print_nats(array1, xw1 + 2); puts("");
			//printf("array2: "); print_nats(array2, xw2 + 2); puts("");

			bool found_difference = false;

			nat width = 0; 
			if (xw2 > xw1) 
				width = xw2; 
			else 
				width = xw1;


			for (nat i = 0; i < width + 2; i++) {
				if (array1[i] != array2[i]) { found_difference = true; break; } 
			}
			//printf("----> found_difference = %d", found_difference);
			//getchar();

			if (found_difference) return 1;

			pointer1 = 0;
		}
		else if (op1 == two) array1[n]++; 
		else if (op1 == six) array1[n] = 0; 
		else if (op1 == three) array1[pointer1]++; 

		byte state1 = 0;
		if (array1[n] < array1[pointer1]) state1 = 1;
		if (array1[n] > array1[pointer1]) state1 = 2;
		if (array1[n] == array1[pointer1]) state1 = 3;
		pc1 = graph1[la1 + state1];




		if (op2 == one) {
			if (pointer2 == n) { puts("fea pointer overflow"); abort(); } 
			pointer2++; 
			if (pointer2 > xw2) xw2 = pointer2;
		} else if (op2 == five) pointer2 = 0;
		else if (op2 == two) array2[n]++; 
		else if (op2 == six) array2[n] = 0; 
		else if (op2 == three) array2[pointer2]++; 

		byte state2 = 0;
		if (array2[n] < array2[pointer2]) state2 = 1;
		if (array2[n] > array2[pointer2]) state2 = 2;
		if (array2[n] == array2[pointer2]) state2 = 3;
		pc2 = graph2[la2 + state2];

	}

	return 0;
}



/*

#define selected_count 100
	nat selected[selected_count] = {0};
	for (nat i = 0; i < selected_count; i++) {
		selected[i] = (nat) rand() % given_list.count;
	}

	struct zlist list = { .values = calloc(selected_count, sizeof(struct zvalue)), .count = 0 };

	for (nat i = 0; i < selected_count; i++) {
		list.values[list.count++] = given_list.values[selected[i]];
	}

	puts("selected indexes: ");
	print_nats(selected, selected_count); puts("");




	//puts("about to compare pairs in:");
	//print_zlist(list);
	//puts("comparing....");
	//const nat zvs_per_dot = count / 128 + 1;
	//nat dot_counter = 0;
	//if (dot_counter >= zvs_per_dot) {
	// putchar('.'); fflush(stdout);
	// dot_counter = 0; 
	//} else dot_counter++;


*/




static void get_graphs_z_value(char string[64], byte* graph) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}


static void append_to_file(char* filename, size_t filename_size, byte* graph, nat origin, nat score) {
	
	char z[64] = {0};    get_graphs_z_value(z, graph); 
	char o[16] = {0};    snprintf(o, sizeof o, "%hhu", (byte) origin);
	char dt[32] = {0};   get_datetime(dt); 
	char sc[32] = {0};   snprintf(sc, sizeof sc, "%llu", score);
	
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;

try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("[%s]: [z=%s]: failed to create filename = \"%s\"\n", dt, z, filename);
			fflush(stdout);
			abort();		}
		snprintf(filename, filename_size, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, z, strlen(z));
	write(file, " ", 1);
	write(file, o, strlen(o));
	write(file, " ", 1);
	write(file, sc, strlen(sc));
	write(file, "\n", 1);
	close(file);
}



static void pair_wise_comparison(struct zlist list) { 

	nat* array1 = calloc(array_size + 1, sizeof(nat));
	nat* array2 = calloc(array_size + 1, sizeof(nat));

	nat count = list.count;

	byte* is_duplicate = calloc(count / 8 + 1, 1);
	nat* equivalent_count = calloc(count, sizeof(nat));
	nat* equivalent_z = calloc(count * count, sizeof(nat));

	const nat zvs_per_dot = count / 128 + 1;
	nat dot_counter = 0;

	for (nat index1 = 0; index1 < count; index1++) {
	
		if (dot_counter >= zvs_per_dot) {
			putchar('.'); fflush(stdout);
			dot_counter = 0; 
		} else dot_counter++;

		if ((is_duplicate[index1 / 8] >> (index1 % 8)) & 1) continue;

		for (nat index2 = index1 + 1; index2 < count; index2++) {

			//printf("info: testing index1=%llu and index2=%llu... ", index1, index2);

			byte* graph1 = list.values[index1].graph;
			byte* graph2 = list.values[index2].graph;
			const byte origin1 = list.values[index1].origin;
			const byte origin2 = list.values[index2].origin;

			bool differ = compare_lifetimes(
				graph1, origin1, array1,
				graph2, origin2, array2
			);

			if (differ) {
				//printf("zvs #%llu and #%llu differ.\n", index1, index2);
			} else {
				//printf("zvs #%llu and #%llu are the SAME!!\n", index1, index2);
				equivalent_z[count * index1 + equivalent_count[index1]] = index2;
				equivalent_count[index1]++;
				is_duplicate[index2 / 8] |= 1 << (index2 % 8);
			}
		}
	}


	puts("list has these empirical lifetime equivalencies: ");
	for (nat i = 0; i < count; i++) {
		printf("%llu: ", i);
		for (nat j = 0; j < equivalent_count[i]; j++) 
			printf(" %llu ", equivalent_z[count * i + j]);
		puts("");
	}
	
	printf("bit array: is_duplicate:  { "); 
	for (nat index = 0; index < count; index++) {
		if (index % 32 == 0) printf("\n\t");
		if (index % 8 == 0) printf("   ");
		printf("%d ", (is_duplicate[index / 8] >> (index % 8)) & 1);
	}
	puts("\n } ");

	puts("de-deuplicating z list...");
	nat unique_count = 0;
	for (nat i = 0; i < count; i++) {
		if ((is_duplicate[i / 8] >> (i % 8)) & 1) continue;
		list.values[unique_count++] = list.values[i];
	}
	list.count = unique_count;

	//puts("final zlist:");
	//print_zlist(list);

	printf("generating zlist with %llu zvs...\n", list.count);

	char filename[4096] = {0};
	for (nat i = 0; i < list.count; i++) {
		append_to_file(
			filename, sizeof filename,
			list.values[i].graph, 
			list.values[i].origin, 
			i
		);
	}

	printf("generated zlist '%s' containing %llu zvs.\n", filename, list.count);
}




int main(int argc, const char** argv) {

	if (argc <= 1) return puts("usage: ./run <zlist_z_file.txt>");
	FILE* file = fopen(argv[1], "r");
	if (not file) { perror("fopen"); exit(1); }

	struct zlist zlist = {0};
	char buffer[1024] = {0};

	while (fgets(buffer, sizeof buffer, file)) {
		char* index = strchr(buffer, ' ');
		if (not index) abort();
		buffer[index - buffer] = 0;
		const byte o = (byte) (buffer[index - buffer + 1] - '0');
		if (graph_count != strlen(buffer)) { puts("graph count or dupilcation_count mismatch!"); abort(); }
		byte* g = calloc(graph_count, 1);
		init_graph_from_string(g, buffer);
		zlist.values = realloc(zlist.values, sizeof(struct zvalue) * (zlist.count + 1));
		zlist.values[zlist.count++] = (struct zvalue) { .graph = g, .origin = o, .dt = "" };

	}
	fclose(file);

	char input[4096] = {0};
loop:
	printf(":%llu: ", zlist.count);
	fgets(input, sizeof input, stdin);

	if (not strcmp(input, "\n")) {}
	else if (not strcmp(input, "quit\n")) exit(0);
	else if (not strcmp(input, "exit\n")) exit(0);
	else if (not strcmp(input, "q\n")) exit(0);
	else if (not strcmp(input, "x\n")) exit(0);
	else if (not strcmp(input, "clear\n") or not strcmp(input, "o\n")) printf("\033[H\033[2J");	
	else if (not strcmp(input, "list\n")) print_zlist(zlist);
	else if (not strcmp(input, "c\n")) pair_wise_comparison(zlist);
	else printf("unknown command %s\n", input);
	goto loop;
}






















