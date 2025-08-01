// a search utility for testing the 4o hypothesis, 
// written on 1202507233.194207 by dwrr
// got the pre graph formation working i think, 1202507244.004153
//
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <iso646.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>

typedef uint8_t byte;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t nat;

#define duplicate_opi one
#define thread_count 1
#define display_rate 1
#define execution_limit 100000000LLU
#define array_size 1000000LLU

static pthread_mutex_t mutex;
static _Atomic byte* current_zv_progress = NULL;
static u32 edge_direction_queue[7000] = {0};
static nat queue_count = 0;
static u32 node_permutations[7000] = {0};
static nat permutation_count = 0;
static char** filenames = NULL;

static byte edgedir_sources[24] = {
	0, 0, 0, 2, 3, 1, 0, 0,
	0, 4, 5, 6, 4, 6, 4, 5,
	5, 6, 7, 7, 7, 7, 7, 7,
};

static byte edgedir_destinations[24] = {
	1, 2, 3, 1, 2, 3, 4, 5, 
	6, 6, 4, 5, 1, 1, 2, 2,
	3, 3, 1, 4, 2, 5, 6, 3,
};

static const byte edges_indexes_per_node[8 * 6] = { 
	 0,  1,  2,  6,  7,  8,  
	 0,  3,  5, 13, 12, 18,
	 3,  4,  1, 14, 15, 20,
	 2,  4,  5, 17, 16, 23,
	 6,  9, 10, 12, 14, 19,
	 7, 16, 11, 10, 15, 21,
	 8, 17, 11,  9, 13, 22,
	18, 19, 20, 21, 22, 23,
};

static const byte edge_sources_per_node[8 * 6] = {
	1, 2, 3, 4, 5, 6,
	0, 2, 3, 6, 4, 7,
	1, 3, 0, 4, 5, 7,
	0, 2, 1, 6, 5, 7,
	0, 6, 5, 1, 2, 7,
	0, 3, 6, 4, 2, 7,
	0, 3, 5, 4, 1, 7,
	1, 4, 2, 5, 6, 3,
};

static const byte direction_is_output_per_node[8 * 6] = {
	0, 0, 0, 0, 0, 0,
	1, 1, 0, 1, 1, 1,
	0, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 1, 1,
	1, 0, 1, 0, 0, 1,
	1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1,
	0, 0, 0, 0, 0, 0,
};

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

	pm_ga_zr6, pm_ga_zr5, pm_ga_ndi,  // if duplicating a 3 / 5 / 6
	pm_ga_ns0, pm_ga_pco, pm_ga_snco,

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

	"pm_ga_zr6", "pm_ga_zr5", "pm_ga_ndi",
	"pm_ga_ns0", "pm_ga_pco", "pm_ga_snco",
};

#define D 3
#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }

static void get_graphs_z_value(char string[64], byte* graph) {
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
}

/*static void print_binary(uint32_t x) {
	for (byte i = 0; i < 32; i++) 
		if ((x >> i) & 1) printf("1"); else printf("0");
}

static void print_bytes(byte* a, byte c) {
	printf("(%hhu):{", c);
	for (byte i = 0; i < c; i++) 
		printf("%hhu, ", a[i]);
	printf("}");
}*/

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

static void try_trichotomy_colorings(
	byte* nfarray,
	byte* graph,
	byte* pre_graph,
	nat* array,
	nat* counts,
	nat thread_index
) {
	for (byte i = 0; i < operation_count; i++) 
		graph[4 * i + 0] = pre_graph[4 * i + 0];
	
	byte pointer = 0;
	goto init;
loop:
	if (nfarray[pointer] < 6 - 1) goto increment;
	if (pointer < operation_count - 1) goto reset_;
	goto done;

increment:
	nfarray[pointer]++;
init:  	pointer = 0;

	for (byte i = 0; i < operation_count; i++) {

		const byte l = 4 * i + 1;
		const byte g = 4 * i + 2;
		const byte e = 4 * i + 3;

		const byte A = pre_graph[l];
		const byte B = pre_graph[g];
		const byte C = pre_graph[e];

	       if (nfarray[i] == 0) { graph[l] = A; graph[g] = B; graph[e] = C; } else 
	       if (nfarray[i] == 1) { graph[l] = A; graph[g] = C; graph[e] = B; } else 

	       if (nfarray[i] == 2) { graph[l] = B; graph[g] = A; graph[e] = C; } else 
	       if (nfarray[i] == 3) { graph[l] = B; graph[g] = C; graph[e] = A; } else 

	       if (nfarray[i] == 4) { graph[l] = C; graph[g] = A; graph[e] = B; } else 
	       if (nfarray[i] == 5) { graph[l] = C; graph[g] = B; graph[e] = A; }
	}
	
	// (do graph_analysis here!)

	for (nat i = 0; i < graph_count; i++) {
		atomic_store_explicit(
			current_zv_progress + 
				graph_count * thread_index + i, 
			graph[i], 
			memory_order_relaxed
		);
	}
	


	byte origin = 0;
	byte at = execute_graph(graph, array, &origin, counts);

	if (not at) {
		append_to_file(filenames[thread_index], 4096, graph, origin);
		goto loop;
	} else goto loop;

reset_:;
	nfarray[pointer] = 0; 
	pointer++;
	goto loop;

done:;
}

static void* worker_thread(void* raw_thread_index) {

	const nat thread_index = *(nat*) raw_thread_index;
	nat* counts = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));
	
	byte nfarray[operation_count] = {0};
	byte graph[graph_count] = {0};
	byte pre_graph[graph_count] = {0};

pull_job_from_queue:
	pthread_mutex_lock(&mutex);
	const u32 edge_direction = queue_count ? edge_direction_queue[--queue_count] : 0;
	pthread_mutex_unlock(&mutex);
	if (not edge_direction) goto terminate;

	for (nat p = 0; p < permutation_count; p++) {
		const u32 permutation = node_permutations[p];

		memset(pre_graph, 0, graph_count);

		for (byte pli = 0; pli < 8; pli++)
			pre_graph[4 * pli + 0] = (permutation >> (3U * pli)) & 7;

		for (byte pli = 0; pli < 8; pli++) {
			byte count = 0;
			for (byte edge = 0; edge < 6; edge++) {
				const byte n = pli * 6 + edge;
				const byte edge_is_complemented = !!(edge_direction & (1 << edges_indexes_per_node[n]));
				const byte is_output = direction_is_output_per_node[n];
				const byte source = edge_sources_per_node[n];

				if (is_output xor edge_is_complemented) {
					if (count >= 3) {
						puts("internal error: node has more than 3 outputs."); 
						abort(); 
					}
					pre_graph[4 * pli + 1 + count] = source;
					count++;
				}
			}
		}

		for (nat i = 0; i < operation_count; i++) {
			bool source = (pre_graph[4 * i + 0] == five);
			bool l = (pre_graph[4 * pre_graph[4 * i + 1] + 0] == one);
			bool g = (pre_graph[4 * pre_graph[4 * i + 2] + 0] == one);
			bool e = (pre_graph[4 * pre_graph[4 * i + 3] + 0] == one);
			if (source and (l or g or e)) {
				counts[pm_ga_pco]++;
				goto next_node_permutation;
			}
		}

		for (nat i = 0; i < operation_count; i++) {
			bool source = (pre_graph[4 * i + 0] == six);
			bool l = (pre_graph[4 * pre_graph[4 * i + 1] + 0] == two);
			bool g = (pre_graph[4 * pre_graph[4 * i + 2] + 0] == two);
			bool e = (pre_graph[4 * pre_graph[4 * i + 3] + 0] == two);
			if (source and (l or g or e)) {
				counts[pm_ga_snco]++;
				goto next_node_permutation;
			}			
		}

		if (duplicate_opi >= three) {
			for (nat i = 0; i < operation_count; i++) {
				bool source = (pre_graph[4 * i + 0] == duplicate_opi);
				bool l = (pre_graph[4 * pre_graph[4 * i + 1] + 0] == duplicate_opi);
				bool g = (pre_graph[4 * pre_graph[4 * i + 2] + 0] == duplicate_opi);
				bool e = (pre_graph[4 * pre_graph[4 * i + 3] + 0] == duplicate_opi);
				if (source and (l or g or e)) {
					if (duplicate_opi == three) counts[pm_ga_ndi]++;
					if (duplicate_opi == five) counts[pm_ga_zr5]++;
					if (duplicate_opi == six) counts[pm_ga_zr6]++;
					goto next_node_permutation;
				}			
			}
		}
		try_trichotomy_colorings(nfarray, graph, pre_graph, array, counts, thread_index);
		next_node_permutation: continue;
	}
	goto pull_job_from_queue;
terminate:
	free(array);
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

static void edgedir_count_outputs(byte* nodes, uint32_t bits) {
	for (byte i = 0; i < 24; i++) {
		if (bits & (1 << i)) { 
			nodes[edgedir_sources[i]]++;
			nodes[edgedir_destinations[i]]--;
		}
	}
}

static void edgedir_count_inputs(byte* nodes, uint32_t bits) {
	for (byte i = 0; i < 24; i++) {
		if (bits & (1 << i)) {
			nodes[edgedir_sources[i]]--;
			nodes[edgedir_destinations[i]]++;
		}
	}
}

static void generate_permutation_data(byte dol2) {

	byte nodes[8] =   {0, 1, 2, 3, 4, 0, 1, dol2};
	byte moduli[8] =  {7, 6, 5, 4, 3, 2, 1, 0};
	byte pointer = 0;
	byte placements[8] = {0};
	byte array[8] = {0};

	goto init;

loop:
	if (array[pointer] < moduli[pointer]) goto increment;
	if (pointer < 7) goto reset_;
	goto done;
increment:
	array[pointer]++;
init:  	pointer = 0;

	memset(placements, 255, 8);
	for (byte i = 0; i < 8; i++) {
		const byte skip_count = array[i];
		byte s = 0;
		bool placed = false;
		for (byte p = 0; p < 8; p++) {
			if (placements[p] != 255) continue;
			if (placements[p] == 255 and s < skip_count) { s++; continue; } 
			if (placements[p] == 255 and s >= skip_count) {
				placements[p] = nodes[i];
				placed = true; break;
			}
		}
		if (not placed) abort();		
	}

	const u32 data = 
		((u32) placements[0] << (3U * 0U)) | 
		((u32) placements[1] << (3U * 1U)) | 
		((u32) placements[2] << (3U * 2U)) | 
		((u32) placements[3] << (3U * 3U)) | 
		((u32) placements[4] << (3U * 4U)) | 
		((u32) placements[5] << (3U * 5U)) | 
		((u32) placements[6] << (3U * 6U)) | 
		((u32) placements[7] << (3U * 7U));

	for (nat i = 0; i < permutation_count; i++)
		if (node_permutations[i] == data) goto loop;
	node_permutations[permutation_count++] = data;
	goto loop;
reset_:
	array[pointer] = 0; 
	pointer++;
	goto loop;
done:
	return;
}

int main(void) {
	srand((unsigned) time(0));

	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);

	current_zv_progress = calloc(graph_count * thread_count, sizeof(_Atomic byte));
	for (nat thread = 0; thread < thread_count; thread++) {
		for (nat i = 0; i < graph_count; i++) 
			atomic_init(current_zv_progress + graph_count * thread + i, 0);
	}
	
	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};

	puts("stage: generating filenames...");
	filenames = calloc(thread_count, sizeof(char*));
	for (nat i = 0; i < thread_count; i++) {
		filenames[i] = calloc(1024, 1);
		char dt[32] = {0};   
		get_datetime(dt);
		snprintf(filenames[i], 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
	}

	puts("stage: computing valid edge direction jobs...");

	for (u32 bits = 0; bits < (1U << 24U); bits++) {
		byte edgedir_outputs[8] = { 0, 5, 5, 5, 3, 3, 3, 0 };
		edgedir_count_outputs(edgedir_outputs, bits);
		for (byte i = 0; i < 8; i++) if (edgedir_outputs[i] != 3) goto next_bit_vector;

		byte edgedir_inputs[8] = { 6, 1, 1, 1, 3, 3, 3, 6 };
		edgedir_count_inputs(edgedir_inputs, bits);
		for (byte i = 0; i < 8; i++) if (edgedir_inputs[i] != 3) goto next_bit_vector; 

		edge_direction_queue[queue_count++] = bits;
		//print_binary(bits); printf(" : GOOD\n");
		continue;
		next_bit_vector: continue;
	}

	printf("---> pushed %llu jobs to the ED queue\n", queue_count);
	const nat total_job_count = queue_count;

	printf("generating node permutation data for DOL[2] = %u\n", duplicate_opi);
	generate_permutation_data(duplicate_opi);
	printf("permutation_count = %llu\n", permutation_count);
	printf("all permutations computed:");

	snprintf(output_string, 4096, "SU: searching [D=%u] space....\n", D);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);
	nat counts[pm_count] = {0};

	for (nat i = 0; i < thread_count; i++) {
		nat* arg = malloc(sizeof(nat));
		*arg = i;
		pthread_create(threads + i, NULL, worker_thread, arg);
	}

	while (1) {
		pthread_mutex_lock(&mutex);
		const nat amount_remaining = queue_count;
		pthread_mutex_unlock(&mutex);
		if (not amount_remaining) goto join_threads;

		//sleep(8); continue;

		printf("\033[H\033[2J");
		printf("\n---------- remaining: %llu / %llu ---------\n", 
			amount_remaining, total_job_count
		);
		printf("\n\t%1.10lf%%\n\n", 
			(double) (total_job_count - amount_remaining) / (double) total_job_count
		);
		puts("");

		for (nat i = 0; i < thread_count; i++) {
			byte local_graph[graph_count] = {0};
			for (nat e = 0; e < graph_count; e++)
				local_graph[e] = atomic_load_explicit(
					current_zv_progress + graph_count * i + e,  
					memory_order_relaxed
				);

			printf(" %llu:    ", i); 
			print_graph_raw(local_graph);
			printf("\n");
		}
		puts("");	
		sleep(1 << display_rate);
		//usleep(10000);
	}
join_threads:
	puts("\nmain: joining threads...\n");
	for (nat i = 0; i < thread_count; i++) {
		nat* result = NULL;
		pthread_join(threads[i], (void**) &result);
		for (nat j = 0; j < pm_count; j++) counts[j] += result[j];
		free(result);
	}

	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec);
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	snprintf(output_string, 4096,
		"su: \n"
		"\t D = %u\n"
		"\t total_job_count = %llu\n"
		"\t execution_limit = %llu\n"
                "\t array_size = %llu\n"
		"\t in %10.2lfs [%s:%s]\n"
		"\n",
		D,
		total_job_count, 
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

	pthread_mutex_destroy(&mutex);
} // main







































































































































	//printf("0x%08x : ", data); 
	//print_bytes(placements, 8);






		//if (check against pco, and snco, and zr5, and zr6 based on pregraph) continue;



					//printf("\t pli = %u, edge = %u\n", pli, edge);
					//printf("\t count = %u\n", count);
					//printf("\t is_output = %u\n", is_output);
					//printf("\t edge_is_complemented = %u\n", edge_is_complemented);
					//printf("\t source = %u\n", source);
					//print_graph_raw(pre_graph);
					//puts(""); getchar();

		//print_graph_raw(pre_graph); puts(""); getchar();

		//printf("DEBUG: permutation = 0x%08x, p = %llu\n", permutation, p);
		//printf("edge dir: "); print_binary(edge_direction); puts("");
		//printf("node perm: "); print_binary(permutation); puts("");


	//printf("DEBUG: edge_direction = 0x%08x, queue_count = %llu\n", edge_direction, queue_count);
	//printf("[thread %llu]: arrived in try_trich(): ", thread_index);
	//print_graph_raw(pre_graph); puts("");



			//printf("DEBUG output count = %u\n", count);




















































































	
	//     nfarray      +     pre_graph        = graph

	/*print_graph_raw(graph); 
	printf(" = "); 
	print_bytes(nfarray, 8); 
	printf(" + ");	
	print_graph_raw(pre_graph); 
	puts(""); 
	//getchar();
	*/
	//printf("[thread %llu]: finished NF trich search! ", thread_index);
	//print_graph_raw(pre_graph); puts("");








/*



		//nfarray[i] == 0    --->    OPI:  { .l=A .g=B .e=C }
		//nfarray[i] == 1    --->    OPI:  { .l=A .g=C .e=B } 
		//nfarray[i] == 2    --->    OPI:  { .l=B .g=A .e=C } 
		//nfarray[i] == 3    --->    OPI:  { .l=B .g=C .e=A } 
		//nfarray[i] == 4    --->    OPI:  { .l=C .g=A .e=B } 
		//nfarray[i] == 5    --->    OPI:  { .l=C .g=B .e=A }






		// search over trich colorings!
		// with nf,   moduli={  6 6 6 6   2 6 6 6  }
		//


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
	init:  	pointer = 0;
	
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
		}



	
		for (nat i = 0; i < graph_count; i++)
			atomic_store_explicit(current_zv_progress + graph_count * thread_index + i, graph[i], memory_order_relaxed);

		byte origin = 0;
		at = execute_graph(graph, array, &origin, counts);

		if (not at) {
			append_to_file(filenames[thread_index], 4096, graph, origin);
			goto loop;
		}

		goto loop

	reset_:;
		graph[pointer] = 0; 
		do pointer++; while (noneditable(pointer));
		goto loop;


*/


	/*
	bad:	if (noneditable(at)) {
			printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
			abort();
		}		
		for (byte i = lsepa; i < at; i++) if (editable(i)) graph[i] = 0;
		pointer = at; goto loop;
	*/












	/*for (nat i = 0; i < permutation_count; i++) {
		if (i % 4 == 0) puts("");
		printf(" %08x ", node_permutations[i]);
	}
	puts("");*/












/*




for (nat i = 0; i < thread_count; i++) {
			const nat size = display_local_progress[i];
			printf(" %llu: [%8llu / %8llu] : ", i, size, machine[machine_index].cores[i].job_count);
			byte local_graph[graph_count] = {0};
			for (nat e = 0; e < graph_count; e++)
				local_graph[e] = atomic_load_explicit(current_zv_progress + graph_count * i + e,  memory_order_relaxed);
			print_graph_raw(local_graph);
			printf(" : ");
			nat amount = size / resolution; if (not amount) amount = 1;
			for (nat j = 0; j < amount; j++) putchar('#'); puts("");
		}
		puts("");






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

/*if ((temporary_display_counter & 0xfffff) == 0) {		
		        printf("\033[H\033[2J\npm counts: z = ");
			print_graph_raw(graph); puts("\n");
	
		        for (nat i = 0; i < pm_count; i++) {
		                if (i and not (i % 2)) puts("");
				printf("%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
		        }
		        puts("done");
			fflush(stdout);
			usleep(1000);
		} 

		temporary_display_counter++;*/





























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













/*
pthread_mutex_init(&mutex, NULL);
pthread_mutex_lock(&mutex);
pthread_mutex_unlock(&mutex);
pthread_mutex_destroy(&mutex);
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





