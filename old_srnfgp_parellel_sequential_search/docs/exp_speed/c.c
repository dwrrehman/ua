#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>


#define reset "\x1B[0m"
#define white  yellow
// #define red   "\x1B[31m"
#define green   "\x1B[32m"
#define blue   "\x1B[34m"
#define yellow   "\x1B[33m"
// #define magenta  "\x1B[35m"
// #define cyan     "\x1B[36m"



typedef unsigned long long nat;

static const nat window_width = 1300;

static const nat unique_operation_count = 5;
static const nat unique_operations[unique_operation_count] = {1, 2, 3, 5, 6};

static nat graph_count = 0;
static nat operation_count = 0;


static void init_graph_from_string(const char* string, nat* graph) {
	for (nat i = 0; i < graph_count; i++) 
		graph[i] = (nat) (string[i] - '0');
}

static void print_graph_as_adj(nat* graph) {
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
	const nat pre_run_count,
	nat* mm
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
					if (i > window_width) break;
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
			if (i > window_width) break;
			if (not array[i]) break; 
			if (modes[i]) {
				printf("%s", (i == pointer ?  green : white));
				printf("█" reset); // (print IA's as a different-colored cell..?)
			} else printf(blue "█" reset);
		}
		puts("");
	}
	if (print_count) puts("[end of lifetime]");


	// n:
	nat le = 0; for (; le < n; le++)  if (not array[le]) break;

	// m:
	nat max = 0; for (nat i = 0; i < n; i++)  if (array[i] > max) max = array[i];
	

	free(array);
	free(modes);

	*mm = max;
	return le;
}



#define lightblue "\033[38;5;67m"
#define red     "\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
//#define blue    "\x1B[34m"
//#define magenta "\x1B[35m"
//#define cyan    "\x1B[36m"
#define bold    "\033[1m"
#define reset   "\x1B[0m"


static void synthesize_graph(nat** zlist, const nat z_count) {

	nat* graph = calloc(graph_count, sizeof(nat));

	
	nat* counts = calloc(graph_count * operation_count, sizeof(nat));

	for (nat z = 0; z < z_count; z++) {

		for (nat i = 0; i < graph_count; i++) {
			if (not (i % 4)) continue;
			counts[i * operation_count + zlist[z][i]]++;
		}
	}

	printf("synthesized graph [over %llu z values]:\n", z_count);

	for (nat i = 0; i < graph_count; i += 4) {
		printf("  " red "#%llu" reset "  :: { .op = %llu, [ %s %llu %s ]   .lge={ \n", i / 4, zlist[0][i], lightblue bold , unique_operations[zlist[0][i]], reset);
		printf("\t\t.l={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = counts[(i + 1) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu] ", o, count == z_count ?  green : yellow, (double) count / z_count, count); 
		}
		printf(" }, \n");
		printf("\t\t.g={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = counts[(i + 2) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == z_count ? green : yellow, (double) count / z_count, count); 
		}
		printf(" }, \n");
		printf("\t\t.e={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = counts[(i + 3) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == z_count ? green : yellow, (double) count / z_count, count); 
		}
		printf(" }, \n");
		printf(" }   \n\n");
	}


puts(
"static const byte _63R[5 * 4] = {" "\n"
"	0,  1, 4, _,      //        3" "\n"
"	1,  0, _, _,      //     6  7 " "\n"
"	2,  0, _, _,      //    10 11" "\n"
"	3,  _, _, _,      // 13 14 15" "\n"
"	4,  2, 0, _,      //       19" "\n"
"};"
);




}








static const nat fea = 100000;
static const nat el = 80000; 
static const nat prt = 100000000;

static void write_to_file(void) { // nat* zlist


	


	FILE* out_file = fopen("_delete_me_.csv", "w+");
	if (not out_file) { perror("fopen"); exit(1); }
	fprintf(out_file, "widths\n");

//	const nat limit = 1000000000;
//	const nat step = 300000;
//	nat p = 0;

/*
	for (; p < limit; p += step) {
		printf("\rprt = %llu / %llu                 ", p, limit);
		fflush(stdout);

		const nat LE = print_lifetime(zlist[0], 2, fea, el, p);
		fprintf(out_file, "%llu\n", LE);
		p += 10000;
	}
*/

	fclose(out_file);



}

int main(int argc, const char** argv) {

	if (argc <= 1) return puts("give input z list filename as an argument!");

	FILE* file = fopen(argv[1], "r");
	if (not file) { perror("fopen"); exit(1); }
	
	nat count = 0; 
	nat** zlist = NULL;

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {

		printf("reading z value %llu: ", count);
		puts(buffer);

		buffer[strlen(buffer) - 1] = 0;

		graph_count = strlen(buffer);
		
		nat* graph = calloc(graph_count, sizeof(nat));
		init_graph_from_string(buffer, graph);

		zlist = realloc(zlist, sizeof(nat*) * (count + 1));
		zlist[count++] = graph;

	}
	operation_count = graph_count / 4;


	fclose(file);

	nat* le_array = calloc(count, sizeof(nat));
	nat* mm_array = calloc(count, sizeof(nat));
	
	for (nat i = 0; i < count; i++) {

		puts("-----------------------------------------------------------");
		print_nats(zlist[i], graph_count); puts("");
		print_graph_as_adj(zlist[i]);

		nat mm = 0;
		const nat le = print_lifetime(zlist[i], 2, fea, el, prt, &mm);
		printf("[LE = %llu, MM = %llu]\n", le, mm);

		//le_array[i] = le;
		//mm_array[i] = mm;

		getchar();
	}

	printf("printing results: \n");
	print_nats(le_array, count); puts("");
	print_nats(mm_array, count); puts("");

	synthesize_graph(zlist, count);
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


