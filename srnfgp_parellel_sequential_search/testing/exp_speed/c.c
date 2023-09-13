#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>


#define lightblue "\033[38;5;67m"
#define red     "\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
//#define blue    "\x1B[34m"
//#define magenta "\x1B[35m"
//#define cyan    "\x1B[36m"
#define bold    "\033[1m"
#define reset   "\x1B[0m"

// #define reset "\x1B[0m"
#define white  yellow
// #define red   "\x1B[31m"
// #define green   "\x1B[32m"
#define blue   "\x1B[34m"
//#define yellow   "\x1B[33m"
// #define magenta  "\x1B[35m"
// #define cyan     "\x1B[36m"


typedef unsigned long long nat;

static const nat window_width = 1300;
static const bool viz = 0;


static const nat similarity_threshold = 22;


static const nat fea = 100000;
static const nat el = 40000; 
static const nat prt = 10000000;

static const nat unique_operation_count = 5;
static const nat unique_operations[unique_operation_count] = {1, 2, 3, 5, 6};

struct zlist {
	nat** values;
	nat count;
};

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



static void synthesize_graph_over_one_group(struct zlist zlist) {

	// nat* graph = calloc(graph_count, sizeof(nat));

	
	nat* counts = calloc(graph_count * operation_count, sizeof(nat));

	for (nat z = 0; z < zlist.count; z++) {

		for (nat i = 0; i < graph_count; i++) {
			if (not (i % 4)) continue;
			counts[i * operation_count + zlist.values[z][i]]++;
		}
	}

	printf("synthesized graph [over %llu z values]:\n", zlist.count);

	for (nat i = 0; i < graph_count; i += 4) {
		printf("  " red "#%llu" reset "  :: { .op = %llu, [ %s %llu %s ]   .lge={ \n", i / 4, 
			zlist.values[0][i], lightblue bold , unique_operations[zlist.values[0][i]], reset);

		printf("\t\t.l={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = counts[(i + 1) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == zlist.count ?  green : yellow, (double) count / zlist.count, count); 
		}
		printf(" }, \n");
		printf("\t\t.g={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = counts[(i + 2) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == zlist.count ? green : yellow, (double) count / zlist.count, count); 
		}
		printf(" }, \n");
		printf("\t\t.e={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = counts[(i + 3) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == zlist.count ? green : yellow, (double) count / zlist.count, count); 
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

puts(
"static const byte _36R[5 * 4] = {" "\n"
"	0,  1, 2, _,      //        3" "\n"
"	1,  0, _, _,      //     6  7 " "\n"
"	2,  _, 4, _,      //    10 11" "\n"
"	3,  _, _, _,      // 13 14 15" "\n"
"	4,  0, 0, _,      //       19" "\n"
"};"
);

}





static void print_zlist(const char* s, nat d, struct zlist list) {
	printf("%s . %llu . zlist (%llu){\n", s, d, list.count);
	for (nat k = 0; k < list.count; k++) {
		putchar(9);
		print_nats(list.values[k], graph_count);
		putchar(10);
	}
	printf("}\n");
}





static nat similarity_count(nat* a, nat* b) {
	nat count = 0;
	for (nat i = 0; i < graph_count; i++) {
		if (a[i] == b[i]) count++;
	}
	return count;
}

static void partition_into_minor_groups(struct zlist total)  {
	
	nat hcount = 0;
	struct zlist* hlist = calloc(total.count, sizeof(struct zlist));

	for (nat z = 0; z < total.count; z++) {

		nat* graph = total.values[z];
	
		for (nat hi = 0; hi < hcount; hi++) {

			for (nat i = 0; i < hlist[hi].count; i++) {
				if (similarity_count(graph, hlist[hi].values[i]) < similarity_threshold) goto next_minor_group;
			}

			
			hlist[hi].values[hlist[hi].count++] = graph;
			goto next_graph;
			next_minor_group: continue;
		}
	
		struct zlist new = {
			.values = calloc(total.count, sizeof(nat*)),
			.count = 0
		};

		new.values[new.count++] = graph;
		hlist[hcount++] = new;
		next_graph: continue;
	}

	for (nat i = 0; i < hcount; i++) {

		puts("------------------------");
		printf("[ printing hlist %llu / %llu ]\n", i, hcount);
		print_zlist("hlist", i, hlist[i]);
		puts("------------------------");
		puts("");
	}
	getchar();

	for (nat i = 0; i < hcount; i++) {

		puts("------------------------");
		printf("[ printing hlist %llu / %llu ]\n", i, hcount);
		print_zlist("hlist", i, hlist[i]);
		puts("------------------------");
		puts("");

		printf("MINOR GROUP #%llu :  performing synthesize graph over MINOR GROUP %llu z values:\n", i, hlist[i].count); 
		if (hlist[i].count > 1) synthesize_graph_over_one_group(hlist[i]); else printf("[ZLIST OF SIZE 1]\n");
		getchar();
		
	}
}



static void find_groups(struct zlist list) {

	struct zlist group = {
		.values = calloc(list.count, sizeof(nat*)), 
		.count = 0
	};

	for (nat i = 0; i < 6; i++) {

		for (nat z = 0; z < list.count; z++) {
			if (list.values[z][7] == 1) continue;
			if (list.values[z][20] == i or i == 5) group.values[group.count++] = list.values[z];
		}

		printf("%llu op is new:   performing synthesize graph over %llu z values:\n", i, group.count); 
		print_zlist("major group", i, group);

		printf("%llu op is new:   performing synthesize graph over MAJOR GROUP %llu z values:\n", i, group.count); 
		synthesize_graph_over_one_group(group);
		getchar();

		partition_into_minor_groups(group);
		group.count = 0;
	}

	puts("[finished all sythesized graphs over all groups.]");
}


int main(int argc, const char** argv) {

	if (argc <= 1) return puts("give input z list filename as an argument!");

	FILE* file = fopen(argv[1], "r");
	if (not file) { perror("fopen"); exit(1); }
	
	struct zlist zlist = {0};

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {

		char* index = strchr(buffer, ' ');
		if (not index) abort();
		buffer[index - buffer] = 0;

		printf("reading z value %llu: ", zlist.count);
		puts(buffer);

		graph_count = strlen(buffer);
		
		nat* graph = calloc(graph_count, sizeof(nat));
		init_graph_from_string(buffer, graph);

		zlist.values = realloc(zlist.values, sizeof(nat*) * (zlist.count + 1));
		zlist.values[zlist.count++] = graph;

	}
	operation_count = graph_count / 4;

	fclose(file);

	nat* le_array = calloc(zlist.count, sizeof(nat));
	nat* mm_array = calloc(zlist.count, sizeof(nat));


	for (nat i = 0; i < zlist.count; i++) {

		if (viz) puts("-----------------------------------------------------------");
		if (viz) print_nats(zlist.values[i], graph_count); puts("");
		if (viz) print_graph_as_adj(zlist.values[i]);

		nat mm = 0;
		const nat le = viz ? print_lifetime(zlist.values[i], 2, fea, el, prt, &mm) : 0;
		if (viz) printf("[LE = %llu, MM = %llu]\n", le, mm);

		//le_array[i] = le;
		//mm_array[i] = mm;

		if (viz) getchar();
	}

	printf("printing results: \n");
	print_nats(le_array, zlist.count); puts("");
	print_nats(mm_array, zlist.count); puts("");

	find_groups(zlist);
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





















































==============================================
starting from 0 we get this synth graph:
==============================================


synthesized graph [over 80 z values]:
  #0  :: { .op = 0, [  1  ]   .lge={ 
		.l={  ->1 : 1.00[80]  }, 
		.g={  ->2 : 1.00[80] }, 
		.e={  ->1 : 0.56[45] ->2 : 0.44[35] }, 
 }   

  #1  :: { .op = 1, [  2  ]   .lge={ 
		.l={  ->0 : 1.00[80]  }, 
		.g={  ->0 : 0.28[22] ->2 : 0.65[52] ->4 : 0.01[1] ->5 : 0.06[5] }, 
		.e={  ->0 : 0.03[2] ->1 : 0.90[72] ->2 : 0.07[6] }, 
 }   

  #2  :: { .op = 2, [  3  ]   .lge={ 
		.l={  ->0 : 0.42[34]  ->3 : 0.45[36]  ->4 : 0.03[2]  ->5 : 0.10[8]  }, 
		.g={  ->4 : 1.00[80] }, 
		.e={  ->3 : 1.00[80] }, 
 }   

  #3  :: { .op = 3, [  5  ]   .lge={ 
		.l={  ->0 : 0.55[44]  ->1 : 0.23[18]  ->5 : 0.23[18]  }, 
		.g={  ->0 : 0.15[12] ->1 : 0.54[43] ->4 : 0.04[3] ->5 : 0.28[22] }, 
		.e={  ->0 : 0.29[23] ->1 : 0.06[5] ->2 : 0.03[2] ->4 : 0.26[21] ->5 : 0.36[29] }, 
 }   

  #4  :: { .op = 4, [  6  ]   .lge={ 
		.l={  ->0 : 1.00[80]  }, 
		.g={  ->0 : 1.00[80] }, 
		.e={  ->0 : 1.00[80] }, 
 }   

  #5  :: { .op = 0, [  1  ]   .lge={ 
		.l={  ->0 : 0.68[54]  ->1 : 0.19[15]  ->2 : 0.09[7]  ->4 : 0.05[4]  }, 
		.g={  ->0 : 1.00[80] }, 
		.e={  ->0 : 1.00[80] }, 
 }   

static const byte _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};













static const nat fea = 100000;
static const nat el = 40000; 
static const nat prt = 10000000;

static void write_to_file(void) { // nat* zlist


	


	FILE* out_file = fopen("_delete_me_.csv", "w+");
	if (not out_file) { perror("fopen"); exit(1); }
	fprintf(out_file, "widths\n");

//	const nat limit = 1000000000;
//	const nat step = 300000;
//	nat p = 0;


	for (; p < limit; p += step) {
		printf("\rprt = %llu / %llu                 ", p, limit);
		fflush(stdout);

		const nat LE = print_lifetime(zlist[0], 2, fea, el, p);
		fprintf(out_file, "%llu\n", LE);
		p += 10000;
	}


	fclose(out_file);



}




*/








/*

	static void partion_into_minor_groups(list)  {
	
		for (z < list) {

			g = list[z];
		
			for (hi < hcount) {

				h = hlist[hi];

				for (i < h) {
					if (similarity_count(g, h[i]) < similarity_thr) goto next_h;
				}

				h.push(g);
				goto next_z;

				next_h: continue;
			}
		
			make new_h = {};
			new_h.push(g);
			hlist.push(new_h);
			
			next_z: continue;
		}

		print_hs(hlist, hcount);
	}







*/





