// major revision to the seach utility, 
// written on 1202602275.170743 by dwrr
// made to be faster for the UO execution path, 
// and also use the PAS system in some ways, including LTRDO
// but still use a zv for the central SU DS. 

#include <signal.h>
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

#define D 2
#define machine_count 10
#define thread_count 10
#define job_digit_count 6

enum operations { one, two, three, five, six };

typedef uint8_t byte;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t nat;

#define execution_limit 100000000LLU
#define array_size 1000000LLU

#define pg0  0x0003000200010010 
#define pg1  0x0000000000000404 
#define pg2  0x0

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static nat queue[30000000] = {0};
static _Atomic nat queue_count = 0;
static _Atomic nat progress[thread_count * 3] = {0};

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
	pm_erp1, pm_erp2,

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
	"pm_erp1", "pm_erp2",
};

static const byte loops[4 * 15] = {
	five, 3, two, 2,
	three, 3, two, 2,
	two, 3, three, 1,
	six, 3, three, 1,
	one, 1, three, 1,
	five, 3, three, 1,
	three, 1, five, 1, 
	three, 1, six, 1,
	three, 2, six, 1,
	three, 3, six, 1,
	five, 1, six, 1,
	five, 2, six, 1,
	five, 3, six, 1,
	one, 1, six, 1,
	three, 1, two, 1, 
};

#define g(x)   (gi(g0, g1, g2, x))

#define lsepa 2
__attribute__((always_inline)) static bool editable(nat p) {
	if (p < 20 and not (p & 3)) return false;
	if (p == 1 or p == 5 or p == 18) return false;	
	return true;
}

__attribute__((always_inline))
static byte gi(nat g0, nat g1, nat g2,  byte pa) {
	return ((pa < 16 ? g0 : pa < 32 ? g1 : g2) >> ((pa & 15) << 2)) & 0xf;
}

__attribute__((always_inline))
static u16 gi16(nat g0, nat g1, nat g2,  byte pa) {
	return ((pa < 16 ? g0 : pa < 32 ? g1 : g2) >> ((pa & 15) << 2)) & 0xffff;
}

static void print_graph_raw(nat g0, nat g1, nat g2) { 
	for (byte i = 0; i < graph_count; i++) 
		printf("%hhu", gi(g0, g1, g2, i)); 
}

static void get_graphs_z_value(char* string, nat g0, nat g1, nat g2) {
	for (byte i = 0; i < graph_count; i++) 
		string[i] = (char) (gi(g0, g1, g2, i) + '0');
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
	nat g0, nat g1, nat g2, 
	byte origin
) {
	char dt[32] = {0};   get_datetime(dt);
	char z[128] = {0};   get_graphs_z_value(z, g0, g1, g2);
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
	nat g0, nat g1, nat g2, 
	const byte origin, 
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
		BDL9_counter = 0, pair_index = 0, pair_count = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;

	nat performed_er_at = 0;
 	byte small_erp_array[max_erp_count]; small_erp_array[0] = 0;
	byte rsi_counter[max_rsi_count]; rsi_counter[0] = 0;

	byte has_executed_5 = 0;
	byte has_executed_6 = 0;
	
	for (nat e = 0; e < execution_limit; e++) {

		const byte op = g(4 * ip);
		
		if (op == one) {
			if (pointer == n) {
				puts("FEA condition violated by a z value: "); 
				print_graph_raw(g0, g1, g2);
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
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = g(4 * ip + state);
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
	nat g0, nat g1, nat g2, 
	byte* origin, 
	nat* array, 
	nat* counts, 
	const nat thread_index
) {
	for (byte o = 0; o < operation_count; o++) {
		byte op = g(4 * o);
		if (op != three) continue;
		const nat pm = execute_graph_starting_at(g0, g1, g2, o, array);
		atomic_store_explicit(progress + 3 * thread_index + 0, g0, memory_order_relaxed);
		atomic_store_explicit(progress + 3 * thread_index + 1, g1, memory_order_relaxed);
		atomic_store_explicit(progress + 3 * thread_index + 2, g2, memory_order_relaxed);
		counts[pm]++;
		if (not pm) { *origin = o; return false; }
		continue;
	}
	return true;
}

//#define printit  print_graph_raw(g0, g1, g2); puts("");

static void* worker_thread(void* raw_thread_index) {

	const nat thread_index = *(nat*) raw_thread_index;
	nat* counts = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));

	register nat g0 = 0;  
	register nat g1 = 0;
	register nat g2 = 0;
	register byte pointer = 0;

pull_job_from_queue:;
	const nat jobs_left = atomic_fetch_sub_explicit(&queue_count, 1, memory_order_relaxed);
	if ((int64_t) jobs_left <= 0) goto terminate;
	const nat job_pos = 3 * (jobs_left - 1);

	g0 = queue[job_pos + 0];
	g1 = queue[job_pos + 1];
	g2 = queue[job_pos + 2];

	//printf("pulled job: "); printit
	//getchar();

	goto init;
bad:	
	//printf("at bad, zskipping at %u: ", pointer); printit
	//getchar();

	if (pointer + job_digit_count >= graph_count) goto pull_job_from_queue;
	for (byte i = 0; i < pointer; i++) {
		if (not editable(i)) continue;
		const nat mask = ~(0xfLLU << ((i & 15) << 2));
		     if (i < 16) g0 &= mask;
		else if (i < 32) g1 &= mask;
		else             g2 &= mask;
	}
loop:
 	//puts("NF LOOP");
	if (g(pointer) < ((pointer & 3) ? operation_count - 1 : 4)) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto pull_job_from_queue;
reset_:;
	//puts("RESET");
	const nat mask = ~(0xfLLU << ((pointer & 15) << 2));
	     if (pointer < 16) g0 &= mask;
	else if (pointer < 32) g1 &= mask;
	else                   g2 &= mask;
	do pointer++; while (not editable(pointer));
	goto loop;
increment:;
	//puts("INCR");
	const nat addend = 1LLU << ((pointer & 15) << 2);
	     if (pointer < 16) g0 += addend;
	else if (pointer < 32) g1 += addend;
	else                   g2 += addend;
init:  	pointer = lsepa;

	//printf("dol:"); printit
	//getchar();

	for (byte i = D; i--;) {
		const byte pa = 4 * (5 + i);
		const byte op = g(pa);
		byte side = 0;

		if (op == six and g(4 * g(pa + 3)) == one) { side = 3; goto prune_edge; } // ns0.6e1
		if (op == six and g(4 * g(pa + 3)) == five) { side = 3; goto prune_edge; } // ns0.6e5
		if (op == six and g(4 * g(pa + 3)) == six) { side = 3; goto prune_edge; }  // zr6.e
		if (op == six and g(pa + 2) != six)        { side = 2; goto prune_edge; }  // 6g
		if (op == six and g(4 * g(pa + 1)) == six) { side = 1; goto prune_edge; }  // zr6.l 
		if (op == five and g(4 * g(pa + 3)) == five) { side = 3; goto prune_edge; } // zr5.e
		if (op == five and g(4 * g(pa + 2)) == five) { side = 2; goto prune_edge; } // zr5.g
		if (op == five and g(4 * g(pa + 1)) == five) { side = 1; goto prune_edge; } // zr5.l
		if (op == three and g(4 * g(pa + 3)) == three) { side = 3; goto prune_edge; } // ndi.e
		if (op == three and g(4 * g(pa + 2)) == three) { side = 2; goto prune_edge; } // ndi.g
		if (op == three and g(4 * g(pa + 1)) == three) { side = 1; goto prune_edge; } // ndi.l
		if (op == two and g(4 * g(pa + 3)) == six) { side = 3; goto prune_edge; } // snco.e
		if (op == two and g(4 * g(pa + 2)) == six) { side = 2; goto prune_edge; } // snco.g
		if (op == two and g(4 * g(pa + 1)) == six) { side = 1; goto prune_edge; } // snco.l
		if (g(4 * g(pa + 2)) == two) { side = 2; goto prune_edge; } // sci
		if (op == one and g(4 * g(pa + 3)) == five) { side = 3; goto prune_edge; } // pco.e
		if (op == one and g(4 * g(pa + 2)) == five) { side = 2; goto prune_edge; } // pco.g
		if (op == one and g(4 * g(pa + 1)) == five) { side = 1; goto prune_edge; } // pco.l
		if (op == one and g(4 * g(pa + 3)) == one) { side = 3; goto prune_edge; } // ns0.1e1
		if (op == one and g(4 * g(pa + 2)) == one) { side = 2; goto prune_edge; } // ns0.1g1 
		if (op == two and g(pa + 1) == pa >> 2) { side = 1; goto prune_edge; } // sndi
		if (op == one and g(pa + 1) == pa >> 2) { side = 1; goto prune_edge; } // lb

		for (byte opi = 5; opi--;) {
			if (	(opi != one and opi != two)
					or
				g(4 * opi + 1) == g(pa + 1)
			) {
				if (op == opi and gi16(g0,g1,g2, 4 * opi) >= gi16(g0,g1,g2, pa)) { 
					pointer = 4 * (opi + 1);
					while (not editable(pointer)) pointer++;
				}
			}
			for (byte j = 0; j < i; j++) {
				if (op == opi and gi16(g0,g1,g2, 4 * (5 + j)) >= gi16(g0,g1,g2, pa)) {
					pointer = 4 * (5 + j + 1);
					while (not editable(pointer)) pointer++;
				} 
			}
		}

		continue; prune_edge:;
		const byte k = 4 * g(pa + side);
		pointer = graph_count;
		if (editable(pa + side) and pointer > pa + side) pointer = pa + side; 
		if (editable(pa) and pointer > pa) pointer = pa; 
		if (editable(k) and pointer > k) pointer = k;
		goto bad;
	}

	//printf("0sp: "); printit
	//getchar();

	if (g(4 * g(4 * six   + 3)) == one)   { pointer = 4 * six + 3; goto bad; } 	// ns0.6e1
	if (g(4 * g(4 * six   + 3)) == five)  { pointer = 4 * six + 3; goto bad; } 	// ns0.6e5
	if (g(4 * g(4 * six   + 3)) == six)   { pointer = 4 * six + 3; goto bad; } 	// zr6.e
	if (g(4 * g(4 * six   + 1)) == six)   { pointer = 4 * six + 1; goto bad; }  	// zr6.l
	if (g(4 * g(4 * five  + 3)) == five)  { pointer = 4 * five + 3; goto bad; }	// zr5.e
	if (g(4 * g(4 * five  + 2)) == five)  { pointer = 4 * five + 2; goto bad; } 	// zr5.g
	if (g(4 * g(4 * five  + 2)) == two)   { pointer = 4 * five + 2; goto bad; }	// sci.5g
	if (g(4 * g(4 * five  + 1)) == five)  { pointer = 4 * five + 1; goto bad; }	// zr5.l
	if (g(4 * g(4 * three + 3)) == three) { pointer = 4 * three + 3; goto bad; }	// ndi.e
	if (g(4 * g(4 * three + 2)) == three) { pointer = 4 * three + 2; goto bad; }	// ndi.g
	if (g(4 * g(4 * three + 2)) == two)   { pointer = 4 * three + 2; goto bad; }	// sci.3g
	if (g(4 * g(4 * three + 1)) == three) { pointer = 4 * three + 1; goto bad; }	// ndi.l
	if (g(4 * g(4 * two   + 3)) == six)   { pointer = 4 * two + 3; goto bad; } 	// snco.e
	if (g(4 * g(4 * two   + 2)) == six)   { pointer = 4 * two + 2; goto bad; } 	// snco.g
	if (g(4 * g(4 * two   + 2)) == two)   { pointer = 4 * two + 2; goto bad; } 	// sci.2g
	if (g(4 * g(4 * one   + 3)) == one)   { pointer = 4 * one + 3; goto bad; } 	// ns0.1e1
	if (g(4 * g(4 * one   + 2)) == one)   { pointer = 4 * one + 2; goto bad; } 	// ns0.1g1
	if (g(4 * g(4 * one   + 3)) == five)  { pointer = 4 * one + 2; goto bad; } 	// pco.e
	if (g(4 * g(4 * one   + 2)) == five)  { pointer = 4 * one + 2; goto bad; } 	// pco.g
	if (g(4 * g(4 * one   + 2)) == two)   { pointer = 4 * one + 2; goto bad; } 	// sci.1g

	//printf(" ga:"); printit
	//getchar();

	u16 was_utilized = 0;
	{ byte pa = graph_count; 
	GA_loop: 
		pa -= 4;
		const byte op = g(pa);
		const byte l = g(pa + 1);
		const byte g = g(pa + 2);
		const byte e = g(pa + 3);

		for (byte i = 0; i < 4 * 15; i += 4) {
			const byte A = loops[i + 0];
			const byte x = loops[i + 1];
			const byte B = loops[i + 2];
			const byte y = loops[i + 3];
			const byte K = g(pa + x);
			const byte E = 4 * K + y;

			if (	op == A and
				g(4 * K) == B and
				g(E) == pa >> 2
			) {
				pointer = graph_count;
				if (editable(pa) and pointer > pa) pointer = pa;
				if (editable(pa + x) and pointer > pa + x) pointer = pa + x;
				if (editable(4 * K) and pointer > 4 * K) pointer = 4 * K;
				if (editable(E) and pointer > E) pointer = E;
				goto bad;
			}
		}
	
		if (l != pa >> 2) 			was_utilized |= 1 << l;
		if (g != pa >> 2 and op != six) 	was_utilized |= 1 << g;
		if (e != pa >> 2) 			was_utilized |= 1 << e;

		if (pa) goto GA_loop; } 

	for (byte la = 0; la < operation_count; la++) {
		if (not ((was_utilized >> la) & 1)) { pointer = lsepa; goto bad; } 
	}

	for (byte pa = 20; pa < graph_count; pa += 4) if (g(pa) == six) goto skip_6_2_check;
	if (	g(4 * g(4 * six + 1)) == two and
		g(4 * g(4 * six + 2)) == two and
		g(4 * g(4 * six + 3)) == two
	) { pointer = 4 * six + 1; goto bad; } skip_6_2_check:; 

	for (byte pa = 20; pa < graph_count; pa += 4) if (g(pa) == five) goto skip_5_1_check;
	if (	g(4 * g(4 * five + 1)) == one and
		g(4 * g(4 * five + 2)) == one and
		g(4 * g(4 * five + 3)) == one
	) { pointer = 4 * five + 1; goto bad; } skip_5_1_check:; 

	for (byte pa = 20; pa < graph_count; pa += 4) if (g(pa) == three) goto skip_3_15_check;
	if (	g(4 * g(4 * three + 1)) == one and
		g(4 * g(4 * three + 2)) == one and
		g(4 * g(4 * three + 3)) == one 
	) {pointer = 4 * three + 1; goto bad; }
	if (	g(4 * g(4 * three + 1)) == five and
		g(4 * g(4 * three + 2)) == five and
		g(4 * g(4 * three + 3)) == five 
	) {pointer = 4 * three + 1; goto bad; }
	skip_3_15_check:; 

	//printf("exg:"); printit
	//getchar();

	//abort();

	byte origin = 255;
	const byte is_bad = execute_graph(g0, g1, g2, &origin, array, counts, thread_index);

	//printf("at finished exg, appending: "); printit
	//getchar();

	if (not is_bad) append_to_file(filenames[thread_index], 4096, g0, g1, g2, origin);
	goto loop;

terminate:
	free(array);
	atomic_store_explicit(progress + 3 * thread_index + 0, g0, memory_order_relaxed);
	atomic_store_explicit(progress + 3 * thread_index + 1, g1, memory_order_relaxed);
	atomic_store_explicit(progress + 3 * thread_index + 2, g2, memory_order_relaxed);
	return counts;
}

static char output_filename[4096] = {0};
static char output_string[4096] = {0};

static void publish(void) {
	char dt[32] = {0};   get_datetime(dt);
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;
try_open:;
	const int file = open(output_filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("print: [%s]: failed to create output_filename = \"%s\"\n", dt, output_filename);
			fflush(stdout);
			abort();
		}
		snprintf(output_filename, sizeof output_filename, "%s_D%u_%08x%08x%08x%08x_output.txt", dt, D,
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}
	dt[17] = ' '; dt[18] = ':';
	dt[19] = ':'; dt[20] = ' ';
	write(file, dt, 21);
	write(file, output_string, strlen(output_string));
	close(file);
}

static nat machine_index = (nat) -1;
static nat total_job_count = 0;

static void hangup_signal_handler(int sig) {

	nat local_progress[3 * thread_count] = {0};
	char progress_string[4096] = {0};
	int string_length = 0;

	const nat amount_remaining = atomic_load_explicit(&queue_count, memory_order_relaxed);

	for (nat thread = 0; thread < thread_count; thread++) {
		const nat g0 = atomic_load_explicit(progress + 3 * thread + 0, memory_order_relaxed);
		const nat g1 = atomic_load_explicit(progress + 3 * thread + 1, memory_order_relaxed);
		const nat g2 = atomic_load_explicit(progress + 3 * thread + 2, memory_order_relaxed);
		local_progress[thread * 3 + 0] = g0;
		local_progress[thread * 3 + 1] = g1;
		local_progress[thread * 3 + 2] = g2;
		string_length += snprintf(progress_string + string_length, 
			sizeof progress_string, 
			" %3llu : ", thread
		); 
		get_graphs_z_value(progress_string + string_length, g0, g1, g2);
		string_length += graph_count;
		progress_string[string_length++] = 10; 
		progress_string[string_length] = 0;
	}

	snprintf(output_string, sizeof output_string, 
		"------------ mi %llu on signal %d: file %s------------\n"	
		"thread_count = %u, machine_count = %u\n"
		"job_digit_count = %u, total_job_count = %llu\n"
		"graph_count = %u, operation_count = %u\n"
		"array_size = %llu, execution_limit = %llu\n"
		"\n"
		"queue count: jobs %llu / %llu : %1.3lf%% remaining\n"
		"%s\n"
		"SU: [PID = %d]: searching [D = %u] space...\n"
	,
		machine_index, sig, output_filename, 
		thread_count, machine_count, 
		job_digit_count, total_job_count, 
		graph_count, operation_count, 
		array_size, execution_limit,
		amount_remaining, total_job_count, 
		100.0 * ((double) amount_remaining / (double) total_job_count),
		progress_string,
		getpid(), D
	); 
	publish();
}

static char* get_command_output(const char* input_command) {
	char command[4096] = {0};
	snprintf(command, sizeof command, "%s 2>&1", input_command);
	FILE* f = popen(command, "r");
	if (not f) { perror("popen"); abort(); }
	char* string = NULL;
	size_t length = 0;
	char line[2048] = {0};
	while (fgets(line, sizeof line, f)) {
		size_t l = strlen(line);
		string = realloc(string, length + l);
		memcpy(string + length, line, l);
		length += l;
	}
	pclose(f);
	return strndup(string, length);
}

static nat translate_hostname_to_machine_index(char* s) {
	if (not strcmp(s, "dwrr-mini.local\n")) return 0;
	if (not strcmp(s, "dwrr-mini1.local\n")) return 1;
	if (not strcmp(s, "dwrr-mini2.local\n")) return 2;
	if (not strcmp(s, "dwrr-mini3.local\n")) return 3;
	if (not strcmp(s, "dwrr-mini4.local\n")) return 4;
	if (not strcmp(s, "dwrr-mini5.local\n")) return 5;
	if (not strcmp(s, "dwrr-mini6.local\n")) return 6;
	if (not strcmp(s, "dwrr-mini7.local\n")) return 7;
	if (not strcmp(s, "dwrr-mini8.local\n")) return 8;
	if (not strcmp(s, "dwrr-mini9.local\n")) return 9;
	if (not strcmp(s, "dwrr.local\n")) return 0;
	puts(s); abort();
}
	
int main(void) { // int argc, const char** argv

	//if (argc != 2) usage_error: return puts("usage: ./run <machine index>");
	//char* end = NULL;
	//machine_index = strtoull(argv[1], &end, 10);
	//if (not *argv[1] or *end) goto usage_error;

	machine_index = translate_hostname_to_machine_index(get_command_output("hostname"));

	signal(SIGHUP, hangup_signal_handler);

	srand(20000000);
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	nat counts[pm_count] = {0};
	memset(queue, 0, sizeof *queue * 65536);
	atomic_store(&queue_count, 0);
	for (nat i = 0; i < thread_count * 3; i++) 
		atomic_store_explicit(progress + i, 0, memory_order_relaxed);

{	byte pointer = 0;
	register nat g0 = pg0;
	register nat g1 = pg1;
	register nat g2 = pg2;
	goto init;

loop: 	if (g(pointer) < ((pointer & 3) ? operation_count - 1 : 4)) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto done;
reset_:;
	const nat mask = ~(0xfLLU << ((pointer & 15) << 2));
	     if (pointer < 16) g0 &= mask;
	else if (pointer < 32) g1 &= mask;
	else                   g2 &= mask;
	do pointer++; while (not editable(pointer));
	goto loop;
increment:;
	const nat addend = 1LLU << ((pointer & 15) << 2);
	     if (pointer < 16) g0 += addend;
	else if (pointer < 32) g1 += addend;
	else                   g2 += addend;
init:	pointer = graph_count - job_digit_count;
	while (not editable(pointer)) pointer++; 
	if ((nat) rand() % machine_count != machine_index) goto loop;
	const nat n = atomic_fetch_add_explicit(&queue_count, 1, memory_order_relaxed);
	queue[3 * n + 0] = g0;
	queue[3 * n + 1] = g1;
	queue[3 * n + 2] = g2;
	total_job_count++;
	goto loop;
done:; }	
	srand((unsigned) time(0));
	filenames = calloc(thread_count, sizeof(char*));
	for (nat i = 0; i < thread_count; i++) {
		filenames[i] = calloc(4096, 1);
		char dt[32] = {0};
		get_datetime(dt);
		snprintf(filenames[i], 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
	}
	printf("SU: [PID %d]: [D %u]: [machine %llu]: starting search call...\n", getpid(), D, machine_index);
	snprintf(output_string, sizeof output_string, 
	     "SU: [PID %d]: [D %u]: [machine %llu]: starting search call...\n", getpid(), D, machine_index
	);
	publish();

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < thread_count; i++) {
		nat* arg = malloc(sizeof(nat)); *arg = i;
		pthread_create(threads + i, NULL, worker_thread, arg);
	}

	puts("[search call running]\n");

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

	hangup_signal_handler(0);

	char string[8192] = {0};
	int length = 0;
	length += snprintf(string + length, sizeof string, "\npm counts:\n");
        for (nat i = 0; i < pm_count; i++) {
                if (i and not (i % 2)) string[length++] = 10;
		length += snprintf(string + length, sizeof string, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
        }
        length += snprintf(string + length, sizeof string, "[done]\n");

	snprintf(output_string, sizeof output_string,
		"SU: finished searching [D = %u] space:\n"
		"\t search took %10.2lf seconds\n"
		"\t from %s\n"
		"\t to   %s\n"
		"\n"
		"%s\n"
		"\n",
		D, seconds, time_begin_dt, time_end_dt,
		string
	);
	publish();
	puts("[search call complete]");
} // main
































































// -----------------------------------------------------------------------------------------------------------------------------------------------











//static void clear_screen(void) { printf("\033[H\033[2J"); } 

/*static void print_counts(nat* counts) {
	printf("\npm counts:\n");
        for (nat i = 0; i < pm_count; i++) {
                if (i and not (i % 2)) puts("");
		printf("%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
	}
	puts("\n[done]");
}*/

/*static void print_binary(nat x) {
	for (nat i = 0; i < 64; i++) {
		if (not (i & 3)) putchar('_');
		printf("%llu", (x >> i) & 1);
	}
}*/





	/*if (debug) {
		printf("printing jobs: (%llu total jobs)\n", total_job_count);
		for (nat i = 0; i < total_job_count; i++) {

			if (not (i % 2)) putchar(' ');
			if (not (i % 4)) putchar(10);
			if (not (i % 8)) putchar(10);

			const nat g0 = queue[3 * i + 0];
			const nat g1 = queue[3 * i + 1];
			const nat g2 = queue[3 * i + 2];
			print_graph_raw(g0, g1, g2); putchar(' ');
		}
		puts("");
	}*/








/* 1202602275.181159 previous uncond and ltrdo: ---------------------------


	// PAS GA:
	// if (gi(g0,g1,g2, 20) == 1 and gi(g0,g1,g2, 5) == 5) goto loop;







	for (nat i = 0; i < ltrdo_count; i += 8) {
		const byte x0 = ltrdo[i + 0];
		const byte x1 = ltrdo[i + 1];
		const byte x2 = ltrdo[i + 2];
		const byte x3 = ltrdo[i + 3];
		const byte x4 = ltrdo[i + 4];
		const byte x5 = ltrdo[i + 5];
		const byte x6 = ltrdo[i + 6];
		const byte x7 = ltrdo[i + 7];



		if (x7 != 255 and gi(g0, g1, x6) != x7) continue;
		if (gi(g0, g1, x0) > gi(g0, g1, x1)) goto prune;
		if (gi(g0, g1, x0) < gi(g0, g1, x1)) continue;
		if (gi(g0, g1, x2) > gi(g0, g1, x3)) goto prune;
		if (gi(g0, g1, x2) < gi(g0, g1, x3)) continue;
		if (gi(g0, g1, x4) > gi(g0, g1, x5)) goto prune;
		if (gi(g0, g1, x4) < gi(g0, g1, x5)) continue;

		prune: if (	not gi(g0, g1, x1) and 
				not gi(g0, g1, x3) and 
				not gi(g0, g1, x6)
			) pointer = x6; else pointer = x0 + 1;
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
			(x5 == 255 or gi(g0, g1, x0 + 2) == x5 or gi(g0, g1, x0 + 2) == x6)
		) { pointer = x0; goto bad; }
	}

	memcpy(graph, partial_graph, sizeof(u16) * operation_count);
	for (byte i = 0; i < pas_count; i++) {
		const byte pa = pas_map[i];
		graph[pa / 4] |= ((decode[i] >> (gi(g0, g1, i) << 2)) & 0xf) << ((pa % 4) << 2);
	}	















	for (byte i = operation_count; i--) {




		for (byte j = 0; j < 4 * 15; j += 4) {
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
				if (rev_pas_map[4 * i + x] != 255 and at > 4 * i + x) at = 4 * i + x;
				if (rev_pas_map[4 * K + y] != 255 and at > 4 * K + y) at = 4 * K + y;
				pointer = rev_pas_map[at];
				goto bad;
			}
		}
		const byte op = 0xf & graph[i];
		const byte l = 0xf & (graph[i] >> 4);
		const byte g = 0xf & (graph[i] >> 8);
		const byte e = 0xf & (graph[i] >> 12);

		if (l != i) 			was_utilized |= 1 << l;
		if (g != i and op != six) 	was_utilized |= 1 << g;
		if (e != i) 			was_utilized |= 1 << e;
	}
	for (byte la = 0; la < operation_count; la++) {
		if (not ((was_utilized >> la) & 1)) { pointer = lsepa; goto bad; } 
	}


*/




	/*if ((g0 & 0xFFFFF) == 0) { 
		clear_screen();
		print_counts(counts);
	}*/






		/*if (x7 != 255) {
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
			//printf("PRUNED VIA LT-RDO: pointer = %hhu    ", pointer);
			//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);		
			//puts("PASSED PRE CHECK!");
			//getchar();
		}*/













/*

g0 = 	(1LLU << 4) |
                (two << (4 * 4)) | 
                (three << (8 * 4)) | 
                (five << (12 * 4));
        g1 =    six | (4LLU << 8);
	g2 = 0;

	for (nat i = 0; i < job_digit_count; i++) {
		const nat pa = graph_count - job_digit_count + i;
		const nat job_digit = (job >> (i << 2)) & 0xf;
		const nat position = (pa & 0xf) << 2;
		const nat j = job_digit << position;
		     if (pa < 16) g0 |= j;
		else if (pa < 32) g1 |= j;
		else              g2 |= j;		
	}


	// placing the job into the zv, based on job digit count!)
	//    0142 1316 2134 3144     4242 0124 1402 3630       4240
	//                                           ^^^^       ^^^^





if (g(20) != one) { pointer = 20; goto bad; } // partial_graph:  DOL must begin with the instruction 1. 

	//if (p == 20) return false;







	puts("pulled job::");
	printf("g: %016llx:%016llx:%016llx\n", g0, g1, g2);
	getchar();



if ((g0 & 0xFFFFFFFFFF) == 0) { 
		//puts("debug:");
		printf("g0 = %016llx, g1 = %016llx\n", g0, g1);  //print_graph_raw(graph); putchar(9); putchar(9); 
		//puts("");
		//getchar();
	}

	//puts("AT INIT, DOING GA:");
	//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
	//puts("");



if ((g0 & 0xFFFFFFFFFF) == 0) { 
		//puts("debug:");
		printf("g0 = %016llx, g1 = %016llx\n", g0, g1);  //print_graph_raw(graph); putchar(9); putchar(9); 
		//puts("");
		//getchar();
	}*

	//puts("AT INIT, DOING GA:");
	//printf("g0 = %016llx, g1 = %016llx\n", g0, g1);
	//puts("");








	0 1 2 3 4 5 6 7 
      0 .
      1   .
      2     . 
      3       . 
      4         . 
      5           . 
      6             .
      7               .










	(A, B, C, D)  	   A < B < C < D



	(A, B, C)  	A < B < C



	(A, B)     A < B














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


*/
















