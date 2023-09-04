#include <stdio.h>
#include <stdlib.h>    // rewrite of the generate algorithm, called "nfgp". 
#include <string.h>    // a rewrite of that rewrite called srnfgp, sub-range nfgp. 
#include <unistd.h>    // used for parellelizing the nfgp across mulitple machines!!
#include <iso646.h>    // rewritten on 202308152.195947 by dwrr.
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h> 
#include <termios.h>

#define reset "\x1B[0m"
#define red   "\x1B[31m"
#define green   "\x1B[32m"
#define blue   "\x1B[34m"
#define yellow   "\x1B[33m"
// #define magenta  "\x1B[35m"
#define cyan     "\x1B[36m"

typedef int8_t byte;
typedef uint64_t nat;

static const byte unique_operations[5] = {1, 2, 3, 5, 6};

enum pruning_metrics {
	PM_fea, PM_ns0, PM_pco,
	PM_zr5, PM_zr6, PM_ndi, 
	PM_rer, PM_oer, PM_r0i, 
	PM_h,   PM_f1e, PM_erc,
	PM_ric, PM_nsvl, PM_eda,
	PM_count
};

static const char* pm_spelling[] = {
	"PM_fea", "PM_ns0", "PM_pco", 
	"PM_zr5", "PM_zr6", "PM_ndi", 
	"PM_rer", "PM_oer", "PM_r0i", 
	"PM_h",   "PM_f1e", "PM_erc", 
	"PM_ric", "PM_nsvl", "PM_eda",
};



static const byte _ = 0;



static const byte _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};

static const nat _63R_hole_count = 9;
static const byte _63R_hole_positions[_63R_hole_count] = {3, 6, 7, 10, 11, 13, 14, 15, 19};




static const byte _36R[5 * 4] = {
	0,  1, 2, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  _, 4, _,      //  9    11
	3,  _, _, _,      // 13 14 15
	4,  0, 0, _,      //       19
};

static const nat _36R_hole_count = 9;
static const byte _36R_hole_positions[_36R_hole_count] = {3, 6, 7, 9, 11, 13, 14, 15, 19};






static const byte initial = _36R_hole_count;              // delete me!!!




static const byte D = 0;       // the duplication count.

static const nat display_rate = 14;
static const bool debug_prints = 0;           // delete me!
static const nat viz = 0;

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const byte hole_count = initial + 4 * D;

static const nat execution_limit = 100000000;
static const nat array_size = 3000;
static const nat rer_count = 40;
static const nat oer_count = 80;
static const nat required_er_count = 6;
static const nat max_acceptable_consecutive_incr = 50;
static const nat max_acceptable_run_length = 7;
static const nat expansion_check_timestep = 10000;

static const nat starting_base = 0;
static const nat pre_run = 10000000;
static const nat acc_ins = 3000000;
static const nat mpp = 60;
static const nat counter_thr = 5;
static const nat blackout_radius = 7;
static const nat safety_factor = 30;
static const nat vertical_line_count_thr = 1;
static const nat required_ia_count = 10;


static const nat max_buffer_count = 10;

struct item {
	char z[64];
	char dt[32];
};


static byte* graph = NULL;
static nat* array = NULL;
static bool* modes = NULL;
static bool* executed = NULL; 
static struct bucket* buckets = NULL;
static struct bucket* scratch = NULL;

static nat counts[PM_count] = {0};


static nat buffer_count = 0;
static struct item buffer[max_buffer_count] = {0};
static char directory[4096] = "./";
static char filename[4096] = {0};



struct list {
	nat count;
	nat* z;
	char* dt;
};

struct bucket {
	nat index;
	nat data;
	nat counter;
	nat uid;
	nat is_moving;
};

static void print_graph(void) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); putchar(10); }

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void get_graphs_z_value(char string[64]) { 
	for (byte i = 0; i < graph_count; i++) string[i] = graph[i] + '0';
	string[graph_count] = 0;
}



static bool graph_analysis(void) {
	for (nat index = 0; index < operation_count; index++) {

		if (	graph[4 * index + 0] == 4 and 
			graph[4 * index + 2]
		) return true;
	}
	return false;
}

static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], counts[i]);
	}
	puts("");
	puts("[done]");
}

static nat compute_xw(nat* a, const nat n) {
	nat i = 0;
	for (; i < n; i++) 
		if (not a[i]) break; 
	return i;
}

static nat gather_buckets_at(

	struct bucket* local_buckets, 
	struct bucket* output, 

	const nat desired_index, 
	const nat radius, 
	const nat bucket_count
) {
	nat out_count = 0;
	
	for (nat b = 0; b < bucket_count; b++) {
		if (	local_buckets[b].index <= desired_index + radius and 
			local_buckets[b].index >= desired_index - radius
		) {
			output[out_count] = local_buckets[b]; 
			output[out_count++].uid = b;
		}
	}
	return out_count;
}

static nat get_max_moving_bucket_uid(struct bucket* local_scratch, const nat scratch_count) {
	nat max_bucket_data = 0;
	struct bucket max_bucket = {0};
	
	for (nat s = 0; s < scratch_count; s++) {

		if (local_scratch[s].is_moving) return local_scratch[s].uid;

		if (local_scratch[s].data >= max_bucket_data) {
			max_bucket_data = local_scratch[s].data;
			max_bucket = local_scratch[s];
		}
	}

	return max_bucket.uid;
}

static bool execute_graph_starting_at(byte origin, bool should_print_pm) {

	const nat n = array_size;
	const double mpp_ratio = (double) mpp / 100.0;
	const double discard_window = (1.0 - mpp_ratio) / 2.0;

	memset(array, 0, (n + 1) * sizeof(nat));
	memset(modes, 0, (n + 1) * sizeof(bool));
	memset(buckets, 0, (n + 1) * sizeof(struct bucket));
	memset(scratch, 0, (n + 1) * sizeof(struct bucket));

	memset(executed, 0, graph_count * sizeof(bool));
	
	byte ip = origin;
	byte 	last_mcal_op = 0;

	nat a = PM_count;
	nat pointer = 0;
	nat 	er_count = 0, 
		RER_er_at = 0, 		RER_counter = 0, 
	    	OER_er_at = 0, 		OER_counter = 0, 
		R0I_counter = 0,     	H_counter = 0;
	
	for (nat b = 0; b < n; b++) {
		buckets[b].index = b;
		buckets[b].uid = b;
	}

	nat base = starting_base, timestep_count = 0, ia_count = 0, batch_count = 0, scratch_count = 0;

	nat e = 0;
	for (; e < execution_limit; e++) {

		if (e >= expansion_check_timestep) {   
			if (array[0] == 1) 			{ a = PM_f1e; goto bad; }
			if (er_count < required_er_count) 	{ a = PM_erc; goto bad; }
		}

		const byte I = ip * 4;
		const byte op = unique_operations[graph[I]];

		if (op == 1) {
			if (pointer == n) 	{ a = PM_fea; goto bad; } 
			if (not array[pointer]) { a = PM_ns0; goto bad; } 
			pointer++;
		}

		else if (op == 5) {
			if (last_mcal_op != 3) 	{ a = PM_pco; goto bad; } 
			if (not pointer) 	{ a = PM_zr5; goto bad; } 

			// rer:
			if (RER_er_at == pointer) RER_counter++; else { RER_er_at = pointer; RER_counter = 0; }
			if (RER_counter == rer_count) { a = PM_rer; goto bad; }

			// oer:
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter == oer_count) { a = PM_oer; goto bad; }

			
			if (*modes) R0I_counter++; else R0I_counter = 0;
			if (R0I_counter > max_acceptable_consecutive_incr) { a = PM_r0i; goto bad; }

			// nsvl
			if (e >= base + pre_run) timestep_count++;
			if (viz and e >= base + pre_run) {
				const nat xw = compute_xw(array, n);
				const nat dw_count = (nat) ((double) xw * (double) discard_window);
				for (nat i = 0; i < n; i++) {	
					if (not array[i]) break;
					if (i < dw_count or i > xw - dw_count) continue;
					scratch_count = gather_buckets_at(buckets, scratch, i, 0, n);
					if (not scratch_count) {
						printf(cyan "█" reset);
						continue;
					}
					if (scratch_count == 1) {
						if (modes[i]) {
							printf("%s", (i == pointer ?  green : yellow));
							printf("█" reset); 

						} else printf(blue "█" reset);
						continue;
					}
					if (scratch_count == 2) {
						if (scratch[0].is_moving) { printf(red "█" reset); continue; } 
						else if (scratch[1].is_moving) { printf(red "█" reset); continue; }
					}
					printf("" "█" reset);
				} puts("");
				usleep(5000);
			}

			memset(modes, 0, (n + 1) * sizeof(bool));
			pointer = 0;
			er_count++;
		}

		else if (op == 2) {
			array[n]++;
		}

		else if (op == 6) {  
			if (not array[n]) 	{ a = PM_zr6; goto bad; }
			array[n] = 0;   
		}

		else if (op == 3) {
			if (last_mcal_op == 3) 	{ a = PM_ndi; goto bad; }

			// h
			if (pointer and modes[pointer - 1]) H_counter++; else H_counter = 0;
			if (H_counter > max_acceptable_run_length) { a = PM_h; goto bad; }

			array[pointer]++;
			modes[pointer] = 1;

			// nsvl
			if (e >= base + pre_run) {
				const nat xw = compute_xw(array, n);
				const nat dw_count = (nat) ((double) xw * (double) discard_window);				
				if (pointer < dw_count or pointer > xw - dw_count) goto dont_accumulate;

				ia_count++;
				const nat desired_index = pointer;
				scratch_count = gather_buckets_at(buckets, scratch, desired_index, 0, n);
				if (not scratch_count) goto dont_accumulate;

				const nat trigger_uid = get_max_moving_bucket_uid(scratch, scratch_count);
				if (not trigger_uid) abort(); 

				buckets[trigger_uid].data++;
				buckets[trigger_uid].counter++;

				scratch_count = gather_buckets_at(buckets, scratch, desired_index, blackout_radius, n);
				if (not scratch_count) return false;

				nat moving_uid = 0;

				if (buckets[trigger_uid].counter == counter_thr) {

					buckets[trigger_uid].counter = counter_thr + 1;
					buckets[trigger_uid].is_moving = false;

					const nat neighbor_position = buckets[trigger_uid].index - 1;
					scratch_count = gather_buckets_at(buckets, scratch, neighbor_position, 0, n);
					if (not scratch_count) abort(); 

					moving_uid = get_max_moving_bucket_uid(scratch, scratch_count);
					if (not moving_uid) abort();

					if (buckets[moving_uid].data) { 
						buckets[moving_uid].index++;
						buckets[moving_uid].counter = counter_thr + 1; 
						buckets[moving_uid].is_moving = true;
					}
				}
				for (nat s = 0; s < scratch_count; s++) {
					if (scratch[s].uid == trigger_uid) continue; 
					if (scratch[s].counter > counter_thr) continue;
					if (scratch[s].uid == moving_uid) continue; 
					buckets[scratch[s].uid].counter = 0;
				} dont_accumulate:;
			}
		}

		if (op == 3 or op == 1 or op == 5) last_mcal_op = op;

		if (e >= base + pre_run + acc_ins) {
			if (ia_count < required_ia_count) { a = PM_ric; goto bad; } 
			const double factor = (double) safety_factor / (double) 100.0;  
			const nat required_data_size = (nat) ((double) factor * (double) timestep_count);
			if (debug_prints) printf("threshold info: \n\n\t\ttimestep_count: %llu,  required_data_size: %llu\n\n", timestep_count, required_data_size);

			nat stats[2][2][2] = {0};
			nat vertical_line_count = 0, good_count = 0;
			for (nat b = 0; b < n; b++) {
				if (	buckets[b].data >= required_data_size and 
					buckets[b].counter > counter_thr and 
					buckets[b].is_moving
				) vertical_line_count++; else good_count++;
				stats[buckets[b].data >= required_data_size][buckets[b].counter > counter_thr][buckets[b].is_moving]++;
			}

			if (debug_prints) printf("FINAL GROUP COUNTS: \n\n\t\tvl_count: %llu,  good_count: %llu\n\n", vertical_line_count, good_count);
			if (debug_prints){	
				printf("BUCKET STATISTICS:\n\t\t[buckets[b].data >= bucket_data_thr][buckets[b].counter > counter_thr][buckets[b].is_moving]\n"); 
				puts("");
				printf("\t  [0][0][0]: %llu  [0][0][1]: %llu\n", stats[0][0][0], stats[0][0][1]);
				printf("\t  [0][1][0]: %llu  [0][1][1]: %llu\n", stats[0][1][0], stats[0][1][1]);
				puts("");
				printf("\t  [1][0][0]: %llu  [1][0][1]: %llu\n", stats[1][0][0], stats[1][0][1]);
				printf("\t  [1][1][0]: %llu  [1][1][1]: %llu\n", stats[1][1][0], stats[1][1][1]);
				puts("");
			}
			if (stats[1][0][0]) { 
				puts("");
				printf( red "NSVLPM ERROR: too low safety_factor parameter! found %llu buckets which where .data >= required_data_size, "
					"but is_moving=false... soft aborting..." reset, stats[1][0][0]
				);
				puts(""); fflush(stdout); sleep(1);
			}

			if (vertical_line_count > vertical_line_count_thr) {
				// if (should_print_pm) 
				printf("NSVL PRUNED ON BATCH COUNT:   %llu batches,     [e = %llu]\n", batch_count, e);
				a = PM_nsvl; 
				goto bad;
			}

			base += pre_run + acc_ins; 
			scratch_count = 0;

			memset(scratch, 0, (n + 1) * sizeof(struct bucket));
			memset(buckets, 0, (n + 1) * sizeof(struct bucket));

			for (nat b = 0; b < n; b++) {
				buckets[b].index = b;
				buckets[b].uid = b;
			}
			timestep_count = 0;
			ia_count = 0;
			batch_count++;
		}

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		executed[I + state] = 1;
		ip = graph[I + state];
	}

	for (byte i = 0; i < graph_count; i += 4) {
		if (not executed[i + 1] and graph[i + 1] or
		    not executed[i + 2] and graph[i + 2] or
		    not executed[i + 3] and graph[i + 3]
		) {
			puts("-------EDA-------------");
			a = PM_eda; 
			goto bad; 
		}
	}
	return false; 
	
bad: 	counts[a]++;
	if (should_print_pm) printf("%7s ( on e=%8llu )\n", pm_spelling[a], e);
	return true;
}

static bool execute_graph(bool b) {
	for (byte o = 0; o < operation_count; o++) {
		if (unique_operations[graph[4 * o]] != 3) continue;
		if (not execute_graph_starting_at(o, b)) return false;
	}
	return true;
}









// ... 







static void append_to_file(nat zindex, nat begin, nat end) {
	
	char newfilename[4096] = {0};

	strlcpy(newfilename, filename, sizeof newfilename);

	const int dir = open(directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", directory); 
		return; 
	}
	int flags = O_WRONLY | O_APPEND;  mode_t m = 0;
try_open:;
	const int file = openat(dir, newfilename, flags, m);
	if (file < 0) {
		if (m) {
			perror("create openat file");
			printf("filename=%s ", newfilename);
			close(dir); return;
		}
		perror("write openat file");
		printf("filename=%s\n", newfilename);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		m     = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

		char dt[32] = {0};
		get_datetime(dt);
		snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu-%llu_z.txt", dt, zindex, begin, end);
		strlcpy(filename, newfilename, sizeof filename);

		goto try_open;
	}

	for (nat i = 0; i < buffer_count; i++) {
		write(file, buffer[i].z, strlen(buffer[i].z));
		write(file, " ", 1);
		write(file, buffer[i].dt, strlen(buffer[i].dt));
		write(file, "\n", 1);
	}

	close(file); 
	
	if (m) {
		printf("write: created %llu z values to ", buffer_count);
		printf("%s : %s\n", directory, newfilename);
		close(dir);  
		return;
	}

	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu-%llu_z.txt", dt, zindex, begin, end);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strlcpy(filename, newfilename, sizeof filename);

	close(dir);

	printf("\033[1mwrite: saved %llu z values to ", buffer_count);
	printf("%s : %s \033[0m\n", directory, newfilename);
}


static void write_graph(nat zindex, nat begin, nat end) {

	get_datetime(buffer[buffer_count].dt);
	get_graphs_z_value(buffer[buffer_count].z);
	buffer_count++;

	if (buffer_count == max_buffer_count) {
		append_to_file(zindex, begin, end);
		buffer_count = 0;
	}

	sleep(1);
	usleep(10000);
}





static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2llu ", v[i]);
	printf("]\n");
}


static void print_bytes(byte* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2hhu ", v[i]);
	printf("] \n");
}


static void print_bytes_raw(byte* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2hhu ", v[i]);
	printf("] - ");
}


int main(int argc, const char** argv) {
	if (argc != 3) return puts("./srnfgp <begin:nat> <end:nat>");

	char* begin_invalid = NULL, * end_invalid = NULL;

	const nat range_begin  = strtoull(argv[1], &begin_invalid, 10);
	const nat range_end    = strtoull(argv[2], &end_invalid, 10);

	if (*begin_invalid) {
		printf(red "ERROR: error parsing range_begin near \"%s\" aborting...\n" reset, begin_invalid);
		abort();
	}
	if (*end_invalid) {
		printf(red "ERROR: error parsing range_end near \"%s\" aborting...\n" reset, end_invalid);
		abort();
	}

	printf("using: [begin=%llu, ...end=%llu]\n", range_begin, range_end);

	graph    = calloc(graph_count, 1);
	array    = calloc(array_size + 1, sizeof(nat));
	modes    = calloc(array_size + 1, sizeof(bool));
	executed = calloc(graph_count,    sizeof(bool));
	buckets  = calloc(array_size + 1, sizeof(struct bucket));
	scratch  = calloc(array_size + 1, sizeof(struct bucket));
	
	byte* end        = calloc(hole_count, 1);
	byte* options    = calloc(hole_count, 1);
	byte* moduli     = calloc(hole_count, 1);
	byte* positions  = calloc(hole_count, 1);

	nat counter = 0;
	byte pointer = 0;

	for (byte i = 0; i < initial; i++) {
		moduli[i] = operation_count;
		positions[i] = _36R_hole_positions[i];
	}
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i;
	for (byte i = 0; i < D; i++) {
		moduli[initial + 4 * i + 0] = 5;
		moduli[initial + 4 * i + 1] = operation_count;
		moduli[initial + 4 * i + 2] = operation_count;
		moduli[initial + 4 * i + 3] = operation_count;
	}

	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		options[i] = (byte) ((range_begin / p) % (nat) moduli[i]);
		p *= (nat) moduli[i];
	}
	if (range_begin >= p) { puts("range_begin is too big!"); printf("%llu\n", range_begin - 1); printf("%llu\n", p); abort(); }

	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[i] = (byte) ((range_end / p) % (nat) moduli[i]);
		p *= (nat) moduli[i];
	}
	if (range_end >= p) { puts("range_end is too big!"); printf("%llu\n", range_end - 1); printf("%llu\n", p); abort(); }

	memcpy(graph, _36R, 20);
	for (byte i = 0; i < hole_count; i++) graph[positions[i]] = options[i];

	goto init;


loop:
	if (not memcmp(options, end, hole_count)) goto done;
	if (options[pointer] < moduli[pointer] - 1) goto increment;
	if (pointer < hole_count - 1) goto reset_; 
	goto done;
increment:
	options[pointer]++;
	graph[positions[pointer]] = options[pointer];
init:  	pointer = 0;

	const bool should_show = not (counter & ((1 << display_rate) - 1));
	counter++;

	//if (should_show) print_bytes(end, hole_count);
	if (should_show) print_bytes_raw(options, hole_count);
	//if (should_show) print_bytes(positions, hole_count);
	//if (should_show) print_bytes(moduli, hole_count);
	// if (should_show) puts("");

	if (graph_analysis()) goto loop;
	if (execute_graph(should_show)) goto loop;
	write_graph(counter - 1, range_begin, range_end);

	printf("\tFOUND:  z = "); print_graph();
	// getchar();
        goto loop;

reset_:
	options[pointer] = 0; 
	graph[positions[pointer]] = options[pointer];
	pointer++;
	goto loop;

done:	append_to_file(counter - 1, range_begin, range_end);
	print_counts();
	printf("\n[finished %hhu-space]: searched over %llu graphs.\n", D, counter);
}









































/**


#define reset    "\x1B[0m"
#define red      "\x1B[31m"
#define green    "\x1B[32m"
#define yellow   "\x1B[33m"
#define blue   	 "\x1B[34m"
// #define magenta  "\x1B[35m"
#define cyan     "\x1B[36m"










static const nat unknown  = 11223344556677;
static const nat deadstop = 90909090909090;
static const nat _ = unknown;
static const nat X = deadstop;

















*/




















/*
202307182.184300:

	hiii!

		so the running of the nfgp 0space call just finished! the output is:



				looked at [1953125:1953125:130] poss!
			printing pm counts:
			PM_fea:        0		PM_ns0:    31801		PM_pco:   505885		PM_zr5:     3389		
			PM_zr6:   813299		PM_ndi:    62631		PM_rer:     3900		PM_oer:        0		
			PM_r0i:      800		  PM_h:   421915		PM_f1e:   109375		PM_erc:        0		
			PM_ric:        0		PM_nsvl:        0		
			[done]





			the first couple numbers,   1953125     thats the total size of the search space,    raw    with no pruning, 
						(graph analysis or pms)


			the second number,             130           is the number of good possibilities!



					ie, graphs that passsed all of our pruning metrics, 


								...except for    nsvlpm  (and ric)       because those arent implemented yet!


					so yeah, 0  space really isnt that big,  just takes a long time to go over lol. 



				so yeah, the next step woudld be to the implement the parellel version of this, 


						..after we integrate nsvlpm. 


					which i think i am going to do now?
							yeah


			but then after that, 

			we coulddddd try to run  1space! using this nfgp  thingy 




			but more likely, i think we should code up     the parellelizeddddddd   0space  version        

				and run that, 

							(which will use opencl of course)


				and then once thats coded up, code up 1space parellelized version, 


				and then run that too!


					and it shouldnt take as much of an eternity     as the nfgp would.. 

			so yeah 



	cool
				probably going to do that now


yay





			




*/






































// {execute instructions using array of size 3 here!! (SFEA)}     <----------- do this one last!!!

























// printf("continue to %llu-space? (ENTER) ", operation_count - 5);
	// if (getchar() == ' ') goto done;   // for debug for now







/*


202307031.153108: 


turns out there are   exactly        1,953,125   (ie, 5^9) possibilities       in raw 0 space!

					thats right.           ZERO space. 


									not 1 or 2 space. 0.



			soooooooo

	
yeah 
				the search spaces



							are 
								somewhatttt bigger..


		lol

	









202307031.163338:

CRAP

uhhhh

		uhhhhhhhhhhhhh

				soooo 1-space search space size 

					was just calculated to be 


			

			         10,885,864,805



		which, 
			if you want to calculate the raw size, 



			it will be:





		0sp:	5^9 = 1,953,125	
			
		1sp:	5 * (6^12) = 10,883,911,680

		
		
		0+1sp:		= 1,953,125 + 10,883,911,680

				=  10,885,864,805


		






hypotheticallyyy 

	2 space will end up being around:


	5^2 * (7^15) = 118,689,037,748,575  ie  118 trillion z values. 

						in the raw space.


			...

	i don't think we are ever going to actually go through that lol.

	so.. 

			WELLLLLLLLL        AT LEAST  WITH    a single threaded application.


		soooooo basicallyyyyyyy the only way we have of actually going over 2sp 

	is to PARELLELIZE THIS to the EXTREME. 

	unless we want to wait like MONTHSSS for it to finish. 



so yeah.
thats fun lol.


0sp  ->  1sp   ->   2sp
     A          B


A =  x5000  increase

B =  x10000  increase

....


so yeah basically 3 space is going to be 

	somewhat large.

just slightly 

just 3sp would be:

	(5^3) * (8^18)  =  2,251,799,813,685,248,000
		
so about about 2 quintillion

ish




yeahhhhhhh hopefully it doesnt come to that


lol







202307031.170434:


	okay, so implementing the graph an. deadstop rule, 


		ie, enforcing that 6.> == 0    always. 



		takes the origin 1sp  size of 10,885,864,805



			and ticks it down to 8709082469


		ie, 

			8,709,082,469      we pruned about 2 billion z values.

						niceeeee


			okay, thats progress i guesss





































	

*/














// push new elements to m1a and m2a here!!!!
	// until we reach the max number of elements in m1/m2



/*
 o#   :=      {          the sequential values  of     an M_2  variable/hole.

		0   ->	mean "i++",    (1)
		1   ->	means "(*n)++" (2)
		2   ->	means "(*i)++" (3)
		3   ->	means "i = 0"  (5)
		4   ->	means "*n = 0" (6)

	}
*/


/*

	the partial graph we are going to test with:


	the 63R:     (RRXFG ("R")    with   6->3         as opposed to 3->6     (which is an alternate valid formulation of the R)

		{

		op   *n<*i  *n>*i *n==*i
		=============================

			1,  2, 3, _,
			3,  0, _, _,
			2,  0, _, _,
			6,  1, X, _,
			5,  _, _, _,

		}




	0	1,  2, 3, _,
	1	3,  0, _, _,
	2	2,  0, _, _,
	3	6,  1, X, _,
	4	5,  _, _, _,


*/














/*

static bool has_vertical_line__batch_opt(

	const nat starting_base,			// 200000 eg
	const nat pre_run, 				// 100000 eg
	const nat acc_ins, 				// 1500000 eg

	const nat mpp,  				// middle portion percentage       	//  60            ie    60 percent	
	const nat counter_thr,                  	// 5ish
	const nat blackout_radius, 			// 7ish
	const nat safety_factor,               		//   eg      90      ie   90 percent. 
	const nat vertical_line_count_thr,              // eg 2
	const nat required_ia_count,                    //  eg like   10 or so 
	const nat origin, 
	struct parameters p, 
	nat* graph, 
	const nat viz,
	nat* array,
	bool* modes,
	struct bucket* buckets, 
	struct bucket* scratch,

	const nat instruction_count
) {
	memset(array, 0, max_array_size * sizeof(nat));
	memset(modes, 0, max_array_size * sizeof(bool));
	memset(buckets, 0, max_array_size * sizeof(struct bucket));
	memset(scratch, 0, max_array_size * sizeof(struct bucket));

	const nat n = p.FEA;

	const double mpp_ratio = (double) mpp / 100.0;
	const double discard_window = (1.0 - mpp_ratio) / 2.0;
	const nat bucket_count = n;
	
	for (nat b = 0; b < bucket_count; b++) {
		buckets[b].index = b;
		buckets[b].uid = b;
	}

	nat base = starting_base;

	nat pointer = 0, ip = origin, timestep_count = 0, ia_count = 0, batch_count = 0, scratch_count = 0;

	for (nat e = 0; e < instruction_count; e++) {

		const nat I = ip * 4;
		const nat op = graph[I];

		if (op == 1) {
			pointer++;
			if (pointer > n) return true;
		}
		else if (op == 5) {

			if (e >= base + pre_run) timestep_count++;

			if (viz and e >= base + pre_run) {

				const nat xw = compute_xw(array, n);
				const nat dw_count = (nat) ((double) xw * (double) discard_window);

				for (nat i = 0; i < n; i++) {	
					if (not array[i]) break;
					if (i < dw_count or i > xw - dw_count) continue;

					scratch_count = gather_buckets_at(buckets, scratch, i, 0, bucket_count);
					
					if (not scratch_count) {
						printf(magenta "█" reset);
						continue;
					}

					if (scratch_count == 1) {
						if (modes[i]) {
							printf("%s", (i == pointer ?  green : yellow));
							printf("█" reset); 

						} else printf(blue "█" reset);
						continue;
					}

					if (scratch_count == 2) {
						if (scratch[0].is_moving) { printf(cyan "█" reset); continue; } 
						else if (scratch[1].is_moving) { printf(cyan "█" reset); continue; }
					}
					printf(red "█" reset);        
				}
				puts("");
			}
			memset(modes, 0, max_array_size * sizeof(bool));
			pointer = 0;
		}
		else if (op == 2) array[n]++;
		else if (op == 6) array[n] = 0;   

		else if (op == 3) {
			array[pointer]++;
			modes[pointer] = 1;
			if (e >= base + pre_run) {
				const nat xw = compute_xw(array, n);
				const nat dw_count = (nat) ((double) xw * (double) discard_window);				
				if (pointer < dw_count or pointer > xw - dw_count) goto dont_accumulate;

				ia_count++;
				const nat desired_index = pointer;
				scratch_count = gather_buckets_at(buckets, scratch, desired_index, 0, bucket_count);
				if (not scratch_count) goto dont_accumulate;

				const nat trigger_uid = get_max_moving_bucket_uid(scratch, scratch_count);
				if (not trigger_uid) abort(); 

				buckets[trigger_uid].data++;
				buckets[trigger_uid].counter++;

				scratch_count = gather_buckets_at(buckets, scratch, desired_index, blackout_radius, bucket_count);
				if (not scratch_count) return false;

				nat moving_uid = 0;

				if (buckets[trigger_uid].counter == counter_thr) {

					buckets[trigger_uid].counter = counter_thr + 1;
					buckets[trigger_uid].is_moving = false;

					const nat neighbor_position = buckets[trigger_uid].index - 1;
					scratch_count = gather_buckets_at(buckets, scratch, neighbor_position, 0, bucket_count);
					if (not scratch_count) abort(); 

					moving_uid = get_max_moving_bucket_uid(scratch, scratch_count);
					if (not moving_uid) abort();

					if (buckets[moving_uid].data) { 
						buckets[moving_uid].index++;
						buckets[moving_uid].counter = counter_thr + 1; 
						buckets[moving_uid].is_moving = true;
					}
				}
				for (nat s = 0; s < scratch_count; s++) {
					if (scratch[s].uid == trigger_uid) continue; 
					if (scratch[s].counter > counter_thr) continue;
					if (scratch[s].uid == moving_uid) continue; 
					buckets[scratch[s].uid].counter = 0;
				} dont_accumulate:;
			}
		}

		if (e >= base + pre_run + acc_ins) {
			if (ia_count < required_ia_count) return true; 
			const double factor = (double) safety_factor / (double) 100.0;  
			const nat required_data_size = (nat) ((double) factor * (double) timestep_count);
			if (debug_prints) printf("threshold info: \n\n\t\ttimestep_count: %llu,  required_data_size: %llu\n\n", timestep_count, required_data_size);

			nat stats[2][2][2] = {0};
			nat vertical_line_count = 0, good_count = 0;
			for (nat b = 0; b < bucket_count; b++) {
				if (	buckets[b].data >= required_data_size and 
					buckets[b].counter > counter_thr and 
					buckets[b].is_moving
				) vertical_line_count++; else good_count++;
				stats[buckets[b].data >= required_data_size][buckets[b].counter > counter_thr][buckets[b].is_moving]++;
			}

			if (debug_prints) printf("FINAL GROUP COUNTS: \n\n\t\tvl_count: %llu,  good_count: %llu\n\n", vertical_line_count, good_count);
			if (debug_prints){	
				printf("BUCKET STATISTICS:\n\t\t[buckets[b].data >= bucket_data_thr][buckets[b].counter > counter_thr][buckets[b].is_moving]\n"); 
				puts("");
				printf("\t  [0][0][0]: %llu  [0][0][1]: %llu\n", stats[0][0][0], stats[0][0][1]);
				printf("\t  [0][1][0]: %llu  [0][1][1]: %llu\n", stats[0][1][0], stats[0][1][1]);
				puts("");
				printf("\t  [1][0][0]: %llu  [1][0][1]: %llu\n", stats[1][0][0], stats[1][0][1]);
				printf("\t  [1][1][0]: %llu  [1][1][1]: %llu\n", stats[1][1][0], stats[1][1][1]);
				puts("");
			}
			if (stats[1][0][0]) { 
				puts("");
				printf( red "NSVLPM ERROR: too low safety_factor parameter! found %llu buckets which where .data >= required_data_size, "
					"but is_moving=false... soft aborting..." reset, stats[1][0][0]
				);
				puts(""); fflush(stdout); sleep(1);
			}

			if (vertical_line_count > vertical_line_count_thr) return true;

			base += pre_run + acc_ins; 
			scratch_count = 0;
			memset(scratch, 0, max_array_size * sizeof(struct bucket));
			memset(buckets, 0, max_array_size * sizeof(struct bucket));
			for (nat b = 0; b < bucket_count; b++) {
				buckets[b].index = b;
				buckets[b].uid = b;
			}
			timestep_count = 0;
			ia_count = 0;
			batch_count++;
		}

		nat state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}


	printf("[passed]\n");
	return false;
}












// one z value that was found to be good/okay   in 1 space:                     

		  (...going to use these z values for determining the exp speeds! for min fea in the opencl search utility.)


		014110212003354042020000
		014110212013354042020000
		014110212023354042020000
		014110212033354042020000
		014110212043354042020000
		014110212053354042020000
		014110212003350442020000
		014110212013350442020000
		014110212023350442020000
		014110212033350442020000
		014110212043350442020000
		014110212053350442020000
		014110212003351442020000
		014110212013351442020000
		014110212023351442020000
		014110212033351442020000
		014110212043351442020000
		014110212053351442020000
		014110212003352442020000
		014110212013352442020000
		014110212023352442020000
		014110212033352442020000
		014110212043352442020000
		014110212053352442020000
		014110212003353442020000
		014110212013353442020000
		014110212023353442020000
		014110212033353442020000
		014110212043353442020000
		014110212053353442020000
		014110212003354442020000
		014110212013354442020000
		014110212023354442020000
		014110212033354442020000
		014110212043354442020000
		014110212053354442020000
		014110212003355442020000
		014110212013355442020000
		014110212023355442020000
		014110212033355442020000
		014110212043355442020000
		014110212043355442020000


		
		
		

	202308126.183048:   going to write a program to calculate the average expansion speed of these z values now!

		and also maybe visualize them, that would be nice too actuallyyy







// static bool is_increment(nat a) { return a < 3; }
// static bool is_reset(nat a) { return a >= 3; }





014110212003354042020000
014110212003350442020000
014110212003354542020000
014110212003350442020200




   // note, 6.> is 0 by force, 
							///ie deadstop, is never executed, doesnt matter what it is!








[ 0 : 1 2 3]
[ 1 : 1 2 3]
[ 2 : 1 2 3]
[ 3 : 1 2 3]
[ 4 : 1 2 3]         <- 0sp

[ 0 : 1 2 3]
[ 0 : 1 2 3]
[ 0 : 1 2 3]
[ 0 : 1 2 3]

// init the m1/m2 arrays based on the subrange passed in:
// init the graph based on the m1/m2 arrays.






opts:	[ 0 0 0  0 0 0  0 0 0  |  {0 : 0 0 0} ]

opts:	[ 0 0 0 0 0 0 0 0 0 0 0 0 0 ]    				assuming D = 1

mod:    [ m1 m1 m1  m1 m1 m1  m1 m1 m1   m2  m1 m1 m1 ]

// where:
//	m1 = operation_count
//	m2 = unique_operation_count (ie, 5)




	*/	









/*	------------------------------------- trash -----------------------------------------

	m2_array[m2_array_count++] = (struct option) {.option = 0, .position = graph_count + 0};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 1};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 2};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 3};

	graph[graph_count + 0] = 0;
	graph[graph_count + 1] = 0;
	graph[graph_count + 2] = 0;
	graph[graph_count + 3] = 0;




	//operation_count++;
	//graph_count = 4 * operation_count;

	//m2_array[m2_pointer].option = 0; 
	//graph[m2_array[m2_pointer].position] = m2_array[m2_pointer].option;
	
	//goto do_a_m2_pointer_reset;

m2_increment:
	m2_array[m2_pointer].option++;
	graph[m2_array[m2_pointer].position] = m2_array[m2_pointer].option;
do_a_m2_pointer_reset: 
	m2_pointer = 0;
	m1_array[m1_pointer].option = 0;
	goto m1_pointer_reset;

m2_reset:

	m2_array[m2_pointer].option = 0; 
	graph[m2_array[m2_pointer].position] = m2_array[m2_pointer].option;

	m2_pointer++;
	goto loop2;


loop2:
	if (not m2_array_count) goto check_if_done;
	if (m2_array[m2_pointer].option < unique_operation_count - 1) goto m2_increment;
	if (m2_pointer < m2_array_count - 1) goto m2_reset;
	check_if_done: 


//if (not (raw_counter & ((1 << display_rate) - 1))) {
	------------------------------------- trash ----------------------------------------- 
*/












/*

const nat _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};

static const nat _63R_hole_count = 9;
static const _63R_hole_positions[_63R_hole_count] = {3, 6, 7, 10, 11, 13, 14, 15, 19};


*/








//struct option {        // delete me!!!!
//	nat option;
//	nat position;
//};




	// void reduce(vector out, nat s, nat radix, nat length) {

	// ....reduce the begin nat to a m1/m2 array state...



//  our  36R that we are using for 0sp:  202309037.163945:


//[0    1 2 - ]
//[1    0 - - ]
//[2    - 4 - ]
//[3    - - - ]
//[4    0 X - ]          (X := deadstop)



/*

static void write_graph(nat* g, nat oc, char dt[32]) {

	nat candidate_count = 0, candidate_capacity = 0, candidate_timestamp_capacity = 0;
	nat* candidates = NULL;
	char* candidate_timestamps = NULL;


	if (graph_count * (candidate_count + 1) > candidate_capacity) {
		candidate_capacity = 4 * (candidate_capacity + graph_count);
		candidates = realloc(candidates, sizeof(nat) * candidate_capacity);
	}
	memcpy(candidates + graph_count * candidate_count, graph, graph_count * sizeof(nat));
	
	if (16 * (candidate_count + 1) > candidate_timestamp_capacity) {
		candidate_timestamp_capacity = 4 * (candidate_timestamp_capacity + 16);
		candidate_timestamps = realloc(candidate_timestamps, sizeof(char) * candidate_timestamp_capacity);
	}
	memcpy(candidate_timestamps + 16 * candidate_count, dt, 16);
}





*/



