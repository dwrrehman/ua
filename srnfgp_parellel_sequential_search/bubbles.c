#include <stdio.h>
#include <stdlib.h>    // rewrite of the generate algorithm, called "nfgp". 
#include <string.h>
#include <iso646.h>    // written on 2307031.171603 by dwrr
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h> 
#include <termios.h>
#include <math.h>
#include <errno.h>

#define reset    "\x1B[0m"
#define red      "\x1B[31m"
#define green    "\x1B[32m"
#define yellow   "\x1B[33m"
#define blue   	 "\x1B[34m"
// #define magenta  "\x1B[35m"
#define cyan     "\x1B[36m"

typedef uint64_t nat;
static const nat unknown  = 11223344556677;
static const nat deadstop = 90909090909090;
static const nat _ = unknown;
static const nat X = deadstop;

static const nat unique_operation_count = 5;
static const nat unique_operations[unique_operation_count] = {1, 2, 3, 5, 6};

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


struct parameters {
	nat execution_limit;
	nat array_size;
	nat rer_count;
	nat oer_count;
	nat required_er_count;
	nat max_acceptable_consecutive_incr;
	nat max_acceptable_run_length;
	nat expansion_check_timestep;

	nat starting_base;
	nat pre_run;
	nat acc_ins;
	nat mpp;
	nat counter_thr;
	nat blackout_radius;
	nat safety_factor;
	nat vertical_line_count_thr;
	nat required_ia_count;
	nat viz;
};

struct list {
	nat count;
	nat* z;
	char* dt;
};

struct option {
	nat option;
	nat position;
};

struct bucket {
	nat index;
	nat data;
	nat counter;
	nat uid;
	nat is_moving;
};



static nat* array = NULL;
static bool* modes = NULL;
static struct bucket* buckets = NULL;
static struct bucket* scratch = NULL;
static bool* edge_was_executed = NULL; 

static nat counts[PM_count] = {0};




static void print_graph(nat* graph, nat graph_count) {
	for (nat i = 0; i < graph_count; i++) {
		if (graph[i] == X) printf("_");
		else if (graph[i] == _) printf("HOLE");
		else printf("%llu", graph[i]);
	}
	putchar(10);
}

/*
static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}
*/

// static bool is_increment(nat a) { return a < 3; }
// static bool is_reset(nat a) { return a >= 3; }

static bool graph_analysis(nat* graph, nat operation_count) {
	for (nat index = 0; index < operation_count; index++) {

		if (	graph[4 * index + 0] == 4 and 
			graph[4 * index + 2]
		) return true;
	}
	return false;
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


static bool debug_prints = 0;


static bool execute_graph_starting_at(nat origin, nat* graph, struct parameters p, bool should_print_pm, nat graph_count) {

	const nat n = p.array_size;
	const double mpp_ratio = (double) p.mpp / 100.0;
	const double discard_window = (1.0 - mpp_ratio) / 2.0;

	memset(array, 0, (n + 1) * sizeof(nat));
	memset(modes, 0, (n + 1) * sizeof(bool));
	memset(buckets, 0, (n + 1) * sizeof(struct bucket));
	memset(scratch, 0, (n + 1) * sizeof(struct bucket));

	memset(edge_was_executed, 0, graph_count * sizeof(bool));
	
	nat pointer = 0, ip = origin;
	nat a = PM_count;

	nat 	last_mcal_op = 0,    	er_count = 0, 
		RER_er_at = 0, 		RER_counter = 0, 
	    	OER_er_at = 0, 		OER_counter = 0, 
		R0I_counter = 0,     	H_counter = 0;
	
	for (nat b = 0; b < n; b++) {
		buckets[b].index = b;
		buckets[b].uid = b;
	}

	nat base = p.starting_base, timestep_count = 0, ia_count = 0, batch_count = 0, scratch_count = 0;

	nat e = 0;
	for (; e < p.execution_limit; e++) {

		if (e >= p.expansion_check_timestep) {   
			if (array[0] == 1) 			{ a = PM_f1e; goto bad; }
			if (er_count < p.required_er_count) 	{ a = PM_erc; goto bad; }
		}

		const nat I = ip * 4;
		const nat op = unique_operations[graph[I]];

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
			if (RER_counter == p.rer_count) { a = PM_rer; goto bad; }

			// oer:
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter == p.oer_count) { a = PM_oer; goto bad; }

			
			if (*modes) R0I_counter++; else R0I_counter = 0;
			if (R0I_counter > p.max_acceptable_consecutive_incr) { a = PM_r0i; goto bad; }

			// nsvl
			if (e >= base + p.pre_run) timestep_count++;
			if (p.viz and e >= base + p.pre_run) {
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
			if (H_counter > p.max_acceptable_run_length) { a = PM_h; goto bad; }

			array[pointer]++;
			modes[pointer] = 1;

			// nsvl
			if (e >= base + p.pre_run) {
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

				scratch_count = gather_buckets_at(buckets, scratch, desired_index, p.blackout_radius, n);
				if (not scratch_count) return false;

				nat moving_uid = 0;

				if (buckets[trigger_uid].counter == p.counter_thr) {

					buckets[trigger_uid].counter = p.counter_thr + 1;
					buckets[trigger_uid].is_moving = false;

					const nat neighbor_position = buckets[trigger_uid].index - 1;
					scratch_count = gather_buckets_at(buckets, scratch, neighbor_position, 0, n);
					if (not scratch_count) abort(); 

					moving_uid = get_max_moving_bucket_uid(scratch, scratch_count);
					if (not moving_uid) abort();

					if (buckets[moving_uid].data) { 
						buckets[moving_uid].index++;
						buckets[moving_uid].counter = p.counter_thr + 1; 
						buckets[moving_uid].is_moving = true;
					}
				}
				for (nat s = 0; s < scratch_count; s++) {
					if (scratch[s].uid == trigger_uid) continue; 
					if (scratch[s].counter > p.counter_thr) continue;
					if (scratch[s].uid == moving_uid) continue; 
					buckets[scratch[s].uid].counter = 0;
				} dont_accumulate:;
			}
		}

		if (op == 3 or op == 1 or op == 5) last_mcal_op = op;

		if (e >= base + p.pre_run + p.acc_ins) {
			if (ia_count < p.required_ia_count) { a = PM_ric; goto bad; } 
			const double factor = (double) p.safety_factor / (double) 100.0;  
			const nat required_data_size = (nat) ((double) factor * (double) timestep_count);
			if (debug_prints) printf("threshold info: \n\n\t\ttimestep_count: %llu,  required_data_size: %llu\n\n", timestep_count, required_data_size);

			nat stats[2][2][2] = {0};
			nat vertical_line_count = 0, good_count = 0;
			for (nat b = 0; b < n; b++) {
				if (	buckets[b].data >= required_data_size and 
					buckets[b].counter > p.counter_thr and 
					buckets[b].is_moving
				) vertical_line_count++; else good_count++;
				stats[buckets[b].data >= required_data_size][buckets[b].counter > p.counter_thr][buckets[b].is_moving]++;
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

			if (vertical_line_count > p.vertical_line_count_thr) {
				 if (should_print_pm) printf("NSVL PRUNED ON BATCH COUNT:   %llu batches,     [e = %llu]\n", batch_count, e);
				 a = PM_nsvl; goto bad;
			}
			

			base += p.pre_run + p.acc_ins; 
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

		nat state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		edge_was_executed[I + state] = 1;
		ip = graph[I + state];
	}

	//eda: empirical deadstop check:

	for (nat i = 0; i < graph_count; i += 4) {
		if (not edge_was_executed[i + 1] and graph[i + 1] or
		    not edge_was_executed[i + 2] and graph[i + 2] or
		    not edge_was_executed[i + 3] and graph[i + 3]

		) {
				puts("------------------------------EDA--------EDA---------EDA-----------------EDA-------------");
				a = PM_eda; 
				goto bad; 
		}
	}

	return false; 
	
bad: 	counts[a]++;
	if (should_print_pm) printf("%7s ( on e=%8llu )\n", pm_spelling[a], e);
	return true;
}





















static bool execute_graph(nat* graph, nat operation_count, struct parameters p, bool b, nat graph_count) {
	for (nat o = 0; o < operation_count; o++) {
		if (unique_operations[graph[4 * o]] != 3) continue;
		if (not execute_graph_starting_at(o, graph, p, b, graph_count)) return false;
	}
	return true;
}





/*
static void write_graph(nat* g, nat oc, char dt[32]) {

	nat candidate_count = 0, candidate_capacity = 0, candidate_timestamp_capacity = 0;
	nat* candidates = NULL;
	char* candidate_timestamps = NULL;


	if (p.graph_count * (candidate_count + 1) > candidate_capacity) {
		candidate_capacity = 4 * (candidate_capacity + p.graph_count);
		candidates = realloc(candidates, sizeof(nat) * candidate_capacity);
	}
	memcpy(candidates + p.graph_count * candidate_count, graph, p.graph_count * sizeof(nat));
	
	if (16 * (candidate_count + 1) > candidate_timestamp_capacity) {
		candidate_timestamp_capacity = 4 * (candidate_timestamp_capacity + 16);
		candidate_timestamps = realloc(candidate_timestamps, sizeof(char) * candidate_timestamp_capacity);
	}
	memcpy(candidate_timestamps + 16 * candidate_count, dt, 16);
}

*/



static void print_counts(void) {
	printf("printing pm counts:\n");
	for (nat i = 0; i < PM_count; i++) {
		if (i and not (i % 4)) puts("");
		printf("%6s: %8llu\t\t", pm_spelling[i], counts[i]);
	}
	puts("");
	puts("[done]");
}


int main(void) {

	const nat D = 1;

	const nat display_rate = 14;

	struct parameters p = {
		.execution_limit = 100000000,
		.array_size = 3000,

		.rer_count = 40,
		.oer_count = 80,
		.required_er_count = 6,
		.max_acceptable_consecutive_incr = 50,
		.max_acceptable_run_length = 7,
		.expansion_check_timestep = 10000,

		.starting_base = 1,
		.pre_run = 9999999,
		.acc_ins = 3000000,

		.mpp = 60,
		.counter_thr = 5,
		.blackout_radius = 7,
		.safety_factor = 30,
		.vertical_line_count_thr = 1,
		.required_ia_count = 10,

		.viz = 0, 						// temporary
	};

	array = calloc(p.array_size + 1, sizeof(nat));
	modes = calloc(p.array_size + 1, sizeof(bool));
	buckets = calloc(p.array_size + 1, sizeof(struct bucket));
	scratch = calloc(p.array_size + 1, sizeof(struct bucket));
	
	const nat max_operation_count = 5 + D;	
	const nat max_graph_count = 4 * max_operation_count;

	nat operation_count = 5;
	nat graph_count = 4 * operation_count;

	nat* graph = calloc(max_graph_count, sizeof(nat));
	edge_was_executed = calloc(max_graph_count, sizeof(bool));


	const nat initial_hole_count = 9;
	const nat _63RRXFG[5 * 4] = {
		0,  1, 4, _,      // 3
		1,  0, _, _,      // 6 7 
		2,  0, _, _,      // 10 11
		3,  _, _, _,      // 13 14 15
		4,  2, 0, _,      // 19                    // note, 6.> is 0 by force, 
							///ie deadstop, is never executed, doesnt matter what it is!
	};

	memcpy(graph, _63RRXFG, graph_count * sizeof(nat));

	nat m1_pointer = 0;
	nat m1_array_count = initial_hole_count;
        struct option* m1_array = calloc(3 * max_operation_count, sizeof(struct option));
	
	m1_array[0].position = 3;
	m1_array[1].position = 6; 
	m1_array[2].position = 7;
	m1_array[3].position = 10;
	m1_array[4].position = 11;
	m1_array[5].position = 13; 
	m1_array[6].position = 14;
	m1_array[7].position = 15;
	m1_array[8].position = 19;

	for (nat i = 0; i < m1_array_count; i++) graph[m1_array[i].position] = 0;
	
	nat m2_pointer = 0;
	nat m2_array_count = 0;
	struct option* m2_array = calloc(D + 1, sizeof(struct option));

	nat raw_counter = 0, maybe_counter = 0, good_counter = 0;
	goto m1_pointer_reset;

loop:
	if (m1_array[m1_pointer].option < operation_count - 1) goto m1_increment;
	if (m1_pointer < m1_array_count - 1) goto m1_reset;
loop2:
	if (not m2_array_count) goto check_if_done;
	if (m2_array[m2_pointer].option < unique_operation_count - 1) goto m2_increment;
	if (m2_pointer < m2_array_count - 1) goto m2_reset;
	check_if_done: 

	printf("\n[finished %llu space]: looked at [%llu:%llu:%llu] poss!\n", operation_count - 5, raw_counter, maybe_counter, good_counter);
	print_counts();

	if (m2_array_count == D) goto done;

	m2_array[m2_array_count++] = (struct option) {.option = 0, .position = graph_count + 0};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 1};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 2};
	m1_array[m1_array_count++] = (struct option) {.option = 0, .position = graph_count + 3};

	graph[graph_count + 0] = 0;
	graph[graph_count + 1] = 0;
	graph[graph_count + 2] = 0;
	graph[graph_count + 3] = 0;

	operation_count++;
	graph_count = 4 * operation_count;

	m2_array[m2_pointer].option = 0; 
	graph[m2_array[m2_pointer].position] = m2_array[m2_pointer].option;
	
	goto do_a_m2_pointer_reset;

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

m1_increment:

	m1_array[m1_pointer].option++;
	graph[m1_array[m1_pointer].position] = m1_array[m1_pointer].option;

	m1_pointer_reset: m1_pointer = 0;

	if (not (raw_counter & ((1 << display_rate) - 1))) {
		for (nat i = 0; i < m1_array_count; i++) printf("%llu ", m1_array[i].option); 
		printf("  -  ");
		for (nat i = 0; i < m2_array_count; i++) printf("%llu ", m2_array[i].option); 
		printf("  ---->  ");

		//print_graph(graph, graph_count);
	}

	//char dt[32] = {0}; get_datetime(dt);

	raw_counter++;

	if (graph_analysis(graph, operation_count)) {
		puts("#");
		goto loop;
	}

	maybe_counter++;

	if (execute_graph(graph, operation_count, p, not (raw_counter & ((1 << display_rate) - 1)), graph_count)) {
		goto loop;
	}

	good_counter++;

	// write_graph(graph, operation_count);

	//if (not (raw_counter & ((1 << display_rate) - 1))) {
		for (nat i = 0; i < m1_array_count; i++) printf("%llu ", m1_array[i].option); 
		printf("  -  ");
		for (nat i = 0; i < m2_array_count; i++) printf("%llu ", m2_array[i].option); 
		printf("  ---->  z = ");
		print_graph(graph, graph_count);
	//}

	getchar();

        goto loop;

m1_reset:

	m1_array[m1_pointer].option = 0; 
	graph[m1_array[m1_pointer].position] = m1_array[m1_pointer].option;

	m1_pointer++; 
	goto loop;

done:	printf("\n [DONE] looked at [%llu:%llu:%llu] poss!\n", raw_counter, maybe_counter, good_counter);
	print_counts();
}







































































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








	*/	
		





















014110212003354042020000
014110212003350442020000
014110212003354542020000
014110212003350442020200



