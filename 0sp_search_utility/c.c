// 0 space search util  
// written on 202410163.164303 dwrr
// size of raw 0 space is: 5^15 = 30,517,578,125
// so not too bad lol 

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

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

#define execution_limit		10000000000LLU
#define array_size 		1000000LLU
#define display_rate 		13


enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good, pm_ga,
	pm_zr5, pm_zr6,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_r0i, pm_r1i,
	pm_h0, pm_h1, pm_erw,
	pm_rmv, pm_pt,
	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good", "pm_ga", 
	"pm_zr5", "pm_zr6", 
	"pm_pco", "pm_per", "pm_ns0", 
	"pm_oer", "pm_r0i", "pm_r1i",    
	"pm_h0", "pm_h1", "pm_erw",
	"pm_rmv", "pm_pt",
};

#define operation_count 5
#define graph_count 20
#define max_er_repetions 50
#define max_modnat_repetions 30
#define max_consecutive_s0_incr 30
#define max_consecutive_s1_incr 30
#define max_consecutive_h0_bouts 12
#define max_consecutive_h1_bouts 24
#define max_erw_count 100

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

static nat execute_graph_starting_at(byte origin, byte* graph, nat* array) {

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0,  pointer = 0,  
		bout_length = 0, 
		RMV_value = 0, 
		OER_er_at = 0,
		pointer_incr_timeout = 0,
		ERW_counter = 0,
		walk_ia_counter = 0;

	byte 	R0I_counter = 0, R1I_counter = 0, 
		 H0_counter = 0,  H1_counter = 0, 
		OER_counter = 0, RMV_counter = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

	for (nat e = 0; e < execution_limit; e++) {

		if (e == 100000000) {
			puts("running a graph for a significant amount of time...");
		}

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
			
			if (pointer == OER_er_at or pointer == OER_er_at + 1) {
				OER_counter++;
				if (OER_counter >= max_er_repetions) return pm_oer;
			} else { OER_er_at = pointer; OER_counter = 0; }
			
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_modnat_repetions) return pm_rmv;
			}

			if (walk_ia_counter < 3) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;
			walk_ia_counter = 0;

			did_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) {
			if (pointer_incr_timeout >= 100000) return pm_pt;
			else pointer_incr_timeout++;

			array[n]++;
		}

		else if (op == six) {  
			if (not array[n]) return pm_zr6;

			array[n] = 0;
		}
		else if (op == three) {

			if (pointer_incr_timeout >= 100000) return pm_pt;
			else pointer_incr_timeout++;

			if (last_mcal_op == five) {
				R0I_counter++;
				if (R0I_counter >= max_consecutive_s0_incr) return pm_r0i; 
			}

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

			walk_ia_counter++;
			bout_length = 0;
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

static nat execute_graph(byte* graph, nat* array, byte* origin, nat* counts) {
	for (byte o = 0; o < operation_count; o++) {
		const nat pm = execute_graph_starting_at(o, graph, array);
		counts[pm]++;
		if (not pm) { *origin = o; return 0; } 
	}
	return 1;
}

int main(void) {
	srand((unsigned) time(0));

	char filename[4096] = {0};
	nat* array = calloc(array_size + 1, sizeof(nat));
	nat* counts = calloc(pm_count, sizeof(nat));
	
	byte graph[graph_count] = {
		0,  0, 0, 0, 
		1,  0, 0, 0, 
		2,  0, 0, 0, 
		3,  0, 0, 0, 
		4,  0, 0, 0, 
	};

	byte pointer = 1;
	nat good_count = 0, bad_count = 0, exg_bad_count = 0, display_counter = 0;

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	goto init;
loop:
	if (graph[pointer] < operation_count - 1) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto done;

increment:
	graph[pointer]++;
init:  	pointer = 1;

	u16 was_utilized = 0;
	byte at = 1;

	for (byte index = operation_count; index--;) {

		if (graph[4 * index + 3] == index) {  
			at = 4 * index + 3; goto bad; 
		}
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  
			at = 4 * index + 2; goto bad; 
		}
		if (graph[4 * index] == six   and graph[4 * index + 2]) {  
			at = 4 * index; goto bad; 
		}

		const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

		if (graph[4 * index] == six and graph[4 * e] == one) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == one) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * e] == five) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == five) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad; 
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

		if (l != index) was_utilized |= 1 << l;
		if (g != index) was_utilized |= 1 << g;
		if (e != index) was_utilized |= 1 << e;
	}

	for (byte index = 0; index < operation_count; index++) 
		if (not ((was_utilized >> index) & 1)) goto loop;

	byte origin = 0;
	const nat is_bad = execute_graph(graph, array, &origin, counts);

	if (not (display_counter & ((1 << display_rate) - 1))) {
		printf("\033[%dm%s:  origin = %hhu, z = ", is_bad ? 31 : 32, is_bad ? "PRUNED" : " FOUND", origin); 
		print_graph_raw(graph); 
		printf("\033[0m\n");
		fflush(stdout);
		display_counter = 1;
	} else display_counter++;

	if (not is_bad) {
		printf("\033[32m  ---> GOOD: origin = %hhu, z = ", origin); 
		print_graph_raw(graph); 
		printf("\033[0m\n");
		fflush(stdout);

		append_to_file(filename, sizeof filename, graph, origin);
		usleep(100000);
		good_count++;
	} else exg_bad_count++;
	goto loop;

bad:	bad_count++;
	if (at % 4 == 0) {
		printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
		abort();
	}
	for (nat i = 0; i < at; i++) if (i % 4) graph[i] = 0;
	pointer = at; goto loop;
reset_:
	graph[pointer] = 0; 
	pointer++; 
	if (pointer % 4 == 0) pointer++;
	goto loop;
done:;
	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec);
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	printf("su: 0 space:\n"
		"\t good    %llu zv\n"
		"\t bad     %llu zv\n"
		"\t exg_bad %llu zv\n"
		"\t execution_limit = %llu\n"
                "\t array_size = %llu\n"
		"\t in %10.2lfs [%s:%s]\n"
		"\n",
		good_count, bad_count, exg_bad_count,
		execution_limit, array_size, 
		seconds, time_begin_dt, time_end_dt
	);
        puts("\npm counts:");
        
        for (nat i = 0; i < pm_count; i++) {
                if (i and not (i % 2)) puts("");
                printf("%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
        }
        puts("[done]");
}













	
	/*
	at = 255;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;

		if (at > 4 * o + 1) at = 4 * o + 1;

		if (graph[4 * graph[4 * o + 1]] == five) continue;
		if (graph[4 * graph[4 * o + 1]] == six) continue;

		const byte eq_case = graph[4 * graph[4 * o + 1] + 3];

		if (graph[4 * graph[4 * o + 1]] == one) {
			if (at > 4 * graph[4 * o + 1] + 3) { at = 4 * graph[4 * o + 1] + 3; if (at == 3) goto exit_ega; }
			
			if (graph[4 * eq_case] == six) continue;

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
				if (graph[4 * graph[4 * eq_case + 2]] == five) continue;
			}

			if (graph[4 * eq_case] == three) {
				if (at > 4 * eq_case + 1) at = 4 * eq_case + 1;
				if (graph[4 * graph[4 * eq_case + 1]] == six) continue;
				if (graph[4 * graph[4 * eq_case + 1]] == one) continue;
			}
	
		} else if (graph[4 * graph[4 * o + 1]] == two) {
			if (at > 4 * graph[4 * o + 1] + 3) at = 4 * graph[4 * o + 1] + 3;

			if (graph[4 * eq_case] == five) continue; 
			if (graph[4 * eq_case] == three) continue;

			if (graph[4 * eq_case] == one) {
				if (at > 4 * eq_case + 2) { at = 4 * eq_case + 2; if (at == 3) goto exit_ega; }
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
			}

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == three) continue;
				if (graph[4 * graph[4 * eq_case + 2]] == five) continue;
			}
		}
		goto exit_ega;
	}
	goto bad;
exit_ega:;
	*/










