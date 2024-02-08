#include <ctype.h>       // visualization and pruning utility to be used on the output of srnfgp search. 
#include <errno.h>       // written on 202311072.145550 by dwrr
#include <fcntl.h>
#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>  
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>




// formula for size of 0 space:
//      (5 ^ 9)

// formula for size of 1 space:
//	(6 ^ 9) * (5) * (6 ^ 3)



// general formula:
//       ((5 + D) ^ 9) * ((5 * ((5 + D) ^ 3)) ^ D)



#define reset "\x1B[0m"
#define red   "\x1B[31m"
#define green   "\x1B[32m"
#define yellow   "\x1B[33m"
#define cyan     "\x1B[36m"
#define bold    "\033[1m"

// #define lightblue "\033[38;5;67m"
//#define blue   "\x1B[34m"

typedef uint8_t byte;
typedef uint64_t nat;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	PM_fea, PM_ns0, PM_pco,
	PM_zr5, PM_zr6, PM_ndi, 
		PM_oer, PM_r0i, 
	PM_h,   PM_f1e, PM_erc,
	PM_nsvl, PM_eda, PM_rmv, PM_ot, PM_mh, 
	
	PM_count
};

static const char* pm_spelling[] = {
	"PM_fea", "PM_ns0", "PM_pco", 
	"PM_zr5", "PM_zr6", "PM_ndi", 
		"PM_oer", "PM_r0i", 
	"PM_h",   "PM_f1e", "PM_erc", 
	 "PM_nsvl", "PM_eda", "PM_rmv", "PM_ot", "PM_mh",
};



// ------ constants: --------



// general purpose ones:

static const nat execution_limit = 1000000000; 
static const nat pre_run_ins = 10000000;



// ./run old_zlists/1202311234.131009_\(1202309214.131350_0_0_0_good.txt\)_114_pruned.txt







//////////////////////////////////////////////////////////////////////

static const nat array_size = 10000;


/// NOTE: we need to compute the xp function for a given graph, if we want to run it in stage 4 ("simplified vertical line") at all.




//////////////////////////////////////////////////////////////////////





// other pruning metrics:


// static const nat fdl_count = 8;

static const nat oer_count = 80;

static const nat max_acceptable_modnat_repetions = 15;
static const nat max_acceptable_consecutive_incr = 50;
static const nat max_acceptable_run_length = 8;

static const nat expansion_check_timestep = 10000;
static const nat required_er_count = 25; 

static const nat expansion_check_timestep2 = 1000; 
static const nat required_s0_increments = 6;  


static const nat viz = 0;

static const nat starting_base = pre_run_ins;
static const nat pre_run = 4000000;
static const nat acc_ins = 24000000;
static const nat counter_thr = 5;
static const nat blackout_radius = 7;
static const nat safety_factor = 20;
static const nat vertical_line_count_thr = 3;


static const nat void_bucket_uid = 9999999999999999;




// major/minor groups
static const nat similarity_threshold = 22;

// image generation
// static const nat image_size = 600;

static const nat row_count = 120;

static const nat paging_row_count = 100;
static const nat window_begin = 0;
static const nat window_end = 10000000000;

static const nat timestep_delay = 80;







// do this:


// ./run 1202311234.131009_\(1202309214.131350_0_0_0_good.txt\)_114_pruned.txt

//     print 012110222543301040010441
//        origin 2










// ------ globals: --------



static byte graph_count = 0;
static byte operation_count = 0;

static nat* array = NULL;
static bool* modes = NULL;
static bool* executed = NULL; 
static struct bucket* buckets = NULL;
static struct bucket* scratch = NULL;

static nat counts[PM_count] = {0};

static char directory[4096] = "./";
static char filename[4096] = {0};

static char image_directory[4096] = "./images/";


struct zlist {
	byte** values;
	nat count;
};

struct bucket {
	nat index;
	nat data;
	nat counter;
	nat uid;
	nat is_moving;
};






static void init_graph_from_string(const char* string, byte* graph) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

static void print_graph_as_adj(byte* graph) {
	puts("graph adjacency list: ");
	puts("{");
	for (byte i = 0; i < graph_count; i += 4) {
		const byte op = graph[i + 0];
		const byte l = graph[i + 1];
		const byte g = graph[i + 2];
		const byte e = graph[i + 3];

		printf("\t#%u: ins(.op = %hhu, .lge = [", i, op);
		printf("%2u,", l);
		printf("%2u,", g);
		printf("%2u]", e);
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


/*
static void print_buckets(struct bucket* buckets, const nat bucket_count) {
	for (nat b = 0; b < bucket_count; b++) {
		if (buckets[b].data) {
			printf("\nBUCKET uid#%llu = { .index = %llu, .data = %llu, .counter = %llu, .uid = %llu,  %c  } \n\n",
				b,
				buckets[b].index,
				buckets[b].data,
				buckets[b].counter,
				buckets[b].uid,
				buckets[b].is_moving ? '#' : ' ' 
			);
		} else 
			printf("@ ");
	}
}*/


static void get_graphs_z_value(byte* graph, char string[64]) { 
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}

static nat print_lifetime(
	byte* graph, 
	const byte origin,
	const nat print_count, 
	const nat er_count,
	const nat pre_run_count
) {
	const nat n = array_size;

	array = calloc(n + 1, sizeof(nat));
	modes = calloc(n + 1, sizeof(bool));

	nat er = 0, E = 0, Eer = 0;
	nat pointer = 0;
	byte ip = origin;
	if (print_count) puts("[starting lifetime...]");
	nat e = 0;



	//char string[64] = {0};
	//get_graphs_z_value(graph, string);

	//const bool debug = not strcmp(string, "012110452543300240021000");



	// printf("[z = %s, origin = %hhu]\n", string, origin);




	for (; e < print_count + pre_run_count; e++) {

		const byte I = ip * 4, op = graph[I];

	/*	if (debug) { 
			printf("[e=%llu]: executing &%hhu: [op=%hhu, .lge={%hhu, %hhu, %hhu}]: "
				"{pointer = %llu, *n = %llu} \n", 
					e, ip, 
					graph[4 * ip + 0], 
					graph[4 * ip + 1], 
					graph[4 * ip + 2], 
					graph[4 * ip + 3], 
					pointer, array[n]
			);
			
			puts(""); print_nats(array, 5); puts(""); 
			puts(""); 
			getchar();
		}*/


		if (op == one) { 
			if (pointer == n) { puts("fea pointer overflow"); abort(); } 
			pointer++; 
		}


		else if (op == five) {

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

				if (timestep_delay) usleep(timestep_delay);

				er++;

				if (er == paging_row_count) Eer = E;

				if (er > er_count) { puts("maxed out er count."); goto done; }
			}

			pointer = 0;

			memset(modes, 0, sizeof(bool) * (n + 1));

			
		}
		else if (op == two) { array[n]++; }

		else if (op == six) { array[n] = 0; }

		else if (op == three) { array[pointer]++; modes[pointer] = 1; }

		byte state = 0;
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

	nat xw = 0;
	for (; xw < n and array[xw]; xw++) { }
	puts("");
	print_nats(array, xw);
	puts("");

	free(array);
	free(modes);
	return Eer;
}














static nat generate_image_for_lifetime(
	byte* graph, 
	const byte origin, 
	const nat print_count,
	const nat pre_run_count
) {

	const int dir = open(image_directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", image_directory); 
		return 2; 
	}
	int flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
	mode_t m  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	
	char string[64] = {0};
	get_graphs_z_value(graph, string);
	snprintf(filename, sizeof filename, "%s_%u.ppm", string, origin);
	printf("filename=%s\n", filename);

	const int file = openat(dir, filename, flags, m);
	if (file < 0) {
		perror("create openat file");
		printf("filename=%s ", filename);
		close(dir); 
		return 1;
	}

	nat n = array_size;

	array = calloc(array_size + 1, sizeof(nat));
	modes = calloc(array_size + 1, sizeof(bool));

	nat er = 0;
	nat e = 0;
	nat pointer = 0;

	byte ip = origin;


	char header[128] = {0};
	snprintf(header, sizeof header, "P6\n%llu %llu\n255\n", window_end - window_begin, row_count);
	write(file, header, strlen(header));

	for (; e < print_count + pre_run_count; e++) {

		const byte I = ip * 4;
		const byte op = graph[I];

		if (op == one) { if (pointer == n) abort(); pointer++; }

		else if (op == five) {
			if (e >= pre_run_count) {
				if (er >= row_count) goto done;
				for (nat i = window_begin; i < window_end; i++) {
					const uint8_t r = modes[i] * 255;
			                write(file, &r, 1);
			                write(file, &r, 1);
			                write(file, &r, 1);
				}
				er++;
			}
			pointer = 0;
			memset(modes, 0, sizeof(bool) * (n + 1));
		}
		else if (op == two) { array[n]++; }
		else if (op == six) { array[n] = 0; }
		else if (op == three) { array[pointer]++; modes[pointer] = 1; }

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
		
done:
	free(array);
	free(modes);
	close(file); 
	
	printf("write: created %llu z values to ", 1LLU);
	printf("%s : %s\n", directory, filename);
	close(dir); 

	return 0;
}

static void generate_images(struct zlist list) {

	printf("generate: generating %llu lifetimes .ppm's...\n", list.count);
	if (mkdir("images/", 0777)) { perror("mkdir"); getchar(); }
	
	for (nat i = 0; i < list.count; i++) {
		for (byte o = 0; o < graph_count; o += 4) {

			if (list.values[i][o + 0] != 2) continue;

			if (not generate_image_for_lifetime(list.values[i], o / 4, execution_limit, pre_run_ins)) continue;

			printf("warning: image for z value already exists... do you wish to continue? ");
			getchar();
		}
	}

	printf("generated %llu images in %s.\n", list.count, directory);
}



static void synthesize_graph_over_one_group(struct zlist zlist) {

	nat* tallys = calloc((size_t) (graph_count * operation_count), sizeof(nat));

	for (nat z = 0; z < zlist.count; z++) {

		for (nat i = 0; i < graph_count; i++) {
			if (not (i % 4)) continue;
			tallys[i * operation_count + zlist.values[z][i]]++;
		}
	}

	printf("synthesized graph [over %llu z values]:\n", zlist.count);

	for (byte i = 0; i < graph_count; i += 4) {
		printf("  " red "#%u" reset "  :: { .op = %u,   .lge={ \n", i / 4,  zlist.values[0][i]);

		printf("\t\t.l={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = tallys[(i + 1) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == zlist.count ?  green : yellow, (double) count / zlist.count, count); 
		}
		printf(" }, \n");
		printf("\t\t.g={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = tallys[(i + 2) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == zlist.count ? green : yellow, (double) count / zlist.count, count); 
		}
		printf(" }, \n");
		printf("\t\t.e={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = tallys[(i + 3) * operation_count + o];
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
		char string[64] = {0};
		get_graphs_z_value(list.values[k], string);
		puts(string);
	}
	printf("}\n");
}


static nat similarity_count(byte* a, byte* b) {
	nat count = 0;
	for (nat i = 0; i < graph_count; i++) {
		if (a[i] == b[i]) count++;
	}
	return count;
}

static void print_bytes(byte* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2hhu ", v[i]);
	printf("] \n");
}

static void partition_into_minor_groups(struct zlist total)  {
	
	nat hcount = 0;
	struct zlist* hlist = calloc(total.count, sizeof(struct zlist));

	for (nat z = 0; z < total.count; z++) {

		byte* graph = total.values[z];
	
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
					print_bytes(hlist[i].values[z], graph_count); puts("");
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











static void write_to_file(byte* graph, const char* previous_filename, const nat previous_list_count) {
	
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
		snprintf(newfilename, sizeof newfilename, "%s_(%s)_%llu_pruned.txt", dt, previous_filename, previous_list_count);
		strlcpy(filename, newfilename, sizeof filename);

		goto try_open;
	}

	char string[64] = {0};
	get_graphs_z_value(graph, string);
	write(file, string, graph_count);
	write(file, " \n", 2);
	close(file); 
	
	if (m) {
		printf("write: created %llu z values to ", 1LLU);
		printf("%s : %s\n", directory, newfilename);
		close(dir);  
		return;
	}

	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_(%s)_%llu_pruned.txt", dt, previous_filename, previous_list_count);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	//printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strlcpy(filename, newfilename, sizeof filename);

	close(dir);

	//printf("\033[1mwrite: saved %llu z values to ",1LLU);
	//printf("%s : %s \033[0m\n", directory, newfilename);
}


static void human_prune(const char* previous_filename, struct zlist list) {

	struct termios terminal;
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_lflag &= ~((size_t) ECHO | ICANON);
	tcsetattr(0, TCSAFLUSH, &copy);

	for (nat z = 0; z < list.count; z++) {

		bool written = false;

		for (byte o = 0; o < graph_count; o += 4) {
			
			if (list.values[z][o + 0] != 2) continue;

			nat offset = pre_run_ins;

			print:;
			printf("\033[H\033[2J");
			puts("");
			puts("");
			for (nat i = 0; i < z * 5; i++) putchar('@');
			for (nat i = 0; i < (list.count - z) * 5; i++) putchar('.');
			puts("");
			puts("");

			const nat e = print_lifetime(list.values[z], o / 4, execution_limit, row_count, offset);
			print_bytes(list.values[z], graph_count); puts("");
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
					write_to_file(list.values[z], previous_filename, list.count);
					puts(bold green "------- Z VALUE WRITTEN TO GOOD LIST -------" reset);
					goto next_z;
			
				} else { puts("write error"); getchar(); goto print; }

			} else { 
				puts("input error"); 
				getchar();
				goto print; 
			}
		}
		next_z: continue;
	}

	tcsetattr(0, TCSAFLUSH, &terminal);
}




static void find_major_groups(struct zlist list) {

	struct zlist group = {
		.values = calloc(list.count, sizeof(nat*)), 
		.count = 0
	};

	for (nat i = 0; i < 6; i++) {

		for (nat z = 0; z < list.count; z++) {
			if (list.values[z][1 * 4 + 3] == 1) continue;
			if (list.values[z][5 * 4 + 0] == i or i == 5) group.values[group.count++] = list.values[z];
		}

		printf("%llu op is new:   performing synthesize graph over %llu z values:\n", i, group.count); 
		print_zlist("major group", i, group);

		printf("%llu op is new:   performing synthesize graph over MAJOR GROUP %llu z values:\n", i, group.count); 

		if (group.count) synthesize_graph_over_one_group(group);
		else printf("ERROR: group size is zero. synthesize_graph_over_one_group(group) was not called. \n");

		getchar();


		partition_into_minor_groups(group);

		group.count = 0;
	}

	puts("[finished all sythesized graphs over all groups.]");
}



static void visualize(char* string) {

	byte* graph = calloc(graph_count, 1);
	init_graph_from_string(string, graph);

	char buffer[128] = {0};
	printf("give the origin: ");
	fgets(buffer, sizeof buffer, stdin);
	byte o = (byte) atoi(buffer);

	print_lifetime(graph, o, execution_limit, row_count, pre_run_ins);
	print_bytes(graph, graph_count); 
	puts("");
	print_graph_as_adj(graph);
}












static nat gather_buckets_at(

	struct bucket* local_buckets, 
	struct bucket* output, 

	const nat desired_index, 
	const nat radius, 
	const nat bucket_count
) {
	nat out_count = 0;
	
	for (nat b = 0; b < bucket_count; b++) {

		if (	local_buckets[b].index <= desired_index + radius 
			and 
			((int64_t) local_buckets[b].index >= (int64_t) desired_index - (int64_t) radius)
		) {
			output[out_count] = local_buckets[b]; 
			output[out_count++].uid = b;
		}
	}
	return out_count;
}


static nat get_max_moving_bucket_uid(struct bucket* local_scratch, const nat scratch_count) {
	nat max_bucket_data = 0;
	struct bucket max_bucket = {.uid = void_bucket_uid};
	
	for (nat s = 0; s < scratch_count; s++) {

		if (local_scratch[s].is_moving) return local_scratch[s].uid;

		if (local_scratch[s].data >= max_bucket_data) {
			max_bucket_data = local_scratch[s].data;
			max_bucket = local_scratch[s];
		}
	}

	return max_bucket.uid;
}






static void write_xp_data_to_file(char* string, nat string_length) {
	
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
		snprintf(newfilename, sizeof newfilename, "%s_xpdata.txt", dt);
		strlcpy(filename, newfilename, sizeof filename);

		goto try_open;
	}

	write(file, string, string_length);
	close(file); 
	
	if (m) {
		printf("write: created %llu z values to ", 1LLU);
		printf("%s : %s\n", directory, newfilename);
		close(dir);  
		return;
	}

	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_xpdata.txt", dt);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	strlcpy(filename, newfilename, sizeof filename);

	close(dir);

}


// static int z_first_timestep_count = 0;



static void run_graph_write_timesteps_starting_at(byte* graph, byte origin) {
	
	char* lifetime_data = NULL;
	nat lifetime_count = 0;

	const nat n = array_size;
	memset(array, 0, (n + 1) * sizeof(nat));
	
	byte ip = origin;
	nat pointer = 0;


	//nat previous_xw = (nat) -1;
	//nat previous_lm = (nat) -1;

	for (nat e = 0; e < execution_limit; e++) {
		const byte I = ip * 4;
		const byte op = graph[I]; 

		if (op == one) pointer++;
		else if (op == five) pointer = 0;
		else if (op == two) array[n]++;
		else if (op == six) array[n] = 0;   
		else if (op == three) array[pointer]++;


		/*

		nat xw = 0;
		for (; xw < n and array[xw]; xw++) { }
		
		nat lm = 0; 
		for (nat i = 0; i < n; i++) {
			if (array[i] > lm) lm = array[i]; 
			if (not array[i]) break;
		}		
		
		if (xw != previous_xw or lm != previous_lm or e == execution_limit - 1) {

			if (e == 0) {
				char string[64] = {0};
				get_graphs_z_value(graph, string);
				printf("[%u] incremented my_count, for z = %s  \n", z_first_timestep_count, string);

				z_first_timestep_count++;
			}

			char string[128] = {0};
			const size_t length = (size_t) snprintf(string, sizeof string, "%llu %llu %llu\n", e, xw, lm);

			lifetime_data = realloc(lifetime_data, lifetime_count + length);
			memcpy(lifetime_data + lifetime_count, string, length);
			lifetime_count += length;

			previous_xw = xw;
			previous_lm = lm;
		}
		*/


		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		executed[I + state] = 1;
		ip = graph[I + state];

	}

	//fwrite(lifetime_data, 1, lifetime_count, stdout);
	//getchar();



	// write_xp_data_to_file(lifetime_data, lifetime_count);


	nat xw = 0;
	for (; xw < n and array[xw]; xw++) { }



	const nat bucket_width = 2;
	const nat bucket_count = 3000;

	nat* buckets = calloc(bucket_count, sizeof(nat));

	for (nat j = 0; j < xw; j++) {
		for (nat i = 0; i < bucket_count; i++) {
			if (array[j] < (i + 1) * bucket_width) {
				buckets[i]++;
				break;
			}
		}
	}

	//printf("\033[H\033[2J");
	for (nat i = 0; i < bucket_count; i++) {
		printf(". %5llu: ", i * bucket_width);
		for (nat j = 0; j < buckets[i]; j++) printf("#");
		puts("");
	}

	puts("");
	free(buckets);


	print_nats(array, xw + 5);

	getchar();
}

 
 

static void run_graph_write_timesteps(byte* graph) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		run_graph_write_timesteps_starting_at(graph, o);
	}
}





static bool execute_graph_starting_at(byte* graph, byte origin) {


	const nat n = array_size;

	memset(array, 0, (n + 1) * sizeof(nat));
	memset(modes, 0, (n + 1) * sizeof(bool));
	memset(buckets, 0, (n + 1) * sizeof(struct bucket));
	memset(scratch, 0, (n + 1) * sizeof(struct bucket));     //todo: delay doing this until you see that the graph is
								 //      actually worth running nsvlpm on!!! 
	memset(executed, 0, graph_count * sizeof(bool));
	
	byte ip = origin;
	byte 	last_mcal_op = 0;

	nat a = PM_count;
	nat pointer = 0;
	nat 	er_count = 0, 
	    	OER_er_at = 0, 		OER_counter = 0, 

		// FDL_er_at = 0, 		FDL_counter = 0, 

		R0I_counter = 0,     	H_counter = 0;
	
	for (nat b = 0; b < n; b++) {                          /// same with this one too. 
		buckets[b].index = b;
		buckets[b].uid = b;
	}

	nat base = starting_base, timestep_count = 0, batch_count = 0, scratch_count = 0;

	nat e = 0;
	for (; e < execution_limit; e++) {

		if (e >= expansion_check_timestep2) {
			if (array[0] < required_s0_increments) { a = PM_f1e; goto bad; }
		}

		if (e >= expansion_check_timestep) {
			if (er_count < required_er_count) 	{ a = PM_erc; goto bad; }
		}

		const byte I = ip * 4;
		const byte op = graph[I];   

		if (op == one) {
			if (pointer == n) 	{ a = PM_fea; goto bad; } 
			if (not array[pointer]) { a = PM_ns0; goto bad; } 
			pointer++;
		}

		else if (op == five) {
			if (last_mcal_op != three) 	{ a = PM_pco; goto bad; } 
			if (not pointer) 		{ a = PM_zr5; goto bad; } 

		//	// rer:
		//	if (RER_er_at == pointer) RER_counter++; else { RER_er_at = pointer; RER_counter = 0; }
		//	if (RER_counter == rer_count) { a = PM_rer; goto bad; }

			// oer:
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter == oer_count) { a = PM_oer; goto bad; }
		


		//	// fdl:
		//	if (pointer == FDL_er_at) { FDL_counter++; if (FDL_er_at) FDL_er_at--; } 
		//	else { FDL_er_at = pointer; FDL_counter = 0; }
		//	if (FDL_counter == fdl_count) { a = PM_fdl; goto bad; }


			
			if (*modes) R0I_counter++; else R0I_counter = 0;
			if (R0I_counter > max_acceptable_consecutive_incr) { a = PM_r0i; goto bad; }

			// nsvl
			if (e >= base + pre_run) timestep_count++;

			if (viz and e >= base + pre_run) {
				
				for (nat i = window_begin; i < window_end; i++) {
					if (not array[i]) break;

					if (modes[i]) {
						printf("%s", (i == pointer ? "@" : "#"));
						continue;
					}

					scratch_count = gather_buckets_at(buckets, scratch, i, 0, n);
					if (not scratch_count) {
						printf(cyan "0" reset );
						continue;
					}
					if (scratch_count == 1) {
						printf(" ");
						continue;
					}
					if (scratch_count == 2) {
						if (scratch[0].is_moving) { printf(red "*" reset); continue; } 
						else if (scratch[1].is_moving) { printf(red "*" reset); continue; }
					}
					printf("?");
				} 
				puts("");

				usleep(100);
			}

			memset(modes, 0, (n + 1) * sizeof(bool));
			pointer = 0;
			er_count++;
		}

		else if (op == two) {
			array[n]++;
		}

		else if (op == six) {  
			if (not array[n]) 	{ a = PM_zr6; goto bad; }
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) 	{ a = PM_ndi; goto bad; }

			// h
			if (pointer and modes[pointer - 1]) H_counter++; else H_counter = 0;
			if (H_counter > max_acceptable_run_length) { a = PM_h; goto bad; }

			array[pointer]++;
			modes[pointer] = 1;

			// nsvl
			if (e >= base + pre_run) {

				const nat desired_index = pointer;
				scratch_count = gather_buckets_at(buckets, scratch, desired_index, 0, n);

				if (not scratch_count) goto dont_accumulate;

				const nat trigger_uid = get_max_moving_bucket_uid(scratch, scratch_count);
				if (trigger_uid == void_bucket_uid) {
					printf(
						"const nat trigger_uid = get_max_moving_bucket_uid(scratch, scratch_count);"
						"if (trigger_uid == void_bucket_uid)\n"
					); 
					
					abort(); 
				}

				buckets[trigger_uid].data++;
				buckets[trigger_uid].counter++;

				if (not scratch_count) {
					printf(red "not scratch_count\n" reset);
					abort();
				}
			
				scratch_count = gather_buckets_at(buckets, scratch, desired_index, blackout_radius, n);

				if (not scratch_count) {
					printf(
						"scratch_count = gather_buckets_at(buckets, scratch, desired_index, blackout_radius, n);"
						"if (not scratch_count)\n"
					); 
					printf("desired index = %llu\n", desired_index);
					// print_buckets(buckets, n);
					abort();
				}

				nat moving_uid = 0;

				if (buckets[trigger_uid].counter == counter_thr) {

					// question:  do we do a blackout (before or after)    when we move a bucket as well? 

					buckets[trigger_uid].counter = counter_thr + 1;
					buckets[trigger_uid].is_moving = false;

					const nat neighbor_position = buckets[trigger_uid].index - 1;
					scratch_count = gather_buckets_at(buckets, scratch, neighbor_position, 0, n);
					if (not scratch_count) { 
						printf(
							"const nat neighbor_position = buckets[trigger_uid].index - 1;"
							"scratch_count = gather_buckets_at(buckets, scratch, neighbor_position, 0, n);"
							"if (not scratch_count)\n"
						); 
						abort(); 
					}

					moving_uid = get_max_moving_bucket_uid(scratch, scratch_count);
					if (moving_uid == void_bucket_uid) {
						printf(
							"moving_uid = get_max_moving_bucket_uid(scratch, scratch_count);"
							"if (moving_uid == void_bucket_uid) \n"
						);
						abort();
					}

					if (buckets[moving_uid].data) { 
						buckets[moving_uid].index++;
						buckets[moving_uid].counter = counter_thr + 1; 
						buckets[moving_uid].is_moving = true;
					}



				} else {
					for (nat s = 0; s < scratch_count; s++) {
						if (scratch[s].uid == trigger_uid) continue; 
						if (scratch[s].counter > counter_thr) continue;
						if (scratch[s].uid == moving_uid) continue; 
						buckets[scratch[s].uid].counter = 0;
					} 
				}
				dont_accumulate:;
			}
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		if (e >= base + pre_run + acc_ins) {


			const double factor = (double) safety_factor / (double) 100.0;  
			const nat required_data_size = (nat) ((double) factor * (double) timestep_count);

			printf("threshold info: \n\n\t\ttimestep_count: %llu,  required_data_size: %llu\n\n", timestep_count, required_data_size);

			nat stats[2][2][2] = {0};
			nat vertical_line_count = 0, good_count = 0;
			for (nat b = 0; b < n; b++) {
				if (	buckets[b].data >= required_data_size and 
					buckets[b].counter > counter_thr and 
					buckets[b].is_moving
				) vertical_line_count++; else good_count++;
				stats[buckets[b].data >= required_data_size][buckets[b].counter > counter_thr][buckets[b].is_moving]++;
			}

			printf("FINAL GROUP COUNTS: \n\n\t\tvl_count: %llu,  good_count: %llu\n\n", vertical_line_count, good_count);
				
			printf("BUCKET STATISTICS:\n\t\t[buckets[b].data >= bucket_data_thr][buckets[b].counter > counter_thr][buckets[b].is_moving]\n"); 
			puts("");
			printf("\t  [0][0][0]: %llu  [0][0][1]: %llu\n", stats[0][0][0], stats[0][0][1]);
			printf("\t  [0][1][0]: %llu  [0][1][1]: %llu\n", stats[0][1][0], stats[0][1][1]);
			puts("");
			printf("\t  [1][0][0]: %llu  [1][0][1]: %llu\n", stats[1][0][0], stats[1][0][1]);
			printf("\t  [1][1][0]: %llu  [1][1][1]: %llu\n", stats[1][1][0], stats[1][1][1]);
			puts("");
			
			if (stats[1][0][0]) { 
				puts("");
				printf( red "NSVLPM ERROR: too low safety_factor parameter! found %llu buckets which where .data >= required_data_size, "
					"but is_moving=false... soft aborting..." reset, stats[1][0][0]
				);
				puts(""); fflush(stdout); 
			}

			if (vertical_line_count > vertical_line_count_thr) {
				// if (should_print_pm) 
				// printf("NSVL PRUNED ON BATCH COUNT:   %llu batches,     [e = %llu]\n", batch_count, e);
				a = PM_nsvl; 
				goto bad;
			}

			base += pre_run + acc_ins; 
			scratch_count = 0;

			memset(scratch, 0, (n + 1) * sizeof(struct bucket));
			memset(buckets, 0, (n + 1) * sizeof(struct bucket));

			for (nat b = 0; b < n; b++) {
				buckets[b].index = b;
				buckets[b].uid = b;
			}
			timestep_count = 0;
			batch_count++;
		}

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		executed[I + state] = 1;
		ip = graph[I + state];
	}

	for (byte i = 0; i < graph_count; i += 4) {
		if (not executed[i + 1] and graph[i + 1] or
		    not executed[i + 2] and graph[i + 2] or
		    not executed[i + 3] and graph[i + 3]
		) {
			a = PM_eda; 
			goto bad; 
		}
	}

	

	return false; 
	
bad: 	counts[a]++;
	printf("%7s ( on e=%8llu )\n", pm_spelling[a], e);
	return true;
}


static bool execute_graph(byte* graph) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not execute_graph_starting_at(graph, o)) return false;

		////////////////////////////////////////////////////////////////////////////////////////////
		
		//TODO: NOTE: is it bad for the xfg to start at any possible 3? does it need to start at 
		//  a particular one, in order to be good? does this matter to the xfg?

		////////////////////////////////////////////////////////////////////////////////////////////

	}
	return true;
}








static bool RMV_execute_graph_starting_at(byte* graph, byte origin) {

	const nat n = array_size;

	memset(executed, 0, graph_count * sizeof(bool));

	memset(array, 0, (n + 1) * sizeof(nat));        //todo:  do the lazy array zeroing optimization. 
	memset(modes, 0, (n + 1) * sizeof(bool));
	
	byte ip = origin, last_mcal_op = 0;

	nat 	a = PM_count, 
		pointer = 0, 
		er_count = 0, 
	    	OER_er_at = 0, 
		OER_counter = 0, 
		R0I_counter = 0,
		H_counter = 0,
		RMV_counter = 0,
		RMV_value = 0;


	nat* timeout = calloc(operation_count, sizeof(nat));

	nat e = 0;
	for (; e < execution_limit; e++) {

		if (e >= expansion_check_timestep2) {
			if (array[0] < required_s0_increments) { a = PM_f1e; goto bad; }
		}

		if (e >= expansion_check_timestep) {
			if (er_count < required_er_count) 	{ a = PM_erc; goto bad; }
		}

		const byte I = ip * 4, op = graph[I];
	
		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] > execution_limit >> 1) { a = PM_ot; goto bad; }
			timeout[i]++;
		}
		timeout[ip] = 0;

		if (op == one) {
			if (pointer == n) 	{ a = PM_fea; goto bad; } 
			if (not array[pointer]) { a = PM_ns0; goto bad; } 
			pointer++;
		}

		else if (op == five) {
			if (last_mcal_op != three) { a = PM_pco; goto bad; } 
			if (not pointer)           { a = PM_zr5; goto bad; } 

			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter == oer_count) { a = PM_oer; goto bad; }
			
			if (*modes) R0I_counter++; else R0I_counter = 0;
			if (R0I_counter > max_acceptable_consecutive_incr) { a = PM_r0i; goto bad; }
			
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < n; i++) {
				if (not array[i]) break;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter == max_acceptable_modnat_repetions) { a = PM_rmv; goto bad; }
			}
			
			memset(modes, 0, (n + 1) * sizeof(bool));
			pointer = 0;
			er_count++;
		}

		else if (op == two) array[n]++;

		else if (op == six) {  
			if (not array[n]) 	{ a = PM_zr6; goto bad; }
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) 	{ a = PM_ndi; goto bad; }

			if (pointer and modes[pointer - 1]) H_counter++; else H_counter = 0;
			if (H_counter > max_acceptable_run_length) { a = PM_h; goto bad; }

			array[pointer]++;
			modes[pointer] = 1;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		executed[I + state] = 1;
		ip = graph[I + state];
	}

	for (byte i = 0; i < graph_count; i += 4) {
		if (not executed[i + 1] and graph[i + 1] or
		    not executed[i + 2] and graph[i + 2] or
		    not executed[i + 3] and graph[i + 3]
		) { a = PM_eda; goto bad; }
	}

	//print_nats(array, n);

	nat xw = 0;
	for (; xw < n and array[xw]; xw++) { }

	// printf("[PM_MH] info: xw = %llu\n", xw);

	nat* values = calloc(n, sizeof(nat));
	for (nat i = 0; i < xw; i++) {
		if (array[i] >= n) { a = PM_mh; goto bad; }
		values[array[i]]++; 
	}

	//printf("[PM_MH] info: {");
	//for (nat i = 0; i < n; i++) {
	//	if (values[i]) printf("%llu:%llu  ", i, values[i]);
	//}
	//printf("}\n");

	for (nat i = 0; i < n; i++) {
		if (values[i] > xw >> 2) { a = PM_mh; goto bad; }
	}
	free(values);

	return false; 
	
bad: 	counts[a]++;
	printf("%7s ([rmv-exec] on e=%8llu )\n", pm_spelling[a], e);

	return true;
}

static bool RMV_execute_graph(byte* graph) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not RMV_execute_graph_starting_at(graph, o)) return false;
	}
	return true;
}






// todo:     do PM_mh here!             2 2 3 2 2 3 2 2 3 2 2 3 2 2 3 2 2 3 2 3 2 2 3 2 3 2 2 3 2 3 2 





	// if (should_print_pm) printf("%7s ( on e=%8llu )\n", pm_spelling[a], e);
		

	//  {{ z = ", ); print_graph_raw(); puts(" }}");
	// }

	






static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], counts[i]);
	}
	puts("");
	puts("[done]");
}

static void prune(const char* previous_filename, struct zlist list) {

	nat good = 0, bad = 0;

	for (nat z = 0; z < list.count; z++) {

		char string[64] = {0};
		get_graphs_z_value(list.values[z], string);
		printf("\r trying z = %s     \n", string);
		
		if (execute_graph(list.values[z])) {
			printf(bold red " ---> BAD (%llu / %llu)" reset "\n", z, list.count);
			bad++;
		} else {
			printf(bold green " ---> GOOD (%llu / %llu)" reset "\n", z, list.count);
			write_to_file(list.values[z], previous_filename, list.count);
			good++;
		}
	}
	
	print_counts();
	printf("\n\n\t\t\tgood: %llu\n\t\t\tbad: %llu\n\n\n", good, bad);
}

static void RMV_prune(const char* previous_filename, struct zlist list) {

	nat good = 0, bad = 0;

	for (nat z = 0; z < list.count; z++) {

		char string[64] = {0};
		get_graphs_z_value(list.values[z], string);
		printf("\r trying z = %s     \n", string);
		
		if (RMV_execute_graph(list.values[z])) {
			printf(bold red " ---> BAD (%llu / %llu)" reset "\n", z, list.count);
			bad++;
		} else {
			printf(bold green " ---> GOOD (%llu / %llu)" reset "\n", z, list.count);
			write_to_file(list.values[z], previous_filename, list.count);
			good++;
		}
	}
	
	print_counts();
	printf("\n\n\t\t\tgood: %llu\n\t\t\tbad: %llu\n\n\n", good, bad);
}



static void record_xp_data(struct zlist list) {     // writes out a single file that has n lifetimes, where n is AT LEAST z_count. might be larger, as some z values must be run from mulitple origins, and thus their lifetimes on all origins will be output, as if they were sepearte z values.

	for (nat z = 0; z < list.count; z++) {

		char string[64] = {0};
		get_graphs_z_value(list.values[z], string);
		printf("\r trying z = %s     \n", string);
		run_graph_write_timesteps(list.values[z]);
	}
}


static void print_help(void) {
	printf("available commands:\n\t quit \n\t list \n\t machine prune \n\t "
		"human prune \n\t generate images \n\t synthesize graph \n\t print \n\t\n");
}

int main(int argc, const char** argv) {

	if (argc <= 1) return puts("give input z list filename as "
		"an argument! eg, ./run old_zlists/1202311234.131009_\\(1202309214.131350_0_0_0_good.txt\\)_114_pruned.txt");


	FILE* file = fopen(argv[1], "r");
	if (not file) { perror("fopen"); exit(1); }
	
	struct zlist zlist = {0};

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {

		char* index = strchr(buffer, ' ');
		if (not index) abort();
		buffer[index - buffer] = 0;

		//printf("reading z value %llu: ", zlist.count);
		//puts(buffer);

		graph_count = (byte) strlen(buffer);

		byte* graph = calloc(graph_count, 1);

		init_graph_from_string(buffer, graph);


		zlist.values = realloc(zlist.values, sizeof(nat*) * (zlist.count + 1));
		zlist.values[zlist.count++] = graph;

	}
	operation_count = graph_count / 4;

	fclose(file);

	array    = calloc(array_size + 1, sizeof(nat));
	modes    = calloc(array_size + 1, sizeof(bool));
	executed = calloc(graph_count,    sizeof(bool));
	buckets  = calloc(array_size + 1, sizeof(struct bucket));
	scratch  = calloc(array_size + 1, sizeof(struct bucket));

	char input[4096] = {0};

loop:
	printf(":%llu: ", zlist.count);
	fgets(input, sizeof input, stdin);
	if (not strcmp(input, "quit\n")) exit(0);
	else if (not strcmp(input, "clear\n")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help\n")) print_help();
	else if (not strcmp(input, "list\n")) print_zlist("current z list", 0, zlist);
	else if (not strcmp(input, "machine prune\n")) prune(argv[1], zlist);
	else if (not strcmp(input, "rmv machine prune\n")) RMV_prune(argv[1], zlist);
	else if (not strcmp(input, "record xp data\n")) record_xp_data(zlist);
	else if (not strcmp(input, "human prune\n")) human_prune(argv[1], zlist);
	else if (not strcmp(input, "generate images\n")) generate_images(zlist);
	else if (not strcmp(input, "synthesize graph\n")) find_major_groups(zlist);
	else if (not strncmp(input, "print ", 6)) visualize(input + 6);
	else printf("unknown command %s\n", input);
	goto loop;
}

















































































// printf("printing results: \n");

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


















// print_nats(le_array, zlist.count); puts("");
	// print_nats(mm_array, zlist.count); puts("");

















































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















/*

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





*/










// #define reset "\x1B[0m"
// #define white  yellow
// #define red   "\x1B[31m"
// #define green   "\x1B[32m"
// #define blue   "\x1B[34m"
// #define yellow   "\x1B[33m"
// #define magenta  "\x1B[35m"
// #define cyan     "\x1B[36m"













	// *mm = max;
	// return le;

	// n:
	//nat le = 0; for (; le < n; le++)  if (not array[le]) break;

	// m:
	//nat max = 0; for (nat i = 0; i < n; i++)  if (array[i] > max) max = array[i];










			//////////////////////////////////////        <--------- delete this!!!

			//	const nat xw = compute_xw(array, n);
			//	const nat dw_count = (nat) ((double) xw * (double) discard_window);
			//	if (pointer < dw_count or pointer > xw - dw_count) goto dont_accumulate;

			//////////////////////////////////////













//					012110222543311540001000








/*static nat compute_xw(nat* a, const nat n) {
	nat i = 0;
	for (; i < n; i++) 
		if (not a[i]) break; 
	return i;
}*/






//done:
	 // BUG HERE!!! obliterating scratch which held the blackout radius buckets, and we havent done a blackout YET!!!!!! 








// write three things to the file:  e, er_count, and xw. to allow for ("coi-ts") ins_count -> xw.









                                                                                                                                                                                                                                                                      

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    






























