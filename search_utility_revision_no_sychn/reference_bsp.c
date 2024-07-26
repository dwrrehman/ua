
// reference    cleaned out   bsp code   for reference to make the su revision  with no synchron. 

/*

202407184.193355:

	 we want/required in the su:


	x	- zero synchronization between workker threads

			- - we need to find some alternative to bsp, which uses a lottt of synchronization!




	x	- low memory usage 

			- - no writing millions of z values to a file or to a memory array

		



	x	- no z indicies, or 128 or 64bit math     (64bit is fine for pm counts or something not that important lol)

		 	 - - use nf increments on the MSB bits     to      "modulo partition"   the space    
					and then give each subspace to a worker_thread / core   to process.




	x	- more efficient application of GA across all NF increments, 

			- - make GA and NF completely intermingled and completely merged, so there is no distinctifying them. 


*/

	


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

static const byte D = 2;        // the duplication count (operation_count = 5 + D)
static const byte R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

// static const nat range_update_frequency = 0;

static const nat minimum_split_size = 6;

static const nat thread_count = 6;
static const nat display_rate = 0;

static const nat cache_line_size = 100;

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
	pm_per, pm_snco,

	pmf_fea, pmf_ns0, 
	pmf_pco, pmf_zr5, 

	pmf_zr6, pmf_ndi, 
	pmf_per, pmf_mcal, 

	pmf_snco, pm_r1i,

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
	"pm_per", "pm_snco",

	"pmf_fea", "pmf_ns0", 
	"pmf_pco", "pmf_zr5", 

	"pmf_zr6", "pmf_ndi", 
	"pmf_per", "pmf_mcal", 

	"pmf_snco", "pm_r1i",
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
static const byte max_consecutive_s1_incr = 30;
static const byte max_consecutive_small_modnats = 200;
static const byte max_bdl_er_repetions = 25;
static const byte max_sn_loop_iterations = 100 * 2;
static const byte max_consecutive_h0_bouts = 10;
static const byte max_consecutive_h2_bouts = 30;
static const byte max_consecutive_h3_bouts = 30;

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

static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, nat* max_pointer) {



h1
ot
mm
snl
bdl2
bdl
erw
csm
snco





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

	byte 	mcal_path = 0, R1I_counter = 0,
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
			
			if (pointer == 1) {
				if (last_mcal_op == three) {
					R1I_counter++;
					if (R1I_counter >= max_consecutive_s1_incr) return pm_r1i;
				} else R1I_counter = 0;
			}

			bout_length++;
			pointer++;

			if (pointer > *max_pointer) *max_pointer = pointer;

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
			// if (array[n] >= 65535) { *out_e = e; return pm_snm; }

			if (last_op == six) SNL_counter++; 
			else if (last_op != two) SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;

			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6;
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

static nat execute_graph(byte* graph, nat* array, byte* origin, nat* max_pointer) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		//nat e = 0;
		pm = execute_graph_starting_at(o, graph, array, max_pointer);
		//if (e > max_e[pm]) max_e[pm] = e;
		if (not pm) { *origin = o; return z_is_good; } 
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


	nat max_pointer = 0;

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

	
	const nat subtract_off_amount = (largest_remaining >> 1);
	const nat job_ends_at = atomic_fetch_sub_explicit(global_range_end + chosen_thread, subtract_off_amount + 1, memory_order_relaxed);

	range_begin = job_ends_at - subtract_off_amount;
	range_end = job_ends_at;

	atomic_store_explicit(global_range_begin + cache_line_size * thread_index, range_begin, memory_order_relaxed);
	atomic_store_explicit(global_range_end + thread_index, range_end, memory_order_relaxed);

	pthread_mutex_unlock(&mutex);
	goto next_job;

process:
	if (graph[positions[pointer]] < (positions[pointer] & 3 ? operation_count - 1 : 4)) goto increment;
	if (pointer < hole_count - 1) goto reset_;
	goto done;

increment:
	graph[positions[pointer]]++;
init:  	pointer = 0;

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

	u16 was_utilized = 0;
	byte at = 0;


	for (byte index = 20; index < graph_count - 4; index += 4) {
		if (graph[index] > graph[index + 4]) { at = index + 4; goto bad; } 
	}

	for (byte index = operation_count; index--;) {

		if (graph[4 * index + 3] == index) {  
			at = 4 * index + 3; goto bad; 
		}
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  
			at = 4 * index; goto bad; 
		}
		if (graph[4 * index] == six   and graph[4 * index + 2]) {  
			at = 4 * index; goto bad; 
		}
		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  
			at = 4 * index + 2 * (index == two); goto bad; 
		}
		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  
			at = 4 * index + 1 * (index == three); goto bad; 
		}		

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

		if (graph[4 * index] == two)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) {
					if (index == two) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { at = 4 * index; goto bad; }
					at = 4 * (index < tohere ? index : tohere); goto bad; 
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
	byte origin;
	nat pm = execute_graph(graph, array, &origin, &max_pointer);
	if (pm) { pms[pm]++; goto loop; } 

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
	printf("worker_thread[%llu]: max_pointer = %llu\n", thread_index, max_pointer);
	free(raw_graph);
	free(raw_end);
	free(array);
	return pms;
}


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

	nat resolution = 360;

	while (1) {

		byte printed_one_char = 0;

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
		printf("%s", output_string);

		snprintf(output_string, 4096, "\n-----------------printing current job allocations (largest_remaining: %llu)-------------------\n", largest_remaining);
		printf("%s", output_string);

		nat sum = 0;

		for (nat i = 0; i < thread_count; i++) {
			const nat size = local_end[i] - local_begin[i];
			sum += size;
		}

		snprintf(output_string, 4096, "\n\t%1.10lf%%\n\n", (double) (space_size - sum) / (double) space_size);
		print(output_filename, 4096, output_string);

		for (nat i = 0; i < thread_count; i++) { 

			snprintf(output_string, 4096, "  %c %020llu : %020llu :: ", i == chosen_thread ? '*' : ' ', local_begin[i], local_end[i]);
			printf("%s", output_string);

			const nat diff = local_end[i] - local_begin[i];
			const nat zs_per_char = space_size / resolution;
			const nat amount = diff / zs_per_char;

			for (nat j = 0; j < amount; j++) {
				snprintf(output_string, 4096, "#");
				printf("%s", output_string);
				printed_one_char = 1;
			}
			snprintf(output_string, 4096, "\n");
			printf("%s", output_string);
		}

		if (not printed_one_char) resolution *= 512;
		if (resolution > space_size) resolution = space_size;
		
		snprintf(output_string, 4096, "\n");
		printf("%s", output_string);

		if (not largest_remaining) break;
		sleep(1 << display_rate);
	}

	nat counts[pm_count] = {0};
	
	for (nat i = 0; i < thread_count; i++) {
		nat* local_counts = NULL;
		pthread_join(threads[i], (void**) &local_counts);

		for (nat j = 0; j < pm_count; j++) {
			counts[j] += local_counts[j];
		}
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
			"\n\tcache_line_size=%llu"
			"\n\tminimum_split_size=%llu"
			
			"\n\tdisplay_rate=%llu"
			"\n\tfea_execution_limit=%llu"
			"\n\texecution_limit=%llu"
			"\n\tarray_size=%llu"
			"\n\n"
			"\n\t""searched %llu zvs"
			"\n\tusing %llu threads"
			"\n\tin %10.2lfs [%s:%s],"
			"\n\tat %10.2lf z/s."
			"\n\n", 

			D, R,   space_size,  thread_count,   cache_line_size,
			minimum_split_size,  display_rate,
			fea_execution_limit,  execution_limit,  array_size,  space_size, 
			thread_count,  seconds,  time_begin_dt,  time_end_dt,  zthroughput
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
	snprintf(output_string, 4096, "\n[done_pm]\n");
	print(output_filename, 4096, output_string);
}
 

