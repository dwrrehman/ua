// modified to be parellel on 202501131.020611: dwrr 202501131.034623:

// D-general version of the 0 space search util  
// written on 202410163.164303 dwrr
// size of raw 0 space is: (5^15) 	                 		=              30,517,578,125    / 5 =    6103515625
// size of raw 1 space is: (6^15) * (5 * (6 ^ 3)) 	                =         507,799,783,342,080  / 6 =    84633297223680
// size of raw 2 space is: (7^15) * (5 * (7 ^ 3)) * (5 * (7 ^ 3)) 	=  13,963,646,602,082,100,175 / 7 =   1994806657440300025 

// rewritten kinda  on 202411144.202807 dwrr

#include <time.h>
#include <string.h>
#include <unistd.h>
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

typedef uint8_t byte;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t nat;
typedef uint64_t chunk;

#define D 0
#define execution_limit 300000000LLU
#define array_size 1000000LLU
#define chunk_count 2
#define display_rate 2
#define update_rate 1

#define total_job_count 30
#define machine_index 0

#define machine0_counter_max 1
#define machine1_counter_max 1

#define machine0_thread_count 4
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

static _Atomic nat* global_progress = NULL;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good,
	pm_zr5, pm_zr6, pm_ndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi,
	pm_h0, pm_h1, pm_erw,
	pm_rmv, pm_imv, pm_csm,

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
	"pm_zr5", "pm_zr6", "pm_ndi",
	"pm_pco", "pm_per", "pm_ns0",
	"pm_oer", "pm_rsi", 
	"pm_h0", "pm_h1", "pm_erw",
	"pm_rmv", "pm_imv", "pm_csm",

	"pm_ga_sdol", 
	"pm_ga_6g",   "pm_ga_ns0", 
	"pm_ga_zr5",  "pm_ga_pco", 
	"pm_ga_ndi",  "pm_ga_sndi", 
	"pm_ga_snco", "pm_ga_sn1", 
	"pm_ga_zr6",  "pm_ga_rdo", 
	"pm_ga_uo", 
};

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

#define max_rsi_count 512
#define max_oer_repetions 50
#define max_rmv_modnat_repetions 30
#define max_imv_modnat_repetions 80
#define max_consecutive_small_modnats 230
#define max_consecutive_s0_incr 30
#define max_consecutive_h0_bouts 12
#define max_consecutive_h1_bouts 24
// #define max_erw_count 200

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



/*static void debug_pm(byte op, byte origin, nat e, nat* history, nat pm) {

	if (op == three or op == one or op == five) {
		memmove(history, history + 1, sizeof(nat) * 8192 - 1);
		history[8192 - 1] = op;
	}

	printf("\n\033[32;1m at origin = %hhu: \n[PRUNED GRAPH VIA <%s> AT %llu]:\033[0m\n", origin, pm_spelling[pm], e);
	puts("[LIFETIME-START]"); 

	for (nat i = 0; i < 8192; i++) {
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
*/



static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, byte* zskip_at) {


	nat history[8192] = {0};
	memset(history, 255, sizeof history);


	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0,  pointer = 0,  
		bout_length = 0, 
		RMV_value = 0, 
		IMV_value = 0,
		OER_er_at = 0;
		// walk_ia_counter = 0;

	byte	H0_counter = 0,  H1_counter = 0, 
		OER_counter = 0, RMV_counter = 0, 
		IMV_counter = 0, CSM_counter = 0;
		// ERW_counter = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

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
			
			if (pointer == OER_er_at or pointer == OER_er_at + 1) {
				OER_counter++;
				if (OER_counter >= max_oer_repetions) return pm_oer;
			} else { OER_er_at = pointer; OER_counter = 0; }
			
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



			// walk_ia_counter = 0;
			did_ier_at = pointer;
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

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

			// walk_ia_counter++;
			bout_length = 0;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;




		//if (op == three or op == one or op == five) { 
			//memmove(history, history + 1, sizeof(nat) * (8192 - 1));
			//history[8192 - 1] = op;
		//}




		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		if (*zskip_at > I + state) *zskip_at = I + state;
		ip = graph[I + state];
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

static byte noneditable(byte pa) { return (pa < 20 and pa % 4 == 0) or pa == 18; }
static byte editable(byte pa) { return not noneditable(pa); }

static void* worker_thread(void* raw_argument) {

	char filename[4096] = {0};
	nat* counts = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));
	void* raw_graph = calloc(1, graph_count + (8 - (graph_count % 8)) % 8);
	void* raw_end = calloc(1, graph_count   + (8 - (graph_count % 8)) % 8);
	byte* graph = raw_graph;
	byte* end = raw_end;
	nat* graph_64 = raw_graph;
	nat* end_64 = raw_end;
	byte pointer = 0;
	nat update_counter = 0;

	const struct joblist list = *(struct joblist*) raw_argument;
	const nat thread_index = list.thread_index;
	const nat count = list.job_count;
	const struct job* jobs = list.jobs;

	for (nat job_index = 0; job_index < count; job_index++) {

		if (not (update_counter & ((1 << update_rate) - 1))) {
			update_counter = 0;
			atomic_store_explicit(global_progress + thread_index, job_index, memory_order_relaxed);
		} else update_counter++;

		memcpy(graph, jobs[job_index].begin, graph_count);
		memcpy(end, jobs[job_index].end, graph_count);
		goto init;

	loop:	for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
			if (graph_64[i] < end_64[i]) goto process;
			if (graph_64[i] > end_64[i]) goto next_job;
		}
		goto next_job;

	process:
		if (graph[pointer] < ((pointer % 4) ? operation_count - 1 : 4)) goto increment;
		if (pointer < graph_count - 1) goto reset_;
		goto next_job;

	increment:
		graph[pointer]++;
	init:  	pointer = 1;

		u16 was_utilized = 0;
		byte at = 1;

		for (byte index = 20; index < graph_count; index += 4) {
			if (index < graph_count - 4 and graph[index] > graph[index + 4]) { 
				at = index + 4; 
				counts[pm_ga_sdol]++; 
				// puts(pm_spelling[pm_ga_sdol]); 
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
				counts[pm_ga_ns0]++;
				//puts(pm_spelling[pm_ga_ns0]);
				goto bad;
			}
	 
			if (graph[4 * index] == six and graph[4 * e] == five) {
				at = graph_count;
				if (editable(4 * index + 3) and at > 4 * index + 3) at = 4 * index + 3;
				if (editable(4 * index) and at > 4 * index) at = 4 * index;
				if (editable(4 * e) and at > 4 * e) at = 4 * e;
				counts[pm_ga_ns0]++;
				//puts(pm_spelling[pm_ga_ns0]);
				goto bad;
			}

			{const byte pairs[3 * 7] = {
				three, three, pm_ga_ndi,
				five, one,  pm_ga_pco,
				five, five, pm_ga_zr5,

				six, six, pm_ga_zr6,
				two, two, pm_ga_sndi,
				two, six, pm_ga_sn1,
				six, two, pm_ga_snco,
			};
			for (nat i = 0; i < 7 * 3; i += 3) {
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
					at = j + (j < 20); 
					counts[pm_ga_rdo]++; 
					// puts("pm_ga_rdo"); 
					goto bad;
				}
			}
		}

		for (byte index = 0; index < operation_count; index++) {
			if (not ((was_utilized >> index) & 1)) { 
				at = 1;
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
				counts[pm_ga_6g]++;
				//puts(pm_spelling[pm_ga_6g]);
				goto bad;
			}
		}

		byte origin = 0;
		at = execute_graph(graph, array, &origin, counts);
		if (at) goto bad;

		append_to_file(filename, sizeof filename, graph, origin);
		usleep(100000);
		goto loop;

	bad:	if (noneditable(at)) {
			printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
			abort();
		}
		for (byte i = 1; i < at; i++) if (editable(i)) graph[i] = 0;
		pointer = at; goto loop;
	reset_:
		graph[pointer] = 0; 
		do pointer++; while (noneditable(pointer));
		goto loop;

	next_job:
		continue;
	}

	printf("worker_thread[%llu]: terminating thread!\n", thread_index);
	
	free(raw_graph);
	free(raw_end);
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

	byte partial_graph[20] = {
		0,  0, 0, 0, 
		1,  0, 0, 0, 
		2,  0, 0, 0, 
		3,  0, 0, 0, 
		4,  0, 4, 0,
	};

	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};

	global_progress = calloc(1, thread_count * sizeof(_Atomic nat));
	for (nat i = 0; i < thread_count; i++)  atomic_init(global_progress + i, 0);

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
	total[0] = expn(5 + D, 14);

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
	nat width_m1[chunk_count] = {0};
	memcpy(width_m1, width, chunk_count * sizeof(nat));
	adc(width_m1, n1, 0);
	nat last_zi[chunk_count] = {0};
	memcpy(last_zi, total, chunk_count * sizeof(nat));
	adc(last_zi, n1, 0);

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
			if (noneditable(i)) continue;
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
			if (noneditable(i)) continue;
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

	const nat job_count_per_core = machine[machine_index].cores[0].job_count;

	for (nat mi = 0; mi < 2; mi++) {
		
		printf("machine #%llu has %llu cores: \n", mi, machine[mi].core_count);

		for (nat i = 0; i < machine[mi].core_count; i++) {

			printf("\tcore #%llu job list: (%llu jobs): \n", i, machine[mi].cores[i].job_count);

			for (nat j = 0; j < machine[mi].cores[i].job_count; j++) {

				printf("\t\t[%llu] = (", j);
				print_graph_raw(machine[mi].cores[i].jobs[j].begin);
				printf(" ... ");
				print_graph_raw(machine[mi].cores[i].jobs[j].end);
				puts(")");
			}
			puts("");
		}
		puts("----------------------------------\n\n");
	}

	getchar();

	snprintf(output_string, 4096, "SU: searching [D=%u] space....\n", D);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < machine[machine_index].core_count; i++) {
		pthread_create(threads + i, NULL, worker_thread, machine[machine_index].cores + i);
	}

	nat resolution = job_count_per_core / 80;
	if (resolution == 0) resolution = 1;

	while (1) {
		nat sum = 0;
		for (nat i = 0; i < thread_count; i++) {
			sum += atomic_load_explicit(global_progress + i, memory_order_relaxed);
		}

		printf("\033[H\033[2J\n-----------------current jobs (jcpc=%llu)-------------------\n", job_count_per_core);
		printf("\n\t%1.10lf%%\n\n", (double) (sum) / (double) total_job_count);

		for (nat i = 0; i < thread_count; i++) {
			const nat size = atomic_load_explicit(global_progress + i, memory_order_relaxed);
			printf("  %c %020llu :: ", i == (nat) -1 ? '*' : ' ', size);
			for (nat j = 0; j < size / resolution; j++) {
				putchar('#');
			}
			puts("");
		}

		puts("");
		for (nat i = 0; i < thread_count; i++) {
			const nat size = atomic_load_explicit(global_progress + i, memory_order_relaxed);
			if (size < job_count_per_core - 1) goto next;
		}
		break;
	next:	sleep(1 << display_rate);
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
		"su: D = %u space:\n"

		"\t chunk_count = %u\n"
		
		"\t total_job_count = %u\n"
		"\t machine_index = %u\n"

		"\t machine0_counter_max = %u\n"
		"\t machine1_counter_max = %u\n"

		"\t machine0_thread_count = %u\n"
		"\t machine1_thread_count = %u\n"

		"\t display_rate = %u\n"
		"\t update_rate = %u\n"

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
		update_rate,

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





			/*if (walk_ia_counter < (e < 500000 ? 1 : 6)) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) {
					debug_pm(op, origin, e, history, pm_erw);
					fflush(stdout);
					return pm_erw;
				}
			} else ERW_counter = 0;*/















/*	nat* array = calloc(array_size + 1, sizeof(nat));

	byte zv[24] = {
		0,  1, 4, 4, 
		1,  5, 3, 5, 
		2,  1, 3, 1, 
		3,  5, 4, 4, 
		4,  2, 4, 2,
		0,  0, 2, 0,
	};

	byte zskip_at_unused = 0;
	const nat pm = execute_graph_starting_at(2, zv, array, &zskip_at_unused);
	printf("pruned by   pm = %s\n", pm_spelling[pm]);

	exit(1);



*/






