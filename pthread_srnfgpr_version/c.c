// 202403041.192520: by dwrr         
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

static const byte D = 2;        // the duplication count (operation_count = 5 + D)
static const byte R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

static const nat job_size = 1000000;
static const nat thread_count = 9;
static const nat display_rate = 3;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good, pm_ga, pm_fea, pm_ns0, 
	pm_pco, pm_zr5, pm_zr6, pm_ndi, 
	pm_oer, pm_r0i, pm_h0, pm_f1e, 
	pm_erc, pm_rmv, pm_ot, pm_csm, 
	pm_mm, pm_snm, pm_bdl, pm_bdl2, 
	pm_erw, pm_mcal, pm_snl, 
	pm_h1, pm_h2, pm_h3, pm_per,
	pmf_fea, pmf_ns0, pmf_pco,
	pmf_zr5, pmf_zr6, pmf_ndi,
	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good", "pm_ga", "pm_fea", "pm_ns0", 
	"pm_pco", "pm_zr5", "pm_zr6", "pm_ndi", 
	"pm_oer", "pm_r0i", "pm_h0", "pm_f1e", 
	"pm_erc", "pm_rmv", "pm_ot", "pm_csm", 
	"pm_mm", "pm_snm", "pm_bdl", "pm_bdl2", 
	"pm_erw", "pm_mcal", "pm_snl", 
	"pm_h1", "pm_h2", "pm_h3", "pm_per",
	"pmf_fea", "pmf_ns0", "pmf_pco",
	"pmf_zr5", "pmf_zr6", "pmf_ndi",
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

static const nat max_er_repetions = 50;
static const nat max_erw_count = 100;
static const nat max_modnat_repetions = 15;
static const nat max_consecutive_s0_incr = 30;
static const nat max_consecutive_small_modnats = 200;
static const nat max_bdl_er_repetions = 25;
static const nat max_sn_loop_iterations = 100 * 2;
static const nat max_consecutive_h0_bouts = 10;
static const nat max_consecutive_h1_bouts = 30;
static const nat max_consecutive_h2_bouts = 30;
static const nat max_consecutive_h3_bouts = 30;

static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;

static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;

// runtime constants:
static nat space_size = 0;
static byte* positions = NULL; 
static pthread_t* threads = NULL; 

// globals:
static _Atomic nat head;

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

static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, nat* timeout) {

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0; 
	memset(timeout, 0, operation_count * sizeof(nat));

	byte ip = origin, last_mcal_op = 255, last_op = 0, mcal_path = 0;
	nat 	e = 0,  xw = 0, 
		pointer = 0,  er_count = 0, bout_length = 0,
		walk_ia_counter = 0, ERW_counter = 0, 
		SNL_counter = 0,   mcal_index = 0,
	    	OER_er_at = 0,  OER_counter = 0, 
		BDL_er_at = 0,  BDL_counter = 0, 
		BDL2_er_at = 0,  BDL2_counter = 0, 
		R0I_counter = 0, H0_counter = 0, 
		H1_counter = 0, H2_counter = 0, H3_counter = 0,
		RMV_counter = 0, RMV_value = 0, CSM_counter = 0;

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

		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return pm_ot; 
			timeout[i]++;
		}
		timeout[ip] = 0;

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
				if (ERW_counter == max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			did_ier_at = pointer;
			walk_ia_counter = 0;
			er_count++;
			pointer = 0;
		}

		else if (op == two) {
			if (array[n] >= 65535) return pm_snm; 
			if (last_op == six) SNL_counter++; else SNL_counter = 0;
			if (SNL_counter == max_sn_loop_iterations) return pm_snl;

			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6; 
			if (last_op == two) SNL_counter++; else SNL_counter = 0;
			if (SNL_counter == max_sn_loop_iterations) return pm_snl;

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

			if (bout_length == 1) {
				H1_counter++;
				if (H1_counter >= max_consecutive_h1_bouts) return pm_h1; 
			} else H1_counter = 0;

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

static nat execute_graph(byte* graph, nat* array, nat* timeout, byte* origin) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = execute_graph_starting_at(o, graph, array, timeout);   
		if (not pm) { *origin = o; return z_is_good; } 
	}
	return pm;
}

static nat fea_execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 

	byte ip = origin, last_mcal_op = 0;
	nat pointer = 0, e = 0, xw = 0;

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
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return pmf_zr6;
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return pmf_ndi;
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

static void* worker_thread(void* __attribute__((unused)) _unused_arg) {

	char filename[4096] = {0};
	nat* pms = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));
	nat* timeout = calloc(operation_count, sizeof(nat));
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);
	byte* graph = raw_graph;
	byte* end = raw_end;
	nat* graph_64 = raw_graph;
	nat* end_64 = raw_end;
	memcpy(graph, R ? _63R : _36R, 20);
	memcpy(end, R ? _63R : _36R, 20);
	byte pointer = 0;
	
next_job:;
	const nat h = atomic_fetch_add_explicit(&head, job_size, memory_order_relaxed);
	if (h >= space_size) goto terminate_thread;

	const nat range_begin = h;
	nat range_end = h + job_size - 1;
	if (range_end >= space_size) range_end = space_size - 1;

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
		if (graph_64[i] > end_64[i]) goto do_zskip; 
	}
	goto next_job;

do_zskip:;
	nat zindex = 0;
	p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}

	atomic_store_explicit(&head, zindex, memory_order_relaxed);
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
		if (graph[index] > graph[index + 4]) { at = index; goto bad; }
	}

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
	    pm = execute_graph(graph, array, timeout, &origin);  if (pm) { pms[pm]++; goto loop; } 

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
	printf("info: [thread with fileid=\"%s\" terminated]\n", filename);
	free(raw_graph);
	free(raw_end);
	free(array);
	free(timeout);
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

	atomic_init(&head, 0);
	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	positions = calloc(hole_count, 1);
	threads = calloc(thread_count, sizeof(pthread_t));
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	snprintf(output_string, 4096, "SRNFGPR: searching [D=%hhu, R=%hhu] space....\n", D, R);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);
	for (nat i = 0; i < thread_count; i++) pthread_create(threads + i, NULL, worker_thread, NULL);

	while (1) {
		const nat h = atomic_fetch_add_explicit(&head, 0, memory_order_relaxed);

		snprintf(output_string, 4096, "%llu .. %lf%%\n", h, (double) h / (double) space_size);
		print(output_filename, 4096, output_string);

		if (h >= space_size) {
			snprintf(output_string, 4096, "info: [all jobs allocated to threads. waiting for them to finish.]\n");
			print(output_filename, 4096, output_string);
			break;
		}
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
			"\n\tthreadcount=%llu"
			"\n\tjobsize=%llu"
			"\n\tspacesize=%llu"
			"\n\tdisplay_rate=%llu"
			"\n\tfea_execution_limit=%llu"
			"\n\texecution_limit=%llu"
			"\n\tarray_size=%llu"
			"\n\n"
			"\n[D=%hhu:R=%hhu]:"
			"\n\t""searched %llu zvs"
			"\n\tusing %llu threads"
			"\n\tin %10.10lfs[%s:%s],"
			"\n\tat %lf z/s."
			"\n\n\npm counts:\n", 
		D, R,  thread_count,  job_size, 
		space_size, display_rate,
		fea_execution_limit, execution_limit, array_size, 
		D, R, space_size, thread_count, seconds, 
		time_begin_dt, time_end_dt, zthroughput
	);
	print(output_filename, 4096, output_string);
	for (nat i = 0; i < pm_count; i++) {
		if (i and not (i % 2)) puts("");
		snprintf(output_string, 4096, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
		print(output_filename, 4096, output_string);
	}
	snprintf(output_string, 4096, "\n[done]\n");
	print(output_filename, 4096, output_string);
}
 






































































































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

