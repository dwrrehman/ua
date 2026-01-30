// SU version 2, using possibility nf-array states (PAS) 
// system for searching, instead of z-values
// written on 1202512151.174454 by dwrr

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

enum operations { one, two, three, five, six };
typedef uint8_t byte;
typedef uint16_t u16;
typedef uint64_t nat;

#define D 4
#define DOL     (    (one << 0) | (two << 4) | (five << 8) | (six << 12)  ) 

#define job_digit_count 4
#define machine_count 1
#define machine_index 0
#define thread_count 1

#define machine0_throughput 1
#define machine1_throughput 1

#define execution_limit 100000000LLU
#define array_size 1000000LLU
#define display_rate 2

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static byte pas_count = 0;
static nat mod0 = 0, mod1 = 0;
static u16 partial_graph[operation_count] = {0};
static nat decode[64] = {0};
static byte rev_pas_map[64] = {0};
static byte pas_map[64] = {0};
static byte job_placement_in_PAS = 0;
static nat job_modulus = 0;
static nat ltrdo_count = 0;
static byte ltrdo[2048] = {0};

static nat uncondga_count = 0;
static byte uncondga[2048] = {0};

static u16 queue[2048] = {0};
static _Atomic nat queue_count = 0;
static _Atomic u16 progress[thread_count * operation_count] = {0};
static char** filenames = NULL;

enum pruning_metrics {
	z_is_good,

	pm_zr5, pm_zr6, pm_ndi, pm_sndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi, 
	pm_mcal,

	pm_h0, pm_h0s, pm_h1, pm_h2, 
	pm_rmv, pm_ormv, pm_imv, pm_csm, pm_lmv, 
	pm_fse, pm_pair, pm_ls0,

	pm_bdl1, pm_bdl2, pm_bdl3, 
	pm_bdl4, pm_bdl5, pm_bdl6, 
	pm_bdl7, pm_bdl8, pm_bdl9, 
	pm_bdl10, pm_bdl11, pm_bdl12, 

	pm_erp1, pm_erp2,

	pm_ga_rdo,  pm_ga_uo, 
	pm_ga_5u1,  pm_ga_6u2,
	pm_ga_3u5,  pm_ga_3u1,
	pm_ga_sndi, pm_ga_h,

	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good",

	"pm_zr5", "pm_zr6", "pm_ndi", "pm_sndi",
	"pm_pco", "pm_per", "pm_ns0",
	"pm_oer", "pm_rsi",
	"pm_mcal",

	"pm_h0", "pm_h0s", "pm_h1", "pm_h2", 
	"pm_rmv", "pm_ormv", "pm_imv", "pm_csm", "pm_lmv", 
	"pm_fse", "pm_pair", "pm_ls0",

	"pm_bdl1", "pm_bdl2", "pm_bdl3", 
	"pm_bdl4", "pm_bdl5", "pm_bdl6", 
	"pm_bdl7", "pm_bdl8", "pm_bdl9", 
	"pm_bdl10", "pm_bdl11", "pm_bdl12", 

	"pm_erp1", "pm_erp2",

	"pm_ga_rdo",  "pm_ga_uo", 
	"pm_ga_5u1",  "pm_ga_6u2",
	"pm_ga_3u5",  "pm_ga_3u1",
	"pm_ga_sndi", "pm_ga_h",
};

static byte banned_edges[] = {

	// sci
	one, 	2, 	two, 
	two, 	2, 	two, 
	three, 	2, 	two, 
	five, 	2, 	two, 
	six, 	2, 	two, 

	// pco 
	one, 	1, 	five, 
	one, 	2, 	five, 
	one, 	3, 	five, 

	// snco 
	two, 	1, 	six, 
	two, 	2, 	six, 
	two, 	3, 	six, 

	// ndi 
	three, 	1, 	three, 
	three, 	2, 	three, 
	three, 	3, 	three, 

	// zr5 
	five, 	1, 	five, 
	five, 	2, 	five, 
	five, 	3, 	five, 

	// zr6 
	six, 	1, 	six, 
	six, 	3, 	six, 

	// ns0
	six, 	3, 	one, 
	six, 	3, 	five, 
	one, 	2, 	one, 
	one, 	3, 	one, 
};

static byte banned_self_edges[] = {
	// lb
	one, 1,

	// sndi
	two, 1,
};

static const byte loops[4 * 17] = {
	one, 1, three, 1,
	one, 1, six, 1,
	three, 1, two, 1, 
	three, 1, five, 1, 
	three, 3, two, 2,
	three, 1, six, 1,
	three, 3, six, 1,
	five, 1, six, 1,
	five, 2, six, 1,
	five, 3, six, 1,
	five, 2, two, 2,
	five, 3, three, 1,
	five, 3, two, 2,
	six, 3, three, 1,
	two, 3, three, 1,
	three, 2, six, 1,
	three, 2, two, 2,
};

static void print_binary(nat x) {
	for (nat i = 0; i < 16; i++) {
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

static void print_graph_raw(u16* graph) { 
	for (u16 i = 0; i < graph_count; i++) 
		printf("%hhu", (byte)
			((
				graph[i / 4LLU] 
				>> 
				(
					(
						(i % 4LLU) 
						* 
						4LLU
					)
				)
			) & 0xfLLU)
		); 
}

static void get_graphs_z_value(char string[64], u16* graph) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) (((
				graph[i / 4LLU] 
				>> 
				(
					(
						(i % 4LLU) 
						* 
						4LLU
					)
				)
			) & 0xfLLU)) + '0';
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void append_to_file(
	char* filename, size_t filename_size, u16* graph, byte origin
) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, graph);
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
	u16* graph,
	nat* array
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

	byte has_executed_5 = 0;
	byte has_executed_6 = 0;
	
	for (nat e = 0; e < execution_limit; e++) {

		const byte op = graph[ip] & 0xf;
		
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
			if (not has_executed_5) { if (pointer > 1) return pm_mcal; }
			has_executed_5 = 1;
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
			if (not has_executed_6) { if (array[n] > 1) return pm_mcal; }
			has_executed_6 = 1;
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
		if (array[n] < array[pointer]) state = 1 * 4;
		if (array[n] > array[pointer]) state = 2 * 4;
		if (array[n] == array[pointer]) state = 3 * 4;
		ip = (graph[ip] >> state) & 0xf;
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
		
static byte execute_graph(
	u16* graph, 
	nat* array, 
	byte* origin, 
	nat* counts, 
	const nat thread_index
) {
	for (byte o = 0; o < operation_count; o++) {
		byte op = graph[o] & 0xf;
		if (op != three) continue;
		const nat pm = execute_graph_starting_at(o, graph, array);
		for (byte i = 0; i < operation_count; i++) 
			atomic_store_explicit(progress + operation_count * thread_index + i, graph[i], memory_order_relaxed);
		counts[pm]++;
		if (not pm) { *origin = o >> 2; return 0; }
		continue;
	}
	return 1;
}

static void* worker_thread(void* raw_thread_index) {

	const nat thread_index = *(nat*) raw_thread_index;
	nat* counts = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));
	u16* graph = calloc(operation_count, sizeof(u16));

	register nat g0 = 0;  
	register nat g1 = 0;
	register byte pointer = 0;

pull_job_from_queue:;
	const nat jobs_left = atomic_fetch_sub_explicit(&queue_count, 1, memory_order_relaxed);
	if ((int64_t) jobs_left <= 0) goto terminate;
	g0 = 0;
	g1 = (nat) queue[(jobs_left - 1)] << job_placement_in_PAS;

	//puts("pulled job::");
	//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
	//puts("");
	//getchar();

	goto init;
loop:
	//puts("AT NF LOOP");
	if (gi(g0, g1, pointer) < (gi(mod0, mod1, pointer) - 1)) goto increment;
	if (pointer < pas_count - 1) goto reset_;
	goto pull_job_from_queue;
reset_:
	//puts("AT NF RESET");
	if (pointer < 16) g0 &= ~(0xfLLU << ((pointer & 15LLU) << 2LLU));
	else              g1 &= ~(0xfLLU << ((pointer & 15LLU) << 2LLU));
	pointer++;
	goto loop;
increment:
	//puts("AT NF INCR");
	if (pointer < 16) g0 += 1LLU << ((pointer & 15LLU) << 2LLU);
	else              g1 += 1LLU << ((pointer & 15LLU) << 2LLU);
init:  	pointer = 0;

	//if ((g0 & 0xFFFFFFFF) == 0) { 
		//puts("debug:");
		//print_graph_raw(graph); putchar(9); putchar(9); printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
		//puts("");
		//getchar();
	//}

	//puts("AT INIT, DOING GA:");
	//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
	//puts("");

	//getchar();

	for (nat i = 0; i < ltrdo_count; i += 9) {
		const byte x0 = ltrdo[i + 0];
		const byte x1 = ltrdo[i + 1];
		const byte x2 = ltrdo[i + 2];
		const byte x3 = ltrdo[i + 3];
		const byte x4 = ltrdo[i + 4];
		const byte x5 = ltrdo[i + 5];
		const byte x6 = ltrdo[i + 6];
		const byte x7 = ltrdo[i + 7];
		const byte x8 = ltrdo[i + 8];

		if (x7 != 255) {
			//puts("PRE CHECK ENABLED!!!");
			if (gi(g0, g1, x6) != x7) {
				//printf("LTRDO PRE-PG-CHECK SUCCEEDED!! :   "
				//	"gi(g0, g1, x6) = %hhu,   x7 = %hhu\n", 
				//	gi(g0, g1, x6), x7
				//);
				continue;
			}
			//printf("LTRDO PRE-PG-CHECK SUCCEEDED!! :   "
			//	"gi(g0, g1, x6) = %hhu,   x7 = %hhu\n", 
			//	gi(g0, g1, x6), x7
			//);
			//puts("PASSED PRE CHECK!");
			//getchar();
		}

		if (gi(g0, g1, x0) > gi(g0, g1, x1)) goto badltrdo;
		if (gi(g0, g1, x0) < gi(g0, g1, x1)) continue;
		if (gi(g0, g1, x2) > gi(g0, g1, x3)) goto badltrdo;
		if (gi(g0, g1, x2) < gi(g0, g1, x3)) continue;
		if (gi(g0, g1, x4) > gi(g0, g1, x5)) goto badltrdo;
		if (gi(g0, g1, x4) < gi(g0, g1, x5)) continue;
	badltrdo: 
		pointer = x8; 
		//printf("PRUNED VIA LT-RDO: pointer = %hhu\n", pointer);
		//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
		//if (x7 != 255) getchar();
		goto bad; 
	}

	for (nat i = 0; i < uncondga_count; i += 7) {
		const byte x0 = uncondga[i + 0];
		const byte x1 = uncondga[i + 1];
		const byte x2 = uncondga[i + 2];
		const byte x3 = uncondga[i + 3];
		const byte x4 = uncondga[i + 4];
		const byte x5 = uncondga[i + 5];
		const byte x6 = uncondga[i + 6];
		if (	(gi(g0, g1, x0 + 0) == x1 or gi(g0, g1, x0 + 0) == x2) and
			(gi(g0, g1, x0 + 1) == x3 or gi(g0, g1, x0 + 1) == x4) and 
			(gi(g0, g1, x0 + 2) == x5 or gi(g0, g1, x0 + 2) == x6)
		) { pointer = x0; goto bad; }
	}

	memcpy(graph, partial_graph, sizeof(u16) * operation_count);
	for (byte i = 0; i < pas_count; i++) {
		const byte pa = pas_map[i];
		graph[pa / 4] |= ((decode[i] >> (gi(g0, g1, i) << 2)) & 0xf) << ((pa % 4) << 2);
	}
	
	//puts("debug:");
	//print_graph_raw(graph); puts("");
	//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
	//puts("");
	//getchar();
	
	u16 was_utilized = 0;
	for (byte i = 0; i < operation_count; i++) {

		for (byte j = 0; j < 4 * 17; j += 4) {

			const byte A = loops[j + 0];
			const byte x = loops[j + 1];
			const byte B = loops[j + 2];
			const byte y = loops[j + 3];
			const byte K = (graph[i] >> (x << 2)) & 0xf;

			if (	(graph[i] & 0xf) == A and
				(graph[K] & 0xf) == B and 
				((graph[K] >> (y << 2)) & 0xf) == i
			) {
				byte at = graph_count - 1;
				if (rev_pas_map[4 * i + x] != 255 and at > 4 * i + x) at = 4 * i + x;							if (rev_pas_map[4 * K + y] != 255 and at > 4 * K + y) at = 4 * K + y;
				pointer = rev_pas_map[at];

				/*printf("pruned zv via 2-loops: \n"
					"pointer = %hhu, at = %hhu, j = %hhu\n", 
					pointer, at, (byte) (j / 4)
				);*/

				//getchar();

				goto bad;
			}
		}

		byte l = 0xf & (graph[i] >> 4);
		byte g = 0xf & (graph[i] >> 8);
		byte e = 0xf & (graph[i] >> 12);
		if (l != i) was_utilized |= 1 << l;
		if (g != i) was_utilized |= 1 << g;
		if (e != i) was_utilized |= 1 << e;
	}

	for (byte la = 0; la < operation_count; la++) {
		if (not ((was_utilized >> la) & 1)) { 

			//printf("pruned by uo...  use:  %016hx\n", was_utilized);
			//getchar();

			counts[pm_ga_uo]++; 
			pointer = 0; goto bad; 
		} 
	}

	//puts("trying to run graph...\n");
	//getchar();

	byte origin = 0;
	const byte is_bad = execute_graph(graph, array, &origin, counts, thread_index);

	/*if ((g0 & 0xFFFFF) == 0) { 
		printf("\033[H\033[2J");
		printf("\npm counts:\n");
        	for (nat i = 0; i < pm_count; i++) {
                	if (i and not (i % 2)) puts("");
			printf("%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
 	       }
 	       puts("\n[done]");
	}*/


	//puts("run graph.\n");

	if (not is_bad) {
		//puts("graph was good!\n");
		//getchar();

		append_to_file(filenames[thread_index], 4096, graph, origin);
	} 

	goto loop;
bad:	
	if (pointer + job_digit_count >= pas_count) goto pull_job_from_queue;

	for (byte i = 0; i < pointer; i++) {
		if (i < 16) g0 &= ~(0xfLLU << ((i & 15LLU) << 2LLU));
		else        g1 &= ~(0xfLLU << ((i & 15LLU) << 2LLU));
	}		
	goto loop;

terminate:
	free(array);
	free(graph);
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

static byte pas_encode(byte paspa, byte la) {
	nat count = 0;
	if (paspa >= 16) count = (mod1 >> ((paspa % 16) << 2)) & 0xf;
	else count = (mod0 >> (paspa << 2)) & 0xf;
	const nat array = decode[paspa];
	for (nat i = 0; i < count; i++) {
		const nat value = (array >> (i << 2)) & 0xf;
		if (value == la) return (byte) i;
	}
	printf("pas_encode: (paspa=%hhu, la=%hhu): invalid arguments to encode\n", paspa, la);
	abort();
}

#define disable_main 0

int main(void) {

	{ pas_count = 12; 
	nat k = 20;
	const u16 actual_partial_graph[operation_count] = {0x0010, 0x0001, 0x0002, 0x0003, 0x0404, };
	const byte zerosp_pas_map[32] = { 2, 3,   6, 7,   9, 10, 11,   13, 14, 15,    17, 19 };
	const byte zerosp_rev_pas_map[20] = {
		255, 	255, 	0, 	1,
		255, 	255, 	2, 	3, 
		255, 	4, 	5, 	6,
		255, 	7, 	8, 	9,
		255, 	10, 	255, 	11,
	};
	memcpy(partial_graph, actual_partial_graph, 5 * sizeof(u16));
	memcpy(pas_map, zerosp_pas_map, pas_count * sizeof(byte));
	memcpy(rev_pas_map, zerosp_rev_pas_map, 20 * sizeof(byte));
	for (byte i = 0; i < D; i++) {
		const byte value = (DOL >> (i << 2)) & 0xf;
		partial_graph[5 + i] = value | (value == six ? 0x0400 : 0);		

		rev_pas_map[k++] = 255;
		rev_pas_map[k++] = pas_count + 0;
		rev_pas_map[k++] = (value == six) ? 255 : pas_count + 1;
		rev_pas_map[k++] = pas_count + 2;
		
		pas_map[pas_count++] = 4 * (5 + i) + 1;
		if (value != six) pas_map[pas_count++] = 4 * (5 + i) + 2;
		pas_map[pas_count++] = 4 * (5 + i) + 3;
	}

	for (nat paspa = 0; paspa < pas_count; paspa++) {
		const nat pa = pas_map[paspa];
		const nat la = pa / 4;
		const nat trich = pa % 4;
		nat value = 0, count = 0;
		for (nat option = 0; option < operation_count; option++) {
			const byte actual_source = partial_graph[la] & 0xf;
			const byte actual_dest = partial_graph[option] & 0xf;
			for (nat i = 0; i < sizeof banned_edges; i += 3) {
				const byte source = banned_edges[i + 0];
				const byte side   = banned_edges[i + 1];
				const byte dest   = banned_edges[i + 2];
				if (actual_source != source) continue;
				if (side != trich) continue;
				if (actual_dest != dest) continue;
				goto skip_option;
			}
			if (la == option) {
				for (nat i = 0; i < sizeof banned_self_edges; i += 2) {
					const byte op   = banned_self_edges[i + 0];
					const byte side = banned_self_edges[i + 1];
					if (actual_source != op) continue;
					if (side != trich) continue;
					goto skip_option;
				}
			}
			value |= (nat) (option << (count << 2LLU));
			count++;
			skip_option: continue;
		}
		decode[paspa] = value;
		const nat loc = paspa % 16; 
		if (paspa >= 16) mod1 |= (nat) (count << (loc << 2));
		else 	         mod0 |= (nat) (count << (loc << 2));			
	}	
	job_placement_in_PAS = (4 * ((pas_count - 16) - (job_digit_count)));
	job_modulus = ((mod1 >> job_placement_in_PAS) & 0xffff);
	}


	// 1202601294.164259 note:    its possible  that LT RDO   can be integrated 
	//	into  the NF itself, to make for a dynamic moduli array,  
	//	making us skip the repetitive NF incr's  
	//	for situations where LT RDO is used constantly. 


	// generating uncond ga data/checks:


	// 3u1
	uncondga[uncondga_count++] = rev_pas_map[4 * three + 1];
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 1], one);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 1], 5);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 2], one);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 2], 5);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 3], one);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 3], 5);

	// 3u5
	uncondga[uncondga_count++] = rev_pas_map[4 * three + 1];
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 1], five);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 1], 7);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 2], five);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 2], 7);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 3], five);
	uncondga[uncondga_count++] = pas_encode(rev_pas_map[4 * three + 3], 7);



	// generating LT-RDO GA data:

	ltrdo[ltrdo_count++] = rev_pas_map[4 * one + 3];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 5 + 3];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * one + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 5 + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * one + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 5 + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 5 + 1];
	ltrdo[ltrdo_count++] = pas_encode(rev_pas_map[4 * 5 + 1], two);
	ltrdo[ltrdo_count++] = rev_pas_map[4 * one + 2];
	

	ltrdo[ltrdo_count++] = rev_pas_map[4 * five + 3];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 7 + 3];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * five + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 7 + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * five + 1];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 7 + 1];
	ltrdo[ltrdo_count++] = 0;
	ltrdo[ltrdo_count++] = 255;
	ltrdo[ltrdo_count++] = rev_pas_map[4 * five + 1];

	ltrdo[ltrdo_count++] = rev_pas_map[4 * six + 3];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 8 + 3];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * six + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 8 + 2];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * six + 1];
	ltrdo[ltrdo_count++] = rev_pas_map[4 * 8 + 1];
	ltrdo[ltrdo_count++] = 0;
	ltrdo[ltrdo_count++] = 255;
	ltrdo[ltrdo_count++] = rev_pas_map[4 * six + 1];




	printf("pas_count = %hhu\n\n", pas_count);
	printf("graph_count = %u\n\n", graph_count);
	printf("job_modulus = 0x%llx\n\n", job_modulus);
	printf("job_placement_in_PAS = %hhu\n\n", job_placement_in_PAS);
	printf("mod0 = 0x%016llx, mod1 = 0x%016llx \n\n", mod0, mod1);
	printf("partial_graph[] = "); print_graph_raw(partial_graph); puts("");
	printf("pas_map[] = { ");
	for (nat i = 0; i < pas_count; i++) {
		printf("%hhu, ", pas_map[i]);
	} printf("}\n");
	printf("rev_pas_map[] = { ");
	for (nat i = 0; i < graph_count; i++) {
		if (i % 4 == 0) puts("");
		printf("%hhu, ", rev_pas_map[i]);
	} printf("}\n");
	printf("decode[] = { ");
	for (nat i = 0; i < pas_count; i++) {
		if (i % 4 == 0) puts("");
		printf("0x%llx, ", decode[i]);
	} printf("}\n");


	printf("ltrdo[] = { ");
	for (nat i = 0; i < ltrdo_count; i++) {
		if (i % 4 == 0) puts("");
		printf("%hhu, ", ltrdo[i]);
	} printf("}\n");

	printf("uncondga[] = { ");
	for (nat i = 0; i < uncondga_count; i++) {
		if (i % 4 == 0) puts("");
		printf("%hhu, ", uncondga[i]);
	} printf("}\n");
	
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
loop:	if (((g0 >> (pointer << 2)) & 0xf) < ((job_modulus >> (pointer << 2)) & 0xf) - 1) goto increment;
	if (pointer < job_digit_count - 1) goto reset_;
	goto done;
reset_:
	g0 &= ~(0xfLLU << (pointer << 2));
	pointer++;
	goto loop;
increment:
	g0 += 1LLU << (pointer << 2);
init:	pointer = 0;
	mi = (mi + 1) % machine_count;
	if (mi != machine_index) goto loop;
	const nat n = atomic_fetch_add_explicit(&queue_count, 1, memory_order_relaxed);
	queue[n] = g0;
	total_job_count++;
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

	u16 graph[operation_count] = {0};
	nat counts[pm_count] = {0};	
	while (1) {
		const nat amount_remaining = atomic_load_explicit(&queue_count, memory_order_relaxed);
		if ((int64_t) amount_remaining <= 0 or disable_main) goto terminate;
		printf("\033[H\033[2J");
		printf("----------------- jobs remaining %llu / %llu -------------------\n", 
			amount_remaining, total_job_count
		);
		printf("\n\t complete %1.10lf%%\n\n", (double) (total_job_count - amount_remaining) / (double) total_job_count);
		for (nat thread = 0; thread < thread_count; thread++) {			
			for (byte i = 0; i < operation_count; i++) 
				graph[i] = atomic_load_explicit(progress + operation_count * thread + i, memory_order_relaxed);

			printf(" %5llu : ", thread);
			print_graph_raw(graph); 
			puts("");
		}
		puts("");
		//sleep(1 << display_rate);
		usleep(100000);
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




















/*

AT NF LOOP
AT NF INCR
AT INIT, DOING GA:
g0 = 0000000103154341, g1 = 0000000002653000

PRE CHECK ENABLED!!!
LTRDO PRE-PG-CHECK SUCCEEDED!! :   gi(g0, g1, x6) = 0,   x7 = 1
debug:


01481055263430204041 0021 100030564746

g0 = 0000

*/




















/*


XXXXXX

	// todo:    code up the version of ltrdo which has the partial graph accounting for  in it, 

	//        so that we can apply ltrdo   to   the instructions/operations  1 and 2  






uncond ga pms:

	5u1    (not needed becuase of our DOL)
	6u2    (not needed becuase of our DOL)

	3u1
	3u5
*/




/*

reverse pas map array:
example:


---------------
op   <  >  =
---------------
-1   -1 0  1
---------------
-1   -1 2  3
---------------
-1   4  5  6
---------------
-1   7  8  9
---------------
-1   10  -1  11
---------------


---------------
-1   12  13  14
---------------
-1   15  16  17
---------------
-1   18  19  20
---------------
-1   21  -1  22
---------------









	// GA ONLY APPLIES TO:   DOL: { 1, 2, 5, 6, } 





	for (i) {	
		if (	(gi(g0, g1, my_array[i + 0]) == my_array[i + 1] 
			or gi(g0, g1, my_array[i + 2]) == my_array[i + 3]) and
			(gi(g0, g1, 5) == 0 or gi(g0, g1, 5) == 3) and
			(gi(g0, g1, 6) == 0 or gi(g0, g1, 6) == 4)
		) { pointer = 4; goto bad; } 
	}


	*if (	(gi(g0, g1, 4) == 2 or gi(g0, g1, 4) == 6) and
		(gi(g0, g1, 5) == 1 or gi(g0, g1, 5) == 4) and
		(gi(g0, g1, 6) == 2 or gi(g0, g1, 6) == 6)
	) { pointer = 4; goto bad; } *

// GA:
	// .... 

	// we're going to put GA here, temporarily,to really see 
	// if putting GA before decode[] stage actaully has a 
	// significant effect on performance at all.  
	//at least, on paper, it should, becuase of LT-RDO.	







	// all PAS GA here     (skip for now)
	//  if (gi[4] < gi[5]) { pointer = 4; goto bad; } 
	//  if (gi[4] < gi[5]) { pointer = 4; goto bad; } 
	//  if (gi[4] < gi[5]) { pointer = 4; goto bad; } 
	//  if (gi[4] < gi[5]) { pointer = 4; goto bad; } 
	// if (gi(g0, g1, 4) < gi(g0, g1, 4))








1202601121.235253

pas_count = 23

job_modulus = 14197

job_placement_in_PAS = 12

mod0 = 0x9535377579799753, mod1 = 0x0000000003775797 

partial_graph[] = 010010002000300040400000100030004040

pas_map[] = { 2, 3, 6, 7, 9, 10, 11, 13, 14, 15, 17, 19, 21, 22, 23, 25, 26, 27, 29, 30, 31, 33, 35, }


decode[] = { 
	0x842, 0x86421, 0x8754320, 0x876543210, 
	0x876543210, 0x8754320, 0x876543210, 0x8654210, 
	0x85420, 0x8654210, 0x7653210, 0x621, 
	0x86421, 0x842, 0x86421, 0x876543210, 
	0x8754320, 0x876543210, 0x8654210, 0x85420, 
	0x8654210, 0x7653210, 0x621, 
}





























#define pas_count 	   23
#define job_digit_count    4

static uint16_t partial_graph[operation_count] = {0x0010, 0x0001, 0x0002, 0x0003, 0x0404,    0x0000, 0x0001, 0x0003, 0x0404, };

#define mod0 0x6537777378887553
#define mod1 0x0000000003777775

static byte pas_map[pas_count] = {
	2, 3,   6, 7,   9, 10, 11,   13, 14, 15,    17, 19,        21, 22, 23,    25, 26, 27,    29, 30, 31,    33, 35, 
};

static nat decode[pas_count] = {
	0x842,
	0x86421,

	0x75320,
	0x7653210,
	
	0x87654310,
	0x87654310,
	0x87654310,

	0x8654210,
	0x8654210,
	0x8654210,

	0x7653210,
	0x621,
	

	0x8654210,
	0x842,
	0x86421,

	0x753210,
	0x75320,
	0x7653210,

	0x8654210,
	0x8654210,
	0x8654210,

	0x7653210,
	0x621,
};



*/















































/*
const byte op = gi(g0, 0, 0);
	const byte l  = gi(g0, 0, 1);
	const byte g  = gi(g0, 0, 2);
	const byte e  = gi(g0, 0, 3);

	if (g == two) goto loop;

	if (op == one and l == 7) goto loop;
	if (op == one and g == 7) goto loop;
	if (op == one and e == 7) goto loop;
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
	if (op == one and l == one) goto loop;
	if (op == one and g == one) goto loop;
	if (op == one and e == one) goto loop;
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










#define dol   ((one << 0) | (two << 4) | (five << 8) | (six << 12))


* const byte pas_pa = pa_map[pa] 
const byte value = ((decode[pas_pa] >> (nfarray[pas_pa] << 2)) & 0xf;






pas_pa = 0  MEANS    opi0.g
pas_pa = 1  MEANS    opi0.e
pas_pa = 2  MEANS    opi1.g
pas_pa = 3  MEANS    opi1.e
pas_pa = 4  MEANS    opi2.l

...

pas_pa = 22   MEANS   addr8.opi4.e







*   DOL = {  1, 2, 5, 6,  }    *



static const nat decode[] = {


0123456789ABCDEFGHIJKLM


--------------------
	PA_MAP:
--------------------
ad | opi  <   >   =
--------------------
 0 | 0 |  /   0   1 
--------------------
 1 | 1 |  /   2   3 
--------------------
 2 | 2 |  4   5   6
--------------------
 3 | 3 |  7   8   9
--------------------
 4 | 4 |  A   /   B
--------------------
 5 | 0 |  C   D   E
--------------------
 6 | 1 |  F   G   H
--------------------
 7 | 3 |  I   J   K
--------------------
 8 | 4 |  L   /   M
--------------------

23 digits, aka  23 nat's   each nat encodes up to 16 possible address



opi0:
	pas[pa=0] = 0    :  MEANS     address 0  --(g)-->  address 2   





                                                   
* paspa 0 *	{   

	0 out [ns0],   
	1 out [sci],   
	2 in,   
	3 out [pco],  
	4 in,   
	5 out  [ns0],   
	6 



iiiiiiiiiiiru

* paspa 1 *	{3, 5, 6, 7, 8}
* paspa 2 *	{3, 5, 6, 7, 8}
* paspa 3 *	{3, 5, 6, 7, 8}
* paspa 4 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 /	{}
* paspa 2 	{}


};





//if (editable(ip * 4) and *zskip_at > ip * 4) *zskip_at = ip * 4;


//if (editable(ip * 4 + state) and *zskip_at > ip * 4 + state) *zskip_at = ip * 4 + state;



*/




/*

1202512291.093439

	notes:

	. first 16 paspa's are stored in g0.
	
	. the final remaining paspa's are stored in g1. 
	
	. g1 always contains the job data. 

	. pas_count is the total number of  clumps of 4 bits   present when looking at g0 and g1 collectetively.

	. job_digit_count is the number of paspa's which are part of the job. 
	





























--------------------
	PA_MAP:
--------------------

--------------------
ad | opi  <   >   =
--------------------

--------------------
 0 | 0 |  /   0   1 
--------------------
 1 | 1 |  /   2   3 
--------------------
 2 | 2 |  4   5   6
--------------------
 3 | 3 |  7   8   9
--------------------
 4 | 4 |  A   /   B
--------------------

--------------------
 5 | 0 |  C   D   E
--------------------
 6 | 1 |  F   G   H
--------------------
 7 | 3 |  I   J   K
--------------------
 8 | 4 |  L   /   M
--------------------



	0x842
	0x86421

	0x75320
	0x7653210
	
	0x87654310
	0x87654310
	0x87654310

	0x8654210
	0x8654210
	0x8654210

	0x7653210
	0x621
	
	0x8654210
	0x842
	0x86421

	0x753210
	0x75320
	0x7653210

	0x8654210
	0x8654210
	0x8654210

	0x7653210
	0x621




paspa 0 : opi0.g

	0 ns0
	1 sci 
	2 
	3 pco
	4 
	5 ns0
	6 sci
	7 pco
	8


paspa 1 : opi0.e

	0 ns0
	1 
	2
	3 pco
	4 
	5 ns0
	6
	7 pco
	8








paspa 2 : opi1.g

	0 
	1 sci
	2 
	3
	4 snco
	5 
	6 sci
	7 
	8 snco


paspa 3 : opi1.e

	0 
	1
	2 
	3
	4 snco
	5 
	6
	7 
	8 snco










paspa 4 : opi2.l

	0 
	1 
	2 ndi
	3 
	4
	5 
	6
	7 
	8

paspa 5 : opi2.g

	0 
	1 
	2 ndi
	3 
	4
	5 
	6
	7 
	8

paspa 6 : opi2.e

	0 
	1 
	2 ndi
	3 
	4
	5 
	6
	7 
	8






paspa 7 : opi3.l

	0 
	1 
	2
	3 zr5
	4 
	5 
	6 
	7 zr5
	8

paspa 8 : opi3.g

	0 
	1 
	2
	3 zr5
	4 
	5 
	6 
	7 zr5
	8

paspa 9 : opi3.e

	0 
	1 
	2
	3 zr5
	4 
	5 
	6 
	7 zr5
	8





paspa 10 : opi4.l

	0 
	1 
	2 
	3
	4 zr6
	5 
	6 
	7
	8 zr6

paspa 11 : opi4.e

	0 ns0
	1 
	2 
	3 ns0
	4 zr6
	5 ns0
	6 
	7 ns0
	8 zr6






paspa 12 : addr5.l

	0 
	1
	2 
	3 pco 
	4 
	5 
	6 
	7 pco
	8

paspa 13 : addr5.g

	0 ns0
	1 sci
	2 
	3 pco
	4 
	5 ns0
	6 sci
	7 pco
	8

paspa 14 : addr5.e

	0 ns0
	1 
	2 
	3 pco
	4 
	5 ns0
	6
	7 pco
	8






paspa 15 : addr6.l

	0
	1
	2
	3
	4 snco
	5
	6 sndi
	7
	8 snco

paspa 16 : addr6.g

	0
	1 sci
	2
	3
	4 snco
	5
	6 sci
	7
	8 snco


paspa 17 : addr6.e

	0
	1
	2
	3
	4 snco
	5
	6
	7
	8 snco






paspa 18 : addr7.l

	0 
	1
	2
	3 zr5
	4 
	5
	6
	7 zr5
	8 

paspa 19 : addr7.g

	0 
	1
	2
	3 zr5
	4 
	5
	6
	7 zr5
	8 

paspa 20 : addr7.e

	0 
	1
	2
	3 zr5
	4 
	5
	6
	7 zr5
	8 







paspa 21 : addr8.l

	0 
	1 
	2 
	3
	4 zr6
	5 
	6 
	7
	8 zr6

paspa 22 : addr8.e

	0 ns0
	1 
	2 
	3 ns0
	4 zr6
	5 ns0
	6 
	7 ns0
	8 zr6












// 3 5   5 7   8 8 8   7 7 7    7 3     7 3 5     6 5 7     7 7 7   7 3 


// 3 5 5 7   8 8 8 7    7 7 7 3   7 3 5 6             5 7 7  7 7 7 3 


















































































1202601132.002139

corrected decode and mod0 and mod1:



	3 5    5 7    8 6 8    7 5 7    7 3            6 3 5    6 5 7    7 5 7    7 3  




	mod0   0x6536377578687553
	mod1   0x3775775


842
86421

75320
7653210

87654310
875430
87654310

8654210
85420
8654210

7653210
621


864210
842
86421

753210
75320
7653210

8654210
85420
8653210

7653210
621







decode:
computed ones:

0x842, 
0x86421, 

0x75320, 
0x7653210, 

0x87654310, 
0x875430, 
0x87654310, 

0x8654210, 
0x85420, 
0x8654210, 

0x7653210, 
0x621, 




0x86421, 
0x842, 
0x86421, 

0x75320, 
0x75320, 
0x7653210, 

0x8654210, 
0x85420, 
0x8654210, 

0x7653210, 
0x621, 

}






paspa 0 : opi0.g

	0 ns0
	1 sci 
	2 
	3 pco
	4 
	5 ns0
	6 sci
	7 pco
	8


paspa 1 : opi0.e

	0 ns0
	1 
	2
	3 pco
	4 
	5 ns0
	6
	7 pco
	8








paspa 2 : opi1.g

	0 
	1 sci
	2 
	3 
	4 snco
	5 
	6 sci
	7 
	8 snco


paspa 3 : opi1.e

	0 
	1
	2 
	3
	4 snco
	5 
	6
	7 
	8 snco










paspa 4 : opi2.l

	0 
	1 
	2 ndi
	3 
	4
	5 
	6
	7 
	8

paspa 5 : opi2.g

	0 
	1 sci
	2 ndi
	3 
	4
	5 
	6 sci
	7 
	8

paspa 6 : opi2.e

	0 
	1 
	2 ndi
	3 
	4
	5 
	6
	7 
	8






paspa 7 : opi3.l

	0 
	1 
	2
	3 zr5
	4 
	5 
	6 
	7 zr5
	8

paspa 8 : opi3.g

	0 
	1 sci
	2
	3 zr5
	4 
	5 
	6 sci
	7 zr5
	8

paspa 9 : opi3.e

	0 
	1 
	2
	3 zr5
	4 
	5 
	6 
	7 zr5
	8





paspa 10 : opi4.l

	0 
	1 
	2 
	3
	4 zr6
	5 
	6 
	7
	8 zr6

paspa 11 : opi4.e

	0 ns0
	1 
	2 
	3 ns0
	4 zr6
	5 ns0
	6 
	7 ns0
	8 zr6






paspa 12 : addr5.l

	0
	1 
	2 
	3 pco 
	4 
	5 lb
	6 
	7 pco
	8

paspa 13 : addr5.g

	0 ns0
	1 sci
	2 
	3 pco
	4 
	5 ns0
	6 sci
	7 pco
	8

paspa 14 : addr5.e

	0 ns0
	1 
	2 
	3 pco
	4 
	5 ns0
	6
	7 pco
	8




paspa 15 : addr6.l

	0
	1
	2
	3
	4 snco
	5
	6 sndi
	7
	8 snco

paspa 16 : addr6.g

	0
	1 sci
	2
	3
	4 snco
	5
	6 sci
	7
	8 snco


paspa 17 : addr6.e

	0
	1
	2
	3
	4 snco
	5
	6
	7
	8 snco






paspa 18 : addr7.l

	0 
	1
	2
	3 zr5
	4 
	5
	6
	7 zr5
	8 

paspa 19 : addr7.g

	0 
	1 sci
	2
	3 zr5
	4 
	5
	6 sci
	7 zr5
	8 

paspa 20 : addr7.e

	0 
	1
	2
	3 zr5
	4 
	5
	6
	7 zr5
	8 







paspa 21 : addr8.l

	0 
	1 
	2 
	3
	4 zr6
	5 
	6 
	7
	8 zr6

paspa 22 : addr8.e

	0 ns0
	1 
	2 
	3 ns0
	4 zr6
	5 ns0
	6 
	7 ns0
	8 zr6









//     










*/




























