// 202407265.081733: dwrr     revised version of the su, 
//  came from bsp,   to make it not have any synchr. 
//   uses a modulo partition to break up the search space in 
//   a way that still gives us a good effective parellelization factor. 
//
//   also changes up th way we do ga and nf to be more efficient.


/*





snco ? 
erw ? 
csm ? 
h3 ? 

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
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t chunk;
#define chunk_count 2

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

static const byte D = 2;
static const byte R = 0;

static const nat thread_count = 6;
static const nat job_count_per_core = 30;

static const nat stage1_execution_limit = 100000; // 1 million

static const nat stage2_execution_limit = 0;

static const nat array_size = 100000;

static const nat update_rate = 3;
static const nat display_rate = 0;

enum operations { one, two, three, five, six };

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

static byte* positions = NULL; 
static pthread_t* threads = NULL; 

static _Atomic nat* global_progress = NULL;

static const nat job_count = thread_count * job_count_per_core;

struct job {
	byte* begin;
	byte* end;
};

struct joblist {
	struct job* jobs;
	nat job_count;
	nat thread_index;
};

// static void print_graph(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

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

static nat execute_graph(byte* graph, nat* array, byte* origin, nat* pms) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		const nat pm = execute_graph_starting_at(o, graph, array);
		pms[pm]++;
		if (not pm) { *origin = o; return z_is_good; } 
	}
	return 1;
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
	byte pointer = 0;

	nat display_counter = 0;

	const struct joblist list = *(struct joblist*) raw_argument;
	const nat thread_index = list.thread_index;
	const nat count = list.job_count;
	const struct job* jobs = list.jobs;

	for (nat job_index = 0; job_index < count; job_index++) {



		if (not (display_counter & ((1 << update_rate) - 1))) {
			display_counter = 0;
			//printf("worker[%llu]: processing  %llu / %llu \n", 
			//	thread_index, job_index, count
			//);
			atomic_store_explicit(global_progress + thread_index, job_index, memory_order_relaxed);
			
		} else display_counter++;

		memcpy(graph, jobs[job_index].begin, graph_count);
		memcpy(end, jobs[job_index].end, graph_count);
		goto init;

	loop:	for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
			if (graph_64[i] < end_64[i]) goto process;
			if (graph_64[i] > end_64[i]) break;
		}
		goto next_job;

	process:
		if (graph[positions[pointer]] < (positions[pointer] & 3 ? operation_count - 1 : 4)) goto increment;
		if (pointer < hole_count - 1) goto reset_;
		goto next_job;

	increment:
		graph[positions[pointer]]++;
	init:  	pointer = 0;
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

			if (graph[4 * index] == one and graph[4 * e] == one) {    // DELETE THIS!?!?!?! soon!!
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
		if (execute_graph(graph, array, &origin, pms)) goto loop;

		// TODO:   push this good z value to a candidates array, for stage2 to process later. 
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
	next_job:
		continue;
	}

	printf("worker_thread[%llu]: terminating thread!\n", thread_index);
	
	free(raw_graph);
	free(raw_end);
	free(array);
	return pms;
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

	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};

	global_progress = calloc(1, thread_count * sizeof(_Atomic nat));
	for (nat i = 0; i < thread_count; i++)  atomic_init(global_progress + i, 0);

	srand((unsigned) time(0));
	threads = calloc(thread_count, sizeof(pthread_t));
	positions = calloc(hole_count, 1);
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	const nat K = expn(5 + D, 3);
	nat total[chunk_count] = {0};
	total[0] = expn(5 + D, 9);

	for (nat _i = 0; _i < D; _i++) {
		mul_small(total, 5);
		mul_small(total, K);
	}

	debug_zi("space_size", total);

	nat divisor[chunk_count] = { [0] = job_count };
	nat q[chunk_count] = {0};
	nat r[chunk_count] = {0};
	divide(q, r, total, divisor);

	nat n1[chunk_count] = {0};
	bitwise_invert(n1);
	nat width[chunk_count] = {0};
	memcpy(width, q, chunk_count * sizeof(nat));
	nat width_m1[chunk_count] = {0};
	memcpy(width_m1, width, chunk_count * sizeof(nat));
	adc(width_m1, n1, 0);
	nat last_zi[chunk_count] = {0};
	memcpy(last_zi, total, chunk_count * sizeof(nat));
	adc(last_zi, n1, 0);

	nat begin[chunk_count] = {0};

	struct joblist* cores = calloc(thread_count, sizeof(struct joblist));
	for (nat core = 0; core < thread_count; core++) {
		cores[core].jobs = calloc(job_count_per_core, sizeof(struct job));
		cores[core].thread_index = core;
	}

	snprintf(output_string, 4096, "SU: partitioning jobs from space....\n");
	print(output_filename, 4096, output_string);

	for (nat job = 0; job < job_count; job++) {

		nat range_begin[chunk_count] = {0};
		memcpy(range_begin, begin, sizeof(nat) * chunk_count);

		nat sum[chunk_count] = {0};
		memcpy(sum, begin, chunk_count * sizeof(nat));
		adc(sum, width_m1, 0);

		nat range_end[chunk_count] = {0};
		memcpy(range_end, job < job_count - 1 ? sum : last_zi, sizeof(nat) * chunk_count);

		adc(begin, width, 0);

		byte* begin_zv = calloc(graph_count, 1);
		memcpy(begin_zv, R ? _63R : _36R, 20);

		nat p_begin[chunk_count] = { [0] = 1 };
		for (nat i = 0; i < hole_count; i++) {
			const nat radix = (nat) (positions[i] & 3 ? operation_count : 5);
			nat radix_mp[chunk_count] = { [0] = radix };
			nat div1[chunk_count] = {0};
			nat rem2[chunk_count] = {0};
			divide(div1, r, range_begin, p_begin);
			divide(q, rem2, div1, radix_mp);
			begin_zv[positions[i]] = (byte) *rem2;
			mul_small(p_begin, radix);
		}

		byte* end_zv = calloc(graph_count, 1);
		memcpy(end_zv, R ? _63R : _36R, 20);

		nat p_end[chunk_count] = { [0] = 1 };
		for (nat i = 0; i < hole_count; i++) {
			const nat radix = (nat) (positions[i] & 3 ? operation_count : 5);
			nat radix_mp[chunk_count] = { [0] = radix };
			nat div1[chunk_count] = {0};
			nat rem2[chunk_count] = {0};
			divide(div1, r, range_end, p_end);
			divide(q, rem2, div1, radix_mp);
			end_zv[positions[i]] = (byte) *rem2;
			mul_small(p_end, radix);
		}

		const nat ti = job % thread_count;
		cores[ti].jobs[cores[ti].job_count++] = (struct job) { .begin = begin_zv, .end = end_zv };
	}

	for (nat i = 0; i < thread_count; i++) {
		printf("core #%llu job list: (%llu jobs): \n", i, cores[i].job_count);
		for (nat j = 0; j < cores[i].job_count; j++) {
			printf("[%llu] = (", j);
			print_graph_raw(cores[i].jobs[j].begin);
			printf(" ... ");
			print_graph_raw(cores[i].jobs[j].end);
			puts(")");
		}
	}
	
	snprintf(output_string, 4096, "SU: searching [D=%hhu, R=%hhu] space....\n", D, R);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < thread_count; i++) {
		pthread_create(threads + i, NULL, worker_thread, cores + i);
	}

	nat resolution = job_count_per_core / 80;
	if (resolution == 0) resolution = 1;


	while (1) {
		// byte printed_one_char = 0;

		nat sum = 0;

		// nat largest_remaining = 0, chosen_thread = 0;

		for (nat i = 0; i < thread_count; i++) {
			const nat size = atomic_load_explicit(global_progress + i, memory_order_relaxed);
			// if (size > largest_remaining) { largest_remaining = size; chosen_thread = i; }
			sum += size;
		}

		printf("\033[H\033[2J");
		snprintf(output_string, 4096, "\n-----------------current jobs (jcpc=%llu)-------------------\n",
			 job_count_per_core // largest_remaining
		);
		printf("%s", output_string);

		snprintf(output_string, 4096, "\n\t%1.10lf%%\n\n", (double) (sum) / (double) job_count);
		print(output_filename, 4096, output_string);

		for (nat i = 0; i < thread_count; i++) { 

			const nat size = atomic_load_explicit(global_progress + i, memory_order_relaxed);

			snprintf(output_string, 4096, "  %c %020llu :: ", 
				i == (nat) -1 ? '*' : ' ', size
			);
			printf("%s", output_string);

			const nat amount = size / resolution;

			for (nat j = 0; j < amount; j++) {
				snprintf(output_string, 4096, "#");
				printf("%s", output_string);
			}
			snprintf(output_string, 4096, "\n");
			printf("%s", output_string);
		}

		// if (not printed_one_char) resolution *= 2;
		
		snprintf(output_string, 4096, "\n");
		printf("%s", output_string);

		for (nat i = 0; i < thread_count; i++) {
			const nat size = atomic_load_explicit(global_progress + i, memory_order_relaxed);
			if (size < job_count_per_core - 1) goto next;
		}
		break;
	next:
		// if (not largest_remaining) break;
		sleep(1 << display_rate);
	}






	nat counts[pm_count] = {0};

	for (nat i = 0; i < thread_count; i++) {
		nat* local_counts = NULL;
		pthread_join(threads[i], (void**) &local_counts);
		for (nat j = 0; j < pm_count; j++) counts[j] += local_counts[j];
		free(local_counts);
	}

	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec);
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	snprintf(output_string, 4096,
		"using [D=%hhu, R=%hhu]:"
		"\n\tthread_count=%llu"
		"\n\tjob_count_per_core=%llu"		
		"\n\tdisplay_rate=%llu"
		"\n\tstage1_execution_limit=%llu"
		"\n\tstage2_execution_limit=%llu"
		"\n\tarray_size=%llu"
		"\n\n"
		"\n\t""searched [%hhu:%hhu] zvs"
		"\n\tusing %llu threads, each with %llu jobs"
		"\n\tin %10.2lfs [%s:%s],"	
		"\n\n", 

		D, R,  thread_count, job_count_per_core, 
		display_rate,  stage1_execution_limit, 
		stage2_execution_limit,   array_size, 
		D, R, thread_count,  job_count_per_core, 
		seconds,  time_begin_dt,  time_end_dt
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













































/*






		//zthroughput = (double) space_size / seconds;

	//"\n\tspace_size=%llu"


	//"\n\tat %10.2lf z/s."
 //  space_size,  
		// BLAH,



	//      space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);



static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}


static nat adc64(nat* d, nat s) {
	*d += s;
	return *d < s;
}


static nat adc128(nat* dl, nat* dh, nat sl, nat sh, nat cin) {
	nat cl = adc64(dl, sl, cin);
	nat c = adc64(dh, sh, cl);
	return c;
}


chunk_count


mul512



3 * 6





	0 1 1 
	1 1 0  ---->   ( (1 0 0)   +    (0 1 0)  )
	----------

	( (0 1 1 )*(1 0 0)   +    (0 1 1 )*(0 1 0)  )





























static void mul128(nat* dl, nat* dh, const nat rl, const nat rh, const nat sl) {
	for (nat i = 0; i < sl; i++) 
		adc128(dl, dh, rl, rh, 0);
}

static void print_binary64(nat x) {
	for (nat i = 0; i < 64; i++) {
		printf("%llu", (x >> i) & 1);
	}
}

static void debug_zi(const char* s, nat l, nat h) {
	printf("%s = ", s);
	print_binary64(l); 
	putchar(' '); 
	print_binary64(h); 
	puts("");
}








		//nat copy[chunk_count] = {0};
		//memcpy(copy, total, chunk_count * sizeof(nat));
		//for (nat i = 0; i < 5 - 1; i++)  adc(total, copy, 0);




*/













/*int main(void) {
	const nat  D  =  0  ; 
 
	nat space_size_64 = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	printf("64-bit space size = %llu\n", space_size_64);

	const nat K = expn(5 + D, 3);
	nat total_l = expn(5 + D, 9);
	nat total_h = 0;
	
	for (nat _ = 0; _ < D; _++) {
		mul128(   &total_l, &total_h,    total_l,total_h,    5);
		mul128(   &total_l, &total_h,    total_l,total_h,    K);
	}
	
	print128("space_size", total_l, total_h);
}*/








//nat space_size_64 = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	//printf("64-bit space size = %llu\n", space_size_64);






// if (array[n] >= 65535) { *out_e = e; return pm_snm; }




			//debug_zi("div1", div1);
			//debug_zi("rem2", rem2);
			//debug_zi("p_begin", p_begin);
			//getchar();
	

			//debug_zi("div1", div1);
			//debug_zi("rem2", rem2);
			//debug_zi("p_end", p_end);
			//getchar();






	//debug_zi("negative_divisor", negative_divisor);
	//debug_zi("total", total);







		//printf("---------------------job #%llu------------------------\n", job);
		//debug_zi("range_begin", range_begin);
		//debug_zi("range_end", range_end);
		//puts("---------------------------------------------");






// terminate_thread:
//printf("worker_thread[%llu]: max_pointer = %llu\n", thread_index, max_pointer);














/*

202408084.172603:

---------------- no sync ---------------------



using [D=2, R=0]:
	thread_count=6
	job_count_per_core=40
	display_rate=0
	stage1_execution_limit=10000000
	stage2_execution_limit=0
	array_size=100000


	searched [2:0] zvs
	using 6 threads, each with 40 jobs
	in     214.00s [1202408084.163115:1202408084.163449],


pm counts:
z_is_good: 76      		 pm_ga: 0       		
pm_fea: 0       		pm_ns0: 85769549		
pm_pco: 101688352		pm_zr5: 1330613485		
pm_zr6: 1829675457		pm_ndi: 359227372		
pm_oer: 17991   		pm_r0i: 649667  		
 pm_h0: 1099549 		pm_f1e: 41873   		
pm_erc: 11802431		pm_rmv: 36188   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 1980    		pm_bdl2: 144     		
pm_erw: 0       		pm_mcal: 527318490		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 12      		 pm_h3: 0       		
pm_per: 1328144 		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 1214    		
[done_pm]









----------------- bsp -----------------------



using [D=2, R=0]:
	space_size=118689037748575
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000
	array_size=100000


	searched 118689037748575 zvs
	using 6 threads
	in     186.00s [1202408084.163932:1202408084.164238],
	at 638113106175.13 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684830700923		
pm_fea: 0       		pm_ns0: 84018547		
pm_pco: 100895566		pm_zr5: 1317636407		
pm_zr6: 1816694631		pm_ndi: 353166042		
pm_oer: 17280   		pm_r0i: 647137  		
 pm_h0: 1086497 		pm_f1e: 41668   		
pm_erc: 11215047		pm_rmv: 36132   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 1980    		pm_bdl2: 144     		
pm_erw: 0       		pm_mcal: 520275382		
pm_snl: 0       	  	 pm_h1: 0       		
 pm_h2: 12      		 pm_h3: 0       		
pm_per: 1313974 		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 1206    		
[done_pm]



  0  1  2  2  

  1  0  5  0

  2  1  4  3

  3  1  1  6

  4  0  0  1


  1  1  1  1

  2  4  1  0
















































//  0122 1050 2143 3156 4002 1313 2403

// 0122 1050 2143 3156 4002 1313 2403
// 0122 1050 2143 3566 4005 1010 2455
// 012_ 10__ 2_4_ 3___ 400_ ____ ____









*/






// static const byte max_sn_loop_iterations = 100 * 2;






			
		/*	
			if (last_op == six) SNL_counter++;
			else if (last_op != two) SNL_counter = 0;

			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;
		*/





// SNL_counter = 0,  





		/*	if (last_op == two) SNL_counter++; 
			else SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;
		*/




