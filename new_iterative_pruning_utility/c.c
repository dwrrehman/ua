// 202403225.180314:   ip util   dwrr   
// used for looking through output z lists, and pruning them down further.

// formula for size of 0 space:
//      (5 ^ 9)
// formula for size of 1 space:
//	(6 ^ 9) * (5) * (6 ^ 3)

// formula for size of 3 space:
//((5 + 3) ^ 9) * ((5 * ((5 + 3) ^ 3)) ^ 3)

// general formula:
//       ((5 + D) ^ 9) * ((5 * ((5 + D) ^ 3)) ^ D)

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

#define reset "\x1B[0m"
#define red   "\x1B[31m"
#define green   "\x1B[32m"
#define yellow   "\x1B[33m"
// #define cyan     "\x1B[36m"
#define bold    "\033[1m"
// #define lightblue "\033[38;5;67m"
//#define blue   "\x1B[34m"

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

enum operations { one, two, three, five, six };

static const byte D = 2;        // the duplication count (operation_count = 5 + D)

static const nat fea_execution_limit = 5000;
static const nat execution_limit = 10000000000;
static const nat pre_run_ins = 100;
static const nat array_size = 10000;

enum pruning_metrics {
	z_is_good, PM_fea, PM_ns0, 
	PM_pco, PM_zr5, PM_zr6, PM_ndi, 
	PM_oer, PM_r0i, PM_h, PM_f1e, 
	PM_erc, PM_rmv, PM_ot, PM_csm, 
	PM_mm, PM_snm, 
	PM_count
};

static const char* pm_spelling[] = {
	"z_is_good", "PM_fea", "PM_ns0", 
	"PM_pco", "PM_zr5", "PM_zr6", "PM_ndi", 
	"PM_oer", "PM_r0i", "PM_h", "PM_f1e", 
	"PM_erc", "PM_rmv", "PM_ot", "PM_csm", 
	"PM_mm", "PM_snm"
};

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const nat max_acceptable_er_repetions = 50;
static const nat max_acceptable_modnat_repetions = 15;
static const nat max_acceptable_consecutive_s0_incr = 30;
static const nat max_acceptable_run_length = 9;
static const nat max_acceptable_consequtive_small_modnats = 200;

static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;

static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;

static char image_directory[] = "./images/";
static nat space_size = 0;


// major/minor groups
static const nat similarity_threshold = 22;

// image generation
// static const nat image_size = 600;

// visualization:
static const nat row_count = 110;
static const nat paging_row_count = 100;
static const nat window_begin = 0;
static const nat window_end = 10000000000;
static const nat timestep_delay = 80;

static nat* array = NULL;
static nat* timeout = NULL;
static byte* graph = NULL;


static nat pm_counts[PM_count] = {0};

struct zlist {
	byte** values;
	nat count;
};

static void set_graph(byte* z) { memcpy(graph, z, graph_count); }

static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

static void init_graph_from_string(const char* string) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}

static void print_bytes(byte* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2hhu ", v[i]);
	printf("] \n");
}

static void print_graph(void) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

static void get_graphs_z_value(char string[64]) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void print_zlist(const char* s, nat d, struct zlist list) {
	printf("%s . %llu . zlist (%llu){\n", s, d, list.count);
	for (nat k = 0; k < list.count; k++) {
		putchar(9);
		set_graph(list.values[k]);
		print_graph();
	}
	printf("}\n");
}

static void print_graph_as_adj(void) {
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

static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], pm_counts[i]);
	}
	puts("");
	puts("[done]");
}


static nat execute_graph_starting_at(byte origin) {

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0; 
	memset(timeout, 0, operation_count * sizeof(nat));

	byte ip = origin, last_mcal_op = 0;

	nat 	e = 0,  xw = 0, 
		pointer = 0,  er_count = 0, 
	    	OER_er_at = 0,  OER_counter = 0, 
		R0I_counter = 0, H_counter = 0,
		RMV_counter = 0, RMV_value = 0, CSM_counter = 0;

	for (; e < execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return PM_f1e; 
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return PM_erc; 
		}
		
		const byte I = ip * 4, op = graph[I];

		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return PM_ot; 
			timeout[i]++;
		}
		timeout[ip] = 0;

		if (op == one) {
			if (pointer == n) return PM_fea; 
			if (not array[pointer]) return PM_ns0; 

			if (last_mcal_op == one) H_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;

			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return PM_pco; 
			if (not pointer) return PM_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_acceptable_er_repetions) return PM_oer; 
			
			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_acceptable_consequtive_small_modnats) return PM_csm; 
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_acceptable_modnat_repetions) return PM_rmv; 
			}

			pointer = 0;
			er_count++;
		}

		else if (op == two) {
			array[n]++;
			if (array[n] >= 65535) return PM_snm; 
		}
		else if (op == six) {  
			if (not array[n]) return PM_zr6; 
			array[n] = 0;   
		}
		else if (op == three) {
			if (last_mcal_op == three) return PM_ndi; 

			if (last_mcal_op == one) {
				H_counter++;
				if (H_counter >= max_acceptable_run_length) return PM_h; 
			}

			if (last_mcal_op == five) {
				R0I_counter++; 
				if (R0I_counter >= max_acceptable_consecutive_s0_incr) return PM_r0i; 
			}

			if (array[pointer] >= 65535) return PM_mm; 
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good;
}

static nat execute_graph(void) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = execute_graph_starting_at(o);   
		if (not pm) return z_is_good;
	}
	return pm;
}

static bool fea_execute_graph_starting_at(byte origin) {

	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 

	byte ip = origin, last_mcal_op = 0;
	nat pointer = 0, e = 0, xw = 0;

	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return PM_fea;
			if (not array[pointer]) return PM_ns0;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return PM_pco;
			if (not pointer) return PM_zr5;
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return PM_zr6;
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return PM_ndi;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good; 
}

static bool fea_execute_graph(void) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = fea_execute_graph_starting_at(o);
		if (not pm) return z_is_good;
	}
	return pm;
}

static void append_to_file(char* filename, size_t size) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z); 
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;

try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("[%s]: [z=%s]: failed to create filename = \"%s\"\n", dt, z, filename);
			fflush(stdout);
			abort();
		}
		snprintf(filename, size, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, z, strlen(z));
	write(file, " ", 1);
	write(file, dt, strlen(dt));
	write(file, "\n", 1);

	close(file);

	printf("[%s]: write: %s z = %s to file \"%s\"\n",                // do this in the workerthread.
		dt, permissions ? "created" : "wrote", z, filename
	);
}

static nat print_lifetime(byte origin, nat print_count, nat er_count, nat pre_run_count) {
	nat n = array_size;
	bool* modes = calloc(n + 1, sizeof(bool));

	nat er = 0, E = 0, Eer = 0;
	nat pointer = 0;
	byte ip = origin;
	if (print_count) puts("[starting lifetime...]");
	nat e = 0;

	for (; e < print_count + pre_run_count; e++) {

		const byte I = ip * 4, op = graph[I];
		if (op == one) { 
			if (pointer == n) { puts("fea pointer overflow"); abort(); } 
			pointer++; 

		} else if (op == five) {

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
	free(modes);
	return Eer;
}

static nat generate_image_for_lifetime(byte origin, nat print_count, nat pre_run_count) {

	const int dir = open(image_directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", image_directory); 
		return 2; 
	}
	int flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
	mode_t m  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	
	char filename[4096] = {0};
	char string[64] = {0};
	get_graphs_z_value(string);
	snprintf(filename, sizeof filename, "%s_%u.ppm", string, origin);
	printf("created ppms filename = \"%s\"\n", filename);

	const int file = openat(dir, filename, flags, m);
	if (file < 0) {
		perror("create openat file");
		printf("filename=%s ", filename);
		close(dir); 
		return 1;
	}

	nat n = array_size;
	bool* modes = calloc(array_size + 1, sizeof(bool));

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
	printf("filename=%s\n", filename);
	close(dir); 

	return 0;
}

static void generate_images(struct zlist list) {

	printf("generate: generating %llu lifetimes .ppm's...\n", list.count);
	if (mkdir("images/", 0777)) { perror("mkdir"); getchar(); }
	
	for (nat i = 0; i < list.count; i++) {
		for (byte o = 0; o < graph_count; o += 4) {

			if (list.values[i][o + 0] != 2) continue;

			set_graph(list.values[i]);
			if (not generate_image_for_lifetime(o / 4, execution_limit, pre_run_ins)) continue;

			printf("warning: image for z value already exists... do you wish to continue? ");
			getchar();
		}
	}
	printf("generated %llu images in %s.\n", list.count, image_directory);
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

static nat similarity_count(byte* a, byte* b) {
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
	

	for (nat i = 0; i < hcount; i++) {

		puts("------------------------");
		printf("[ printing hlist %llu / %llu ]\n", i, hcount);
		print_zlist("hlist", i, hlist[i]);
		puts("------------------------");
		puts("");

		printf("going over lifetimes for all z values within this hlist...\n");
		printf("\n-----!#!#!#!#!#!#!#!#-----"); fflush(stdout);

		for (nat z = 0; z < hlist[i].count; z++) {

			for (nat o = 0; o < graph_count; o += 4) {
				if (hlist[i].values[z][o + 0] == 2)  {
					puts("-----------------------------------------------------------");
					print_bytes(hlist[i].values[z], graph_count); puts("");
					set_graph(hlist[i].values[z]);
					print_graph_as_adj();
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



static bool graph_analysis(void) {

	u16 was_utilized = 0;
	nat a = 0;
	byte previous_op = graph[20];     // make this not use this temporary variable, by using   index and index + 4   
	for (byte index = 20; index < graph_count; index += 4) {// (except if index+4==graphcount, then we will  just say its index.. yeah)
		const byte op = graph[index];
		if (previous_op > op) { a = index; goto bad; }
		previous_op = op;
	}
	// constructive GA here

	for (byte index = operation_count; index--;) {
		if (graph[4 * index + 3] == index) {  a = 4 * index + 3; goto bad; }
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  a = 4 * index; goto bad; } 			     
		if (graph[4 * index] == six   and graph[4 * index + 2])          {  a = 4 * index; goto bad; }
		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  a = 4 * index + 2 * (index == two); goto bad; }  
		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  a = 4 * index + 1 * (index == three); goto bad; }

		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == one) {
			if (index == six) { a = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == one) { a = 4 * index; goto bad; }
			a = 4 * (index < tohere ? index : tohere); goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == five) {
			if (index == six) { a = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == five) { a = 4 * index; goto bad; }
			a = 4 * (index < tohere ? index : tohere); goto bad; 
		}
 
		const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

		if (graph[4 * index] == one and graph[4 * e] == one) {
			if (index == one) { a = 4 * index + 3; goto bad; }
			if (e == one) { a = 4 * index; goto bad; }
			a = 4 * (index < e ? index : e); goto bad;
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == one) {
			if (index == five) { a = 4 * index + 1; goto bad; } 
			if (e == one) { a = 4 * index; goto bad; }
			a = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == three) {
			if (index == five) { a = 4 * index + 1; goto bad; } 
			if (e == three) { a = 4 * index; goto bad; }
			a = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == one)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == one) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { a = 4 * index; goto bad; }
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}
		
		if (graph[4 * index] == five) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == five) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { a = 4 * index; goto bad; } 
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == six) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) { 
					if (index == six) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { a = 4 * index; goto bad; } 
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == three) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == three) { 
					if (index == three) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == three) { a = 4 * index; goto bad; } 
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}
		
		if (l != index) was_utilized |= 1 << l;
		if (g != index) was_utilized |= 1 << g;
		if (e != index) was_utilized |= 1 << e;
	}

	for (byte index = 0; index < operation_count; index++) 
		if (not ((was_utilized >> index) & 1)) goto bad;

	return false;
bad:	return true;
}

static nat graph_was_pruned_by(void) {
	nat pm = graph_analysis();     if (pm) return pm;
	    pm = fea_execute_graph();  if (pm) return pm;
	    pm = execute_graph();      return pm;
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
		else printf("ERROR: the group had no members!!! thus, synthesize_graph_over_one_group(group) was not called on it. \n");

		getchar();
		//partition_into_minor_groups(group);

		group.count = 0;
	}
	puts("[finished all sythesized graphs over all groups.]");
}

static void count_dup_types(struct zlist list) {

	const nat max_group_count = expn(5, D);
	nat* counts = calloc(max_group_count, sizeof(nat));
	nat** groups = calloc(max_group_count, sizeof(nat*));

	for (nat i = 0; i < list.count; i++) {
		
	}
}




static void visualize(char* string) {

	init_graph_from_string(string);

	char buffer[128] = {0};
	printf("give the origin: ");
	fgets(buffer, sizeof buffer, stdin);
	byte o = (byte) atoi(buffer);
	print_lifetime(o, execution_limit, row_count, pre_run_ins);
	printf("\nz = %.*s\n", (int) graph_count, string);
	print_bytes(graph, graph_count); 
	print_graph_as_adj();
}

static void visualize_list(struct zlist list) {

	/*


			202403225.181528:

			TODO:    add   the new visualization method in addition to this binary lifetime one


				where we actually print out a certain number of "#" characters per line, equal to the cell value, 

					line zero is representing *0,  line one, *1, etc,    the "#" count   is the cell value,  ie  like a histogram kinda. 


						ie, visualizing the array state in cardinal unary!!! so useful. 

							also, print a fixedwidth integer at the beginning of the array, as well, easy little thing 

	*/

	struct termios terminal;
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_lflag &= ~((size_t) ECHO | ICANON);
	tcsetattr(0, TCSAFLUSH, &copy);

	for (nat z = 0; z < list.count; z++) {

		// bool written = false;

		for (byte o = 0; o < graph_count; o += 4) {
			
			if (list.values[z][o + 0] != 2) continue;

			nat offset = pre_run_ins;
		print:;
			printf("\033[H\033[2J");
			puts(""); puts("");
			for (nat i = 0; i < z * 5; i++) putchar('@');
			for (nat i = 0; i < (list.count - z) * 5; i++) putchar('.');
			puts(""); puts("");

			set_graph(list.values[z]);
			const nat e = print_lifetime(o / 4, execution_limit, row_count, offset);
			print_bytes(graph, graph_count); 
			puts(""); puts("");
			for (nat t = 0; t < o / 4; t++) {
				for (nat i = 0; i < 300; i++) printf("%u", o / 4);
				puts("");
			}

			print_graph_as_adj(); puts("");
			printf(":ready: ");
			fflush(stdout);

			int c = getchar();
			if (c == 10) goto print; 
			else if (c == ' ') {  offset += e; goto print; } 
			else if (c == '\t') {  if (offset > e) offset -= e; goto print; } 
			else if (c == 'q') goto return_;
			else if (c == '[') {  if (z) { z -= 2; continue; } else goto print; } 
			else if (c == ']' or c == 't') continue;
			else {
				puts("input error"); 
				getchar();
				goto print; 
			}
		}
	}
return_:
	tcsetattr(0, TCSAFLUSH, &terminal);
}

static void machine_prune(struct zlist list) { //  const char* previous_filename, 

	char filename[4096] = {0};
	nat good = 0, bad = 0;
	for (nat z = 0; z < list.count; z++) {
		printf("\r trying z = ");
		set_graph(list.values[z]);
		print_graph();
		nat pm = graph_was_pruned_by();
		const char* color = pm ? red : green;
		const char* type =  pm ? "BAD" : "GOOD";
		printf(bold "%s ---> %s (%llu / %llu) -- ( via %s )" reset "\n", color, type, z, list.count, pm_spelling[pm]);
		if (pm) bad++; else { append_to_file(filename, list.count); good++; } 
		pm_counts[pm]++;
	}
	print_counts();
	printf("\n\n\t\t\tgood: %llu\n\t\t\tbad: %llu\n\n\n", good, bad);
}

static void print_help(void) {
	printf("available commands:\n"
		"\t quit \n"
		"\t clear \n"
		"\t help \n"
		"\t list \n"
		"\t viz \n"
		"\t viz list \n"
		"\t generate images \n"
		"\t synthesize graph \n"
		"\t machine prune \n"
		"\t\n"
	);
}

int main(int argc, const char** argv) {

	// compiletime computation:
	srand((unsigned)time(0)); rand();
	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	
	

	array = calloc(array_size + 1, sizeof(nat));
	timeout = calloc(operation_count, sizeof(nat));
	graph = calloc(graph_count, 1);

	// runtime computation:
	printf("using [D=%hhu]: spacesize=%llu\n", D, space_size);

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
		if (graph_count != strlen(buffer)) { puts("graph count or dupilcation_count mismatch!"); abort(); }
		init_graph_from_string(buffer);
		byte* g = calloc(graph_count, 1);
		memcpy(g, graph, graph_count);
		zlist.values = realloc(zlist.values, sizeof(nat*) * (zlist.count + 1));
		zlist.values[zlist.count++] = g;

	}
	fclose(file);

	char input[4096] = {0};
loop:
	printf(":%llu: ", zlist.count);
	fgets(input, sizeof input, stdin);

	if (not strcmp(input, "\n")) {}
	else if (not strcmp(input, "quit\n")) exit(0);
	else if (not strcmp(input, "clear\n")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help\n")) print_help();

	else if (not strcmp(input, "list\n")) print_zlist("current z list", 0, zlist);
	else if (not strncmp(input, "viz ", 10)) visualize(input + 10);
	else if (not strcmp(input, "viz list\n")) visualize_list(zlist);
	else if (not strcmp(input, "generate images\n")) generate_images(zlist);

	else if (not strcmp(input, "synthesize graph\n")) find_major_groups(zlist);
	else if (not strcmp(input, "count_dup_types\n")) count_dup_types(zlist);

	else if (not strcmp(input, "machine prune\n")) machine_prune(zlist); // argv[1]

	else printf("unknown command %s\n", input);
	goto loop;
}









































/*


else if (c == 'a') { 
				if (not written) { 
					written = true; 
					write_to_file(list.values[z], previous_filename, list.count);
					puts(bold green "------- Z VALUE WRITTEN TO GOOD LIST -------" reset);
					continue;
			
				} else { puts("write error"); getchar(); goto print; }
			} 



*/






/*


static const byte _ = 0;

static const byte _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};

#define _63R_hole_count 9
static const byte _63R_hole_positions[_63R_hole_count] = {3, 6, 7, 10, 11, 13, 14, 15, 19};

static const byte _36R[5 * 4] = {
	0,  1, 2, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  _, 4, _,      //  9    11
	3,  _, _, _,      // 13 14 15
	4,  0, 0, _,      //       19
};


#define _36R_hole_count 9
static const byte _36R_hole_positions[_36R_hole_count] = {3, 6, 7, 9, 11, 13, 14, 15, 19};

//static const byte initial = R ? _63R_hole_count : _36R_hole_count;


static const byte hole_count = initial + 4 * D;


positions = calloc(hole_count, 1);


for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 


static byte* positions = NULL; 





// static const bool R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)





*/







