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



// todo:                      reset the pruning metric counts!


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

static const nat stage1_execution_limit = 4000000;
static const nat execution_limit = stage1_execution_limit;
static const nat array_size = 100000;
static const nat pre_run_ins = 0;

enum pruning_metrics {
	z_is_good, pm_ga, 
	pm_fea, pm_ns0, 

	pm_pco, pm_zr5, 
	pm_zr6, pm_ndi, 

	pm_oer, pm_r0i, 
	pm_h0, pm_f1e, 

	pm_erc, pm_rmv, 
	pm_csm, 

	pm_bdl, pm_bdl2, 
	pm_erw, pm_mcal, 
	
	pm_h2, pm_h3, 
	pm_per, pm_snco,

	pm_r1i,
	pm_pt,

	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good", "pm_ga", 
	"pm_fea", "pm_ns0", 

	"pm_pco", "pm_zr5", 
	"pm_zr6", "pm_ndi", 

	"pm_oer", "pm_r0i", 
	"pm_h0", "pm_f1e", 

	"pm_erc", "pm_rmv", 
	"pm_csm", 

	"pm_bdl", "pm_bdl2", 
	"pm_erw", "pm_mcal", 
	
	"pm_h2", "pm_h3", 
	"pm_per", "pm_snco",

	 "pm_r1i",
	"pm_pt",
};


static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const byte max_er_repetions = 50;
static const byte max_erw_count = 100;
static const byte max_modnat_repetions = 15;
static const byte max_consecutive_s0_incr = 30;
static const byte max_consecutive_s1_incr = 30;
static const byte max_consecutive_small_modnats = 200;
static const byte max_bdl_er_repetions = 25;

static const byte max_consecutive_h0_bouts = 10;
static const byte max_consecutive_h2_bouts = 30;
static const byte max_consecutive_h3_bouts = 30;

static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;

static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;


// static nat space_size = 0;


static const nat row_count = 110;
static const nat paging_row_count = 100;
static const nat window_begin = 0;
static const nat window_end = 10000000000;
static const nat timestep_delay = 80;

static nat pm_counts[pm_count] = {0};

struct zlist {
	byte** values;
	nat count;
};

static void set_graph(byte* graph, byte* z) { memcpy(graph, z, graph_count); }

/*static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}*/

static void init_graph_from_string(byte* graph, const char* string) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%lld ", v[i]);
	}
	printf("]");
}

static void print_bytes(byte* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2hhu ", v[i]);
	printf("] \n");
}

static void print_graph(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

static void get_graphs_z_value(byte* graph, char string[64]) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void print_zlist(byte* graph, const char* s, nat d, struct zlist list) {
	printf("%s . %llu . zlist (%llu){\n", s, d, list.count);
	for (nat k = 0; k < list.count; k++) {
		putchar(9);
		set_graph(graph, list.values[k]);
		print_graph(graph);
	}
	printf("}\n");
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

static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < pm_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], pm_counts[i]);
	}
	puts("");
	puts("[done]");
}



static void debug_pm(byte op, byte origin, nat e, nat* history, nat pm) {

	if (op == three or op == one or op == five) {
		memmove(history, history + 1, sizeof(nat) * 99);
		history[99] = op;
	}

	printf("\n\033[32;1m at origin = %hhu: \n[PRUNED GRAPH VIA <%s> AT %llu]:\033[0m\n", origin, pm_spelling[pm], e);
	puts("[LIFETIME-START]"); 

	for (nat i = 0; i < 100; i++) {
		// if (history[i] == (nat) -1) {}
		if (history[i] == one) { 
			if (i and (history[i - 1] == one or history[i - 1] == five)) { 
				printf("."); 
				fflush(stdout); 
			} 
		} 
		else if (history[i] == five) { printf("\n"); fflush(stdout); } 
		else if (history[i] == three) { printf("#"); fflush(stdout); } 
	}
	getchar();
}



static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }





static nat execute_graph_starting_at(byte origin, byte* graph, nat* array) {


	nat history[100] = {0};
	memset(history, 255, sizeof history);



	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	e = 0,  xw = 0,  pointer = 0,  
		er_count = 0, 
		mcal_index = 0, 
		bout_length = 0, 
		walk_ia_counter = 0, 
		RMV_value = 0, 
		OER_er_at = 0,
		BDL_er_at = 0,
		BDL2_er_at = 0,
		pointer_incr_timeout = 0;

	byte 	mcal_path = 0, R1I_counter = 0,
		ERW_counter = 0, OER_counter = 0,  BDL_counter = 0,  
		BDL2_counter = 0,  R0I_counter = 0, 
		H0_counter = 0, H2_counter = 0, H3_counter = 0, 
		RMV_counter = 0, CSM_counter = 0;

	byte ip = origin;
	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

	for (; e < stage1_execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return pm_f1e;
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return pm_erc;
		}
		
		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return pm_fea;
			if (not array[pointer]) return pm_ns0; 

			if (last_mcal_op == one)  H0_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;
			
			if (pointer == 1) {
				if (last_mcal_op == three) {
					R1I_counter++;
					if (R1I_counter >= max_consecutive_s1_incr) return pm_r1i;
				} else R1I_counter = 0;
			}

			bout_length++;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}

			pointer_incr_timeout = 0;
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_er_repetions) return pm_oer;

			if (BDL_er_at and pointer == BDL_er_at - 1) { BDL_counter++; BDL_er_at--; }
			else { BDL_er_at = pointer; BDL_counter = 0; }
			if (BDL_counter >= max_bdl_er_repetions)  return pm_bdl;

			if (BDL2_er_at > 1 and pointer == BDL2_er_at - 2) { BDL2_counter++; BDL2_er_at -= 2; }
			else { BDL2_er_at = pointer; BDL2_counter = 0; }
			if (BDL2_counter >= max_bdl_er_repetions) return pm_bdl2;

			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_modnat_repetions) return pm_rmv;
			}

			if (walk_ia_counter == 1) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			did_ier_at = pointer;
			walk_ia_counter = 0;
			er_count++;
			pointer = 0;
		}

		else if (op == two) {

			
			if (pointer_incr_timeout >= stage1_execution_limit >> 2) return pm_pt;
			else pointer_incr_timeout++;


			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6;
			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pm_snco;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three)  return pm_ndi;

			if (last_mcal_op == five) {
				R0I_counter++;
				if (R0I_counter >= max_consecutive_s0_incr) return pm_r0i; 
			}

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (bout_length == 2) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (bout_length == 3) {
				H3_counter++;
				if (H3_counter >= max_consecutive_h3_bouts) return pm_h3; 
			} else H3_counter = 0;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

			bout_length = 0;
			walk_ia_counter++;
			array[pointer]++;
		}

		// if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		if (op == three or op == one or op == five) { 
			last_mcal_op = op; mcal_index++; 

			memmove(history, history + 1, sizeof(nat) * 99);
			history[99] = op;
		}


		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	return pm_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pm_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pm_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pm_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pm_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pm_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pm_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pm_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pm_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good;
}

static nat execute_graph(byte* graph, nat* array, byte* origin, nat* pms) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		const nat pm = execute_graph_starting_at(o, graph, array);
		printf("execute_graph_starting_at:   origin = %hhu\n", o);
		printf("pm = %s\n", pm_spelling[pm]);
		printf("\n   at origin o = %hhu, z = ", o); 
		print_graph_raw(graph); 
		printf("\n"); 
		fflush(stdout);

		pms[pm]++;
		if (not pm) { *origin = o; return z_is_good; } 
	}
	return 1;
}


static void append_to_file(byte* graph, char* filename) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(graph, z); 
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
		snprintf(filename, 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
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

static nat print_lifetime(byte* graph, nat* array, byte origin, nat print_count, nat er_count, nat pre_run_count) {
	nat n = array_size;
	bool* modes = calloc(n + 1, sizeof(bool));

	nat er = 0, E = 0, Eer = 0;
	nat pointer = 0;
	byte ip = origin;
	if (print_count) puts("[starting lifetime...]");
	nat e = 0;

	for (; e < print_count + pre_run_count; e++) {

		const byte I = ip * 4, op = graph[I];

		//printf("op = %hhu\n", op); getchar();
		//if (op == two) { putchar('.'); fflush(stdout); } 
		//else if (op == six) { putchar('/'); fflush(stdout); } 
		//else {  } 


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




static void synthesize_graph_over_one_group(struct zlist zlist) {

	nat* tallys = calloc((size_t) (graph_count * operation_count), sizeof(nat));

	for (nat z = 0; z < zlist.count; z++) {
		for (nat i = 0; i < graph_count; i++) {
			tallys[i * operation_count + zlist.values[z][i]]++;
		}
	}

	printf("synthesized graph [over %llu z values]:\n", zlist.count);

	for (byte i = 0; i < graph_count; i += 4) {
		printf("  " red "#%u" reset "  :: { \n", i / 4);

		printf("\t\t.op={ ");
		for (nat o = 0; o < operation_count; o++) {
			const nat count = tallys[(i + 0) * operation_count + o];
			if (count) printf(" ->%llu : %s%.2lf\033[0m[%llu]", o, count == zlist.count ?  green : yellow, (double) count / zlist.count, count); 
		}
		printf(" }, \n");
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




static nat graph_was_pruned_by(byte* graph, nat* array, nat* pms) {

	//nat pm = fea_execute_graph(graph, array);
	//if (pm) return pm;

	//puts("graph_was_pruned_by: graph got past the fea_execute_stage....");

	byte origin;
	nat pm = execute_graph(graph, array, &origin, pms);
	
	return pm;
}

static void find_major_groups(byte* graph, struct zlist list) {

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
		print_zlist(graph, "major group", i, group);

		printf("%llu op is new:   performing synthesize graph over MAJOR GROUP %llu z values:\n", i, group.count); 

		if (group.count) synthesize_graph_over_one_group(group);
		else printf("ERROR: the group had no members!!! thus, synthesize_graph_over_one_group(group) was not called on it. \n");

		getchar();
		//partition_into_minor_groups(group);

		group.count = 0;
	}
	puts("[finished all sythesized graphs over all groups.]");
}


static void visualize(byte* graph, nat* array, char* string) {

	init_graph_from_string(graph, string);

	char buffer[128] = {0};
	printf("give the origin: ");
	fgets(buffer, sizeof buffer, stdin);
	byte o = (byte) atoi(buffer);
	print_lifetime(graph, array, o, execution_limit, row_count, pre_run_ins);
	printf("\nz = %.*s\n", (int) graph_count, string);
	print_bytes(graph, graph_count); 
	print_graph_as_adj(graph);
}

static void machine_prune(byte* graph, nat* array, struct zlist list) { //  const char* previous_filename, 

	char filename[4096] = {0};
	nat good = 0, bad = 0;
	for (nat z = 0; z < list.count; z++) {
		printf("\r trying z = ");
		set_graph(graph, list.values[z]);
		print_graph(graph);
		nat pm = graph_was_pruned_by(graph, array, pm_counts);
		const char* color = pm ? red : green;
		const char* type =  pm ? "BAD" : "GOOD";
		printf(bold "%s ---> %s (%llu / %llu) -- ( via %s )" reset "\n", color, type, z, list.count, pm_spelling[pm]);
		if (pm) bad++; else { append_to_file(graph, filename); good++; } 
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
		"\t synthesize graph (sg)\n"
		"\t machine prune (m)\n"
		"\t\n"
	);
}

// "\t viz list \n"
		// "\t generate images \n"



int main(int argc, const char** argv) {

	// compiletime computation:
	srand((unsigned)time(0)); rand();
	// space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);

	nat* array = calloc(array_size + 1, sizeof(nat));
	byte* graph = calloc(graph_count, 1);

	// runtime computation:
	// printf("using [D=%hhu]: spacesize=%llu\n", D, space_size);

	if (argc <= 1) return puts("give input z list filename as an argument!");

	FILE* file = fopen(argv[1], "r");
	if (not file) { perror("fopen"); exit(1); }
	
	struct zlist zlist = {0};

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {
		char* index = strchr(buffer, ' ');
		if (not index) abort();
		buffer[index - buffer] = 0;
		if (graph_count != strlen(buffer)) { puts("graph count or dupilcation_count mismatch!"); abort(); }
		init_graph_from_string(graph, buffer);
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
	else if (not strcmp(input, "exit\n")) exit(0);
	else if (not strcmp(input, "q\n")) exit(0);
	else if (not strcmp(input, "x\n")) exit(0);
	else if (not strcmp(input, "clear\n")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help\n")) print_help();
	else if (not strcmp(input, "list\n")) print_zlist(graph, "current z list", 0, zlist);
	else if (not strncmp(input, "viz ", 4)) visualize(graph, array, input + 4);
	else if (not strcmp(input, "synthesize graph\n") or not strcmp(input, "sg\n")) find_major_groups(graph, zlist);
	else if (not strcmp(input, "machine prune\n") or not strcmp(input, "m\n")) machine_prune(graph, array, zlist);
	else printf("unknown command %s\n", input);
	goto loop;
}






//else if (not strcmp(input, "viz list\n")) visualize_list(zlist);
// else if (not strcmp(input, "generate images\n")) generate_images(zlist);
	//else if (not strcmp(input, "count_dup_types\n")) count_dup_types(zlist);  //todo: redo synthesize graph when we go to 3 space.














/*

static char image_directory[] = "./images/";
// static const nat image_size = 600;





static nat generate_image_for_lifetime(byte* graph, nat* array, byte origin, nat print_count, nat pre_run_count) {

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
	get_graphs_z_value(graph, string);
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

static void generate_images(byte* graph, struct zlist list) {

	printf("generate: generating %llu lifetimes .ppm's...\n", list.count);
	if (mkdir("images/", 0777)) { perror("mkdir"); getchar(); }
	
	for (nat i = 0; i < list.count; i++) {
		for (byte o = 0; o < graph_count; o += 4) {

			if (list.values[i][o + 0] != 2) continue;

			set_graph(graph, list.values[i]);
			if (not generate_image_for_lifetime(o / 4, execution_limit, pre_run_ins)) continue;

			printf("warning: image for z value already exists... do you wish to continue? ");
			getchar();
		}
	}
	printf("generated %llu images in %s.\n", list.count, image_directory);
}*/





































/*
static void count_dup_types(struct zlist list) {

	const nat max_group_count = expn(5, D);
	nat* counts = calloc(max_group_count, sizeof(nat));
	nat** groups = calloc(max_group_count, sizeof(nat*));

	for (nat i = 0; i < list.count; i++) {
		
	}
}
*/








/*

















			202403225.181528:

			TODO:    add   the new visualization method in addition to this binary lifetime one


				where we actually print out a certain number of "#" characters per line, equal to the cell value, 

					line zero is representing *0,  line one, *1, etc,    the "#" count   is the cell value,  ie  like a histogram kinda. 


						ie, visualizing the array state in cardinal unary!!! so useful. 

							also, print a fixedwidth integer at the beginning of the array, as well, easy little thing 

	





static void visualize_list(struct zlist list) {

	

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














Dedup:   14

--------------

* 0121102521433102400102624001
* 0122102521433062400106614201


0  1  2  1  1  0  4  5  2  0  4  3  3  6  0  2  4  0  0  6   0  1  6  6   1  1  2  4 
 
0  1  2  1  1  0  4  5  2  0  4  3  3  1  1  2  4  0  0  6   0  1  6  6   1  1  2  4 

0  1  2  1  1  0  4  5  2  0  4  3  3  6  1  2  4  0  0  6   0  1  6  6   1  1  2  4 

0  1  2  1  1  0  4  5  2  0  4  3  3  1  6  2  4  0  0  6   0  1  6  6   1  1  2  4 

0  1  2  1  1  0  4  5  2  0  4  3  3  6  6  2  4  0  0  6   0  1  6  6   1  1  2  4 

0  1  4  6  1  0  6  5  2  0  0  3  3  0  1  1  4  2  0  6   0  1  6  6   1  1  2  4 

0  1  4  6  1  0  6  5  2  0  0  3  3  0  4  1  4  2  0  6   0  1  6  6   1  1  2  4 

0  1  4  6  1  0  3  5  2  0  0  1  3  5  6  4  4  2  0  6   0  1  6  6   1  1  2  4 




0  1  4  6  1  0  2  5  2  0  3  3  3  0  4  0  4  2  0  6   0  4  1  1   1  4  1  4 

0  1  4  6  1  0  2  5  2  0  3  0  3  1  4  0  4  2  0  6   0  4  1  1   1  4  1  4 
 
0  1  4  6  1  0  2  5  2  0  3  0  3  0  4  1  4  2  0  6   0  4  1  1   1  4  1  4 




0  1  4  6  1  0  6  5  2  0  0  3  3  0  1  1  4  2  0  6   0  6  1  6   1  2  2  4 





ip    6:
---------------


012110452043360240060166 1124
012110452043361240060166 1124
012110452043364240060166 1124
012110452043366240060166 1124

01461065200330114206 0616 1224
01461065200330614206 0616 1224






*/























/*


202405013.192700:

	for (nat i = 0; i < graph_count; i++) {

		if (not (i & 3)) continue;

		if (edge_was_executed & (1 << i)) continue;

		if (graph[i]) {      // if we can think of a CORRECT way to canonicalize the z list w.r.t empirical deadstops,
						// then we can have EDA. until then, no we can't.  its wrong. 

					// we can't require   0   to be located at a deadstop          willy nilly,   because GA CARES ABOUT THAT

														A LOT




			printf("edge_was_executed = %llx\n", edge_was_executed);
			printf("found an EDA error in   graph = ");
			print_graph();
			puts("");
			printf("the error was that graph[%llu] was found to be nonzero, (value %hhu).", i, graph[i]);

			return PM_eda;
		}
	}


						this code prunes          0122102521433062400106614201 1202403155.232327     

									   when it shouldnt. 

							this graph is fine, perfectly good,    it was just constrained by GA    when in search


							to have    1       at position   [19]    which is not executed in this graph due to an emergent deadstop, but shouldnt be forced to be 0, becuase GA pruned that graph that had that.











 0122102521433062400106614201 1202403155.232327  

	















        [   main  ]   [  x  ]               x := extra
	3 1 3 5 3 1   3 1 3 [5]   <--- deleted  the thing in brackets

        [   main  ]   [ x ]
	3 1 3 5 3 1   1 3 5 [X]   <--- deleted  the thing in brackets









const nat mcal_string0_length = 10;
		const byte mcal_string0[] = { 255, three, one, three, five, three, one,   three, one, three, five };

		const nat mcal_string1_length = 9;
		const byte mcal_string1[] = { 255, three, one, three, five, three, one,   one, three, five };


		if (	mcal_index < mcal_string0_length and 
			last_mcal_op != mcal_string0[mcal_index]

			and 

			mcal_index < mcal_string1_length and 
			last_mcal_op != mcal_string1[mcal_index]

			) return PM_mcal;

		





		if (mcal_index == 7  and last_mcal_op != x) return PM_mcal;
		if (mcal_index == 8  and last_mcal_op != x) return PM_mcal;
		if (mcal_index == 9  and last_mcal_op != x) return PM_mcal;
		if (mcal_index == 10 and last_mcal_op != x) return PM_mcal;
		

		if (mcal_index == 7  and last_mcal_op != x) 	return PM_mcal;
		if (mcal_index == 8  and last_mcal_op != x) return PM_mcal;
		if (mcal_index == 9  and last_mcal_op != x) return PM_mcal;
		






// printf("[MCAL index = %llu]: expected %hhu, "); 




*/













// edge_was_executed |= 1 << (I + state);



	//bool debug = false;

	//char z[64] = {0};    get_graphs_z_value(z); 

	/*if (not strcmp(z, "0122102025433510400605662103")) {

		puts("testing this z value in particular...");
		getchar();

		//if (origin == 6) { puts("origin 6 "); abort(); } 
		//else { printf("origin OTHER = %hhu\n", origin); abort();  } 

		printf("has origin = %hhu\n", origin);
		debug = true;
	}*/


		/*for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return PM_ot; 
			timeout[i]++;
		}
		timeout[ip] = 0;*/









	//memset(timeout, 0, operation_count * sizeof(nat));













/*




// if (debug)  { printf("[DID_IER!]"); fflush(stdout); } 




PER:
========



  [*]  [ ]  [*]  [ ]  [ ]  [ ]  [*]  [ ]  [(3,5)IER]

  [ ]  [ ]  [ ]  [ ]  [ ]  [ ]  [ ]  [ ]  [(3;1)IA]    [(3;5)IER]






when we say 3,
we keep track of the cell index we did it at


the then say 5,   set a flag that we had said 5,

and if we say 3 again, then we check the index we had said 3 at   
the second time 

and if the given new index is greater (>=!!!! this means 
	PER and ANDI are the same PM!!) 

	than the previous index   then we prune it 



		ie, it needs to be less than!!!









*/




/*




static nat graph_analysis(byte* graph) {

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

	if (a) a = 0;   // for now lol.   a is only used in search lololol.


	for (byte index = 0; index < operation_count; index++) 
		if (not ((was_utilized >> index) & 1)) goto bad;

	return z_is_good;
bad:	return PM_ga;
}






      -       202404195.164643:  redo this sg stuff completely, when we go to 3 space! 





// major/minor groups
static const nat similarity_threshold = 22;


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

		byte* local_graph = total.values[z];
		for (nat hi = 0; hi < hcount; hi++) {
			for (nat i = 0; i < hlist[hi].count; i++) {
				if (similarity_count(local_graph, hlist[hi].values[i]) < similarity_threshold) goto next_minor_group;
			}
			hlist[hi].values[hlist[hi].count++] = local_graph;
			goto next_graph;
			next_minor_group: continue;
		}
	
		struct zlist new = {
			.values = calloc(total.count, sizeof(nat*)),
			.count = 0
		};

		new.values[new.count++] = local_graph;
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
























static nat execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	
	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	e = 0,  xw = 0,  pointer = 0,  
		er_count = 0, 
		mcal_index = 0, 
		bout_length = 0, 
		walk_ia_counter = 0, 
		RMV_value = 0, 
		OER_er_at = 0,
		BDL_er_at = 0,
		BDL2_er_at = 0;

	byte 	mcal_path = 0,
		ERW_counter = 0, SNL_counter = 0,  OER_counter = 0,  BDL_counter = 0, 
		BDL2_counter = 0,  R0I_counter = 0, 
		H0_counter = 0, H2_counter = 0, H3_counter = 0, 
		RMV_counter = 0, CSM_counter = 0;

	byte ip = origin;
	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;


	nat history[100] = {0};
	memset(history, 255, sizeof history);


	for (; e < execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return pm_f1e; 
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return pm_erc; 
		}
		
		const byte I = ip * 4, op = graph[I];



		if (op == one) {
			if (pointer == n) return pm_fea; 
			if (not array[pointer]) return pm_ns0; 

			if (last_mcal_op == one)  H0_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;

			bout_length++;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco; 
			if (not pointer) return pm_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_er_repetions) return pm_oer; 

			if (BDL_er_at and pointer == BDL_er_at - 1) { BDL_counter++; BDL_er_at--; }
			else { BDL_er_at = pointer; BDL_counter = 0; }
			if (BDL_counter >= max_bdl_er_repetions) return pm_bdl; 

			if (BDL2_er_at > 1 and pointer == BDL2_er_at - 2) { BDL2_counter++; BDL2_er_at -= 2; }
			else { BDL2_er_at = pointer; BDL2_counter = 0; }
			if (BDL2_counter >= max_bdl_er_repetions) return pm_bdl2; 

			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm; 
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_modnat_repetions) return pm_rmv; 
			}

			if (walk_ia_counter == 1) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			did_ier_at = pointer;
			walk_ia_counter = 0;
			er_count++;
			pointer = 0;
		}

		else if (op == two) {
			if (array[n] >= 65535) return pm_snm; 

			if (last_op == six) SNL_counter++; 
			else if (last_op != two) SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) { 
				return pm_zr6;  
			}

			if (last_op == two) SNL_counter++; 
			else SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three) return pm_ndi; 

			if (last_mcal_op == five) {
				R0I_counter++; 
				if (R0I_counter >= max_consecutive_s0_incr) return pm_r0i; 
			}

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (bout_length == 2) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (bout_length == 3) {
				H3_counter++;
				if (H3_counter >= max_consecutive_h3_bouts) return pm_h3; 
			} else H3_counter = 0;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per;
				did_ier_at = (nat) ~0;
			}

			bout_length = 0;
			walk_ia_counter++;

			if (array[pointer] >= 65535) return pm_mm; 
			array[pointer]++;
		}

		//if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }

		if (op == three or op == one or op == five) { 
			last_mcal_op = op; mcal_index++; 

			memmove(history, history + 1, sizeof(nat) * 99);
			history[99] = op;
		}


		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 3  and last_mcal_op != three) goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 5  and last_mcal_op != three) goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	goto prune_via_mcal; // return PM_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) goto prune_via_mcal; // return PM_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	goto prune_via_mcal; // return PM_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	goto prune_via_mcal; // return PM_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	goto prune_via_mcal; // return PM_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	goto prune_via_mcal; // return PM_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		
		ip = graph[I + state];

		
		continue;
		
	prune_via_mcal:
		
		printf("\n------- MCAL PRUNING -------\n");
		fflush(stdout);
		printf("op = %hhu, mcal_index = %llu, mcal_path = %hhu, last_mcal_op = %hhu, last_op = %hhu\n",
			op, 	   mcal_index,        mcal_path,        last_mcal_op,        last_op
		);
		fflush(stdout);
		debug_pm(op, origin, e, history, pm_mcal);
		fflush(stdout);
		return pm_mcal;
	}
	return z_is_good;
}












static nat execute_graph(byte* graph, nat* array, byte* origin) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = execute_graph_starting_at(o, graph, array);   
		if (not pm) { *origin = o; return z_is_good; } 

		if (pm == pm_mcal) {
			printf("origin = %hhu\n", o);
			printf("FOUND A Z VALUE THAT GOT PRUNED BY PM MCAL!!!\n");
			printf("\n    FOUND THIS:  o = %hhu, z = ", o); 
			print_graph_raw(graph); 
			printf("\n"); 
			fflush(stdout);
		}

		else if (pm == pm_snl) {
			printf("origin = %hhu\n", o);
			printf("FOUND A Z VALUE THAT GOT PRUNED BY PM SNL!!!\n");
			printf("\n    FOUND THIS:  o = %hhu, z = ", o); 
			print_graph_raw(graph); 
			printf("\n"); 
			fflush(stdout);
		} else {


		}
	}
	return pm;
}
*/









/*static nat fea_execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 
	byte ip = origin, last_mcal_op = 255, mcal_path = 0;
	nat pointer = 0, e = 0, xw = 0, mcal_index = 0;
	nat did_ier_at = (nat)~0;

	nat history[100] = {0};
	memset(history, 255, sizeof history);


	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return pmf_fea;
			if (not array[pointer]) return pmf_ns0;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pmf_pco;
			if (not pointer) return pmf_zr5;

			did_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return pmf_zr6;
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return pmf_ndi;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pmf_per;
				did_ier_at = (nat) ~0;
			}

			array[pointer]++;
		}

		if (op == three or op == one or op == five) { 
			last_mcal_op = op; 
			mcal_index++; 

			memmove(history, history + 1, sizeof(nat) * 99);
			history[99] = op;
		}

		if (mcal_index == 1  and last_mcal_op != three) goto prune_via_mcal; 
		if (mcal_index == 2  and last_mcal_op != one) 	goto prune_via_mcal; 
		if (mcal_index == 3  and last_mcal_op != three) goto prune_via_mcal; 
		if (mcal_index == 4  and last_mcal_op != five) 	goto prune_via_mcal; 
		if (mcal_index == 5  and last_mcal_op != three) goto prune_via_mcal; 
		if (mcal_index == 6  and last_mcal_op != one) 	goto prune_via_mcal; 

		if (mcal_index == 7) {
			if (last_mcal_op == five) goto prune_via_mcal; 
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	goto prune_via_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	goto prune_via_mcal; // return pmf_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	goto prune_via_mcal; 
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	goto prune_via_mcal; 

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	goto prune_via_mcal; 

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];

		continue;


	prune_via_mcal:
		fflush(stdout);
		printf("\n------- PMF MCAL PRUNING -------\n");
		fflush(stdout);
		printf("op = %hhu, mcal_index = %llu, mcal_path = %hhu, last_mcal_op = %hhu\n",
			op, 	   mcal_index,        mcal_path,        last_mcal_op
		);
		fflush(stdout);
		debug_pm(op, origin, e, history, pmf_mcal);
		fflush(stdout);
		
		return pmf_mcal;
	}
	return z_is_good; 
}

static nat fea_execute_graph(byte* graph, nat* array) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = fea_execute_graph_starting_at(o, graph, array);
		if (not pm) return z_is_good;

		if (pm == pmf_mcal) {
			fflush(stdout);
			printf("origin = %hhu\n", o);
			printf("FOUND A Z VALUE THAT GOT PRUNED BY PM-FEA MCAL!!!\n");
			printf("\n    FOUND THIS:  o = %hhu, z = ", o); 
			print_graph_raw(graph); 
			printf("\n"); 
			fflush(stdout);
		}


	}
	return pm;
}



static const nat execution_limit = ;














static nat execute_graph_starting_at(byte origin, byte* graph, nat* array) {

printf("-------------------------------------execute_graph_starting_at(%hhu) {------------------------------------------\n", origin);
	fflush(stdout);


	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	e = 0,  xw = 0,  pointer = 0,  
		er_count = 0, 
		mcal_index = 0, 
		bout_length = 0, 
		walk_ia_counter = 0, 
		RMV_value = 0, 
		OER_er_at = 0,
		BDL_er_at = 0,
		BDL2_er_at = 0,
		pointer_incr_timeout = 0;

	byte 	mcal_path = 0, R1I_counter = 0,
		ERW_counter = 0, SNL_counter = 0,  OER_counter = 0,  BDL_counter = 0, 
		BDL2_counter = 0,  R0I_counter = 0, 
		H0_counter = 0, H2_counter = 0, H3_counter = 0, 
		RMV_counter = 0, CSM_counter = 0;

	byte ip = origin;
	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

	nat history[100] = {0};
	memset(history, 255, sizeof history);

	for (; e < stage1_execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return pm_f1e;
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return pm_erc;
		}


		const byte I = ip * 4, op = graph[I];


		//if (op == 1) { if (e < 5000) { printf("."); fflush(stdout); } else if (e % 16 == 0) { printf("."); fflush(stdout); } }   

		//	else { printf("%hhu", op);
		//	fflush(stdout); } 

		// getchar();


		if (op == one) {
			if (pointer == n) return pm_fea;
			if (not array[pointer]) return pm_ns0; 

			if (last_mcal_op == one)  H0_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;
			
			if (pointer == 1) {
				if (last_mcal_op == three) {
					R1I_counter++;
					if (R1I_counter >= max_consecutive_s1_incr) return pm_r1i;
				} else R1I_counter = 0;
			}

			bout_length++;
			pointer++;

			// if (pointer > *max_pointer) *max_pointer = pointer;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}

			pointer_incr_timeout = 0;
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_er_repetions) return pm_oer;

			if (BDL_er_at and pointer == BDL_er_at - 1) { BDL_counter++; BDL_er_at--; }
			else { BDL_er_at = pointer; BDL_counter = 0; }
			if (BDL_counter >= max_bdl_er_repetions)  return pm_bdl;

			if (BDL2_er_at > 1 and pointer == BDL2_er_at - 2) { BDL2_counter++; BDL2_er_at -= 2; }
			else { BDL2_er_at = pointer; BDL2_counter = 0; }
			if (BDL2_counter >= max_bdl_er_repetions) return pm_bdl2;

			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_modnat_repetions) return pm_rmv;
			}

			if (walk_ia_counter == 1) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			did_ier_at = pointer;
			walk_ia_counter = 0;
			er_count++;
			pointer = 0;
		}

		else if (op == two) {

			if (pointer_incr_timeout >= stage1_execution_limit >> 2) return pm_pt;
			else pointer_incr_timeout++;

			
			if (last_op == six) SNL_counter++;
			else if (last_op != two) SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) {
				

				//fflush(stdout);
				//debug_pm(op, origin, e, history, pm_zr6);
				//fflush(stdout);
				return pm_zr6;  

				// return pm_zr6;
			}

			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pm_snco;

			if (last_op == two) SNL_counter++; 
			else SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three)  return pm_ndi;

			if (last_mcal_op == five) {
				R0I_counter++;
				if (R0I_counter >= max_consecutive_s0_incr) return pm_r0i; 
			}

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (bout_length == 2) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (bout_length == 3) {
				H3_counter++;
				if (H3_counter >= max_consecutive_h3_bouts) return pm_h3; 
			} else H3_counter = 0;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

			bout_length = 0;
			walk_ia_counter++;
			array[pointer]++;
		}

		//if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		
		if (op == three or op == one or op == five) { 
			last_mcal_op = op; mcal_index++; 

			memmove(history, history + 1, sizeof(nat) * 99);
			history[99] = op;
		}


		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	return pm_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pm_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pm_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pm_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pm_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pm_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pm_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pm_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pm_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good;
}

static nat execute_graph(byte* graph, nat* array, byte* origin) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		
		pm = execute_graph_starting_at(o, graph, array);

			printf("execute_graph_starting_at:   origin = %hhu\n", o);
			printf("pm = %s\n", pm_spelling[pm]);
			printf("\n   at origin o = %hhu, z = ", o); 
			print_graph_raw(graph); 
			printf("\n"); 
			fflush(stdout);


		if (not pm) { *origin = o; return z_is_good; } 
	}
	return pm;
}







*/









// static const byte max_sn_loop_iterations = 100 * 2;





