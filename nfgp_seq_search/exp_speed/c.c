#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>


#define reset "\x1B[0m"
#define white  yellow
#define red   "\x1B[31m"
#define green   "\x1B[32m"
#define blue   "\x1B[34m"
#define yellow   "\x1B[33m"
#define magenta  "\x1B[35m"
#define cyan     "\x1B[36m"



typedef unsigned long long nat;

static const nat unique_operation_count = 5;
static const nat unique_operations[unique_operation_count] = {1, 2, 3, 5, 6};



static void init_graph_from_string(const char* string, nat* graph, nat graph_count) {
	for (nat i = 0; i < graph_count; i++) 
		graph[i] = (nat) (string[i] - '0');
}

static void print_graph_as_adj(nat* graph, nat graph_count) {
	puts("graph adjacency list: ");
	puts("{");
	for (nat i = 0; i < graph_count; i += 4) {
		const nat op = graph[i + 0];
		const nat l = graph[i + 1];
		const nat g = graph[i + 2];
		const nat e = graph[i + 3];

		printf("\t#%llu: ins(.op = %llu, .lge = [", i, op);
		printf("%2llu,", l);
		printf("%2llu,", g);
		printf("%2llu]", e);
		printf(")\n\n");
	}
	printf("}\n\n");
}

static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}

static nat print_lifetime(
	nat* graph, 
	const nat origin, 
	const nat n, 
	const nat print_count, 
	const nat pre_run_count
) {
	nat* array = calloc(n + 1, sizeof(nat));
	bool* modes = calloc(n + 1, sizeof(bool));

	nat pointer = 0, ip = origin;

	if (print_count) puts("[starting lifetime...]");
	for (nat e = 0; e < print_count + pre_run_count; e++) {

		const nat I = ip * 4;
		const nat op = unique_operations[graph[I]];

		if (op == 1) { if (pointer == n) abort(); pointer++; }
		else if (op == 5) {
			if (e >= pre_run_count) {
				for (nat i = 0; i < n; i++) {
					if (not array[i]) break;   // LE
					if (modes[i]) {
						printf("%s", (i == pointer ?  green : white));
						printf("█" reset); // (print IA's as a different-colored cell..?)
					} else printf(blue "█" reset);
				}
				puts("");
			}
			pointer = 0;
			memset(modes, 0, sizeof(bool) * (n + 1));
		}
		else if (op == 2) { array[n]++; }
		else if (op == 6) { array[n] = 0; }
		else if (op == 3) { array[pointer]++; modes[pointer] = 1; }

		nat state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	if (print_count) {
		for (nat i = 0; i < n; i++) {
			if (not array[i]) break; 
			if (modes[i]) {
				printf("%s", (i == pointer ?  green : white));
				printf("█" reset); // (print IA's as a different-colored cell..?)
			} else printf(blue "█" reset);
		}
		puts("");
	}
	if (print_count) puts("[end of lifetime]");

	nat le = 0;
	for (; le < n; le++) 
		if (not array[le]) break;
	
	// printf("[LE = %llu]\n", le);

	free(array);
	free(modes);

	return le;
}





int main(int argc, const char** argv) {
	// if (argc <= 1) return puts("give the prt as an argument!");


	FILE* file = fopen("z.txt", "r");
	if (not file) { perror("fopen"); exit(1); }


	
	nat graph_count = 0;
	nat count = 0; 
	nat** zlist = NULL;

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {

		printf("reading z value %llu: ", count);
		puts(buffer);

		buffer[strlen(buffer) - 1] = 0;

	
		graph_count = strlen(buffer);
		
		nat* graph = calloc(graph_count, sizeof(nat));
		init_graph_from_string(buffer, graph, graph_count);

		zlist = realloc(zlist, sizeof(nat*) * (count + 1));
		zlist[count++] = graph;

	}

	fclose(file);

	const nat fea = 12000;
	const nat el = 0;

	FILE* out_file = fopen("widths2.csv", "w+");
	if (not out_file) { perror("fopen"); exit(1); }
	
	fprintf(out_file, "widths\n");

	const nat limit = 1000000000;
	const nat step = 300000;

	for (nat prt = 0; prt < limit; prt += step) {
		printf("\rprt = %llu / %llu                 ", prt, limit);
		fflush(stdout);

		const nat LE = print_lifetime(zlist[0], 2, fea, el, prt);
		fprintf(out_file, "%llu\n", LE);
		prt += 10000;
	}

	fclose(out_file);
}


		//puts("-----------------------------------------------------------");
		// print_nats(zlist[i], graph_count); puts("");
		// print_graph_as_adj(zlist[i], graph_count);
		// getchar();






/*



			x * log(x)             vs          log(x)












	-------------------------------------------
	
	0.1mil		:		

	0.25mil		:		

	0.5mil		:		

	1mil		:		

	5mil 		: 		541
	10mil 		:		726
	15mil		: 		864
	20mil		:		976
	25mil		:		1078

	30mil		:		1173
	35mil		:		1255
	40mil		:		1339
	45mil		:		1414

	50mil		:		1488

	75mil		:		1815

	100mil		:		2092

	125mil		:		2340

	150mil 		: 		2554
	
	200mil		: 		2925
	

	-------------------------------------------









const nat length = strlen(buffer);
length /= 4;
length -= 5;
printf("D = %llu\n", length);


202308126.202243:

014110212003350442020200              <------------ include this in our test z values! it probably has a different lifetime!

014110212013350442020200


014110212023350442020200

014110212033350442020200

014110212043350442020200

014110212053350442020200




014110212003351442020200

014110212013351442020200


014110212023351442020200


014110212033351442020200

014110212043351442020200

014110212053351442020200




014110212003352442020200

014110212013352442020200

014110212023352442020200

014110212033352442020200

014110212043352442020200

014110212053352442020200



014110212003353442020200






*/


