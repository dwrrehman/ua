#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // 202309074.165637:   
#include <stdlib.h> //  another rewrite to make the editor simpler, and non-volatile- to never require saving. 
#include <string.h> 
#include <fcntl.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>


#define lightblue "\033[38;5;67m"
#define red     "\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
// #define blue    "\x1B[34m"
// #define magenta "\x1B[35m"
// #define cyan    "\x1B[36m"
#define bold    "\033[1m"
#define reset   "\x1B[0m"

// #define reset "\x1B[0m"
// #define white  yellow
// #define red   "\x1B[31m"
// #define green   "\x1B[32m"
// #define blue   "\x1B[34m"
// #define yellow   "\x1B[33m"
// #define magenta  "\x1B[35m"
// #define cyan     "\x1B[36m"


typedef unsigned long long nat;

static const nat window_begin = 0;
static const nat window_end = 1500;

static const nat row_count = 220;
static const nat el = 10000000; // page size for paging the lifetime
static const nat prt = 100000;   // iniitial page offset in lifetime. 

static const nat fea = 3000000;

// static const bool viz = 1;

static const nat similarity_threshold = 22;


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
	const nat er_count,
	const nat pre_run_count
) {
	nat* array = calloc(n + 1, sizeof(nat));
	bool* modes = calloc(n + 1, sizeof(bool));

	nat er = 0, E = 0, Eer = 0;
	nat pointer = 0, ip = origin;
	if (print_count) puts("[starting lifetime...]");
	nat e = 0;
	for (; e < print_count + pre_run_count; e++) {

		const nat I = ip * 4;
		const nat op = unique_operations[graph[I]];

		if (op == 1) { if (pointer == n) abort(); pointer++; }
		else if (op == 5) {
			if (e >= pre_run_count) {
				for (nat i = 0; i < n; i++) {
					if (i < window_begin) continue;
					if (i > window_end) break;
					if (not array[i]) break;
					if (not modes[i]) putchar(' ');
					else if (i == pointer) putchar('@');
					else putchar('#');
				}
				puts("");
				er++;

				if (er == 10) Eer = E;

				if (er > er_count) { puts("maxed out er count."); goto done; }
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

		if (e >= pre_run_count) E++;
	}
	if (print_count) {
		for (nat i = 0; i < n; i++) {
			if (i < window_begin) continue;
			if (i > window_end) break;
			if (not array[i]) break;
			if (not modes[i]) putchar(' ');
			else if (i == pointer) putchar('@');
			else putchar('#');
		}
		puts("");
	}
	puts("finished lifetime via el.");
	
done:
	if (print_count) puts("[end of lifetime]");
	free(array);
	free(modes);
	return Eer;
}















/*


	// *mm = max;
	// return le;

	// n:
	//nat le = 0; for (; le < n; le++)  if (not array[le]) break;

	// m:
	//nat max = 0; for (nat i = 0; i < n; i++)  if (array[i] > max) max = array[i];






static void generate_lifetime_images(
	const char* z_list_filename, 
	const char* destination_dir, 
	byte* graph, byte start, 
	nat begin_timestep, nat end_timestep,
	nat begin_cell, nat end_cell,
	nat maximum
) {
	nat list_count = 0;
	byte** list = read_z_list_from_file(z_list_filename, &list_count);
	if (not list) return;

	printf("generate: generating %llu lifetimes .ppm's...\n", list_count);

	const nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));
	nat pointer = 0;

	for (nat z = 0; z < list_count; z++) {

		memcpy(graph, list[z], 32);
		memset(array, 0, array_size * sizeof(nat));
		pointer = 0;

		byte i = start;

		nat timestep = 0;
		
		printf("\r [ ( %llu / %llu ) ] generating lifetime:  %s            ", 
				z, list_count, hex_string(graph));

		fflush(stdout);

		
		char path[4096] = {0}, filename[4096] = {0};
		sprintf(filename, "z_%s.ppm", hex_string(graph));
		strcpy(path, destination_dir);
		strcat(path, "/");
		strcat(path, filename);

		FILE* file = fopen(path, "wb");

		if (not file) {
			fprintf(stderr, "error: %s: could not open file for writing: %s\n", 
				path, strerror(errno));
			return;
		}

		fprintf(file, "P6\n%llu %llu\n255\n", end_cell - begin_cell, end_timestep - begin_timestep);

		do {
			
			if (i == 5) {
				if (timestep >= begin_timestep and 
				    timestep < end_timestep) {
					for (nat j = begin_cell; j < end_cell; j++) {
						double x = (double) array[j] / (double) maximum;  

						// nat x = array[j];
				   //              unsigned char 
							// r = x ? 255 : 0, 
							// g = x ? 255 : 0, 
							// b = x ? 255 : 0;

						unsigned char 
							r = (unsigned char)(x * 255.0), 
							g = (unsigned char)(x * 255.0), 
							b = (unsigned char)(x * 255.0);
				                fwrite(&r, 1, 1, file);
				                fwrite(&g, 1, 1, file);
				                fwrite(&b, 1, 1, file);
					}
				}
				timestep++;
			}


			if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[pointer])];
			} else if (i == 0xC) {
				i = graph[i * 2 + (array[n] != array[pointer])];
			} else if (i == 0xF) {
				i = graph[i * 2 + (array[n] > array[pointer])];
			}

			else if (i == 1) { pointer++; 			i = graph[i * 2];  }
			else if (i == 2) { array[n]++; 			i = graph[i * 2];  }
			else if (i == 3) { array[pointer]++; 		i = graph[i * 2];  }
			else if (i == 5) { pointer = 0; 		i = graph[i * 2];  }
			else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  }
			
		} while (timestep < end_timestep);

		fclose(file);


	}
	printf("generate: generated all images.\n");
	free(array);
}


*/





static void synthesize_graph_over_one_group(struct zlist zlist) {


	
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

	// printf("\n-----!#!#!#!#!#!#!#!#-----"); fflush(stdout);
	

	for (nat i = 0; i < hcount; i++) {

		puts("------------------------");
		printf("[ printing hlist %llu / %llu ]\n", i, hcount);
		print_zlist("hlist", i, hlist[i]);
		puts("------------------------");
		puts("");



		printf("going over lifetimes for all z values within this hlist...\n");


		printf("\n-----!#!#!#!#!#!#!#!#-----"); fflush(stdout);
		

		
		// if ((0)) 

		for (nat z = 0; z < hlist[i].count; z++) {

			for (nat o = 0; o < graph_count; o += 4) {
				if (hlist[i].values[z][o + 0] == 2)  {
					puts("-----------------------------------------------------------");
					print_nats(hlist[i].values[z], graph_count); puts("");
					print_graph_as_adj(hlist[i].values[z]);
					// print_lifetime(hlist[i].values[z], o, fea, el, prt);
					puts("-----------------------------------------------------------");

					printf("\n-----!#!#!#!#!#!#!#!#-----"); fflush(stdout);
					getchar();
				}
			}
		}

		printf("MINOR GROUP #%llu :  performing synthesize graph over MINOR GROUP %llu z values:\n", i, hlist[i].count); 
		if (hlist[i].count > 1) synthesize_graph_over_one_group(hlist[i]); else printf("[ZLIST OF SIZE 1]\n");

		printf("\n-----!#!#!#!#!#!#!#!#-----"); fflush(stdout);

		
	}

	printf("\n-----!#!#!#!#!#!#!#!#-----"); fflush(stdout);
}









static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}



static char directory[4096] = "./";
static char filename[4096] = {0};


static void get_graphs_z_value(nat* graph, char string[64]) { 
	for (nat i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}


static void write_to_file(nat* graph) {
	
	char newfilename[4096] = {0};

	strlcpy(newfilename, filename, sizeof newfilename);

	const int dir = open(directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", directory); 
		return; 
	}
	int flags = O_WRONLY | O_APPEND;  mode_t m = 0;
try_open:;
	const int file = openat(dir, newfilename, flags, m);
	if (file < 0) {
		if (m) {
			perror("create openat file");
			printf("filename=%s ", newfilename);
			close(dir); return;
		}
		perror("write openat file");
		printf("filename=%s\n", newfilename);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		m     = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

		char dt[32] = {0};
		get_datetime(dt);
		snprintf(newfilename, sizeof newfilename, "%s_%u_%u-%u_good.txt", dt, 0, 0, 0);
		strlcpy(filename, newfilename, sizeof filename);

		goto try_open;
	}

	char string[64] = {0};
	get_graphs_z_value(graph, string);
	write(file, string, graph_count);
	write(file, "\n", 1);
	close(file); 
	
	if (m) {
		printf("write: created %llu z values to ", 1LLU);
		printf("%s : %s\n", directory, newfilename);
		close(dir);  
		return;
	}

	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_%u_%u_%u_good.txt", dt, 0, 0, 0);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strlcpy(filename, newfilename, sizeof filename);

	close(dir);

	printf("\033[1mwrite: saved %llu z values to ",1LLU);
	printf("%s : %s \033[0m\n", directory, newfilename);
}







static void human_prune(struct zlist list) {

	for (nat z = 0; z < list.count; z++) {

		bool written = false;

		for (nat o = 0; o < graph_count; o += 4) {
			
			if (list.values[z][o + 0] == 2)  {
				nat offset = prt;

			print:;
				printf("\033[H\033[2J");
				puts("");
				puts("");
				for (nat i = 0; i < z * 5; i++) putchar('@');
				for (nat i = 0; i < (list.count - z) * 5; i++) putchar('.');
				puts("");
				puts("");

				const nat e = print_lifetime(list.values[z], o / 4, fea, el, row_count, offset);
				print_nats(list.values[z], graph_count); puts("");
				print_graph_as_adj(list.values[z]);
				
				puts("");
				printf(":ready: ");
				fflush(stdout);
				usleep(1000);
				
				int c = getchar();

				if (c == 10) goto print; 

				else if (c == ' ') { 
					offset += e; 
					goto print; 

				} else if (c == '\t') { 
					if (offset > e) offset -= e; 
					goto print; 

				} else if (c == 'q') return;

				else if (c == '[') { 
					if (z) { 
						z -= 2; 
						goto next_z; 
					} else goto print;

				} else if (c == ']') goto next_z;

				else if (c == 't') goto next_z;

				else if (c == 'a') { 
					if (not written) { 
						written = true; 
						write_to_file(list.values[z]);
						puts(bold green "------- Z VALUE WRITTEN TO GOOD LIST -------" reset);
						goto next_z;
				
					} else { puts("write error"); getchar(); goto print; }

				} else { 
					puts("input error"); 
					getchar();
					goto print; 
				}
			}
		}
		next_z: continue;
	}
}










static void find_major_groups(struct zlist list) {

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
		// getchar();

		//partition_into_minor_groups(group);

		if (i == 5) human_prune(group);

		group.count = 0;
	}

	puts("[finished all sythesized graphs over all groups.]");
}


int main(int argc, const char** argv) {

	if (argc <= 1) return puts("give input z list filename as an argument!");



	struct termios terminal;
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_lflag &= ~((size_t) ECHO | ICANON);
	tcsetattr(0, TCSAFLUSH, &copy);
	write(1, "\033[?1049h", 8);




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

	// nat* le_array = calloc(zlist.count, sizeof(nat));
	// nat* mm_array = calloc(zlist.count, sizeof(nat));


	// for (nat i = 0; i < zlist.count; i++) {

		//if (viz) puts("-----------------------------------------------------------");
		//if (viz) print_nats(zlist.values[i], graph_count); puts("");
		//if (viz) print_graph_as_adj(zlist.values[i]);

		// const nat le = viz ? print_lifetime(zlist.values[i], 2, fea, el, 99999999, prt) : 0;
		// if (viz) printf("[LE = %llu, MM = %llu]\n", le, mm);

		//le_array[i] = le;
		//mm_array[i] = mm;

		// if (viz) getchar();
	// }


	printf("printing results: \n");
	// print_nats(le_array, zlist.count); puts("");
	// print_nats(mm_array, zlist.count); puts("");

	find_major_groups(zlist);


	write(1, "\033[?1049l", 8);
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
		// fflush(stdout);

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







z value found on 202309166.223511:    has sort of good lifetime, even looking out to instruction 1 billion!   interesting. 

(24)[ 0 1 2 2 1 0 2 5 2 5 4 3 3 0 1 0 4 0 0 0 0 4 1 1 ]
graph adjacency list: 
{
	#0: ins(.op = 0, .lge = [ 1, 2, 2])

	#4: ins(.op = 1, .lge = [ 0, 2, 5])

	#8: ins(.op = 2, .lge = [ 5, 4, 3])

	#12: ins(.op = 3, .lge = [ 0, 1, 0])

	#16: ins(.op = 4, .lge = [ 0, 0, 0])

	#20: ins(.op = 0, .lge = [ 4, 1, 1])

}










                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
[end of lifetime]
-----------------------------------------------------------
-----------------------------------------------------------
(24)[ 0 1 2 1 1 0 2 2 2 5 4 3 3 1 1 4 4 0 0 0 0 4 1 1 ]
graph adjacency list: 
{
	#0: ins(.op = 0, .lge = [ 1, 2, 1])

	#4: ins(.op = 1, .lge = [ 0, 2, 2])

	#8: ins(.op = 2, .lge = [ 5, 4, 3])

	#12: ins(.op = 3, .lge = [ 1, 1, 4])

	#16: ins(.op = 4, .lge = [ 0, 0, 0])

	#20: ins(.op = 0, .lge = [ 4, 1, 1])

}

[starting lifetime...]









[end of lifetime]
-----------------------------------------------------------

-----------------------------------------------------------
(24)[ 0 1 2 1 1 0 2 2 2 5 4 3 3 0 1 5 4 0 0 0 0 4 1 1 ]
graph adjacency list: 
{
	#0: ins(.op = 0, .lge = [ 1, 2, 1])

	#4: ins(.op = 1, .lge = [ 0, 2, 2])

	#8: ins(.op = 2, .lge = [ 5, 4, 3])

	#12: ins(.op = 3, .lge = [ 0, 1, 5])

	#16: ins(.op = 4, .lge = [ 0, 0, 0])

	#20: ins(.op = 0, .lge = [ 4, 1, 1])

}

[starting lifetime...]







----------------------------------------------------------
-----------------------------------------------------------
(24)[ 0 1 2 1 1 0 2 2 2 5 4 3 3 1 1 5 4 0 0 0 0 4 1 1 ]
graph adjacency list: 
{
	#0: ins(.op = 0, .lge = [ 1, 2, 1])

	#4: ins(.op = 1, .lge = [ 0, 2, 2])

	#8: ins(.op = 2, .lge = [ 5, 4, 3])

	#12: ins(.op = 3, .lge = [ 1, 1, 5])

	#16: ins(.op = 4, .lge = [ 0, 0, 0])

	#20: ins(.op = 0, .lge = [ 4, 1, 1])

}



























202309214.114509:





-----!#!#!#!#!#!#!#!#-----
-----------------------------------------------------------
(24)[ 0 1 2 1 1 0 2 2 2 5 4 3 3 1 1 5 4 0 0 0 0 4 1 1 ]
graph adjacency list: 
{
	#0: ins(.op = 0, .lge = [ 1, 2, 1])

	#4: ins(.op = 1, .lge = [ 0, 2, 2])

	#8: ins(.op = 2, .lge = [ 5, 4, 3])

	#12: ins(.op = 3, .lge = [ 1, 1, 5])

	#16: ins(.op = 4, .lge = [ 0, 0, 0])

	#20: ins(.op = 0, .lge = [ 4, 1, 1])

}

[starting lifetime...]









[starting lifetime...]
=================================================================
==10064==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x000107222e40 at pc 0x000104088dc8 bp 0x00016bd7e220 sp 0x00016bd7e218
READ of size 8 at 0x000107222e40 thread T0
    #0 0x104088dc4 in print_lifetime c.c:123
    #1 0x104087ff0 in human_prune c.c:521
    #2 0x1040858b4 in find_major_groups c.c:606
    #3 0x10408465c in main c.c:684
    #4 0x18e257f24  (<unknown module>)

0x000107222e40 is located 64 bytes to the right of 192-byte region [0x000107222d40,0x000107222e00)
allocated by thread T0 here:
    #0 0x10446f234 in wrap_calloc+0x9c (libclang_rt.asan_osx_dynamic.dylib:arm64e+0x43234) (BuildId: f0a7ac5c49bc3abc851181b6f92b308a32000000200000000100000000000b00)
    #1 0x1040842e4 in main c.c:649
    #2 0x18e257f24  (<unknown module>)

SUMMARY: AddressSanitizer: heap-buffer-overflow c.c:123 in print_lifetime
Shadow bytes around the buggy address:
  0x007020e64570: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x007020e64580: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  0x007020e64590: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x007020e645a0: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  0x007020e645b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x007020e645c0: fa fa fa fa fa fa fa fa[fa]fa fa fa fa fa fa fa
  0x007020e645d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x007020e645e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x007020e645f0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x007020e64600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x007020e64610: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==10064==ABORTING
zsh: abort      ./run 1202309052.022153_10883911679_0-10883911679_z_36R.txt
visualization_utility: ]]]]]]]]]]]]]]]












*/






