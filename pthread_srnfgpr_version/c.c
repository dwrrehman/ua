// 202403041.192520: by dwrr         
//      the prthead  cpu-parellelized version of the srnfgpr.
//
#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <iso646.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdnoreturn.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

static const byte D = 1;        // the duplication count (operation_count = 5 + D)
static const bool R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

static const nat job_size = 10000000000;
static const nat thread_count = 64;
static const nat display_rate = 4;

enum operations { one, two, three, five, six };

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
static const nat execution_limit = 1000000000;
static const nat array_size = 10000;

static const nat max_acceptable_er_repetions = 50;
static const nat max_acceptable_modnat_repetions = 15;
static const nat max_acceptable_consecutive_s0_incr = 30;
static const nat max_acceptable_run_length = 9;
static const nat max_acceptable_consequtive_small_modnats = 200;

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

static bool execute_graph_starting_at(byte origin, byte* graph, nat* array, nat* timeout) {

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
}

//if (executed[I + state] < 253) executed[I + state]++;

static bool execute_graph(byte* graph, nat* array, nat* timeout) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not execute_graph_starting_at(o, graph, array, timeout)) return false;
	}
	return true;
}

static bool fea_execute_graph_starting_at(byte origin, byte* graph, nat* array) {

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
}

static bool fea_execute_graph(byte* graph, nat* array) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not fea_execute_graph_starting_at(o, graph, array)) return false;
	}
	return true;
}

static void append_to_file(char* filename, size_t size, byte* graph) {
	char dt[32] = {0};   get_datetime(dt);                            ////  these should be happening in the workerthread.
	char z[64] = {0};    get_graphs_z_value(z, graph);                //
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
	write(file, dt, strlen(dt));
	write(file, "\n", 1);

	close(file);

	printf("[%s]: write: %s z = %s to file \"%s\"\n",                // do this in the workerthread.
		dt, permissions ? "created" : "wrote", z, filename
	);
}

static void* worker_thread(void* __attribute__((unused)) _unused_arg) {

	char filename[4096] = {0};
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
						// TODO:  use "memory_order_relaxed" !?!?!?!?!?!?!??!?????
						// CHANGED: yup, we can just use memory_order_relaxed. 

	if (h >= space_size) goto terminate_thread;

	const nat range_begin = h;
	nat range_end = h + job_size - 1;
	if (range_end >= space_size) range_end = space_size - 1;

	nat p = 1;
	for (nat i = 0; i < hole_count; i++) {
		graph[positions[i]] = (byte) ((range_begin / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_begin >= p) { puts("range_begin is too big!"); printf("range_begin = %llu, p = %llu\n", range_begin, p); abort(); } // delete me?

	p = 1;
	for (nat i = 0; i < hole_count; i++) {
		end[positions[i]] = (byte) ((range_end / p) % (nat) (positions[i] & 3 ? operation_count : 5));
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
	if (range_end >= p) { puts("range_end is too big!"); printf("range_end = %llu, p = %llu\n", range_end, p); abort(); }  // delete me?

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
	byte a = 0;  // rename this to "zskip_at"

	byte previous_op = graph[20];     

					// make this not use this temporary variable, by using   index and index + 4   

	for (byte index = 20; index < graph_count; index += 4) {                 


					// (except if index+4==graphcount, then we will  just say its index.. yeah)

		const byte op = graph[index];
		if (previous_op > op) { a = index; goto bad; }
		previous_op = op;
	}

	// constructive GA here

	for (byte index = operation_count; index--;) {

		if (graph[4 * index + 3] == index) {  a = 4 * index + 3; goto bad; }
		
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  a = 4 * index; goto bad; } 			      //  5:   0 x 5 x

		if (graph[4 * index] == six   and graph[4 * index + 2])          {  a = 4 * index; goto bad; }

		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  a = 4 * index + 2 * (index == two); goto bad; }   //  5:   1 x 5 x

		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  a = 4 * index + 1 * (index == three); goto bad; } //  5:   2 5 x x

		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == one) {
			if (index == six) { a = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == one) { a = 4 * index; goto bad; }
			a = 4 * (index < tohere ? index : tohere); goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == five) {
			if (index == six) { a = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == five) { a = 4 * index; goto bad; }
			a = 4 * (index < tohere ? index : tohere); goto bad; 
		}
 
		const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

		if (graph[4 * index] == one and graph[4 * e] == one) {
			if (index == one) { a = 4 * index + 3; goto bad; }
			if (e == one) { a = 4 * index; goto bad; }
			a = 4 * (index < e ? index : e); goto bad;
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == one) {
			if (index == five) { a = 4 * index + 1; goto bad; } 
			if (e == one) { a = 4 * index; goto bad; }
			a = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == three) {
			if (index == five) { a = 4 * index + 1; goto bad; } 
			if (e == three) { a = 4 * index; goto bad; }
			a = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == one)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == one) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { a = 4 * index; goto bad; }
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}
		
		if (graph[4 * index] == five) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == five) { 
					if (index == five) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == five) { a = 4 * index; goto bad; } 
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == six) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) { 
					if (index == six) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { a = 4 * index; goto bad; } 
					a = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		if (graph[4 * index] == three) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == three) { 
					if (index == three) { a = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == three) { a = 4 * index; goto bad; } 
					a = 4 * (index < tohere ? index : tohere); goto bad; 
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
		if (positions[i] == a) { pointer = i; goto loop; } else graph[positions[i]] = 0;
	}
	abort();
	
try_executing:
	if (fea_execute_graph(graph, array)) goto loop;
	if (execute_graph(graph, array, timeout)) goto loop;
	append_to_file(filename, sizeof filename, graph);

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
	return 0;
}

static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

int main(void) {
	srand((unsigned)time(0)); rand();

	// compiletime computation:
	atomic_init(&head, 0);
	space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	positions = calloc(hole_count, 1);
	threads = calloc(thread_count, sizeof(pthread_t));
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	// runtime computation:
	printf("using [D=%hhu, R=%hhu]: threadcount=%llu, spacesize=%llu\n", D, R, thread_count, space_size);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);
	for (nat i = 0; i < thread_count; i++) pthread_create(threads + i, NULL, worker_thread, NULL);

	while (1) {
		const nat h = atomic_fetch_add_explicit(&head, 0, memory_order_relaxed);   

				// TODO:  use "memory_order_relaxed" !?!?!?!?!?!?!??!?????
				// CHANGED:   yup, its correct i think.
		
		printf("%llu .. %lf%%\n", h, (double) h / (double) space_size);
		if (h >= space_size) {
			printf("info: [all jobs allocated to threads. waiting for them to finish.]\n");
			break;
		}

		sleep(1 << display_rate);
	}

	for (nat i = 0; i < thread_count; i++) pthread_join(threads[i], NULL);
	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec), zthroughput = (double) space_size / seconds;
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));
	printf("\n[D=%hhu:R=%hhu]: searched %llu zvs using %llu threads in %10.10lfs[%s:%s], at %lf z/s.\n", 
		D, R, space_size, thread_count, seconds, time_begin_dt, time_end_dt, zthroughput
	);
}
 




























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
	//		executed[i + 3] < 5 and graph[i + 3]) { a = PM_eda; goto bad; }
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



