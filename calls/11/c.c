// modified to have job redistribution to improve the parellelism for calls that use many cores! written on 1202504281.210831 by dwrr

// modified to be parellel on 202501131.020611: dwrr 202501131.034623:

// d-general version of the 0 space search util  
// written on 202410163.164303 dwrr

// size of raw 0 space is: (5^15) 	                 			=              30,517,578,125    / 5 =    6103515625
// size of raw 1 space is: (6^15) * (5 * (6 ^ 3)) 	              		=         507,799,783,342,080  / 6 =    84633297223680
// size of raw 2 space is: (7^15) * (5 * (7 ^ 3)) * (5 * (7 ^ 3)) 		=  13,963,646,602,082,100,175 / 7 =   1994806657440300025 

// size of raw 3 space is: (8^15) * (5 * (8 ^ 3)) * (5 * (8 ^ 3) * (5 * (8 ^ 3)) =  5.9029581036 × 10²³ 
//with 6.g and skip loop (1.l + 2.l) applied in PG:
//	(8^15) * (5 * (8 ^ 3)) * (5 * (8 ^ 3) * (5 * (8 ^ 3)) / (8 * 8 * 8)         = 1.1529215046×10²¹
//			1,000,000,000,000,000,000,000
//			s  q   Q   t   b   m   t   h
//	1.1 Sexillion zv to search over, with our partial graph supplied 


// rewritten kinda  on 202411144.202807 dwrr
// fixed bug relating to ERW for 2sp, added various bdl checks, instead.  202502053.011729
// searching 3sp on 1202504126.183639, added partial graph with skip loop on 1202504126.200838

/*

1202504281.202601

	just fixed the bug with the partial graph and the 3sp code! 
	was as simple as a job distribution bug relating to the computation of the total size of the search space not accounting for the noneditable PAs lol


the new sizes of all of the spaces are the following:


0-space: 
	 (5^15) / (5 ^ 4) = 48,828,125

1-space: 
	 (6^15) * (5 * (6 ^ 3)) / (6 ^ 4) = 391,820,820,480

2-space: 
	 (7^15) * (5 * (7 ^ 3)) * (5 * (7 ^ 3)) / (7 ^ 4) = 5,815,762,849,680,175

3-space: 
	 (8^15) * (5 * (8 ^ 3)) * (5 * (8 ^ 3) * (5 * (8 ^ 3)) / (8 ^ 4) = 1.4411518808×10²⁰

4-space: 
	(9^15)
	* (5 * (9 ^ 3)) 
	* (5 * (9 ^ 3)) 
	* (5 * (9 ^ 3)) 
	* (5 * (9 ^ 3))
	/ (9 ^ 4)   =               5.5393363248×10²⁴





//#define all_jobs_have_finished  0x0FFFFFFFFFFFFFF0
//#define no_job_has_started      ((nat) -1)
// 1202505051.144537 added this change: 
//#define ordering memory_order_relaxed




if (not (update_counter & ((1 << update_rate) - 1))) {
			update_counter = 0;
		} else update_counter++;



		for (nat i = 0; i < thread_count; i++) {
			const nat progress = atomic_load_explicit(global_progress + i, ordering);
			if (progress == all_jobs_have_finished) goto terminate_thread;
		}





     The atomic_compare_exchange_strong() operation 

		stores the "desired" value into atomic variable object, 

			but only if the atomic variable is equal to the expected value.  

	. Upon success, the operation returns true.  

	. Upon failure, the "*expected" value is   overwritten   with the contents of the atomic variable
			and false is returned.



const nat  b =    (na) !!(job_index >= count and count);


	(... nat* expected, nat desired, _Atomic nat* flag) {

		if (*expected == *flag) {
			*flag = desired; return true;
		} else {
			*expected = *flag; return false;
		}
	}



worker_thread() {
....

	const bool nonzero_amount_of_jobs_finished = job_index >= count and count;
	nat expected = 0;
	const bool was_equal = atomic_compare_exchange_strong(&flag, &expected, nonzero_amount_of_jobs_finished));
	if (not was_equal or nonzero_amount_of_jobs_finished) goto terminate;


...	

}



*/






#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
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
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef uint8_t byte;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t nat;
typedef uint64_t chunk;

#define D 1
#define execution_limit 50000000LLU
#define array_size 1000000LLU
#define chunk_count 2
#define display_rate 2

#define total_job_count 10000
#define machine_index 0

#define machine0_counter_max 1
#define machine1_counter_max 1

#define machine0_thread_count 10
#define machine1_thread_count 0

#define  thread_count  ( machine_index ? machine1_thread_count : machine0_thread_count ) 
struct job {
	byte* begin;
	byte* end;
};

struct joblist {
	struct job* jobs;
	nat job_count;
	nat thread_index;
};

struct machine {
	struct joblist* cores;
	nat core_count;
};

static _Atomic nat flag = 0;
static _Atomic nat* display_progress = NULL;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good,
	pm_zr5, pm_zr6, pm_ndi, pm_sndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi,
	pm_h0, pm_h0s, pm_h1, pm_h2, 
	pm_rmv, pm_ormv, pm_imv, pm_csm,
	pm_fse, pm_pair, pm_ls0,
	pm_bdl1, pm_bdl2, pm_bdl3, 
	pm_bdl4, pm_bdl5, pm_bdl6, 
	pm_bdl7, pm_bdl8, pm_bdl9, 
	pm_bdl10, pm_bdl11, pm_bdl12, 
	pm_erp1, pm_erp2,

	pm_ga_sdol, 
	pm_ga_6g,    pm_ga_ns0, 
	pm_ga_zr5,   pm_ga_pco, 
	pm_ga_ndi,   pm_ga_sndi, 
	pm_ga_snco,  pm_ga_sn1, 
	pm_ga_zr6,   pm_ga_rdo, 
	pm_ga_uo, 
	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good",
	"pm_zr5", "pm_zr6", "pm_ndi", "pm_sndi", 
	"pm_pco", "pm_per", "pm_ns0",
	"pm_oer", "pm_rsi",
	"pm_h0", "pm_h0s", "pm_h1", "pm_h2", 
	"pm_rmv", "pm_ormv", "pm_imv", "pm_csm",
	"pm_fse", "pm_pair", "pm_ls0",
	"pm_bdl1", "pm_bdl2", "pm_bdl3", 
	"pm_bdl4", "pm_bdl5", "pm_bdl6", 
	"pm_bdl7", "pm_bdl8", "pm_bdl9", 
	"pm_bdl10", "pm_bdl11", "pm_bdl12", 
	"pm_erp1", "pm_erp2",

	"pm_ga_sdol", 
	"pm_ga_6g",    "pm_ga_ns0",
	"pm_ga_zr5",   "pm_ga_pco", 
	"pm_ga_ndi",   "pm_ga_sndi", 
	"pm_ga_snco",  "pm_ga_sn1", 
	"pm_ga_zr6",   "pm_ga_rdo", 
	"pm_ga_uo", 
};

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }

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

static void append_to_file(char* filename, size_t size, byte* graph, byte origin) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, graph); 
	char o[16] = {0};    snprintf(o, sizeof o, "%hhu", origin);

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
	write(file, o, strlen(o));
	write(file, " ", 1);
	write(file, dt, strlen(dt));
	write(file, "\n", 1);
	close(file);

	printf("[%s]: write: %s z = %s to file \"%s\"\n",
		dt, permissions ? "created" : "wrote", z, filename
	);
}

static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, byte* zskip_at) {

#define max_erp_count 20
#define max_rsi_count 512
#define max_oer_repetions 50
#define max_rmv_modnat_repetions 15
#define max_ormv_modnat_repetions 15
#define max_imv_modnat_repetions 40
#define max_consecutive_small_modnats 230
#define max_consecutive_s0_incr 30
#define max_consecutive_h0_bouts 10
#define max_consecutive_h1_bouts 16
#define max_consecutive_h2_bouts 24
#define max_consecutive_h0s_bouts 7
#define max_consecutive_pairs 8

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0,  pointer = 0,  bout_length = 0, 
		OER_ier_at = 0,
		BDL_ier_at = 0,
		PER_ier_at = (nat) ~0;

	byte	H0_counter = 0,  H0S_counter = 0, SNDI_counter = 0,
		H1_counter = 0, H2_counter = 0, OER_counter = 0,
		BDL1_counter = 0, BDL2_counter = 0,
		BDL3_counter = 0, BDL4_counter = 0,
		BDL5_counter = 0, BDL6_counter = 0,
		BDL7_counter = 0, BDL8_counter = 0,
		BDL9_counter = 0, BDL10_counter = 0, 
		BDL11_counter = 0, BDL12_counter = 0,
		pair_index = 0, pair_count = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;

	nat performed_er_at = 0;
 	byte small_erp_array[max_erp_count]; small_erp_array[0] = 0;
	byte rsi_counter[max_rsi_count]; rsi_counter[0] = 0;

	//nat first = 1; // debug

	for (nat e = 0; e < execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		/*if (e >= 100000000) {
			if (first) {
				print_graph_raw(graph); puts("");
				getchar();
				first = 0;
			}
			printf("%hhu ", op); fflush(stdout);
			usleep(20000);
		}*/

		if (op == one) {
			if (pointer == n) { 
				puts("FEA condition violated by a z value: "); 
				print_graph_raw(graph); 
				puts(""); 
				abort(); 
			}
  
			if (not array[pointer]) return pm_ns0; 
			if (last_mcal_op == one)  H0_counter = 0;
			if (last_mcal_op == one)  H0S_counter = 0;

			if (pointer < max_rsi_count) {
				if (last_mcal_op == three) {
					rsi_counter[pointer]++;
					if (rsi_counter[pointer] >= max_consecutive_s0_incr) return pm_rsi;
				} else rsi_counter[pointer] = 0;
			}

			if (pair_index == 1) pair_index = 2;
			else if (pair_index == 3) pair_index = 4;
			else if (pair_index == 4) { pair_index = 0; pair_count++; if (pair_count >= max_consecutive_pairs) return pm_pair; } 
			else if (pair_index) { pair_count = 0; pair_index = 0; }

			SNDI_counter = 0;

			bout_length++;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
				if (pointer < max_rsi_count) rsi_counter[pointer] = 0;
				if (pointer < max_erp_count) small_erp_array[pointer] = 0;
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (pointer == OER_ier_at or pointer == OER_ier_at + 1) {
				OER_counter++;
				if (OER_counter >= max_oer_repetions) return pm_oer;
			} else { OER_ier_at = pointer; OER_counter = 0; }
			
			byte CSM_counter = 0;
			nat RMV_value = (nat) -1;
			byte RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 8) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_rmv_modnat_repetions) return pm_rmv;
			}

			RMV_value = (nat) -1;
			RMV_counter = 0;
			byte RMV_state = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] == RMV_value + RMV_state) { 
					RMV_state = not RMV_state;
					RMV_counter++;
					if (RMV_counter >= max_ormv_modnat_repetions) return pm_ormv; 
				} else { 
					RMV_value = array[i]; 
					RMV_counter = 0; 
					RMV_state = 0;
				}
			}

			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] == RMV_value + 1) { RMV_counter++; RMV_value++; } else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_imv_modnat_repetions) return pm_imv;
			}

			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i += 2) {
				if (array[i] == RMV_value + 1) { RMV_counter++; RMV_value++; } else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= 2 * max_imv_modnat_repetions) return pm_imv;
			}

			if (pointer + 1 == BDL_ier_at) {
				BDL1_counter++; 
				if (BDL1_counter >= 8) return pm_bdl1; 
			} else BDL1_counter = 0;

			if (pointer + 2 == BDL_ier_at) {
				BDL2_counter++; 
				if (BDL2_counter >= 8) return pm_bdl2; 
			} else BDL2_counter = 0;

			if (pointer + 3 == BDL_ier_at) {
				BDL3_counter++;
				if (BDL3_counter >= 30) return pm_bdl3; 
			} else BDL3_counter = 0;

			if (	pointer     == BDL_ier_at or 
				pointer + 1 == BDL_ier_at or 
				pointer + 2 == BDL_ier_at or
				pointer + 3 == BDL_ier_at or
				pointer + 4 == BDL_ier_at
			) {
				BDL4_counter++; 
				if (BDL4_counter >= 150 and e >= 500000) return pm_bdl4; 
			} else BDL4_counter = 0;


			if (pointer + 5 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL5_counter++; 
				if (BDL5_counter >= 80 and e >= 500000) return pm_bdl5; 
			} else BDL5_counter = 0;

			if (pointer + 6 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL6_counter++; 
				if (BDL6_counter >= 80 and e >= 500000) return pm_bdl6; 
			} else BDL6_counter = 0;

			if (pointer + 7 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL7_counter++; 
				if (BDL7_counter >= 80 and e >= 500000) return pm_bdl7; 
			} else BDL7_counter = 0;

			if (pointer + 8 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL8_counter++; 
				if (BDL8_counter >= 80 and e >= 500000) return pm_bdl8; 
			} else BDL8_counter = 0;


			if (pointer + 9 == BDL_ier_at) { 
				BDL9_counter++; 
				if (BDL9_counter >= 30 and e >= 500000) return pm_bdl9; 
			} else BDL9_counter = 0;

			if (pointer + 10 == BDL_ier_at) { 
				BDL10_counter++; 
				if (BDL10_counter >= 30 and e >= 500000) return pm_bdl10; 
			} else BDL10_counter = 0;

			if (pointer + 11 == BDL_ier_at) { 
				BDL11_counter++; 
				if (BDL11_counter >= 30 and e >= 500000) return pm_bdl11; 
			} else BDL11_counter = 0;

			if (pointer + 12 == BDL_ier_at) { 
				BDL12_counter++; 
				if (BDL12_counter >= 30 and e >= 500000) return pm_bdl12; 
			} else BDL12_counter = 0;

			if (pair_index == 3) { pair_index = 0; pair_count++; if (pair_count >= max_consecutive_pairs) return pm_pair; } 
			else if (pair_index) { pair_count = 0; pair_index = 0; }
		
			if (pointer < 64) performed_er_at |= (1LLU << pointer);
			if (pointer < max_erp_count and small_erp_array[pointer] < 250) {
				small_erp_array[pointer]++;
			}

			SNDI_counter = 0;

			BDL_ier_at = pointer;
			PER_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) {
			SNDI_counter++;
			if (SNDI_counter >= 10) return pm_sndi;
			array[n]++;
		}

		else if (op == six) {  
			if (not array[n]) return pm_zr6;
			SNDI_counter = 0;
			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three) return pm_ndi;

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (last_mcal_op == one) {
				H0S_counter++;
				if (H0S_counter >= max_consecutive_h0s_bouts and e >= 100000) return pm_h0s; 
			}

			if (bout_length == 2) {
				H1_counter++;
				if (H1_counter >= max_consecutive_h1_bouts) return pm_h1; 
			} else H1_counter = 0;

			if (bout_length == 3) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (PER_ier_at != (nat) ~0) {
				if (pointer >= PER_ier_at) return pm_per; 
				PER_ier_at = (nat) ~0;
			}

			if (not pair_index) pair_index = 1;
			else if (pair_index == 2) pair_index = 3;
			else { pair_count = 0; pair_index = 0; }

			SNDI_counter = 0;

			bout_length = 0;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;
		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		if (*zskip_at > I + state) *zskip_at = I + state;
		ip = graph[I + state];
	}


	if (xw < 11) return pm_fse;
	for (nat i = 0; i < 10; i++) {
		if (array[i] < 20) return pm_fse;
	}

	const nat average = ((array[3] + array[4] + array[5] + array[6]) / 4);
	const nat max_modnat_value = (average * 3) / 2;
	const bool l0 = array[0] > max_modnat_value;
	const bool l1 = array[1] > max_modnat_value;
	const bool l2 = array[2] > max_modnat_value;
	const bool should_prune = (l0 or l1 or l2) and (max_modnat_value >= 100);
	if (should_prune) return pm_ls0;

	if (xw >= 100) {
		const nat max_position = xw < 64 ? xw : 64;
		for (nat i = 1; i < max_position; i++) {
			if (not ((performed_er_at >> i) & 1LLU)) return pm_erp1;
		}
		const nat max_position2 = xw < max_erp_count ? xw : max_erp_count;
		for (nat i = 1; i < max_position2; i++) {
			if (small_erp_array[i] < 5) return pm_erp2;
		}
	}

	return z_is_good;
}

static byte execute_graph(byte* graph, nat* array, byte* origin, nat* counts) {
	byte at = graph_count;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three and graph[4 * o] != two) continue;
		const nat pm = execute_graph_starting_at(o, graph, array, &at);
		counts[pm]++;
		if (not pm) { *origin = o; return 0; }
	}
	return at;
}


#define lsepa 1
static byte noneditable(byte pa) { 
	//return (pa < 20 and pa % 4 == 0) or 
	//	pa == 18 or pa == 19 or
	//	pa == 1 or pa == 5;

	return (pa < 20 and pa % 4 == 0) or pa == 18;
}

static byte editable(byte pa) { return not noneditable(pa); }

static void* worker_thread(void* raw_argument) {

	char filename[4096] = {0};
	nat* raw_counts = calloc(1 + pm_count, sizeof(nat));
	nat* counts = raw_counts + 1;
	nat* current_progress = raw_counts;
	nat* array = calloc(array_size + 1, sizeof(nat));
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);
	byte* graph = raw_graph;
	byte* end = raw_end;
	nat* graph_64 = raw_graph;
	nat* end_64 = raw_end;
	byte pointer = 0;

	const struct joblist list = *(struct joblist*) raw_argument;
	const nat thread_index = list.thread_index;
	const nat count = list.job_count;
	const struct job* jobs = list.jobs;

	nat job_index = 0;

	next_job:;
		const bool nonzero_amount_of_jobs_finished = job_index >= count and count;
		nat expected = 0;
		const bool was_equal = atomic_compare_exchange_strong(&flag, &expected, nonzero_amount_of_jobs_finished);
		if (not was_equal or job_index >= count) goto terminate;

		atomic_store_explicit(display_progress + thread_index, job_index, memory_order_relaxed);
		memcpy(graph, jobs[job_index].begin, graph_count);
		memcpy(end, jobs[job_index].end, graph_count);		
		goto init;

	loop:
		for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
			if (graph_64[i] < end_64[i]) goto process;
			if (graph_64[i] > end_64[i]) break;
		}
		goto prepare_next_job;

	process:
		if (graph[pointer] < ((pointer % 4) ? operation_count - 1 : 4)) goto increment;
		if (pointer < graph_count - 1) goto reset_;

	prepare_next_job:
		job_index++;
		goto next_job;

	increment:
		graph[pointer]++;
	init:  	pointer = lsepa;

		u16 was_utilized = 0;
		byte at = lsepa;

		for (byte index = 20; index < graph_count; index += 4) {
			if (index < graph_count - 4 and graph[index] > graph[index + 4]) {
				at = index + 4;
				counts[pm_ga_sdol]++;
				//puts(pm_spelling[pm_ga_sdol]);
				goto bad;
			} 
		}

		for (byte index = operation_count; index--;) {

			const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

			if (graph[4 * index] == six and graph[4 * e] == one) {
				at = graph_count;
				if (editable(4 * index + 3) and at > 4 * index + 3) at = 4 * index + 3;
				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (editable(4 * e) and at > 4 * e) at = 4 * e;
				if (at == graph_count) abort();
				counts[pm_ga_ns0]++;
				//puts(pm_spelling[pm_ga_ns0]); 
				goto bad;
			}
	 
			if (graph[4 * index] == six and graph[4 * e] == five) {
				at = graph_count;
				if (editable(4 * index + 3) and at > 4 * index + 3) at = 4 * index + 3;
				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (editable(4 * e) and at > 4 * e) at = 4 * e;
				if (at == graph_count) abort();
				counts[pm_ga_ns0]++;
				//puts(pm_spelling[pm_ga_ns0]);
				goto bad;
			}


			

			/*

			2 x ----------(*n > *i)---------> x

			
			2 x ----------(*n > *i)---------> 2 y
			  y ----------(*n > *i)---------> x


			2 x ----------(*n > *i)---------> 2 y
			  y ----------(*n > *i)---------> 2 z
			  z ----------(*n > *i)---------> x


			2 x ----------(*n > *i)---------> 2 y
			  y ----------(*n > *i)---------> 2 z
			  z ----------(*n > *i)---------> 2 w
			  w ----------(*n > *i)---------> x
		*/


			// 2 x ----------(*n > *i)---------> x

			/// 0 1 1 4   1 0 2 5   2 1 3 6   ...  1 x 5 x   ...    0 1 4 0 
		
			if (graph[4 * index + 0] == two and g == index) {
				at = graph_count;
				if (editable(4 * index + 2) and at > 4 * index + 2) at = 4 * index + 2;
				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (at == graph_count) abort();
				counts[pm_ga_sndi]++;
				//puts(pm_spelling[pm_ga_sndi]);
				goto bad;
			}



			//2 x ----------(*n > *i)---------> 2 y
			//  y ----------(*n > *i)---------> x

			/// 0 1 1 4   1 0 2 5   2 1 3 6   ...  1 x 5 x   ...    0 1 4 0 

			// g is y
			// index is x
		
			if (	graph[4 * index + 0] == two and     //x is a two
				graph[4 * g + 0] == two and         //y is a two
				graph[4 * g + 2] == index        //y take g side to go to x
				//graph[4 * index + 2] == g and       //x take g side to go to y
			) {
				at = graph_count;

				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (editable(4 * g) and at > 4 * g) at = 4 * g;

				if (editable(4 * g + 2) and at > 4 * g + 2) at = 4 * g + 2;
				if (editable(4 * index + 2) and at > 4 * index + 2) at = 4 * index + 2;
				
				if (at == graph_count) abort();
				counts[pm_ga_sndi]++;
				//puts(pm_spelling[pm_ga_sndi]);
				goto bad;
			}




			if (	graph[4 * index + 0] == two and 
				graph[4 * graph[4 * index + 2] + 0] == two and  
				graph[4 * graph[4 * graph[4 * index + 2] + 2] + 0] == two and				
				graph[4 * graph[4 * graph[4 * index + 2] + 2] + 2] == index
			) {
				at = graph_count;

				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (editable(4 * g) and at > 4 * g) at = 4 * g;

				if (editable(4 * graph[4 * graph[4 * index + 2] + 2]) and 
					at > 4 * graph[4 * graph[4 * index + 2] + 2]) 
					at = 4 * graph[4 * graph[4 * index + 2] + 2];

				if (editable(4 * index + 2) and at > 4 * index + 2) at = 4 * index + 2;

				if (editable(4 * g + 2) and at > 4 * g + 2) at = 4 * g + 2;

				if (editable(4 * graph[4 * graph[4 * index + 2] + 2] + 2) and 
					at > 4 * graph[4 * graph[4 * index + 2] + 2] + 2) 
					at = 4 * graph[4 * graph[4 * index + 2] + 2] + 2;
			
				if (at == graph_count) abort();
				counts[pm_ga_sndi]++;
				//puts(pm_spelling[pm_ga_sndi]);
				goto bad;
			}



			{const byte pairs[3 * 5] = {
				three, three, pm_ga_ndi,
				five, one,  pm_ga_pco,
				five, five, pm_ga_zr5,

				six, six, pm_ga_zr6,
				//two, two, pm_ga_sndi,
				//two, six, pm_ga_sn1,
				six, two, pm_ga_snco,
			};
			for (nat i = 0; i < 5 * 3; i += 3) {
				const byte source = pairs[i + 1], destination = pairs[i + 0];
				if (graph[4 * index] == source) {
					for (byte offset = 1; offset < 4; offset++) {
						const byte dest = graph[4 * index + offset];
						if (graph[4 * dest] != destination) continue;
						at = graph_count;
						if (editable(4 * index + offset) and at > 4 * index + offset) at = 4 * index + offset;
						if (editable(4 * index) and at > 4 * index) at = 4 * index;
						if (editable(4 * dest) and at > 4 * dest) at = 4 * dest;
						if (at == graph_count) continue;
						counts[pairs[i + 2]]++;
						//puts(pm_spelling[pairs[i + 2]]);
						goto bad;
					}
				} 
			}}

			if (l != index) was_utilized |= 1 << l;
			if (g != index) was_utilized |= 1 << g;
			if (e != index) was_utilized |= 1 << e;

			const byte j = 4 * index;
			for (byte i = graph_count - 4; i >= 20 and j < i; i -= 4) {
				if (not memcmp(graph + i, graph + j, 4)) { 
					at = graph_count;
					if (editable(4 * index + 3) and at > 4 * index + 3) at = 4 * index + 3;
					if (editable(4 * index + 2) and at > 4 * index + 2) at = 4 * index + 2;
					if (editable(4 * index + 1) and at > 4 * index + 1) at = 4 * index + 1;
					if (editable(4 * index + 0) and at > 4 * index + 0) at = 4 * index + 0;
					counts[pm_ga_rdo]++; 
					//puts(pm_spelling[pm_ga_rdo]);
					goto bad;
				}
			}


		}

		for (byte index = 0; index < operation_count; index++) {
			if (not ((was_utilized >> index) & 1)) { 
				at = lsepa;
				counts[pm_ga_uo]++; 
				//puts("pm_ga_uo"); 
				goto bad; 
			} 
		}

		for (byte index = operation_count; index--;) { 
			if (graph[4 * index] == six and graph[4 * index + 2] != 4) {
				at = graph_count;
				if (editable(4 * index + 2) and at > 4 * index + 2) at = 4 * index + 2;
				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (at == graph_count) abort();
				counts[pm_ga_6g]++;
				//puts(pm_spelling[pm_ga_6g]);
				goto bad;
			}
		}

		byte origin = 0;
		at = execute_graph(graph, array, &origin, counts);

		if (not at) {
			append_to_file(filename, sizeof filename, graph, origin);
			//usleep(100000);
			goto loop;
		}

		//while (at > lsepa and noneditable(at)) at--;
		//if (at < lsepa) at = lsepa;

	bad:	if (noneditable(at)) {
			printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
			abort();
		}		
		for (byte i = lsepa; i < at; i++) if (editable(i)) graph[i] = 0;
		pointer = at; goto loop;
	reset_:
		graph[pointer] = 0; 
		do pointer++; while (noneditable(pointer));
		goto loop;

terminate:
	free(raw_graph);
	free(raw_end);
	free(array);
	*current_progress = job_index;
	return raw_counts;
}

static void print(char* filename, size_t size, const char* string) {
	char dt[32] = {0};   get_datetime(dt);

	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;
try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("print: [%s]: failed to create filename = \"%s\"\n", dt, filename);
			fflush(stdout);
			abort();
		}
		snprintf(filename, size, "%s_D%u_%08x%08x%08x%08x_output.txt", dt, D,
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, string, strlen(string));
	close(file);
	printf("%s", string);
	fflush(stdout);
}

static nat adc_raw(nat* d, nat s) {
	*d += s;
	return *d < s;
}

static nat adc_chunk(nat* d, nat s, nat cin) {
	nat cout = adc_raw(d, s) | adc_raw(d, cin);
	return cout;
}

static nat adc(chunk* d, chunk* s, nat cin) {
	nat c = cin;
	for (nat i = 0; i < chunk_count; i++) {
		c = adc_chunk(d + i, s[i], c);
	}
	return c;
}

static void mul_small(chunk* d, nat s) {
	nat copy[chunk_count] = {0};
	memcpy(copy, d, chunk_count * sizeof(nat));
	for (nat i = 0; i < s - 1; i++)  adc(d, copy, 0);
}

static nat shift_left(chunk* d) {
	nat c = 0;
	for (nat i = 0; i < chunk_count; i++) {
		const nat save_c = !!(d[i] & 0x8000000000000000);
		d[i] = (d[i] << 1) | c;
		c = save_c;
	}
	return c;
}

static void bitwise_invert(chunk* d) {
	for (nat i = 0; i < chunk_count; i++) 
		d[i] = ~(d[i]);
}

static nat get_bit(chunk* a, nat i) {
	const nat chunk = i / 64LLU;
	const nat bit_in_chunk = i % 64LLU;
	return (a[chunk] >> bit_in_chunk) & 1LLU;
}

static void set_bit(chunk* a, nat i) {
	const nat chunk = i / 64LLU;
	const nat bit_in_chunk = i % 64LLU;
	a[chunk] |= 1LLU << bit_in_chunk;
}

static void print_binary64(nat x) {
	for (nat i = 0; i < 64; i++) 
		printf("%llu", (x >> i) & 1);
}

static void debug_zi(const char* s, chunk* number) {
	printf("%s = {\n", s);
	for (nat i = 0; i < chunk_count; i++) {
		printf("[%llu]: ", i);
		print_binary64(number[i]); 
		printf("  (dec %llu)\n", number[i]);
	}
	puts("} ");
}

static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

static nat ge_r_s(nat* r, nat* s) {
	nat copy[chunk_count] = {0};
	memcpy(copy, s, chunk_count * sizeof(nat));
	bitwise_invert(copy);
	return adc(copy, r, 1);
}

// computes total / divisor
static void divide(chunk* q, chunk* r, chunk* total, chunk* divisor) {

	memset(q, 0, sizeof(nat) * chunk_count);
	memset(r, 0, sizeof(nat) * chunk_count);

	nat negative_divisor[chunk_count] = {0};
	memcpy(negative_divisor, divisor, chunk_count * sizeof(nat)); 
	bitwise_invert(negative_divisor);
	adc(negative_divisor, r, 1);

	for (nat i = 64 * chunk_count; i--;) {
		shift_left(r);
		*r |= get_bit(total, i);
		if (ge_r_s(r, divisor)) {
			adc(r, negative_divisor, 0);
			set_bit(q, i);
		}
	}
}

int main(void) {
	srand((unsigned) time(0));

#define noneditable_pa_count  1 //4

	const byte u = 0;
	/*byte partial_graph[20] = {
		0,  1, u, u,
		1,  0, u, u,
		2,  u, u, u,
		3,  u, u, u,
		4,  u, 4, 2,
	};*/

	byte partial_graph[20] = {
		0,  u, u, u,
		1,  u, u, u,
		2,  u, u, u,
		3,  u, u, u,
		4,  u, 4, u,
	};
	
	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};
	
	atomic_init(&flag, 0);

 	display_progress = calloc(1, thread_count * sizeof(_Atomic nat));
	for (nat i = 0; i < thread_count; i++)  atomic_init(display_progress + i, 0);
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));

	struct machine* machine = calloc(2, sizeof(struct machine));
	for (nat mi = 0; mi < 2; mi++) {
		const nat core_count = mi ? machine1_thread_count : machine0_thread_count;
		machine[mi].cores = calloc(core_count, sizeof(struct joblist));
		machine[mi].core_count = core_count; 

		for (nat core = 0; core < core_count; core++) {
			machine[mi].cores[core].jobs = NULL;
			machine[mi].cores[core].job_count = 0;
			machine[mi].cores[core].thread_index = core;
		}
	}

	const nat K = expn(5 + D, 3);
	nat total[chunk_count] = {0};
	total[0] = expn(5 + D, 15 - noneditable_pa_count);

	for (nat _i = 0; _i < D; _i++) {
		mul_small(total, 5);
		mul_small(total, K);
	}

	debug_zi("space_size", total);

	nat divisor[chunk_count] = { [0] = total_job_count };
	nat q[chunk_count] = {0};
	nat r[chunk_count] = {0};
	divide(q, r, total, divisor);

	nat n1[chunk_count] = {0};
	bitwise_invert(n1);
	nat width[chunk_count] = {0};
	memcpy(width, q, chunk_count * sizeof(nat));

	debug_zi("width", width);
	
	nat width_m1[chunk_count] = {0};
	memcpy(width_m1, width, chunk_count * sizeof(nat));
	adc(width_m1, n1, 0);
	nat last_zi[chunk_count] = {0};
	memcpy(last_zi, total, chunk_count * sizeof(nat));
	adc(last_zi, n1, 0);

	debug_zi("last_zi", last_zi);
	getchar();

	nat begin[chunk_count] = {0};
	nat machine0_counter = 0, machine1_counter = 0, machine_state = 0;
	nat core_counter[2] = {0};

	for (nat job = 0; job < total_job_count; job++) {

		nat range_begin[chunk_count] = {0};
		memcpy(range_begin, begin, sizeof(nat) * chunk_count);

		nat sum[chunk_count] = {0};
		memcpy(sum, begin, chunk_count * sizeof(nat));
		adc(sum, width_m1, 0);

		nat range_end[chunk_count] = {0};
		memcpy(range_end, job < total_job_count - 1 ? sum : last_zi, sizeof(nat) * chunk_count);
	
		adc(begin, width, 0);

		byte* begin_zv = calloc(graph_count, 1);
		memcpy(begin_zv, partial_graph, 20);
		nat p_begin[chunk_count] = { [0] = 1 };

		for (byte i = 0; i < graph_count; i++) {
			if (noneditable(i)) { continue; } 
			const nat radix = (nat) (i % 4 ? operation_count : 5);
			nat radix_mp[chunk_count] = { [0] = radix };
			nat div1[chunk_count] = {0};
			nat rem2[chunk_count] = {0};
			divide(div1, r, range_begin, p_begin);
			divide(q, rem2, div1, radix_mp);
			begin_zv[i] = (byte) *rem2;
			mul_small(p_begin, radix);
		}

		byte* end_zv = calloc(graph_count, 1);
		memcpy(end_zv, partial_graph, 20);
		nat p_end[chunk_count] = { [0] = 1 };

		for (byte i = 0; i < graph_count; i++) {
			if (noneditable(i)) { continue; } 
			const nat radix = (nat) (i % 4 ? operation_count : 5);
			nat radix_mp[chunk_count] = { [0] = radix };
			nat div1[chunk_count] = {0};
			nat rem2[chunk_count] = {0};
			divide(div1, r, range_end, p_end);
			divide(q, rem2, div1, radix_mp);
			end_zv[i] = (byte) *rem2;
			mul_small(p_end, radix);
		}
	
		nat mi;

		if (machine_state) {
			if (machine1_counter < machine0_counter_max) { machine1_counter++; mi = 1; } 
			else { machine_state = 0; machine0_counter = 1; mi = 0; }
		} else {
			if (machine0_counter < machine1_counter_max) { machine0_counter++; mi = 0; } 
			else { machine_state = 1; machine1_counter = 1; mi = 1; }
		}

		if (machine1_thread_count == 0) mi = 0;
		
		if (core_counter[mi] < machine[mi].core_count - 1) core_counter[mi]++; else core_counter[mi] = 0;
		const nat c = core_counter[mi];
	
		machine[mi].cores[c].jobs = realloc(machine[mi].cores[c].jobs, sizeof(struct job) * (machine[mi].cores[c].job_count + 1));
		machine[mi].cores[c].jobs[machine[mi].cores[c].job_count++] = (struct job) { .begin = begin_zv, .end = end_zv };
	}

	//const nat job_count_per_core = machine[machine_index].cores[0].job_count;

	snprintf(output_string, 4096, "SU: searching [D=%u] space....\n", D);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	nat display_local_progress[thread_count] = {0};
	nat counts[pm_count] = {0};

	nat job_indexes[thread_count] = {0};

	nat completed_so_far = 0;


start_up_threads:

	for (nat i = 0; i < machine[machine_index].core_count; i++) {
		pthread_create(threads + i, NULL, worker_thread, machine[machine_index].cores + i);
	}

	nat max_job_size = 0;
	for (nat i = 0; i < machine[machine_index].core_count; i++) {
		if (machine[machine_index].cores[i].job_count > max_job_size) max_job_size = machine[machine_index].cores[i].job_count;
	}

	nat resolution = max_job_size / 200;
	if (resolution == 0) resolution = 1;

	while (1) {
		nat expected = 0;
		if (not atomic_compare_exchange_strong(&flag, &expected, 0)) goto terminate;

		nat sum = 0;
		for (nat i = 0; i < thread_count; i++) {
			display_local_progress[i] = atomic_load_explicit(display_progress + i, memory_order_relaxed);
			sum += display_local_progress[i];
		}

		printf("\033[H\033[2J");
		printf("\n-----------------current jobs (max_job_size=%llu)-------------------\n", max_job_size);
		printf("\n\t%1.10lf%%\n\n", (double) (completed_so_far + sum) / (double) total_job_count);
		printf("  resolution = %llu\n", resolution);

		for (nat i = 0; i < thread_count; i++) {
			const nat size = display_local_progress[i];
			printf(" %llu: [%8llu / %8llu] :: ", i, size, machine[machine_index].cores[i].job_count);

			nat amount = size / resolution; if (not amount) amount = 1;

			for (nat j = 0; j < amount; j++) {
				putchar('#');
			}
			puts("");
		}
		puts("");
		sleep(1 << display_rate);
		//usleep(10000);
	}

terminate:
	puts("\nmain: joining threads...\n");
	for (nat i = 0; i < thread_count; i++) {
		nat* result = NULL;
		pthread_join(threads[i], (void**) &result);
		for (nat j = 0; j < pm_count; j++) counts[j] += result[j + 1];
		job_indexes[i] = *result;
		free(result);
	}

	const nat mi = machine_index;
	struct job remaining_jobs[total_job_count] = {0};
	nat remaining_count = 0;

	for (nat i = 0; i < thread_count; i++) {
		const nat k = job_indexes[i];
		for (nat j = k; j < machine[mi].cores[i].job_count; j++) 
			remaining_jobs[remaining_count++] = machine[mi].cores[i].jobs[j];
	}
	if (not remaining_count) goto all_threads_have_finished;

	for (nat i = 0; i < thread_count; i++) machine[mi].cores[i].job_count = 0;
	nat counter = 0;
	for (nat i = 0; i < remaining_count; i++) {
		if (counter < machine[mi].core_count - 1) counter++; 
		else counter = 0;
		const nat c = counter;	
		machine[mi].cores[c].jobs = realloc(machine[mi].cores[c].jobs, sizeof(struct job) * (machine[mi].cores[c].job_count + 1));
		machine[mi].cores[c].jobs[machine[mi].cores[c].job_count++] = remaining_jobs[i];
	}
	atomic_store(&flag, 0);
	for (nat i = 0; i < thread_count; i++) atomic_store_explicit(display_progress + i, 0, memory_order_relaxed);

	completed_so_far = total_job_count - remaining_count;

	goto start_up_threads;

all_threads_have_finished:;
	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec);
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	snprintf(output_string, 4096,
		"su: D = %u space:\n"
		"\t chunk_count = %u\n"		
		"\t total_job_count = %u\n"
		"\t machine_index = %u\n"
		"\t machine0_counter_max = %u\n"
		"\t machine1_counter_max = %u\n"
		"\t machine0_thread_count = %u\n"
		"\t machine1_thread_count = %u\n"
		"\t display_rate = %u\n"
		"\t execution_limit = %llu\n"
                "\t array_size = %llu\n"
		"\t in %10.2lfs [%s:%s]\n"
		"\n",
		D, chunk_count,
		total_job_count, 
		machine_index, 
		machine0_counter_max, 
		machine1_counter_max, 		
		machine0_thread_count, 
		machine1_thread_count,
		display_rate, 
		execution_limit, 
		array_size, 
		seconds, time_begin_dt, time_end_dt
	);
	print(output_filename, 4096, output_string);

        snprintf(output_string, 4096, "\npm counts:\n");
	print(output_filename, 4096, output_string);

        for (nat i = 0; i < pm_count; i++) {
                if (i and not (i % 2)) {
			snprintf(output_string, 4096, "\n");
			print(output_filename, 4096, output_string);
		}
		snprintf(output_string, 4096, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
		print(output_filename, 4096, output_string);
        }
        snprintf(output_string, 4096, "[done]\n");
	print(output_filename, 4096, output_string);
}







































































/*




static void print(char* filename, size_t size, const char* string) {
	char dt[32] = {0};   get_datetime(dt);

	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;
try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("print: [%s]: failed to create filename = \"%s\"\n", dt, filename);
			fflush(stdout);
			abort();
		}
		snprintf(filename, size, "%s_D%u_%08x%08x%08x%08x_output.txt", dt, D,
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, string, strlen(string));
	close(file);
	printf("%s", string);
	fflush(stdout);
}



*/































	/*const nat job_count_per_core = machine[machine_index].cores[0].job_count;

	for (nat mi = 0; mi < 2; mi++) {
		
		printf("machine #%llu has %llu cores: \n", mi, machine[mi].core_count);

		for (nat i = 0; i < machine[mi].core_count; i++) {

			printf("\tcore #%llu job list: (%llu jobs): \n", i, machine[mi].cores[i].job_count);

			for (nat j = 0; j < machine[mi].cores[i].job_count; j++) {

				printf("\t\t[%6llu] = {uid=%llu}(", j, machine[mi].cores[i].jobs[j].uid);
				print_graph_raw(machine[mi].cores[i].jobs[j].begin);
				printf(" ... ");
				print_graph_raw(machine[mi].cores[i].jobs[j].end);
				puts(")");
			}
			puts("");
		}
		puts("----------------------------------\n\n");
	}
	getchar();*/
















































































	//printf("thread_had_terminate: redistributed %llu jobs onto the cores again!\n", remaining_count);
	//getchar();
	/*for (nat i = 0; i < thread_count; i++) {
		const nat done_job_index = atomic_load_explicit(global_progress + i, ordering);
		if (done_job_index == all_jobs_have_finished) { machine[machine_index].cores[i].job_count = 0; continue; } 	
		nat first_job_index = done_job_index + 1;
		if (done_job_index == no_job_has_started) first_job_index = 0;
		memmove(machine[machine_index].cores[i].jobs, 
		        machine[machine_index].cores[i].jobs + first_job_index, 
			sizeof(struct job) * (machine[machine_index].cores[i].job_count - first_job_index)
		);
		machine[machine_index].cores[i].job_count -= first_job_index;		
	}*/
	//for (nat i = 0; i < thread_count; i++) atomic_init(global_progress + i, no_job_has_started);






	
		/*byte best_zv_in_2sp[28] = {
			0,  1, 1, 4,
			1,  0, 2, 5,
			2,  1, 3, 6,
			3,  1, 4, 1,
			4,  2, 4, 2,
			0,  4, 1, 4,
			0,  1, 4, 0,
		};

		byte best_end_graph[28] = {
			0,  1, 6, 6,
			1,  0, 6, 6,
			2,  6, 6, 6,
			3,  6, 6, 6,
			4,  6, 4, 2,
			4,  6, 6, 6,
			4,  6, 6, 6,
		};


		memcpy(graph, best_zv_in_2sp, graph_count);
		memcpy(end, best_end_graph, graph_count);

		*/


	
		/*


			if (best >= range_begin and best <= range_end) { ... } 
		*/
























		//if (e and not (e & ((1 << 28) - 1))) {
			//printf("[e = %9llu], o = %hhu z = ", e, origin); 
			//print_graph_raw(graph); puts(""); 
		//}












/*

#define total_job_count ...



space_size / total_job_count         width of every job ever            ever


static const nat machine0_counter_max = 1;
static const nat machine1_counter_max = 1;

nat machine0_counter = 0;
nat machine1_counter = 0;
nat machine_state = 0;

for (nat j = 0; j < total_job_count; j++) {

	nat mi;

	if (machine_state) {
		if (machine1_counter < machine0_counter_max) { machine1_counter++; mi = 1; } 
		else { machine_state = 0; machine0_counter = 1; mi = 0; }
	} else {
		if (machine0_counter < machine1_counter_max) { machine0_counter++; mi = 0; } 
		else { machine_state = 1; machine1_counter = 1; mi = 1; }
	}

	const nat c = j % (mi ? 64 : 10);
	machine[mi].core[c].jobs[job_count++] = (struct job){.range_begin = ..., .range_end = ... };
}


  --->   thread(machine[machine_index].core[c].jobs, job_count);










4000  ==>    2000   +   2000     ===>    64*30     +     200 * 10


64/10 = j/k = 30/200 = 0.15    




#define machine0_thread_count 64
#define machine1_thread_count 10

#define machine0_job_count_per_core 30
#define machine1_job_count_per_core 200


*/



// #define total_job_count (thread_count * job_count_per_core)







//const nat ti = job % thread_count;
		//cores[ti].jobs[cores[ti].job_count++] = (struct job) { .begin = begin_zv, .end = end_zv };







/*


			if (walk_ia_counter < (e < 500000 ? 3 : 6)) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;





static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, byte* zskip_at) {

#define max_rsi_count 512
#define max_oer_repetions 50
#define max_rmv_modnat_repetions 30
#define max_imv_modnat_repetions 80
#define max_consecutive_small_modnats 230
#define max_consecutive_s0_incr 30
#define max_consecutive_h0_bouts 12
#define max_consecutive_h1_bouts 24
#define max_consecutive_bld_walk_count 150
#define max_consecutive_bld_walk_count_small 30

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0,  pointer = 0,  
		bout_length = 0, 
		RMV_value = 0, 
		IMV_value = 0,
		OER_ier_at = 0,
		BDL_ier_at = 0,
		PER_ier_at = (nat) ~0;

	byte	H0_counter = 0,  H1_counter = 0, 
		OER_counter = 0, RMV_counter = 0, 
		IMV_counter = 0, CSM_counter = 0,
		BDL1_counter = 0, BDL2_counter = 0,
		BDL3_counter = 0, BDL4_counter = 0,
		BDL5_counter = 0, BDL6_counter = 0,
		BDL7_counter = 0, BDL8_counter = 0,
		BDL9_counter = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;

	byte rsi_counter[max_rsi_count];
	rsi_counter[0] = 0;

	for (nat e = 0; e < execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];


		

		if (op == one) {
			if (pointer == n) { 
				puts("FEA condition violated by a z value: "); 
				print_graph_raw(graph); 
				puts(""); 
				abort(); 
			}
  
			if (not array[pointer]) return pm_ns0; 
			if (last_mcal_op == one)  H0_counter = 0;

			if (pointer < max_rsi_count) { 
				if (last_mcal_op == three) {
					rsi_counter[pointer]++;
					if (rsi_counter[pointer] >= max_consecutive_s0_incr) return pm_rsi;
				} else rsi_counter[pointer] = 0;
			}

			bout_length++;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
				if (pointer < max_rsi_count) rsi_counter[pointer] = 0;
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (pointer == OER_ier_at or pointer == OER_ier_at + 1) {
				OER_counter++;
				if (OER_counter >= max_oer_repetions) return pm_oer;
			} else { OER_ier_at = pointer; OER_counter = 0; }
			
			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 8) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_rmv_modnat_repetions) return pm_rmv;
			}

			IMV_value = (nat) -1;
			IMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] == IMV_value + 1) { IMV_counter++; IMV_value++; } else { IMV_value = array[i]; IMV_counter = 0; }
				if (IMV_counter >= max_imv_modnat_repetions) return pm_imv;
			}

			IMV_value = (nat) -1;
			IMV_counter = 0;
			for (nat i = 0; i < xw; i += 2) { // note the "i += 2".
				if (array[i] == IMV_value + 1) { IMV_counter++; IMV_value++; } else { IMV_value = array[i]; IMV_counter = 0; }
				if (IMV_counter >= 2 * max_imv_modnat_repetions) return pm_imv;
			}


			if (pointer + 1 == BDL_ier_at) {
				BDL1_counter++; 
				if (BDL1_counter >= max_consecutive_bld_walk_count_small) return pm_bdl; 
			} else BDL1_counter = 0;

			if (pointer + 2 == BDL_ier_at) {
				BDL2_counter++; 
				if (BDL2_counter >= max_consecutive_bld_walk_count_small) return pm_bdl; 
			} else BDL2_counter = 0;

			if (pointer + 3 == BDL_ier_at) {
				BDL3_counter++; 
				if (BDL3_counter >= max_consecutive_bld_walk_count_small) return pm_bdl; 
			} else BDL3_counter = 0;

			if (	pointer     == BDL_ier_at or 
				pointer + 1 == BDL_ier_at or 
				pointer + 2 == BDL_ier_at or
				pointer + 3 == BDL_ier_at or
				pointer + 4 == BDL_ier_at
			) {
				BDL4_counter++; 
				if (BDL4_counter >= max_consecutive_bld_walk_count and e >= 500000) return pm_bdl; 
			} else BDL4_counter = 0;

			if (pointer + 5 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL5_counter++; 
				if (BDL5_counter >= max_consecutive_bld_walk_count and e >= 500000) return pm_bdl; 
			} else BDL5_counter = 0;

			if (pointer + 6 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL6_counter++; 
				if (BDL6_counter >= max_consecutive_bld_walk_count and e >= 500000) return pm_bdl; 
			} else BDL6_counter = 0;

			if (pointer + 7 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL7_counter++; 
				if (BDL7_counter >= max_consecutive_bld_walk_count and e >= 500000) return pm_bdl; 
			} else BDL7_counter = 0;

			if (pointer + 8 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL8_counter++; 
				if (BDL8_counter >= max_consecutive_bld_walk_count and e >= 500000) return pm_bdl; 
			} else BDL8_counter = 0;

			if (pointer + 9 == BDL_ier_at or pointer == BDL_ier_at) { 
				BDL9_counter++; 
				if (BDL9_counter >= max_consecutive_bld_walk_count and e >= 500000) return pm_bdl; 
			} else BDL9_counter = 0;


			BDL_ier_at = pointer;
			PER_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) {
			array[n]++;
		}

		else if (op == six) {  
			if (not array[n]) return pm_zr6;
			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three) return pm_ndi;

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (bout_length == 2) {
				H1_counter++;
				if (H1_counter >= max_consecutive_h1_bouts) return pm_h1; 
			} else H1_counter = 0;

			if (PER_ier_at != (nat) ~0) {
				if (pointer >= PER_ier_at) return pm_per; 
				PER_ier_at = (nat) ~0;
			}

			bout_length = 0;
			array[pointer]++;
		}
		if (op == three or op == one or op == five) last_mcal_op = op;
		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		if (*zskip_at > I + state) *zskip_at = I + state;
		ip = graph[I + state];
	}

	if (xw < 11) return pm_fse;
	for (nat i = 0; i < 10; i++)  if (array[i] < 20) return pm_fse;

	return z_is_good;
}

static byte execute_graph(byte* graph, nat* array, byte* origin, nat* counts) {
	byte at = graph_count;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three and graph[4 * o] != two) continue;
		const nat pm = execute_graph_starting_at(o, graph, array, &at);
		counts[pm]++;
		if (not pm) { *origin = o; return 0; }
	}
	return at;
}





      . . .         ....##.......##....#...##.............##........##...##.......##....     . . . 

		if (op == one)  { 
			if (pair_index == 1) pair_index = 2;
			else if (pair_index == 3) pair_index = 4;
			else if (pair_index == 4) { 
				pair_index = 0; 
				pair_count++; 
				if (pair_count >= max_consecutive_pairs) return pm_pair; 
			} 
			else if (pair_index) { pair_count = 0; pair_index = 0; }
			...
		}





	// 0144 1535 2133 3545 4242 0020

	// 014415352133354542420020

	byte z_graph[24] = {
		0,  1, 4, 4, 
		1,  5, 3, 5, 
		2,  1, 3, 1, 
		3,  5, 4, 5, 
		4,  2, 4, 2,
		0,  0, 2, 0,
	};

	byte zskip_at = 0;
	nat* array = calloc(array_size + 1, sizeof(nat));
	nat x = execute_graph_starting_at(2, z_graph, array, &zskip_at);
	printf("pm = %llu %s\n", x, pm_spelling[x]);
	exit(0);










*/





