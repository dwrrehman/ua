// rewritting the 3sp search utility to 
// use a simpler job partitioning scheme, 
// and also optimize GA more, and add more 
// GA checks to help 3sp search happen faster
// written on 1202508144.203248 by dwrr

// made D general on 1202510293.025431

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <iso646.h>

typedef uint8_t byte;
typedef uint16_t u16;
typedef uint64_t nat;

#define D 2

#define machine_count 1
#define machine_index 0
#define thread_count 10

#define machine0_throughput 1
#define machine1_throughput 1

#define execution_limit 100000000LLU
#define array_size 1000000LLU
#define display_rate 2

static u16 queue[4096] = {0};
static _Atomic nat queue_count = 0;
static _Atomic nat progress[thread_count * 2] = {0};
static char** filenames = NULL;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good,

	pm_zr5, pm_zr6, pm_ndi, pm_sndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi,

	pm_h0, pm_h0s, pm_h1, pm_h2, 
	pm_rmv, pm_ormv, pm_imv, pm_csm, pm_lmv, 
	pm_fse, pm_pair, pm_ls0,

	pm_bdl1, pm_bdl2, pm_bdl3, 
	pm_bdl4, pm_bdl5, pm_bdl6, 
	pm_bdl7, pm_bdl8, pm_bdl9, 
	pm_bdl10, pm_bdl11, pm_bdl12, 

	pm_erp1, pm_erp2,


	pm_ga_sdol, 

	pm_ga_6g,    pm_ga_ns0, 
	pm_ga_zr5,   pm_ga_pco, 
	pm_ga_ndi,   pm_ga_snco,  
	pm_ga_zr6,   pm_ga_rdo, 

	pm_ga_uo,    pm_ga_il,

	pm_ga_5u1,   pm_ga_6u2,
	pm_ga_3u5,   pm_ga_3u1,

	pm_ga_sndi,  pm_ga_h,	

	pm_ga_sci,   pm_ga_6e,

	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good",

	"pm_zr5", "pm_zr6", "pm_ndi", "pm_sndi",
	"pm_pco", "pm_per", "pm_ns0",
	"pm_oer", "pm_rsi",

	"pm_h0", "pm_h0s", "pm_h1", "pm_h2", 
	"pm_rmv", "pm_ormv", "pm_imv", "pm_csm", "pm_lmv", 
	"pm_fse", "pm_pair", "pm_ls0",

	"pm_bdl1", "pm_bdl2", "pm_bdl3", 
	"pm_bdl4", "pm_bdl5", "pm_bdl6", 
	"pm_bdl7", "pm_bdl8", "pm_bdl9", 
	"pm_bdl10", "pm_bdl11", "pm_bdl12", 

	"pm_erp1", "pm_erp2",


	"pm_ga_sdol", 

	"pm_ga_6g",    "pm_ga_ns0", 
	"pm_ga_zr5",   "pm_ga_pco", 
	"pm_ga_ndi",   "pm_ga_snco",  
	"pm_ga_zr6",   "pm_ga_rdo", 

	"pm_ga_uo",    "pm_ga_il",

	"pm_ga_5u1",   "pm_ga_6u2",
	"pm_ga_3u5",   "pm_ga_3u1",

	"pm_ga_sndi",  "pm_ga_h",
	
	"pm_ga_sci",   "pm_ga_6e",
};

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static void print_binary(nat x) {
	for (nat i = 0; i < 64; i++) {
		if (not (i & 3)) putchar('_');
		printf("%llu", (x >> i) & 1);
	}
}

__attribute__((always_inline))
static byte gi(nat graph0, nat graph1, byte pa) {
	return (
		(pa < 16 ? graph0 : graph1) 
		>> 
		((pa & 15) << 2)
	) & 0xf;
}

__attribute__((always_inline))
static u16 gi16(nat graph0, nat graph1, byte pa) {
	return (
		(pa < 16 ? graph0 : graph1) 
		>> 
		((pa & 15) << 2)
	) & 0xffff;
}

static void print_graph_raw(nat g0, nat g1) { 
	for (byte i = 0; i < graph_count; i++) 
		printf("%hhu", gi(g0, g1, i)); 
}

static void get_graphs_z_value(char string[64], nat g0, nat g1) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) gi(g0, g1, i) + '0';
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void append_to_file(
	char* filename, size_t filename_size, 
	nat g0, nat g1, byte origin
) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, g0, g1);
	char o[16] = {0};    snprintf(o, sizeof o, " %hhu ", origin);

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
		snprintf(filename, filename_size, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}

	write(file, z, graph_count);
	write(file, o, 3);
	write(file, dt, 17);
	write(file, "\n", 1);
	close(file);

	printf("[%s]: write: %s z = %s to file \"%s\"\n",
		dt, permissions ? "created" : "wrote", z, filename
	);
}

#define lsepa 2

__attribute__((always_inline))
static byte noneditable(byte pa) {
	if (pa < 20 and not (pa & 3)) return true;
	if (pa == 18 or pa == 1 or pa == 5) return true;
	return false;
}

__attribute__((always_inline))
static byte editable(byte pa) { return not noneditable(pa); }

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

static nat execute_graph_starting_at(
	const byte origin, 
	const nat g0, const nat g1, 
	nat* array, 
	byte* zskip_at
) {	
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

	for (nat e = 0; e < execution_limit; e++) {

		const byte op = gi(g0, g1, ip * 4);
		if (editable(ip * 4) and *zskip_at > ip * 4) *zskip_at = ip * 4;
		
		if (op == one) {
			if (pointer == n) {
				puts("FEA condition violated by a z value: "); 
				print_graph_raw(g0, g1);
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
			byte RMV_counter = 0;
			nat RMV_value = (nat) -1;

			byte ORMV_counter = 0;
			nat ORMV_value = (nat) -1;
			byte ORMV_state = 0;

			nat IMV_value = (nat) -1;
			byte IMV_counter = 0;

			nat IMV2_value = (nat) -1;
			byte IMV2_counter = 0;

			for (nat i = 0; i < xw + 1; i++) {

				{ const nat a = array[i];
				const nat b = array[i + 1];
				const bool which = a < b;
				const nat difference = which ? b - a : a - b;
				const nat min = which ? a : b;
				nat above_minimum_size = which ? (b >= 200) : (a >= 200);
				if (above_minimum_size and difference >= min / 3) return pm_lmv; }

				if (array[i] < 8) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_rmv_modnat_repetions) return pm_rmv;

				if (array[i] == ORMV_value + ORMV_state) { 
					ORMV_state = not ORMV_state;
					ORMV_counter++;
					if (ORMV_counter >= max_ormv_modnat_repetions) return pm_ormv; 
				} else { 
					ORMV_value = array[i]; 
					ORMV_counter = 0; 
					ORMV_state = 0;
				}

				if (array[i] == IMV_value + 1) { IMV_counter++; IMV_value++; } else { IMV_value = array[i]; IMV_counter = 0; }
				if (IMV_counter >= max_imv_modnat_repetions) return pm_imv;

				if (i & 1) continue;

				if (array[i] == IMV2_value + 1) { IMV2_counter++; IMV2_value++; } else { IMV2_value = array[i]; IMV2_counter = 0; }
				if (IMV2_counter >= 2 * max_imv_modnat_repetions) return pm_imv;
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

		if (editable(ip * 4 + state) and *zskip_at > ip * 4 + state) *zskip_at = ip * 4 + state;
		ip = gi(g0, g1, ip * 4 + state);
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
		
static byte execute_graph(nat g0, nat g1, nat* array, byte* origin, nat* counts, const nat thread_index) {
	byte at = graph_count, op = 0, pa = 0;
	uint16_t instruction_data = 0;
	for (byte o = 0; o < graph_count; o += 4) {
		pa = o;
		instruction_data = gi16(g0, g1, pa);
		op = instruction_data & 0xf;
		if (op != three) continue;
		const nat pm = execute_graph_starting_at(o >> 2, g0, g1, array, &at);
		atomic_store_explicit(progress + 2 * thread_index + 0, g0, memory_order_relaxed);
		atomic_store_explicit(progress + 2 * thread_index + 1, g1, memory_order_relaxed);
		counts[pm]++;
		if (not pm) { *origin = o; return 0; }
		continue;
	}
	return at;
}

static void* worker_thread(void* raw_thread_index) {
	const nat thread_index = *(nat*) raw_thread_index;
	nat* counts = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));

	register nat g0 = 0;  
	register nat g1 = 0;
	register byte pointer = 0;

pull_job_from_queue:;
	const nat jobs_left = atomic_fetch_sub_explicit(&queue_count, 1, memory_order_relaxed);
	if ((int64_t) jobs_left <= 0) goto terminate;

	g0 = 	(1LLU << 4) |
		(1LLU << (4 * 4)) | 
		(2LLU << (8 * 4)) | 
		(3LLU << (12 * 4));
	g1 = 	4LLU | (4LLU << 8) |
		((nat) queue[jobs_left - 1] << ((4 * D) * 4));
	goto init;

	loop:
		if (gi(g0, g1, pointer) < ((pointer & 3) ? operation_count - 1 : 4)) goto increment;
		if (pointer < graph_count - 5) goto reset_;
		goto pull_job_from_queue;

	increment:
		if (pointer < 16) g0 += 1LLU << ((pointer & 15LLU) << 2LLU);
		else              g1 += 1LLU << ((pointer & 15LLU) << 2LLU);

	init:  	pointer = lsepa;
		u16 was_utilized = 0;
		byte at = lsepa;

		for (byte pa = 20; pa < graph_count; pa += 4) {
			if (pa < graph_count - 4 and gi(g0, g1, pa) > gi(g0, g1, pa + 4)) {
				at = pa + 1;
				counts[pm_ga_sdol]++;
				goto bad;
			} 
		}

		for (byte pa = graph_count; pa -= 4; ) {
	
			const nat op = gi(g0, g1, pa);
			const byte l = gi(g0, g1, pa + 1);
			const byte g = gi(g0, g1, pa + 2);
			const byte e = gi(g0, g1, pa + 3);

			if (gi(g0, g1, 4 * g) == two) {
				at = graph_count;
				if (editable(pa + 2) and at > pa + 2) at = pa + 2;
				if (editable(4 * g) and at > 4 * g) at = 4 * g;
				if (at == graph_count) abort();
				counts[pm_ga_sci]++;
				goto bad;
			}


			if (op == six and gi(g0, g1, 4 * l) == six) {
				at = graph_count;
				if (editable(pa + 1) and at > pa + 1) at = pa + 1;
				if (editable(pa) and at > pa) at = pa;
				if (editable(4 * l) and at > 4 * l) at = 4 * l;
				if (at == graph_count) abort();
				counts[pm_ga_zr6]++;
				goto bad;
			}

			if (op == six and gi(g0, g1, 4 * e) == one) {
				at = graph_count;
				if (editable(pa + 3) and at > pa + 3) at = pa + 3;
				if (editable(pa) and at > pa) at = pa;
				if (editable(4 * e) and at > 4 * e) at = 4 * e;
				if (at == graph_count) abort();
				counts[pm_ga_ns0]++;
				goto bad;
			}

			if (op == six and gi(g0, g1, 4 * e) == five) {
				at = graph_count;
				if (editable(pa + 3) and at > pa + 3) at = pa + 3;
				if (editable(pa) and at > pa) at = pa;
				if (editable(4 * e) and at > 4 * e) at = 4 * e;
				if (at == graph_count) abort();
				counts[pm_ga_ns0]++;
				goto bad;
			}
		
			if (op == two and g == pa >> 2) {
				at = graph_count;
				if (editable(pa + 2) and at > pa + 2) at = pa + 2;
				if (editable(pa) and at > pa) at = pa;
				if (at == graph_count) abort();
				counts[pm_ga_il]++;
				goto bad;
			}

			if (op == two and l == pa >> 2) {
				at = graph_count;
				if (editable(pa + 1) and at > pa + 1) at = pa + 1;
				if (editable(pa) and at > pa) at = pa;
				if (at == graph_count) abort();
				counts[pm_ga_sndi]++;
				goto bad;
			}

			if (	op == one and
				l == pa >> 2 and 
				g == pa >> 2 and 
				e == pa >> 2
			) { 
				at = graph_count;
				if (editable(pa + 1) and at > pa + 1) at = pa + 1;
				if (editable(pa + 2) and at > pa + 2) at = pa + 2;
				if (editable(pa + 3) and at > pa + 3) at = pa + 3;
				if (editable(pa) and at > pa) at = pa;
				if (at == graph_count) abort();
				counts[pm_ga_il]++;
				goto bad;
			}

			{const byte loops[5 * 18] = {
				two, 2, two, 2, pm_ga_il,
				one, 1, three, 1, pm_ga_h,
				one, 1, six, 1, pm_ga_zr6,
				three, 1, two, 1, pm_ga_ndi,
				three, 1, five, 1, pm_ga_zr5,
				three, 3, two, 2, pm_ga_ndi,
				three, 1, six, 1, pm_ga_ndi,
				three, 3, six, 1, pm_ga_ndi, 
				five, 1, six, 1, pm_ga_zr5,
				five, 2, six, 1, pm_ga_zr5,
				five, 3, six, 1, pm_ga_zr5,					
				five, 2, two, 2, pm_ga_zr5,
				five, 3, three, 1, pm_ga_zr5,
				five, 3, two, 2, pm_ga_zr5,
				six, 3, three, 1, pm_ga_ndi,
				two, 3, three, 1, pm_ga_ndi,
				three, 2, six, 1, pm_ga_ndi,
				three, 2, two, 2, pm_ga_ndi,
			};

			for (byte i = 0; i < 5 * 18; i += 5) {

				const byte A = loops[i + 0];
				const byte x = loops[i + 1];
				const byte B = loops[i + 2];
				const byte y = loops[i + 3];
				const byte method = loops[i + 4];
				const byte K = gi(g0, g1, pa + x);

				if (	op == A and
					gi(g0, g1, 4 * K) == B and
					gi(g0, g1, 4 * K + y) == pa >> 2
				) {
					at = graph_count;
					if (editable(pa) and at > pa) at = pa;
					if (editable(pa + x) and at > pa + x) at = pa + x;
					if (editable(4 * K) and at > 4 * K) at = 4 * K;
					if (editable(4 * K + y) and at > 4 * K + y) at = 4 * K + y;
					if (at == graph_count) abort();
					counts[method]++;
					goto bad;
				}
			}}

			{const byte pairs[3 * 4] = {
				three, three, pm_ga_ndi,
				five, one,  pm_ga_pco,
				five, five, pm_ga_zr5,
				six, two, pm_ga_snco,
			};

			for (byte i = 0; i < 4 * 3; i += 3) {
				const byte source = pairs[i + 1], destination = pairs[i];
				if (op == source) {
					for (byte offset = 1; offset < 4; offset++) {
						const byte dest = gi(g0, g1, pa + offset);
						if (gi(g0, g1, 4 * dest) != destination) continue;

						at = graph_count;
						if (editable(pa + offset) and at > pa + offset) at = pa + offset;
						if (editable(pa) and at > pa) at = pa;
						if (editable(4 * dest) and at > 4 * dest) at = 4 * dest;
						if (at == graph_count) continue;
						counts[pairs[i + 2]]++;
						goto bad;
					}
				} 
			}}

			if (l != pa >> 2) was_utilized |= 1 << l;
			if (g != pa >> 2) was_utilized |= 1 << g;
			if (e != pa >> 2) was_utilized |= 1 << e;

			for (byte i = graph_count - 4; i >= 20 and pa < i; i -= 4) {
				if (gi16(g0, g1, i) == gi16(g0, g1, pa)) {
					at = graph_count;
					if (editable(pa + 3) and at > pa + 3) at = pa + 3;
					if (editable(pa + 2) and at > pa + 2) at = pa + 2;
					if (editable(pa + 1) and at > pa + 1) at = pa + 1;
					if (editable(pa) and at > pa) at = pa;
					counts[pm_ga_rdo]++; 
					goto bad;
				}
			}
		}

		for (byte la = 0; la < operation_count; la++) {
			if (not ((was_utilized >> la) & 1)) { 
				at = lsepa;
				counts[pm_ga_uo]++; 
				goto bad; 
			} 
		}


		for (byte pa = 20; pa < graph_count; pa += 4) {
			if (gi(g0, g1, pa) == five) goto skip_5_1_check;
		}

		{ const byte l = gi(g0, g1, 4 * five + 1);
		const byte g = gi(g0, g1, 4 * five + 2);
		const byte e = gi(g0, g1, 4 * five + 3);

		if (	gi(g0, g1, 4 * l) == one and
			gi(g0, g1, 4 * g) == one and
			gi(g0, g1, 4 * e) == one
		) {
			at = 13; 
			counts[pm_ga_5u1]++;
			goto bad;
		} } 
		skip_5_1_check:; 

		for (byte pa = 20; pa < graph_count; pa += 4) {
			if (gi(g0, g1, pa) == six) goto skip_6_2_check;
		}

		{ const byte l = gi(g0, g1, 4 * six + 1);
		const byte e = gi(g0, g1, 4 * six + 3);

		if (	gi(g0, g1, 4 * l) == two and
			gi(g0, g1, 4 * e) == two
		) {
			at = 17;
			counts[pm_ga_6u2]++;
			goto bad;
		} }
		skip_6_2_check:; 

		for (byte pa = 20; pa < graph_count; pa += 4) {
			if (gi(g0, g1, pa) == three) goto skip_3_15_check;
		}

		{ const byte l = gi(g0, g1, 4 * three + 1);
		const byte g = gi(g0, g1, 4 * three + 2);
		const byte e = gi(g0, g1, 4 * three + 3);

		if (	gi(g0, g1, 4 * l) == one and
			gi(g0, g1, 4 * g) == one and
			gi(g0, g1, 4 * e) == one
		) {
			at = 9;
			counts[pm_ga_3u1]++;
			goto bad;
		} 

		if (	gi(g0, g1, 4 * l) == five and
			gi(g0, g1, 4 * g) == five and 
			gi(g0, g1, 4 * e) == five
		) {
			at = 9;
			counts[pm_ga_3u5]++;
			goto bad;
		} }
		skip_3_15_check:; 

		for (byte pa = graph_count; pa -= 4;) {
			if (gi(g0, g1, pa) != six) continue;
			for (byte i = 20; i < graph_count; i += 4) {
				if (
					gi(g0, g1, i) == one
						and
					gi(g0, g1, i + 2) == pa >> 2
				) goto skip_6e_check;
			}	
			if (gi(g0, g1, pa + 3) != six) {
				at = lsepa;
				counts[pm_ga_6e]++;
				goto bad;
			}
			skip_6e_check:;
		}

		for (byte pa = graph_count; pa -= 4;) { 
			if (gi(g0, g1, pa) == six and gi(g0, g1, pa + 2) != six) {
				at = graph_count;
				if (editable(pa + 2) and at > pa + 2) at = pa + 2;
				if (editable(pa) and at > pa) at = pa;
				if (at == graph_count) abort();
				counts[pm_ga_6g]++;
				goto bad;
			}
		}

		byte origin = 0;
		at = execute_graph(g0, g1, array, &origin, counts, thread_index);

		if (not at) {
			append_to_file(filenames[thread_index], 4096, g0, g1, origin);
			goto loop;
		}

	bad:	if (noneditable(at)) {
			printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
			abort();
		}

		if (at >= graph_count - 4) goto pull_job_from_queue;

		for (byte i = lsepa; i < at; i++) {
			if (editable(i)) {
				if (i < 16) g0 &= ~(0xfLLU << ((i & 15LLU) << 2LLU));
				else        g1 &= ~(0xfLLU << ((i & 15LLU) << 2LLU));
			}
		}
		pointer = at; goto loop;

	reset_:
		if (pointer < 16) g0 &= ~(0xfLLU << ((pointer & 15LLU) << 2LLU));
		else              g1 &= ~(0xfLLU << ((pointer & 15LLU) << 2LLU));

		do pointer++; while (noneditable(pointer));
		goto loop;

terminate:
	free(array);
	atomic_store_explicit(progress + 2 * thread_index + 0, g0, memory_order_relaxed);
	atomic_store_explicit(progress + 2 * thread_index + 1, g1, memory_order_relaxed);
	return counts;
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

int main(void) {
	srand((unsigned) time(0));
	
	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};
	
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	atomic_init(&queue_count, 0);
	for (nat i = 0; i < thread_count; i++) {
		atomic_init(progress + 2 * i + 0, 0);
		atomic_init(progress + 2 * i + 1, 0);
	}

	filenames = calloc(thread_count, sizeof(char*));
	for (nat i = 0; i < thread_count; i++) {
		filenames[i] = calloc(4096, 1);
		char dt[32] = {0};
		get_datetime(dt);
		snprintf(filenames[i], 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
	}

	nat total_job_count = 0;

{	byte mi = 0, pointer = 0;
	u16 g0 = 0;
	goto init;
loop:
	if (gi(g0, 0, pointer) < (pointer ? operation_count - 1 : 4)) goto increment;
	if (pointer < 4 - 1) goto reset_;
	goto done;
increment:
	g0 += 1LLU << ((pointer & 15LLU) << 2LLU);
init:	pointer = 0;

	const byte op = gi(g0, 0, 0);
	const byte l  = gi(g0, 0, 1);
	const byte g  = gi(g0, 0, 2);
	const byte e  = gi(g0, 0, 3);

	if (g == two) goto loop;
	if (op == one and l == 7 and g == 7 and e == 7) goto loop;
	if (op == two and l == 7) goto loop;
	if (op == two and g == 7) goto loop;
	if (op == three and l == 7) goto loop;
	if (op == three and g == 7) goto loop;
	if (op == three and e == 7) goto loop;
	if (op == five and l == 7) goto loop;
	if (op == five and g == 7) goto loop;
	if (op == five and e == 7) goto loop;
	if (op == six and l == 7) goto loop;
	if (op == six and e == 7) goto loop;
	if (op == six and e == one) goto loop;
	if (op == six and e == five) goto loop;
	if (op == one and l == five) goto loop;
	if (op == one and g == five) goto loop;
	if (op == one and e == five) goto loop;
	if (op == two and l == six) goto loop;
	if (op == two and g == six) goto loop;
	if (op == two and e == six) goto loop;
	if (op == three and l == three) goto loop;
	if (op == three and g == three) goto loop;
	if (op == three and e == three) goto loop;
	if (op == five and l == five) goto loop;
	if (op == five and g == five) goto loop;
	if (op == five and e == five) goto loop;
	if (op == six and l == six) goto loop;
	if (op == six and g != six) goto loop;
	if (op == six and e == six) goto loop;

	mi = (mi + 1) % machine_count;
	if (mi != machine_index) goto loop;
	const nat n = atomic_fetch_add_explicit(&queue_count, 1, memory_order_relaxed);
	queue[n] = g0;
	total_job_count++;

	goto loop;
reset_:
	g0 &= ~(0xfLLU << ((pointer & 15LLU) << 2LLU));
	pointer++;
	goto loop;

done:; }
	printf("printing jobs: (%llu total jobs)\n", total_job_count);
	for (nat i = 0; i < total_job_count; i++) {
		if (not (i % 2)) putchar(' ');
		if (not (i % 4)) putchar(' ');
		if (not (i % 8)) putchar(10);
		printf("%04hx ", queue[i]);
	}
	puts("");
	getchar();

	snprintf(output_string, 4096, "SU: searching [D=%u] space....\n", D);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < thread_count; i++) {
		nat* arg = malloc(sizeof(nat));
		*arg = i;
		pthread_create(threads + i, NULL, worker_thread, arg);
	}

	nat counts[pm_count] = {0};
	const bool disable_main = 0;
	while (1) {

		const nat amount_remaining = atomic_load_explicit(&queue_count, memory_order_relaxed);
		if ((int64_t) amount_remaining <= 0 or disable_main) goto terminate;

		printf("\033[H\033[2J");
		printf("----------------- jobs remaining %llu / %llu -------------------\n", 
			amount_remaining, total_job_count
		);
		printf("\n\t complete %1.10lf%%\n\n", (double) (total_job_count - amount_remaining) / (double) total_job_count);
		for (nat i = 0; i < thread_count; i++) {
			const nat g0 = atomic_load_explicit(progress + 2 * i + 0, memory_order_relaxed);
			const nat g1 = atomic_load_explicit(progress + 2 * i + 1, memory_order_relaxed);
			printf(" %5llu : ", i);
			print_graph_raw(g0, g1); puts("");
		}
		puts("");
		sleep(1 << display_rate);
	}

terminate:
	puts("\nmain: joining threads...\n");
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
		"su: D = %u space:\n"
		"\t thread_count = %u\n"
		"\t machine_index = %u\n"
		"\t machine0_throughput = %u\n"
		"\t machine1_throughput = %u\n"
		"\t display_rate = %u\n"
		"\t execution_limit = %llu\n"
                "\t array_size = %llu\n"
		"\t in %10.2lfs [%s:%s]\n"
		"\n",
		D, thread_count,
		machine_index,
		machine0_throughput,
		machine1_throughput,
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

} // main
















































/*#define take_side(x) \
	if (editable(pa + x) and at > pa + x) at = pa + x; \
	pa = (byte) (((instruction_data >> (x << 2)) & 0xf) << 2); \
	instruction_data = gi16(g0, g1, pa); \
	op = instruction_data & 0xf; \
	if (editable(pa) and at > pa) at = pa; \

*/

/*
	...

	if (editable(pa + x) and at > pa + x) at = pa + x; \
	pa = (byte) (((instruction_data >> (x << 2)) & 0xf) << 2); \


	------

	instruction_data = gi16(g0, g1, pa); \
	op = instruction_data & 0xf; \


	if (editable(pa) and at > pa) at = pa; \
	
	...

*/

/*

		goto ok;

		take_side(1);
		if (op == one) {
			take_side(3);
			if (op == two) {
				take_side(2);
				if (op == three) { 
					take_side(3);
					if (op == one) goto ok;
					if (op == two) goto ok;
					if (op == five) goto ok;
					if (op == six) goto ok;
				}
			} else if (op == three) {
				take_side(1);
				if (op == one) {
					take_side(3);
					if (op == two) goto ok;
					if (op == three) goto ok;

				} else if (op == two) {
					take_side(3);
					if (op == two) goto ok;
					if (op == one) goto ok;
					if (op == five) goto ok;

				} else if (op == five) { 
					take_side(1);
					if (op == two) goto ok;
					if (op == one) goto ok;
					if (op == three) goto ok;
				}
			}

		} else if (op == two) {
			take_side(3);
			if (op == one) {
				take_side(2);
				if (op == three) {
					take_side(3);
					if (op == one) goto ok;
					if (op == two) goto ok;
					if (op == six) goto ok;
					if (op == five) goto ok;
				} else if (op == six) {
					take_side(3);
					if (op == two) goto ok;
					if (op == three) goto ok;
				}
			} else if (op == two) {
				take_side(2);
				if (op == one) {
					take_side(2);
					if (op == three) goto ok;
					if (op == six) goto ok;
				}
			}
		}
		continue; 

*/






















































// why using 512 jobs is not sufficient:

// 8^3 = 512

// oc = 8
// la is are in lrs(8) 
// log2(8) = 3
// 3 bits per la
// 3 * 3 = 9,  9 bits per job
// we'll still be using a u16 for each job.

// 64 cores  /  10 cores

// (512/2)/64 = 4 jobs per core          (512/2)/10 = 26 or 25 jobs per core



// ------------------------------------------------------------
// 1202508144.204759:
// instead, we are going to be using:

//      5 * 8^3 = 2560    jobs     


//      2560 / 2 = 1280   jobs in the queue for both machines

// machines:

//    (2560 / 2) / 64 = 20      jobs per core on the riscv machine

//    (2560 / 2) / 10 = 128    jobs per core on the mac mini


  //     queue size is at maximum:   (5 * 8^3) / 2 jobs  and i'll round it up to 2048.






/*




1l3,3l1   1l6,6l1   3l2,2l3   3l5,5l3    
3l6,6l3   5l6,6l5   5g6,6l5   5g2,2g5    
6e3,3l6   5e6,6l5   5e3,3l5   2e3,3l2
3e6,6l3   3e2,2g3   5e2,2g5   



1202509125.221238
TABLE:

legend:
----------------------------------------------------------------------------

	~ = already had it in GA

	* = added, done, added a new GA PM

	E = ...error, ignoring this element in the table....

	? = i dont know if this is good or bad lol.. look into this?...


----------------------------------------------------------------------------

A ----x----> B      B -----y-----> A

5 -----(=)-----> 2
2 -----(>)-----> 5

A 5
B 6
x =
y <


       x
      B1   2   3   5   6        1   2   3   5   6        1   2   3   5   6    
A    +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
y    | . |   |   |   |   |    | . | . | . | ~ | . |    | . | . | . | ~ | . |   
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | . | ~ |   |   |   |    | . | - | - | E | ~ |    | . | - | * | E | ~ | 
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+   
     | * | * | ~ |   |   |    | . | - | ~ | E | E |    | . | - | ~ | E | * | 
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | ~ | E | * | ~ |   |    | ~ | E | E | ~ | * |    | ~ | E | * | ~ | * |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | * | ? | * | * | ~ |    | ~ | ~ | ~ | ~ | ~ |    | ~ | ~ | * | ~ | ~ |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    


     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | . |   |   |   |   |    | . | . | . | ~ | ~ |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | . | ~ |   |   |   |    | . | - | - | E | ~ |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | . | ? | ~ |   |   |    | . | * | ~ | E | * |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | ~ | * | ? | ~ |   |    | ~ | * | E | ~ | ~ |  
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | ~ | ~ | ~ | ~ | ~ |    | ~ | ~ | E | ~ | ~ |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    


     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | . |   |   |   |   |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | . | - |   |   |   |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | . | - | ~ |   |   |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | ~ | E | E | ~ |   |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | ~ | ~ | E | ~ | ~ |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    



A ----x----> B      B -----y-----> A

2 -----(>)-----> 5 
5 -----(<)-----> 2


A 2
B 5

x >
y <





COPY OF THE TABLE: ORIGINAL:
0000000000000000000000000000000000000



      B1   2   3   5   6        1   2   3   5   6        1   2   3   5   6    
A    +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | . | / | / | / | / |    | . | . | . | P | . |    | . | . | . | P | . |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | . | s | / | / | / |    | . | - | - | p | S |    | . | - | n | p | S |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | h | n | n | / | / |    | . | - | n | p | n |    | . | - | n | p | n |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | P | p | p | p | / |    | P | p | p | p | p |    | P | p | p | p | p |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     | 6 | S | 6 | 6 | 6 |    | g | S | g | g | 6 |    | 0 | S | 6 | 0 | 6 |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    


     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | . | / | / | / | / |    | . | . | . | P | g |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | . | s | / | / | / |    | . | - | - | p | S |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | . | n | n | / | / |    | . | n | n | 5 | g |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | P | p | p | p | / |    | P | p | p | p | g |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    | g | S | g | g | 6 |    | 0 | S | 6 | 0 | 6 |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    


     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | . | / | / | / | / |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | . | - | / | / | / |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | . | - | n | / | / |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | P | p | p | p | / |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    
     |   |   |   |   |   |    |   |   |   |   |   |    | 0 | S | 6 | 0 | 6 |    
     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    








terminating...
total_count = 225
LEGEND:
   -  :   z_is_good
   i  :   pm_infinite_loop
   5  :   pm_zr5
   6  :   pm_zr6
   n  :   pm_ndi
   s  :   pm_sndi
   p  :   pm_pco
   e  :   pm_per
   0  :   pm_ns0
   o  :   pm_oer
   r  :   pm_rsi
   h  :   pm_h0
   k  :   pm_h0s
   1  :   pm_h1
   2  :   pm_h2
   a  :   pm_pair





1l3,3l1   1l6,6l1   3l2,2l3   3l5,5l3    
3l6,6l3   5l6,6l5   5g6,6l5   5g2,2g5    
6e3,3l6   5e6,6l5   5e3,3l5   2e3,3l2
3e6,6l3   3e2,2g3   5e2,2g5   



*/



























		/*if ((1)) {
			printf("EXG: trying: (origin = %d) : \n", pa >> 2);
			print_graph_raw(g0, g1); putchar(10);
			printf("[ttp = %llu, pm = %s]\n", ttp, pm_spelling[pm]);
			getchar();
		}*/



















































/*

	const byte pa = 4 * la;
		const byte dla = (here >> (4 * side)) & 0xf;

		if (editable(pa + side) and at > pa + side) at = pa + side;
		if (editable(pa) and at > pa) at = pa;
		if (editable(4 * dla) and at > 4 * dla) at = 4 * dla;



//if (editable(4 * la) and at > 4 * la) at = 4 * la; \
//if (editable(4 * la + side) and at > 4 * la + side) at = 4 * la + side; \

//atomic_store_explicit(&execution_counter, 0, memory_order_relaxed);



*/

























/*



 (1695 total jobs, prior to making these pms active)

if (destination,sourcel,sourceg,sourcee < 5) {


		// 1 ----(l or g or e)--> address #3    (pco)

		// 2 ----(l or g or e)--> address #4    (snco)

		// 3 ----(l or g or e)--> address #2    (ndi)

		// 5 ----(l or g or e)--> address #3    (zr5)

		// 6 ----(l or e)--> address #4    	(zr6)

		// 6 ----(e)--> address #0		(ns0)

		// 6 ----(e)--> address #3		(ns0)





2,560 is the total raw number of jobs,

	we got it down to: just     1093 total jobs


	



*/















/*
1l3,3l1
1l6,6l1
3l2,2l3
3l5,5l3
3e2,2g3
3l6,6l3
3e6,6l3

5l6,6l5
5g6,6l5
5e6,6l5

5g2,2g5
5e3,3l5
5e2,2g5

6e3,3l6
2e3,3l2
*/


/*

1202509125.233450

1l3,3l1   1l6,6l1   3l2,2l3   3l5,5l3    
3l6,6l3   5l6,6l5   5g6,6l5   5g2,2g5    
6e3,3l6   5e6,6l5   5e3,3l5   2e3,3l2
3e6,6l3   3e2,2g3   5e2,2g5   



     1 -----(<)------> 3    <




1l3,3l1
1l6,6l1

3l2,2l3
3l5,5l3
3e2,2g3
3l6,6l3
3e6,6l3

5l6,6l5
5g6,6l5
5g2,2g5
5e6,6l5
5e3,3l5
5e2,2g5

6e3,3l6

2e3,3l2



*/


















/*

















	
		if (5.l == 3) goto skip_5_1_check;
		if (5.l == 1) goto check_5_g;
		
		if (5.l == 2) {
			
		}

		if (5.l == 6) {
			if (
		}


	check_5_g:
		if (we see that 5.g == 3) goto skip_5_1_check;
		if (we see that 5.g == 1) goto check_5_e;



	check_5_e:
		if (we see that 5.e == 3) goto skip_5_1_check;
		
		at = 1; 
		counts[pm_ga_n0i]++;
		goto bad; 

		} skip_5_1_check:;








 1202508262.224306

	pruning the jobs that we push to the queue, 
	in main before we push them!!


		2-----(>)----->US

			. {2, x, 7, x}


		3-----(<=>)----->US     

			. {3, 7, x, x}

			. {3, x, 7, x}

			. {3, x, x, 7}


		5----(<=>)----->US

			. {5, 7, x, x}

			. {5, x, 7, x}

			. {5, x, x, 7}


		6-----(>)---->( != 4)


			. {6, x, !=4, x}

			. {6, 7, x, x}

			. {6, x, x, 7}






test this zv out with ip util:

	00071273200534544041000010001767

*/










/*

1202508155.005725
lets write this one out on paper, to see whats wrong with it!!


		00071273200534544041000010001767

	nothing special, just a random zv that we had run lol 






*/







// graph0: 0000_0000_0000_0000__0000_0000_0000_0000__0000_0000_0000_0000__0000_0000_0000_0000
// graph1: 0000_0000_0000_0000__0000_0000_0000_0000__0000_0000_0000_0000__0000_0000_0000_0000




/*
 	byte machine_state = 0;
	nat mi;
	if (machine_state xor machine_index) {
		if (machine1_counter < machine0_counter_max) { machine1_counter++; mi = 1; } 
		else { machine_state = 0; machine0_counter = 1; mi = 0; }
	} else {
		if (machine0_counter < machine1_counter_max) { machine0_counter++; mi = 0; } 
		else { machine_state = 1; machine1_counter = 1; mi = 1; }
	}

*/









/*


if (	graph[4 * index + 0] == two and 
				graph[4 * graph[4 * index + 2] + 0] == two and  
				graph[4 * graph[4 * graph[4 * index + 2] + 2] + 0] == two and								graph[4 * graph[4 * graph[4 * index + 2] + 2] + 2] == index
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


*/








/*

#include <iso646.h>
#include <stdint.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/stat.h>

#include <sys/types.h>

#include <string.h>
#include <errno.h>
*/


















	//nat first = 1; // debug

/*if (e >= 100000000) {
			if (first) {
				print_graph_raw(graph); puts("");
				getchar();
				first = 0;
			}
			printf("%hhu ", op); fflush(stdout);
			usleep(20000);
		}*/
			

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





