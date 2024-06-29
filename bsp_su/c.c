// 202406086.154106: dwrr
//   the binary space partitioning  srnfgpr   version of the search utiltiy. 
//    designed to optimally split up the jobs in the search space   amoung a set of threads, 
//    while splitting up jobs the least, and incurring the least amount 
//     of sychronization between threads. 

// old 202403041.192520: by dwrr         
//      the prthead  cpu-parellelized version of the srnfgpr.
//
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

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

static const byte D = 1;        // the duplication count (operation_count = 5 + D)
static const byte R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

static const nat range_update_frequency = 0;
static const nat minimum_split_size = 6;

static const nat thread_count = 10;
static const nat display_rate = 0;

static const nat cache_line_size = 1;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good, pm_ga, 
	pm_fea, pm_ns0, 

	pm_pco, pm_zr5, 
	pm_zr6, pm_ndi, 

	pm_oer, pm_r0i, 
	pm_h0, pm_f1e, 

	pm_erc, pm_rmv, 
	pm_ot, pm_csm, 

	pm_mm, pm_snm, 
	pm_bdl, pm_bdl2, 

	pm_erw, pm_mcal, 
	pm_snl, pm_h1, 

	pm_h2, pm_h3, 
	pm_per, pmf_fea, 

	pmf_ns0, pmf_pco,
	pmf_zr5, pmf_zr6, 

	pmf_ndi, pmf_per, 
	pmf_mcal,

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
	"pm_ot", "pm_csm", 

	"pm_mm", "pm_snm", 
	"pm_bdl", "pm_bdl2", 

	"pm_erw", "pm_mcal", 
	"pm_snl", "pm_h1", 

	"pm_h2", "pm_h3", 
	"pm_per", "pmf_fea", 

	"pmf_ns0", "pmf_pco",
	"pmf_zr5", "pmf_zr6", 

	"pmf_ndi", "pmf_per", 
	"pmf_mcal",

};

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

static const byte initial = R ? _63R_hole_count : _36R_hole_count;

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const byte hole_count = initial + 4 * D;

static const nat fea_execution_limit = 5000;
static const nat execution_limit = 10000000000;
static const nat array_size = 100000;

static const byte max_er_repetions = 50;
static const byte max_erw_count = 100;
static const byte max_modnat_repetions = 15;
static const byte max_consecutive_s0_incr = 30;
static const byte max_consecutive_small_modnats = 200;
static const byte max_bdl_er_repetions = 25;
static const byte max_sn_loop_iterations = 100 * 2;
static const byte max_consecutive_h0_bouts = 10;
static const byte max_consecutive_h2_bouts = 30;
static const byte max_consecutive_h3_bouts = 30;

// static const byte max_consecutive_h1_bouts = 30;

static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;

static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;

// runtime constants:
static nat space_size = 0;
static byte* positions = NULL; 
static pthread_t* threads = NULL; 


// runtime variables:
static _Atomic nat* global_range_begin = NULL;
static _Atomic nat* global_range_end = NULL;
static pthread_mutex_t mutex;


static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

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
			if (not array[n]) return pm_zr6; 

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

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
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
		if (not pm) { *origin = o; return z_is_good; } 
	}
	return pm;
}

static nat fea_execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 
	byte ip = origin, last_mcal_op = 255, mcal_path = 0;
	nat pointer = 0, e = 0, xw = 0, mcal_index = 0;
	nat did_ier_at = (nat)~0;

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

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }

		if (mcal_index == 1  and last_mcal_op != three) return pmf_mcal; 
		if (mcal_index == 2  and last_mcal_op != one) 	return pmf_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pmf_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pmf_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pmf_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pmf_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pmf_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pmf_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pmf_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pmf_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pmf_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pmf_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good; 
}

static nat fea_execute_graph(byte* graph, nat* array) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = fea_execute_graph_starting_at(o, graph, array);
		if (not pm) return z_is_good;
	}
	return pm;
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

static void* worker_thread(void* raw_argument) {

	char filename[4096] = {0};
	nat* pms = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);
	byte* graph = raw_graph;
	byte* end = raw_end;
	nat* graph_64 = raw_graph;
	nat* end_64 = raw_end;
	memcpy(graph, R ? _63R : _36R, 20);
	memcpy(end, R ? _63R : _36R, 20);
	byte pointer = 0;
	nat publish_counter = 0;

	const nat thread_index = *(nat*) raw_argument;
	free(raw_argument);

	nat range_begin = atomic_load_explicit(global_range_begin + cache_line_size * thread_index, memory_order_relaxed);
	nat range_end = atomic_load_explicit(global_range_end + thread_index, memory_order_relaxed);

	//printf("worker_thread[%llu]: starting with job:  [range_begin=%llu, range_end=%llu]\n", thread_index, range_begin, range_end);
	
next_job:;
	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		graph[positions[i]] = (byte) ((range_begin / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}

	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	goto init;

loop:	for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
		if (graph_64[i] < end_64[i]) goto process;
		if (graph_64[i] > end_64[i]) break;
	}

done:
	//printf("worker_thread[%llu]: finished with job! waiting for mutex...[%llu : %llu]\n", thread_index, range_begin, range_end);
	pthread_mutex_lock(&mutex);

	nat largest_remaining = 0, chosen_thread = 0;

	for (nat thread = 0; thread < thread_count; thread++) {
		const nat b = atomic_load_explicit(global_range_begin + cache_line_size * thread, memory_order_relaxed);
		const nat e = atomic_load_explicit(global_range_end + thread, memory_order_relaxed);
		const nat size =  e >= b ? e - b : 0;
		if (size > largest_remaining) { largest_remaining = size; chosen_thread = thread; }
	}

	if (largest_remaining < minimum_split_size) {
		printf("worker_thread[%llu]: largest remaining (%llu) was less than minimum split size of %llu\n", thread_index, largest_remaining, minimum_split_size);
		pthread_mutex_unlock(&mutex);
		goto terminate_thread;
	}

	//printf("worker_thread[%llu]: performing a split: using thread #%llu, which had a largest remaining of %llu\n", thread_index, chosen_thread, largest_remaining);
	
	const nat subtract_off_amount = (largest_remaining >> 1);
	const nat job_ends_at = atomic_fetch_sub_explicit(global_range_end + chosen_thread, subtract_off_amount + 1, memory_order_relaxed);

	range_begin = job_ends_at - subtract_off_amount;
	range_end = job_ends_at;

	atomic_store_explicit(global_range_begin + cache_line_size * thread_index, range_begin, memory_order_relaxed);
	atomic_store_explicit(global_range_end + thread_index, range_end, memory_order_relaxed);

	pthread_mutex_unlock(&mutex);

	//printf("worker_thread[%llu]: performed a split, now processing this job:  [range_begin=%llu, range_end=%llu]\n", thread_index, range_begin, range_end);

	goto next_job;

process:
	if (graph[positions[pointer]] < (positions[pointer] & 3 ? operation_count - 1 : 4)) goto increment;
	if (pointer < hole_count - 1) goto reset_;
	goto done;

increment:
	graph[positions[pointer]]++;
init:  	pointer = 0;

	if (publish_counter >= range_update_frequency) {

		
		const nat local_range_end = atomic_load_explicit(global_range_end + thread_index, memory_order_relaxed);
		p = 1;
		for (nat i = 0; i < hole_count; i++) {
			end[positions[i]] = (byte) ((local_range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
			p *= (nat) (positions[i] & 3 ? operation_count : 5);
		}


		nat zindex = 0;
		p = 1;
		for (byte i = 0; i < hole_count; i++) {
			zindex += p * graph[positions[i]];
			p *= (nat) (positions[i] & 3 ? operation_count : 5);
		}
		atomic_store_explicit(global_range_begin + cache_line_size * thread_index, zindex, memory_order_relaxed);

		
		//printf("worker_thread[%llu]: updating begin and end,   pulled range_end=%llu, published range_begin=%llu]\n", thread_index, local_range_end, zindex);

		publish_counter = 0;

	} else publish_counter++;

	u16 was_utilized = 0;
	byte at = 0;


	for (byte index = 20; index < graph_count - 4; index += 4) {
		if (graph[index] > graph[index + 4]) { at = index + 4; goto bad; } // WRONG: was     at = index   which is incorrect. 
	}                                                                      //       don't edit this to get the original semantics.


	for (byte index = operation_count; index--;) {

		if (graph[4 * index + 3] == index) {  at = 4 * index + 3; goto bad; }
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  at = 4 * index; goto bad; }
		if (graph[4 * index] == six   and graph[4 * index + 2])          {  at = 4 * index; goto bad; }
		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  at = 4 * index + 2 * (index == two); goto bad; }
		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  at = 4 * index + 1 * (index == three); goto bad; }

		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == one) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == one) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == five) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == five) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad; 
		}
 
		const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

		if (graph[4 * index] == one and graph[4 * e] == one) {
			if (index == one) { at = 4 * index + 3; goto bad; }
			if (e == one) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad;
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == one) {
			if (index == five) { at = 4 * index + 1; goto bad; } 
			if (e == one) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == three) {
			if (index == five) { at = 4 * index + 1; goto bad; } 
			if (e == three) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == one)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == one) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { at = 4 * index; goto bad; }
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}
		
		if (graph[4 * index] == five) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == five) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == six) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) { 
					if (index == six) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == three) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == three) { 
					if (index == three) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == three) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}




		if (graph[4 * index] == six and graph[4 * l] == two and graph[4 * l + 1] == index) { 
			at = 4 * l; goto bad; 
		}


		
		if (l != index) was_utilized |= 1 << l;
		if (g != index) was_utilized |= 1 << g;
		if (e != index) was_utilized |= 1 << e;
	}

	for (byte index = 0; index < operation_count; index++) 
		if (not ((was_utilized >> index) & 1)) goto loop;
	goto try_executing;
bad:
	for (byte i = 0; i < hole_count; i++) {
		if (positions[i] == at) { pointer = i; goto loop; } else graph[positions[i]] = 0;
	}
	abort();
	
try_executing:;
	nat pm = fea_execute_graph(graph, array);       if (pm) { pms[pm]++; goto loop; } 
	byte origin;
	    pm = execute_graph(graph, array, &origin);  if (pm) { pms[pm]++; goto loop; } 

	pms[z_is_good]++; 
	append_to_file(filename, sizeof filename, graph, origin);

	printf("\n    FOUND:  z = "); 
	print_graph_raw(graph); 
	printf("\n"); 
	fflush(stdout);
        goto loop;

reset_:
	graph[positions[pointer]] = 0; 
	pointer++;
	goto loop;

terminate_thread:

	pthread_mutex_lock(&mutex);
	atomic_store_explicit(global_range_begin + cache_line_size * thread_index, 0, memory_order_relaxed);
	atomic_store_explicit(global_range_end   + thread_index, 0, memory_order_relaxed);
	pthread_mutex_unlock(&mutex);

	printf("worker_thread[%llu]: terminating thread!  (thread with filename=\"%s\")\n", thread_index, filename);
	free(raw_graph);
	free(raw_end);
	free(array);
	return pms;
}





		/*


				*n = 0
				*i = 4
				(*n)++;  -----[*n < *i]----->    *n = 0;   ------[*n < *i]----->   (*n)++;




				*n = 0
				*i = 1
				(*n)++;  -----[*n == *i]----->    *n = 0;   ------[*n < *i]----->   (*n)++;



				*n = 0
				*i = 0
				(*n)++;  -----[*n > *i]----->    *n = 0;   ------[*n == *i]----->   (*n)++;












			LA:		5		              6		                  7
--------------------------------------------------------------------------------------------------------------------------


		 ......	     DOL = {    1 : { .l=7 .g=X .e=X }       4 : { .l=X .g=X .e=X }      4 : { .l=5 .g=X .e=X }      }






				*n = 0
				*i = 4
				(*n)++;  -----[*n < *i]----->    *n = 0;   ------[*n < *i]----->   (*n)++;

		*/








static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
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
		snprintf(filename, size, "%s_D%hhuR%hhu_%08x%08x%08x%08x_output.txt", dt, D, R,
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

int main(void) {
	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};
	
	srand((unsigned)time(0)); rand();
	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	positions = calloc(hole_count, 1);
	threads = calloc(thread_count, sizeof(pthread_t));
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	snprintf(output_string, 4096, "SRNFGPR: searching [D=%hhu, R=%hhu] space....\n", D, R);
	print(output_filename, 4096, output_string);

	global_range_begin = calloc(1, cache_line_size * thread_count * sizeof(_Atomic nat));
	global_range_end = calloc(thread_count, sizeof(_Atomic nat));

	pthread_mutex_init(&mutex, NULL);

	const nat width = space_size / thread_count;
	nat begin = 0;
	for (nat i = 0; i < thread_count; i++) {
		atomic_init(global_range_begin + cache_line_size * i, begin);
		atomic_init(global_range_end + i, i < thread_count - 1 ? begin + width - 1 : space_size - 1);
		begin += width;
	}

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < thread_count; i++) {
		nat* thread_index = malloc(sizeof(nat));
		*thread_index = i;
		pthread_create(threads + i, NULL, worker_thread, thread_index);
	}

	nat* local_begin = calloc(thread_count, sizeof(nat));
	nat* local_end = calloc(thread_count, sizeof(nat));

	while (1) {

		for (nat i = 0; i < thread_count; i++) {
			local_begin[i] = atomic_load_explicit(global_range_begin + cache_line_size * i, memory_order_relaxed);
			local_end[i] = atomic_load_explicit(global_range_end + i, memory_order_relaxed);
		}
		
		nat largest_remaining = 0, chosen_thread = 0;

		for (nat i = 0; i < thread_count; i++) {
			if (local_begin[i] > local_end[i]) local_begin[i] = local_end[i];
			const nat size = local_end[i] - local_begin[i];
			if (size > largest_remaining) { largest_remaining = size; chosen_thread = i; }
		}

		snprintf(output_string, 4096, "\033[H\033[2J");
		print(output_filename, 4096, output_string);

		snprintf(output_string, 4096, "\n-----------------printing current job allocations (largest_remaining: %llu)-------------------\n", largest_remaining);
		print(output_filename, 4096, output_string);

		nat sum = 0;

		for (nat i = 0; i < thread_count; i++) {
			const nat size = local_end[i] - local_begin[i];
			sum += size;
		}

		snprintf(output_string, 4096, "\n\t%1.10lf%%\n\n", (double) (space_size - sum) / (double) space_size);
		print(output_filename, 4096, output_string);

		for (nat i = 0; i < thread_count; i++) { 

			snprintf(output_string, 4096, "  %c %020llu : %020llu :: ", i == chosen_thread ? '*' : ' ', local_begin[i], local_end[i]);
			print(output_filename, 4096, output_string);

			const nat diff = local_end[i] - local_begin[i];
			const nat zs_per_char = space_size / 360;
			const nat amount = diff / zs_per_char;

			for (nat j = 0; j < amount; j++) {
				snprintf(output_string, 4096, "#");
				print(output_filename, 4096, output_string);
			}
			snprintf(output_string, 4096, "\n");
			print(output_filename, 4096, output_string);
			
		}
		snprintf(output_string, 4096, "\n");
		print(output_filename, 4096, output_string);

		if (not largest_remaining) break;
		sleep(1 << display_rate);
	}

	nat counts[pm_count] = {0};
	for (nat i = 0; i < thread_count; i++) {
		nat* local_counts = NULL;
		pthread_join(threads[i], (void**) &local_counts);
		for (nat j = 0; j < pm_count; j++) counts[j] += local_counts[j];
		free(local_counts);
	}

	nat sum = 0;
	for (nat i = 0; i < pm_count; i++) sum += counts[i];
	counts[pm_ga] = space_size - sum;

	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec), zthroughput = (double) space_size / seconds;
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	snprintf(output_string, 4096, "using [D=%hhu, R=%hhu]:"
			"\n\tspace_size=%llu"
			"\n\tthread_count=%llu"
			"\n\tminimum_split_size=%llu"
			"\n\trange_update_frequency=%llu"
			"\n\tdisplay_rate=%llu"
			"\n\tfea_execution_limit=%llu"
			"\n\texecution_limit=%llu"
			"\n\tarray_size=%llu"
			"\n\n"
			"\n\t""searched %llu zvs"
			"\n\tusing %llu threads"
			"\n\tin %10.2lfs [%s:%s],"
			"\n\tat %10.2lf z/s."
			"\n\n\npm counts:\n", 

			D, R,   space_size,  thread_count,  
			minimum_split_size,  range_update_frequency, display_rate,
			fea_execution_limit,  execution_limit,  array_size,  space_size, 
			thread_count,  seconds,  time_begin_dt,  time_end_dt,  zthroughput
	);

	print(output_filename, 4096, output_string);
	for (nat i = 0; i < pm_count; i++) {
		if (i and not (i % 2)) {
			snprintf(output_string, 4096, "\n");
			print(output_filename, 4096, output_string);
		}
		snprintf(output_string, 4096, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
		print(output_filename, 4096, output_string);
	}
	snprintf(output_string, 4096, "\n[done]\n");
	print(output_filename, 4096, output_string);
}
 

































/*
202405293.180937:

implementing the binary space partition (BSP) opt:   a solution to 
	parellelizing while maining sequential executeion 
		as much as posisble for z skips. 


	the implementation goes as follows:


			first partition the space into 64 parts, each one given to a worker thread, 

				run each one using the sr,      supplying it a range_begin and range_end   

					both of which are global and atomic, 

						on initial execution:

						the thread reads the range begin, inits graph to it via a reduce()


						the thread reads range_end and inits end[] to it, via a reduce()

					the graph then does the standard loop containing ga, fea execute, and execute, 


					every 100th (or so) iteration,   we will "publish the current graph state, 

						via this code:





					nat zindex = 0;
					p = 1;
					for (byte i = 0; i < hole_count; i++) {
						zindex += p * graph[positions[i]];
						p *= (nat) (positions[i] & 3 ? operation_count : 5);
					}
					atomic_store_explicit(&range_begin[this_thread], zindex, memory_order_relaxed);
				


				



				ie, we are doing a unreduce on the graph, to get a zindex  and then we are storing that zi into our thread's global atomic range_begin state.   ie, we loose the original value of range_begin, set by main.  we don't need it anymore. 

				


				additionally, after updating  or "publishing"   our progress, via range_begin, 


				we update our local copy of    end[]  to reflect the current state of range_end   which might have been edited by other worker threads, from them taking half of our job's z values. 



				to do this, we just use this code:








				const nat range_end = atomic_load_explicit(global__range_end, memory_order_relaxed);
				p = 1;
				for (nat i = 0; i < hole_count; i++) {
					end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
					p *= (nat) (positions[i] & 3 ? operation_count : 5);
				}
				








				so end    and thus end_64      are now reflecting the true values   


					ie     in this case,   the global atomic values     global__range_end, are the master copy, 


					but in the case of range_begin,    the local_copies are master copies,   


						and synch'ing up again involves ovewrwritting the non-master copy with the master copy, of course


					
				



		so once all of this in place, 

			now the binary space partitoning part   comes in-

				when a thread finishes, it simply checks the global_range_begin  and global_range_end   arrays, 


			and searches it and finds the i such that    global_range_end[i] - global_range_begin[i]   is the largest



				and then it takes     that difference    divides it in half,   and then subtracts that much from global_range_end[i]



						this is the thread taking half of the work from the thread that has the least amount done!



				thus binary partitioning the search space!    pretty cool 





		then, the thread continues with this job-  given by simply:    

		local_range_begin = global_range_end[i]     (but after the subtraction was performed)
		
		local_range_end = global_range_end[i]       (but BEFORE  the subtraction was performed)



		then the process starts over, using those local range begin and end. 





















publish_begin() {

	nat zindex = 0;
	p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	atomic_store_explicit(&global_range_begin[thread_index], zindex, memory_order_relaxed);
}



pull_end() {

	const nat range_end = atomic_load_explicit(global_range_end[thread_index], memory_order_relaxed);
	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
				
}





main


	pthread_mutex_init(&mutex, 0);





	
worker_thread() {

start_first_job:

	// load range_begin and range_end   
	// from global arrays  at thread index (TI)

	nat range_begin = atomic_load(global_range_begin[thread_index], mor);
	nat range_end = atomic_load(global_range_end[thread_index], mor);


next_job:
	...reduce range_begin and range_end   to be graph   and graph_end  (end)...
	// now we are ready to start the job

	loop:

		
		check if job is done, (ie graph is at end) if so goto done

	init:
		if (publish_counter >= 500) {

			publish_begin();
			pull_end();

			publish_counter = 0;
		} else publish_counter++;
		...do job...

		goto loop;

	done:

		pthread_mutex_lock(&mutex);

		nat max_diff = 0;
		nat max_ti = 0;
		for (each thread index   ti < 64) {
			const nat b = atomic_load(global_range_begin[ti], mor);
			const nat e = atomic_load(global_range_end[ti], mor);
			const nat diff =  e - b;
			if (diff > max_diff) { max_diff = diff; max_ti = ti; }
		}

		if (max_diff < minimum_split_size) goto terminate;
		
		const nat amount = (max_diff >> 1);
		const nat e = atomic_fetch_sub(global_range_end[max_ti], amount + 1, mor);
		pthread_mutex_unlock(&mutex);
		range_begin = e - amount;
		range_end = e;
		goto next_job;
terminate:
	...
	pthread_exit(0);
	free(...);
}










[ 0 ... N )     [ N ... W )                W = 10


bob's range_begin = 0
bob's range_end = 9


          2 3 4 5     6     9
      D D X X X X     X X X X 
          ^







bob:
       XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX P XXXXXXXXXXX......M..............
							     |
      zi=0                             zi=1000              -|-          zi=5000
						             |
						             |
					         joe:         XXXXXXXXXXXXXX









*/





























































































/*


	// TODO:  use "memory_order_relaxed" !?!?!?!?!?!?!??!?????
				// CHANGED:   yup, its correct i think.











static nat execute_graph_starting_at(byte origin) {


}










static nat execute_graph(void) {
	
}

static nat fea_execute_graph_starting_at(byte origin) {

	
}

static nat fea_execute_graph(void) {

}












*/

























			// update progress bar here!!!   // ie, atomic load of "head".
			// (would detect if head is >= spacesize, and then start the joining process, if it is.
















/*




static void write_graph(nat b, nat e) {

	get_datetime(buffer[buffer_count].dt);
	get_graphs_z_value(buffer[buffer_count].z);
	buffer_count++;

	if (buffer_count == max_buffer_count) {
		append_to_file(b, e);
		buffer_count = 0;
	}
}










	//for (byte i = 0; i < graph_count; i += 4) {
	//
	//	if (	executed[i + 1] < 5 and graph[i + 1] or         // slightly concerning here... think about this more...? 
	//		executed[i + 2] < 5 and graph[i + 2] or 
	//		executed[i + 3] < 5 and graph[i + 3]) { a = pm_eda; goto bad; }
	//}



///////////////
//	if (not (display_counter & ((1 << display_rate) - 1))) { print_graph_raw(); putchar(10); fflush(stdout); }
//	display_counter++;
//	nat zindex = 0;
//	nat p = 1;
//	for (byte i = 0; i < hole_count; i++) {
//		zindex += p * graph[positions[i]];
//		p *= (nat) (positions[i] & 3 ? operation_count : 5);
//	}
////////////////










x	array = calloc(array_size + 1, sizeof(nat));
x	timeout = calloc(operation_count, sizeof(nat));

x	positions = calloc(hole_count, 1); 
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);

	graph = raw_graph;
	end = raw_end;
	uint64_t* graph_64 = raw_graph;
	uint64_t* end_64 = raw_end;
	nat display_counter = 0, found = 0;
	byte pointer = 0;

	memcpy(graph, R ? _63R : _36R, 20);
	memcpy(end, R ? _63R : _36R, 20);


	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		graph[positions[i]] = (byte) ((range_begin / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_begin >= p) { puts("range_begin is too big!"); printf("range_begin = %llu, p = %llu\n", range_begin, p); abort(); }

	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_end >= p) { puts("range_end is too big!"); printf("range_end = %llu, p = %llu\n", range_end, p); abort(); }

	
	goto init;










	if (argc != 2) { printf("./pth-srnfgpr [D=%hhu][R=%hhu] <threadcount:nat>\n", D, R); exit(0); }

	char* threadcount_invalid = NULL;
	const nat threadcount = strtoull(argv[1], &threadcount_invalid, 10);
	if (*threadcount_invalid) {
		printf("ERROR: error parsing threadcount near \"%s\" aborting...\n", threadcount_invalid);
		abort();
	}




// printf("sleeping for %5.2lfs until:\n\n\talarm: %s\n\tnow:   %s\n\n", difference, alarm_spelling, now_spelling);





static volatile int quit = 0;
static noreturn void handler(int __attribute__((unused))_) {
	puts("\ncanceled alarm.");
	quit = 1;
	abort();
}



int main(int argc, const char** argv) {
	if (argc < 2) return puts("usage: ./alarm <string> <datetime:1%Y%m%d%u.%H%M%S>");

	struct sigaction action = {.sa_handler = handler}; 
	sigaction(SIGINT, &action, NULL);

	const char* purpose = argv[1];
	const char* alarm_string = argv[2];
	char now_spelling[32] = {0};
	char alarm_spelling[32] = {0};

	struct tm alarm_details = {0};
	strptime(alarm_string, "1%Y%m%d%u.%H%M%S", &alarm_details);
	struct timeval the_alarm = { .tv_sec = timelocal(&alarm_details), .tv_usec = 0 };
	
	struct timeval now;
	gettimeofday(&now, NULL);
	const double difference = difftime(the_alarm.tv_sec, now.tv_sec);

	strftime(alarm_spelling, 32, "1%Y%m%d%u.%H%M%S", localtime(&the_alarm.tv_sec));
	strftime(  now_spelling, 32, "1%Y%m%d%u.%H%M%S", localtime(&now.tv_sec));
	printf("sleeping for %5.2lfs until:\n\n\talarm: %s\n\tnow:   %s\n\n", difference, alarm_spelling, now_spelling);

	puts("\tasleep...");
	sleep((unsigned) difference);

	gettimeofday(&now, NULL);
	strftime(now_spelling, 32, "1%Y%m%d%u.%H%M%S", localtime(&now.tv_sec));
	printf("\n\n\t\t[ \033[1;31mALARM\033[0m ] : \033[1m %s \033[0m : \033[1m %s \033[0m\n\n", purpose, now_spelling);
	while (not quit) { system("afplay bell.wav"); sleep(1); }

}













typedef long long nat;

static nat thread_count = 9;
static const nat job_count = 1000000;

struct thread_arguments {
	nat index;
};

static pthread_mutex_t input_mutex;
static pthread_mutex_t output_mutex;

static _Atomic nat output_count = 0;
static _Atomic nat input_count = 0;

static nat* output = NULL; 
static nat* input = NULL;


int main(int argc, const char** argv) {
	if (argc == 1) return puts("give the thread count as an argument!");

	thread_count = atoi(argv[1]);

	srand(42);

	//clock_t begin = clock();

	pthread_mutex_init(&output_mutex, NULL);
	pthread_mutex_init(&input_mutex, NULL);

	input = calloc(job_count, sizeof(nat));
	output = calloc(job_count, sizeof(nat));
	
	for (nat i = 0; i < job_count; i++) 
		input[input_count++] = (nat) (rand() % 400);
	
	pthread_t* thread = calloc((size_t) thread_count, sizeof(pthread_t));

	for (nat i = 0; i < thread_count; i++) {
		struct thread_arguments* args = calloc(1, sizeof(struct thread_arguments));
		args->index = i;
		pthread_create(thread + i, NULL, worker_thread, args);
	}

	for (nat i = 0; i < thread_count; i++) 
		pthread_join(thread[i], NULL);

	printf("info: number of jobs completed = %llu\n", output_count);

	pthread_mutex_destroy(&output_mutex);
	pthread_mutex_destroy(&input_mutex);

	free(input);
	free(output);

	//clock_t end = clock();
	//double total_time = (double)(end - begin) / CLOCKS_PER_SEC;
	//printf("\t time for %llu threads:   %10.10lfs\n", thread_count, total_time);
}








// atomic_fetch_add_explicit(&acnt, 1, memory_order_relaxed);










































static void append_to_file(char* filename, size_t sizeof_filename, byte* graph) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, graph);
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;

try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("filename=%s ", newfilename);
			close(dir); 	
			123456_ERROR:   DO SOMETHING TO FIX THIS!?!!?!
			abort();
		}
		char created_at[32] = {0};
		get_datetime(created_at);
		snprintf(filename, sizeof_filename, "%s_%08x%08x_z.txt", 
			created_at, arc4random(), arc4random()
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
	printf("[%s]: write: %s z = %s to file \"%s\"\n", 
		dt, permission ? "created" : "wrote", z, filename
	);
}














static void write_file(const char* directory, char* name, size_t maxsize) {
	int flags = O_WRONLY | O_TRUNC;
	mode_t permission = 0;
	if (not *name) {
		srand((unsigned)time(0)); rand();
		char datetime[32] = {0};
		struct timeval t = {0};
		gettimeofday(&t, NULL);
		struct tm* tm = localtime(&t.tv_sec);
		strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);
		snprintf(name, maxsize, "%s%s_%08x%08x.txt", directory, datetime, rand(), rand());
		flags |= O_CREAT | O_EXCL;
		permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	}
	int file = open(name, flags, permission);
	if (file < 0) { perror("save: open file"); puts(name); getchar(); }
	write(file, text, count);
	close(file);
}










char newfilename[4096] = {0};
	strncpy(newfilename, filename, sizeof newfilename);
	const int dir = open(directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", directory); 
		return; 
	}




//     srand((unsigned)time(0)); rand();





char datetime[32] = {0};
		struct timeval t = {0};
		gettimeofday(&t, NULL);
		struct tm* tm = localtime(&t.tv_sec);
		strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);





perror("write openat file");
		printf("filename=%s\n", newfilename);






	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu_z.txt", dt, b, e);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strncpy(filename, newfilename, sizeof filename);

	close(dir);








*/

















/*static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], counts[i]);
	}
	puts("");
	puts("[done]");
}*/









// if (not array[pointer])    { a = PM_ne0; goto bad; }     // delete me!!!!     redundant becuaes of pco.

/*










for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not fea_execute_graph_starting_at(o, graph, array)) return false;
	}
	return true;
















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
				if (array[i] < required_s0_increments) return true; 
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return true; 
		}
		
		const byte I = ip * 4, op = graph[I];

		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return true; 
			timeout[i]++;
		}
		timeout[ip] = 0;

		if (op == one) {
			if (pointer == n) return true; 
			if (not array[pointer]) return true; 

			if (last_mcal_op == one) H_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;

			pointer++;

			if (pointer > xw and pointer < n) {    // <--------- ALSO CHANGED THIS ONE TOO.
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return true; 
			if (not pointer) return true; 
				
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_acceptable_er_repetions) return true; 
			
			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_acceptable_consequtive_small_modnats) return true; 
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_acceptable_modnat_repetions) return true; 
			}

			pointer = 0;
			er_count++;
		}

		else if (op == two) {
			array[n]++;
			if (array[n] >= 65535) return true; 
		}

		else if (op == six) {  
			if (not array[n]) return true; 
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return true; 

			if (last_mcal_op == one) {
				H_counter++;
				if (H_counter >= max_acceptable_run_length) return true; 
			}

			if (last_mcal_op == five) {
				R0I_counter++; 
				if (R0I_counter >= max_acceptable_consecutive_s0_incr) return true; 
			}

			if (array[pointer] >= 65535) return true; 
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return false;








for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not execute_graph_starting_at(o, graph, array, timeout)) return false;
	}
	return true;














	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 

	byte ip = origin, last_mcal_op = 0;
	nat pointer = 0, e = 0, xw = 0;

	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return true;
			if (not array[pointer]) return true;
			pointer++;

			// new correct lazy zeroing:

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}


			// WAS:  if (pointer > xw) { xw = pointer; array[pointer] = 0; }   // ERROR HERE!!!!
			
			

			/////////////////////////////////////////////////////////////////////////////////////////
			

				//  WHAT IF pointer ALIAS's   STAR N!!!!!!!   CRAPPP

				// we'll reset it, and thus change the graphs behavior!!!!





					// we had added the lazy zeroing opt to the fea pass,  and thus the "i == n" alias condition will occur MUCH more likely!!!

			/////////////////////////////////////////////////////////////////////////////////////////




		}

		else if (op == five) {
			if (last_mcal_op != three) return true;
			if (not pointer) return true;
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return true;
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return true;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return false; 








*/




//if (executed[I + state] < 253) executed[I + state]++;







// TODO:  use "memory_order_relaxed" !?!?!?!?!?!?!??!?????
						// CHANGED: yup, we can just use memory_order_relaxed. 





// only open part of the multithreading-zskip solution that we devised:    the sychronization part-
// we need this to COME BEFORE the atomic_fetch_add_explicit() call. 
// (using better  memory orderings  probably???...)






// {0 1 3 [4} ]
	// byte previous_op = graph[20];
	// make this not use this temporary variable, by using   index and index + 4   
	// (except if index+4==graphcount, then we will  just say its index.. yeah)

	// constructive GA here





     // here, we know we are doing a zskip, becuase we are over the end.




/*
if (range_begin >= p) { puts("range_begin is too big!"); printf("range_begin = %llu, p = %llu\n", range_begin, p); abort(); } // delete me?
	if (range_end >= p) { puts("range_end is too big!"); printf("range_end = %llu, p = %llu\n", range_end, p); abort(); }  // delete me?
*/




/*	nat zindex = 0;
	p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}

	atomic_store_explicit(&head, zindex, memory_order_relaxed);














const nat h = atomic_fetch_add_explicit(&head, job_size, memory_order_relaxed);
	if (h >= space_size) goto terminate_thread;

// if (range_end >= space_size) range_end = space_size - 1; // is this neccessary now?...







if (h >= space_size) {
			snprintf(output_string, 4096, "info: [all jobs allocated to threads. waiting for them to finish.]\n");
			print(output_filename, 4096, output_string);
			break;
		}





*/


// memset(timeout, 0, operation_count * sizeof(nat));



		/*for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return pm_ot; 
			timeout[i]++;
		}
		timeout[ip] = 0;*/



			/*if (bout_length == 1) {
				H1_counter++;
				if (H1_counter >= max_consecutive_h1_bouts) return pm_h1; 
			} else H1_counter = 0;*/

