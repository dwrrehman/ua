// rewrite of the generate algorithm, called "nfgp". 
// a rewrite of that rewrite called srnfgp, sub-range nfgp. 
// used for parellelizing the nfgp across mulitple machines!!
// rewritten on 202308152.195947 by dwrr.

// 202401217.162352: this is a copy of the srnfgp  that uses rmv,   doing some testing to see if vl's are correlated with having rmv sequences.
//             if so, we are ripping out svl/nsvl,  in favor of having rmv. 
//            i also implemented two other pruning metrics to prune simple patterns and infinite loops, including 
//           operation_timeout_pm  (ot)   and majority_histogram_pm  (mh)
//

// formula for size of 0 space:
//      (5 ^ 9)

// formula for size of 1 space:
//	(6 ^ 9) * (5) * (6 ^ 3)

// formula for size of 2 space:
//       ((5 + 2) ^ 9) * ((5 * ((5 + 2) ^ 3)) ^ 2)

// general formula:
//       ((5 + D) ^ 9) * ((5 * ((5 + D) ^ 3)) ^ D)

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <iso646.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>  
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

static const byte D = 1;        // the duplication count (operation_count = 5 + D)
static const bool R = 0;   	// which partial graph we are using. (1 means 63R, 0 means 36R.)

static const nat display_rate = 24;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	PM_fea, PM_ns0, PM_pco, PM_zr5, PM_zr6, PM_ndi, 
	PM_oer, PM_r0i, PM_h, PM_f1e, PM_erc, PM_ne0,
	PM_eda, PM_rmv, PM_ot, PM_csm, PM_mm, PM_snm,
	PM_count
};

static const char* pm_spelling[] = {
	"PM_fea", "PM_ns0", "PM_pco", "PM_zr5", "PM_zr6", "PM_ndi", 
	"PM_oer", "PM_r0i", "PM_h", "PM_f1e", "PM_erc", "PM_ne0",
	"PM_eda", "PM_rmv", "PM_ot", "PM_csm", "PM_mm", "PM_snm"
};


static const byte _ = 0;

static const byte _63R[5 * 4] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};

static const byte _63R_hole_count = 9;
static const byte _63R_hole_positions[_63R_hole_count] = {3, 6, 7, 10, 11, 13, 14, 15, 19};


static const byte _36R[5 * 4] = {
	0,  1, 2, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  _, 4, _,      //  9    11
	3,  _, _, _,      // 13 14 15
	4,  0, 0, _,      //       19
};

static const byte _36R_hole_count = 9;
static const byte _36R_hole_positions[_36R_hole_count] = {3, 6, 7, 9, 11, 13, 14, 15, 19};

static const byte initial = R ? _63R_hole_count : _36R_hole_count;

static const byte max_buffer_count = 1;

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

static const nat expansion_check_timestep2 = 10000;        // <-------------- changed this!!! was 1000
static const nat required_s0_increments = 5;               //

// static const byte required_executed_count = 8;    // PM_eda is removed.

struct item {
	char z[64];
	char dt[32];
};

struct list {
	nat count;
	nat* z;
	char* dt;
};

static byte* graph = NULL;
static byte* end = NULL;
static byte* positions = NULL; 
// static byte* executed = NULL; 
static nat* timeout = NULL; 

static nat* array = NULL;

static nat counts[PM_count] = {0};

static nat buffer_count = 0;
static struct item buffer[max_buffer_count] = {0};

static char directory[4096] = "./";
static char filename[4096] = {0};

static void print_graph_raw(void) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void get_graphs_z_value(char string[64]) { 
	for (byte i = 0; i < graph_count; i++) string[i] = (char) graph[i] + '0';
	string[graph_count] = 0;
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

static bool execute_graph_starting_at(byte origin) {

	const nat n = array_size;
	array[n] = 0; array[0] = 0; 
	//memset(executed, 0, graph_count);
	memset(timeout, 0, operation_count * sizeof(nat));

	byte ip = origin, last_mcal_op = 0;

	nat 	e = 0,  a = 0, xw = 0,
		pointer = 0,  er_count = 0, 
	    	OER_er_at = 0,  OER_counter = 0, 
		R0I_counter = 0, H_counter = 0,
		RMV_counter = 0, RMV_value = 0, CSM_counter = 0;

	for (; e < execution_limit; e++) {

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) { a = PM_f1e; goto bad; } 
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) { a = PM_erc; goto bad; } 
		}
		
		const byte I = ip * 4, op = graph[I];

		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) { a = PM_ot; goto bad; }
			timeout[i]++;
		}
		timeout[ip] = 0;

		if (op == one) {
			if (pointer == n) 	{ a = PM_fea; goto bad; }
			if (not array[pointer]) { a = PM_ns0; goto bad; }

			if (last_mcal_op == one) H_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;

			pointer++;
			if (pointer > xw) { xw = pointer; array[pointer] = 0; }
		}

		else if (op == five) {
			if (last_mcal_op != three) { a = PM_pco; goto bad; } 
			if (not pointer)           { a = PM_zr5; goto bad; } 

			if (not array[pointer])    { a = PM_ne0; goto bad; }     // delete me!!!!     redundant becuaes of pco.

			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_acceptable_er_repetions) { a = PM_oer; goto bad; }
			

			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_acceptable_consequtive_small_modnats) { a = PM_csm; goto bad; } 
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_acceptable_modnat_repetions) { a = PM_rmv; goto bad; }
			}

			pointer = 0;
			er_count++;
		}

		else if (op == two) {
			array[n]++;
			if (array[n] >= 65535) { a = PM_snm; goto bad; }
		}

		else if (op == six) {  
			if (not array[n]) { a = PM_zr6; goto bad; }
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) { a = PM_ndi; goto bad; }

			if (last_mcal_op == one) {
				H_counter++;
				if (H_counter >= max_acceptable_run_length) { a = PM_h; goto bad; }
			}

			if (last_mcal_op == five) {
				R0I_counter++; 
				if (R0I_counter >= max_acceptable_consecutive_s0_incr) { a = PM_r0i; goto bad; }
			}

			if (array[pointer] >= 65535) { a = PM_mm; goto bad; }
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		//if (executed[I + state] < 253) executed[I + state]++;
		ip = graph[I + state];
	}
	//for (byte i = 0; i < graph_count; i += 4) {
	//
	//	if (	executed[i + 1] < 5 and graph[i + 1] or         // slightly concerning here... think about this more...? 
	//		executed[i + 2] < 5 and graph[i + 2] or 
	//		executed[i + 3] < 5 and graph[i + 3]) { a = PM_eda; goto bad; }
	//}
	return false;
bad: 	counts[a]++;
	return true;
}


static bool execute_graph(void) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not execute_graph_starting_at(o)) return false;
	}
	return true;
}

static bool fea_execute_graph_starting_at(byte origin) {

	const nat n = 5;
	memset(array, 0, (n + 1) * sizeof(nat));
	
	byte ip = origin, last_mcal_op = 0;
	nat a = PM_count, pointer = 0, e = 0;

	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) 	{ a = PM_fea; goto bad; } 
			if (not array[pointer]) { a = PM_ns0; goto bad; } 
			pointer++;
		}

		else if (op == five) {
			if (last_mcal_op != three)  { a = PM_pco; goto bad; } 
			if (not pointer) 	    { a = PM_zr5; goto bad; }
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) 	{ a = PM_zr6; goto bad; }
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) { a = PM_ndi; goto bad; }
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
	
bad: 	counts[a]++;
	return true;
}

static bool fea_execute_graph(void) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (not fea_execute_graph_starting_at(o)) return false;
	}
	return true;
}

static void append_to_file(nat b, nat e) {
	
	char newfilename[4096] = {0};

	strncpy(newfilename, filename, sizeof newfilename);

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
		snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu_z.txt", dt, b, e);
		strncpy(filename, newfilename, sizeof filename);

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
	snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu_z.txt", dt, b, e);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strncpy(filename, newfilename, sizeof filename);

	close(dir);

	printf("\033[1mwrite: saved %llu z values to ", buffer_count);
	printf("%s : %s \033[0m\n", directory, newfilename);
}

static void write_graph(nat b, nat e) {

	get_datetime(buffer[buffer_count].dt);
	get_graphs_z_value(buffer[buffer_count].z);
	buffer_count++;

	if (buffer_count == max_buffer_count) {
		append_to_file(b, e);
		buffer_count = 0;
	}
}

static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}

int main(int argc, const char** argv) {

	if (argc != 3) {
		printf("./srnfgp [D=%hhu][R=%hhu] <begin:nat(0)> <end:nat(%llu)>\n", 
			D, R, expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D) - 1);
		exit(0);
	}

	char* begin_invalid = NULL, * end_invalid = NULL;

	const nat range_begin  = strtoull(argv[1], &begin_invalid, 10);
	const nat range_end    = strtoull(argv[2], &end_invalid, 10);

	if (*begin_invalid) {
		printf("ERROR: error parsing range_begin near \"%s\" aborting...\n", begin_invalid);
		abort();
	}
	if (*end_invalid) {
		printf("ERROR: error parsing range_end near \"%s\" aborting...\n", end_invalid);
		abort();
	}

	printf("using: %s:[begin=%llu, ...end=%llu]\n", R ? "63R" : "36R", range_begin, range_end);

	array = calloc(array_size + 1, sizeof(nat));
	//executed = calloc(graph_count, 1);
	timeout = calloc(operation_count, sizeof(nat));
	positions = calloc(hole_count, 1); 
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

	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

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

	const clock_t time_begin = clock();
	goto init;

loop:	for (byte i = (operation_count & 1) + (operation_count >> 1); i--;) {
		if (graph_64[i] <  end_64[i]) goto process;
		if (graph_64[i] >  end_64[i]) break;
		if (graph_64[i] == end_64[i]) continue;
	}
	goto done;
process:
	if (graph[positions[pointer]] < (positions[pointer] & 3 ? operation_count - 1 : 4)) goto increment;
	if (pointer < hole_count - 1) goto reset_;
	goto done;

increment:
	graph[positions[pointer]]++;
init:  	pointer = 0;


///////////////
	if (not (display_counter & ((1 << display_rate) - 1))) { print_graph_raw(); putchar(10); fflush(stdout); }
	display_counter++;
	nat zindex = 0;
	nat p = 1;
	for (byte i = 0; i < hole_count; i++) {
		zindex += p * graph[positions[i]];
		p *= (nat) (positions[i] & 3 ? operation_count : 5);
	}
////////////////




	u16 was_utilized = 0;
	byte a = 0;  // rename this to "zskip_at"

	byte previous_op = graph[20];     // make this not use this temporary variable, by using   index and index + 4   
	for (byte index = 20; index < graph_count; index += 4) {                 // (except if index+4==graphcount, then we will  just say its index.. yeah)
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
	if (fea_execute_graph()) goto loop;
	if (execute_graph()) goto loop;
	write_graph(range_begin, range_end);
	printf("\r     FOUND:  z = "); print_graph_raw(); printf("\033[K\n"); fflush(stdout);
	found++;
        goto loop;

reset_:
	graph[positions[pointer]] = 0; 
	pointer++;
	goto loop;

done:;
	const clock_t time_end = clock();
	const double elapsed_seconds = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
	if (buffer_count) append_to_file(range_begin, range_end);
	print_counts();
	printf("\n[finished %hhu-space-(R=%hhu)]: found %llu graphs, "
		"searched over %llu graphs in %10.10lf seconds [throughput = %10.10lf z/s] \n", 
		D, R, found, (range_end - range_begin), elapsed_seconds, 
		(double) (range_end - range_begin) / elapsed_seconds);
}



























/*





		//if (executed[i + 1] < required_executed_count) { a = PM_eda; goto bad; }
		//if (executed[i + 2] < required_executed_count) { a = PM_eda; goto bad; }
		//if (executed[i + 3] < required_executed_count) { a = PM_eda; goto bad; }      WRONGGGGGG!!!!!!!!!!!    6.g!!!!!!!





	//	char string[64] = {0};
	//	get_graphs_z_value(string);
	//	const bool debug = not strcmp(string, "0122106523433510400006112000");
	//	if (debug) printf("[z = %s, origin = %hhu]\n", string, origin);


	static void print_nats(nat* v, nat l) {
		printf("(%llu)[ ", l);
		for (nat i = 0; i < l; i++) printf("%2llu ", v[i]);
		printf("]\n");
	}




if (debug) { 
			printf("[e=%llu]: executing &%hhu: [op=%hhu, .lge={%hhu, %hhu, %hhu}]: "
				"{pointer = %llu, *n = %llu} \n", 
					e, ip, 
					graph[4 * ip + 0], 
					graph[4 * ip + 1], 
					graph[4 * ip + 2], 
					graph[4 * ip + 3], 
					pointer, array[n]
			);
			puts(""); print_nats(array, 10); puts(""); 
			puts(""); 
			getchar();
		}




202402272.170826:
	how the    pruning unsorted duplicate operation list    pm       in ga     will work:



example z value:


//	0120 1052 2345 3264 4003 3012 2300




5 space dup op list:

	0 2 4 3 3

	1 2 4 3 3

	2 2 4 3 3

	3 2 4 3 3
	4 2 4 3 3
	0 3 4 3 3
	...
	4 3 4 3 3
	0 4 4 3 3
	0 4 4 3 3
	....
	4 4 4 3 3
	0 0 0 4 3
	1 0 0 4 3
	...
	4 4 4 4 3
	0 0 0 0 4




	0 1 2 3 3

	1 1 2 3 3

	2 1 2 3 3  -->   0 2 2 3 3


	0 2 2 3 3

	1 2 2 3 3

	2 2 2 3 3

	3 2 2 3 3  -->  0 3 2 3 3

	0 3 2 3 3  -->  0 0 3 3 3

	0 0 3 3 3

	1 0 3 3 3  -->  0 1 3 3 3

	0 1 3 3 3

	1 1 3 3 3

	2 1 3 3 3

	0 2 3 3 3

	1 2 3 3 3

	2 2 3 3 3

	3 2 3 3 3  -->  0 3 3 3 3

	0 3 3 3 3

	1 3 3 3 3

	2 3 3 3 3

	3 3 3 3 3

	4 3 3 3 3  -->  0 4 3 3 3

	0 4 3 3 3  -->  0 0 4 3 3

	0 0 4 3 3  -->  0 0 0 4 3

	0 0 0 4 3  -->  0 0 0 0 4

	0 0 0 0 4  

	1 0 0 0 4  

	2 0 0 0 4  

	






//donn't use counter!!!!   use rend-rbegin       as the count!!!! more correct!!!!!!!!




	//memset(was_utilized, 0, operation_count); // make this NOT a memset, by making the was_utilized, NOT an array!
							//       simply store the Op_count bits in a     single  uint16_t!!!!





//		if (l == index and g == index and e == index) {  a = 4 * index + (index == five); goto bad; };











	//	byte graph_data[] = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  0, 3, 3, 4,  0, 0, 0, 0};
	//                                                                                               *            *  *  *  * 
				                                                                         @            
									                                              ^






	//	byte graph_data[] = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  0, 3, 3, 5,  0, 0, 1, 0};
	//                                                                                               *        * 
				                                                                                  @
									                                 ^


    //  set according to where you found PA  a in  positions!!!







	//	byte graph_data[] = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  3, 3, 3, 4,  3, 3, 3, 4};
	//                                                                                               *  *         *
													 @
									                                 ^



	//	byte graph_data[] = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  3, 3, 3, 4,  3, 3, 3, 4};
	//                                                                                               *            *  *
													 @
									                                              ^


	//	byte graph_data[] = {0, 1, 2, 7,   1, 0, 4, 4,   2, 1, 4, 8,   3, 4, 0, 0,  4, 0, 9, 3,  3, 3, 3, 4,  2, 3, 3, 4};
	//                                                                     *  *                      * 
										  @
									       ^




struct winsize window = {0};
	ioctl(0, TIOCGWINSZ, &window);
	const nat window_width = window.ws_col - 80;





static void print_progress(nat x, nat total, nat window_width) {       // todo: figure out a new way to print this that is actually correct!

	const double ratio = (double) x / (double) total;
	const nat w = (nat) ((double) ratio * (double) window_width);
	const nat rest = window_width - w;
	putchar('\r');
	print_graph_raw();
	putchar(':');
	printf("%020llu:%.*e:[", x, 8, ratio); 
	for (nat i = 0; i < w; i++) printf("*");
	for (nat i = 0; i < rest; i++) printf("`");
	putchar(']');
	fflush(stdout);
}








////////////////////////////////////////////////////////////////////////////////////////////////////////


		// todo: loop over the  positions array       to find    where     PA    a    is located!
		//       then init an temp_nfpointer to that index that you found that PA at,   and 
		//       then just use the original nf algorithm,    and set pointer to the place you want to do the increment at!


///	const byte mod = (a % 4 == 0) ? 4 : operation_count - 1;
//	if (graph[a] < mod) goto GA_incr;
//	else if (a == graph_count - 1) goto done;
//	graph[a] = 0; 
//	a++;
//	goto bad;

//GA_incr: 
//	graph[a]++;



////////////////////////////////////////////////////////////////////////////////////////////////////////















				// end state check   that will work with the zskip opt:
	////////////////////////////////////////////////////////////////////////////////////////// test seperately in its own file plz.
	for (byte i = 1 + (operation_count >> 1); i--;) {
		if (graph_64[i] <  end_64[i]) goto continue_;
		if (graph_64[i] >  end_64[i]) break;
		if (graph_64[i] == end_64[i]) continue;
	}
	goto done;
continue_:
	//////////////////////////////////////////////////////////////////////////////////////////






	
				 {0 1 2 2  1 0 5 4}    2 0 4 3  3 1 1 5   4 0 0 2  2 1 1 4   0 0 0 0  0 0 0 0 


				0x00 0x01 0x02 0x02 0x01 0x00 0x05 0x4

*/

/*


			6 2 5 1 4 1 6 2 4 2 5 1 3 (0 0 4 6) 5 1 4 2
			                               ^








	{1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {1} {3 5}

	{1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {3 1} {3 1} {1} {3 5}

	{3 1} {3 1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {3 1} {3 1} {1} {3 5}

	{3 1} {3 1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {3 1} {3 1} {1} {3 5}

	{3 1} {3 1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {3 1} {3 1} {1} {3 5}

	{3 1} {3 1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {3 1} {3 1} {1} {3 5}

	{3 1} {3 1} {3 1} {1} {3 1} {3 1} {1} {3 1} {1} {3 1} {3 1} {3 1} {3 1} {1} {3 5}


*/












//if (should_show) print_bytes(end, hole_count);
	//if (should_show) print_bytes(positions, hole_count);
	//if (should_show) print_bytes(moduli, hole_count);
	// if (should_show) puts("");

























/*
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
*/





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





















	./run 0 10883911679

	1813985280
	3627970560
	5441955840
	7255941120
	9069926400
	





./run 0 10883911679

./run 0 10883911679

./run 0 10883911679

./run 0 10883911679

./run 0 10883911679

./run 0 10883911679





*/











/*

63R 1sp calls that we executed: 202309166.140547:


	./run 0 1360488959

	./run 1360488960 2720977919

	./run 2720977920 4081466879

	./run 4081466880 5441955839

	./run 5441955840 6802444799

	./run 6802444800 8162933759

	./run 8162933760 9523422719

	./run 9523422720 10883911679





````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22326762 ) {{ z = 01211022204334104000 }}
 PM_rmv ( on e= 5352979 ) {{ z = 01221020214334104000 }}
339969:1.74064217e-01:[***********************************```````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22325781 ) {{ z = 01211022204334144000 }}
 PM_rmv ( on e= 5352110 ) {{ z = 01221020214334144000 }}
413697:2.11812972e-01:[*******************************************```````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    1651 ) {{ z = 01211024204332104001 }}
417793:2.13910126e-01:[********************************************``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22326762 ) {{ z = 01211022204334104001 }}
 PM_rmv ( on e= 5352979 ) {{ z = 01221020214334104001 }}
548865:2.81019024e-01:[*********************************************************`````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330024001 }}
561153:2.87310483e-01:[***********************************************************```````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330124001 }}
577537:2.95699095e-01:[************************************************************``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330224001 }}
593921:3.04087708e-01:[**************************************************************````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330324001 }}
610305:3.12476320e-01:[****************************************************************``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330424001 }}
733185:3.75390912e-01:[*****************************************************************************`````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22325781 ) {{ z = 01211022204334144001 }}
 PM_rmv ( on e= 5352110 ) {{ z = 01221020214334144001 }}
802817:4.11042514e-01:[************************************************************************************``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    1651 ) {{ z = 01211024204332104002 }}
811009:4.15236821e-01:[*************************************************************************************`````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22326762 ) {{ z = 01211022204334104002 }}
 PM_rmv ( on e= 5352979 ) {{ z = 01221020214334104002 }}
937985:4.80248566e-01:[**************************************************************************************************````````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330024002 }}
954369:4.88637178e-01:[****************************************************************************************************``````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330124002 }}
970753:4.97025790e-01:[******************************************************************************************************````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330224002 }}
983041:5.03317250e-01:[*******************************************************************************************************```````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330324002 }}
999425:5.11705862e-01:[*********************************************************************************************************`````````````````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=    2502 ) {{ z = 01211024214330424002 }}
1122305:5.74620454e-01:[**********************************************************************************************************************````````````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22325781 ) {{ z = 01211022204334144002 }}
 PM_rmv ( on e= 5352110 ) {{ z = 01221020214334144002 }}
1200129:6.14466363e-01:[******************************************************************************************************************************````````````````````````````````````````````````````````````````````````````````] PM_rmv ( on e=22326762 ) {{ z = 01211022204334104003 }}
 PM_rmv ( on e= 5352979 ) {{ z = 01221020214334104003 }}
1511425:7.73849996e-01:[***************************************************************************************************************************************************************```````````````````````````````````````````````] PM_rmv ( on e=22325781 ) {{ z = 01211022204334144003 }}
 PM_rmv ( on e= 5352110 ) {{ z = 01221020214334144003 }}
1949697:9.98245375e-01:[*************************************************************************************************************************************************************************************************************`]write openat file: No such file or 



// static const byte unique_operations[5] = {1, 2, 3, 5, 6};




static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2llu ", v[i]);
	printf("]\n");
}





*/





















                                                                                                                                                                           /*








srnfgp_rmv_test: b
srnfgp_rmv_test: b release
srnfgp_rmv_test: ./run    
./srnfgp [D=1][R=0] <begin:nat(0)> <end:nat(10883911679)>
srnfgp_rmv_test: ./run 0 10883911679
using: 36R:[begin=0, ...end=10883911679]
624951297:5.74197325e-02:[******````````````````````````````````````````````````````````````````````````````````````````````````````````] PM_zr5 ( on e=       1 ) 0 2 0 ]
 PM_f1e ( on e=     500 )
775946241:7.12929564e-02:[*******```````````````````````````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 3 2 0 ]
1228931073:1.12912628e-01:[************``````````````````````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      32 ) 0 4 0 ]
1568669697:1.44127382e-01:[***************```````````````````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      32 ) 1 5 0 ]
1738539009:1.59734759e-01:[*****************`````````````````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      32 ) 4 5 0 ]
 PM_zr6 ( on e=       1 )
1880096769:1.72740906e-01:[*******************```````````````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 1 0 1 ]
1917845505:1.76209212e-01:[*******************```````````````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 2 0 1 ]
2027945985:1.86325105e-01:[********************``````````````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 4 0 1 ]
2182086657:2.00487354e-01:[**********************````````````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 1 1 1 ]
2427453441:2.23031343e-01:[************************``````````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 0 2 1 ]
2474639361:2.27366726e-01:[*************************`````````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 1 2 1 ]
2729443329:2.50777791e-01:[***************************```````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 0 3 1 ]
2776629249:2.55113174e-01:[****************************``````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 1 3 1 ]
2792357889:2.56558301e-01:[****************************``````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 1 3 1 ]
   PM_h ( on e=      18 )
2825912321:2.59641240e-01:[****************************``````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      38 ) 2 3 1 ]
2877292545:2.64361989e-01:[*****************************`````````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 3 3 1 ]
2984247297:2.74188856e-01:[******************************````````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 5 3 1 ]
3053453313:2.80547417e-01:[******************************````````````````````````````````````````````````````````````````````````````````] PM_zr5 ( on e=       1 ) 0 4 1 ]
   PM_h ( on e=      27 )
3239051265:2.97599922e-01:[********************************``````````````````````````````````````````````````````````````````````````````] PM_f1e ( on e=     500 ) 4 4 1 ]
3352297473:3.08004840e-01:[*********************************`````````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      32 ) 0 5 1 ]
   PM_h ( on e=      30 )
3603955713:3.31126880e-01:[************************************``````````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 5 5 1 ]
 PM_ndi ( on e=       1 )
4031774721:3.70434347e-01:[****************************************``````````````````````````````````````````````````````````````````````]   PM_h ( on e=      32 ) 2 1 2 ]
4211081217:3.86908801e-01:[******************************************````````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 5 1 2 ]
 PM_ndi ( on e=       1 )
4238344193:3.89413689e-01:[******************************************````````````````````````````````````````````````````````````````````]   PM_h ( on e=      38 ) 0 2 2 ]
4406116353:4.04828382e-01:[********************************************``````````````````````````````````````````````````````````````````]   PM_h ( on e=      16 ) 3 2 2 ]
 PM_zr5 ( on e=       1 )
        FOUND:  z = 012110252143310240010242
5211422721:4.78818909e-01:[****************************************************``````````````````````````````````````````````````````````] PM_zr5 ( on e=       1 ) 1 5 2 ]
 PM_f1e ( on e=     500 )
6325010433:5.81133936e-01:[***************************************************************```````````````````````````````````````````````]   PM_h ( on e=      32 ) 5 2 3 ]
 PM_ndi ( on e=       1 )
6579814401:6.04545001e-01:[******************************************************************````````````````````````````````````````````]   PM_h ( on e=      32 ) 4 3 3 ]
 PM_zr6 ( on e=       1 )
7017070593:6.44719546e-01:[**********************************************************************````````````````````````````````````````] PM_f1e ( on e=     500 ) 1 5 3 ]
7463763969:6.85761166e-01:[***************************************************************************```````````````````````````````````]   PM_h ( on e=      16 ) 4 0 4 ]
7523532801:6.91252651e-01:[****************************************************************************``````````````````````````````````]   PM_h ( on e=      16 ) 5 0 4 ]
7570718721:6.95588033e-01:[****************************************************************************``````````````````````````````````]   PM_h ( on e=      32 ) 0 1 4 ]
7633633281:7.01368543e-01:[*****************************************************************************`````````````````````````````````]   PM_h ( on e=      16 ) 1 1 4 ]
   PM_h ( on e=      18 )
7728005121:7.10039308e-01:[******************************************************************************````````````````````````````````]   PM_h ( on e=      32 ) 3 1 4 ]
7778336769:7.14663716e-01:[******************************************************************************````````````````````````````````]   PM_h ( on e=      16 ) 4 1 4 ]
7825522689:7.18999099e-01:[*******************************************************************************```````````````````````````````]   PM_h ( on e=      32 ) 5 1 4 ]
8570011649:7.87401800e-01:[**************************************************************************************````````````````````````]   PM_h ( on e=      32 ) 2 4 4 ]
8618246145:7.91833524e-01:[***************************************************************************************```````````````````````]   PM_h ( on e=      32 ) 3 4 4 ]
8797552641:8.08307978e-01:[****************************************************************************************``````````````````````]   PM_h ( on e=      16 ) 0 5 4 ]
   PM_h ( on e=      16 )
10883170305:9.99931883e-01:[*************************************************************************************************************`]write openat file: No such file or directory
filename=
write: created 1 z values to ./ : 1202402073.223329_10883911679_0_10883911679_z.txt
printing pm counts:
PM_fea: 39694252                PM_ns0: 59823459                PM_pco: 148705593               PM_zr5: 398252350               
PM_zr6: 319103143               PM_ndi: 494764522               PM_oer:    78304                PM_r0i:   127089                
  PM_h: 27885701                PM_f1e: 28790797                PM_erc:       18                PM_nsvl:        0
PM_eda:        4                PM_rmv:   682516                 PM_ot:      610                PM_csm:      270                
[done]

[finished 1-space]: searched over 10883911680 graphs.
srnfgp_rmv_test: 









*/                                                                                                                                                                                                                                                                                                                                                                                                                                                 











































	// char string[64] = {0};
	// get_graphs_z_value(string);
	// const bool debug = not strcmp(string, "012110452543300240021000");

	// if (debug) printf("[z = %s, origin = %hhu]\n", string, origin);

	

	// memset(array, 0, (n + 1) * sizeof(nat));



	//	if (debug) { 
	//		printf("[e=%llu]: executing &%hhu: [op=%hhu, .lge={%hhu, %hhu, %hhu}]: "
	//			"{pointer = %llu, *n = %llu} \n",  e, ip, 
	//			graph[4 * ip + 0], 
	//			graph[4 * ip + 1], 
	//			graph[4 * ip + 2], 
	//			graph[4 * ip + 3], 
	//			pointer, array[n]
	//		);
	//		
	//		puts(""); print_nats(array, 5); puts(""); 
	//		puts(""); 
	//		getchar();
	//	}





	/*	if (debug) { 
			printf("[e=%llu]: executing [op=%hhu]: {pointer = %llu, *n = %llu} "
				"{s0_was_incremented = %hhu} "
				"{R0I_counter = %llu, max = %llu}\n", 
					e, ip, pointer, array[n], 
					s0_was_incremented,
					R0I_counter, max_acceptable_consecutive_s0_incr
			);
			//puts(""); print_nats(array, xw); puts(""); 
			//puts(""); 
			getchar();
		}



	//memset(array, 0, (n + 1) * sizeof(nat));              //todo:  do the lazy array zeroing optimization. 




xw   lazy zeroing:


			[ 9 8 7 5 4 2 3 1 1 0 0 0 0 0 0 0 0 0 0 ]
                                            ^





MCAL operation sequence:

         *0    *1   *2   *3    *4   *5   *6  *7  *8   *9   *10
	{3 1} {1} {3 1} {3 1} {1} {3 1} {1} {1} {3 1} {1} {3 1} {1} 3 1 3 1 3 1 1 3 1 1 1 3 1 3 1 1 3 1 3 1 1 1 1 1 3 1 3 1 1 3 1 1 3 1

         1     0    1    1     0    1    0   0   1    0     1    0 



os	3 1    3 1    3 1   3 1    3 1   3 1   3 1   3 1    3 1

modes    1      1      1     1      1     1     1     1      1




os	3 1    3 1    3 1   3 1    3 1    1     3 1    3 1

modes    1      1      1     1      1     0      1      1






















// static void print_graph(void) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); putchar(10); }


#define reset "\x1B[0m"
#define red   "\x1B[31m"
//#define green   "\x1B[32m"
//#define blue   "\x1B[34m"
//#define yellow   "\x1B[33m"
// #define magenta  "\x1B[35m"
//#define cyan     "\x1B[36m"

static void print_bytes_raw(byte* v, nat l) {
	printf("(%llu)[", l);
	for (nat i = 0; i < l; i++) printf("%2hhu", v[i]);
	printf(" ]");
}














*/











//	nat xw = 0;
//	for (; xw < n and array[xw]; xw++) { }
//	memset(values, 0, n * sizeof(nat));
//	for (nat i = 0; i < xw; i++) {
//		if (array[i] >= n) { a = PM_mh; goto bad; }
//		values[array[i]]++; 
//	}
//	for (nat i = 0; i < n; i++) 
//		if (values[i] > xw >> 2) { a = PM_mh; goto bad; }












/*
graph adjacency list:  {

log = logical address    (index)
phy = physical address   (index * 4)


	log     phy       op idx          l   g  e
	idx     [i]       +0              +1  +2 +3
========================================================================
	&0	#0:  ins(.op = 0, .lge = [ 1, 4, 1 ])

	&1	#4:  ins(.op = 1, .lge = [ 0, 2, 2 ])

	&2	#8:  ins(.op = 2, .lge = [ 0, 5, 3 ])

	&3	#12: ins(.op = 3, .lge = [ 1, 1, 4 ])

	&4	#16: ins(.op = 4, .lge = [ 2, 0, 1 ])

	&5	#20: ins(.op = 0, .lge = [ 0, 5, 0 ])
========================================================================

}



static bool graph_analysis(void) {


}


118689037748574
118689037748574




		//
			// WRONG:  
			//    consider the various 5 -x-> XXX -e-> 1    possibliities...  we can't account for all of them. 
			//
			//    if (graph[4 * index] != five and graph[4 * e] == one) {  a = 4 * index + 3; goto bad; }     // the revised version.
			//
			//	if (graph[4 * index] == one and graph[4 * e] == one) {          ///    NOTE:   this changed!!!! talk about this!!!!
			//		//if (index == one) { a = 4 * index + 3; goto bad; }      // i noticed that one->one uncond, is a subset of 
			//		//if (e == one) { a = 4 * index; goto bad; }              // one->one on eq,  which is stronger! 
			//		//a = 4 * (index < e ? index : e); goto bad;              //    and also prunable because fea constraint,. 
			//									//
			//							// ..then i realized that the one source doesnt need to be there!
			//	}
			//



 ///    NOTE:   this changed!!!! talk about this!!!!
 // i noticed that one->one uncond, is a subset of 
 // one->one on eq,  which is stronger! 
 //    and also prunable because fea constraint,. 
//
// ..then i realized that the one source doesnt need to be there!..?








*/


















