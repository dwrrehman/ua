// a new version of the search utility which focuses on graph analysis 
// and trying to guarantee a good epf when running stage1 exg. 
// this utility is just the stage0 portion. 
// written on 202408224.204643: by dwrr 
//

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

static const byte D = 3;
static const byte R = 0;

// static const nat stage0_execution_limit = 5;
// static const nat stage0_array_size = 100;

static const nat display_rate = 21;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good, pm_ga, 
	pm_fea, pm_ns0, 

	pm_pco, pm_zr5, 
	pm_zr6, pm_ndi, 

	pm_oer, pm_r0i, 
	pm_h0, pm_f1e, 

	pm_erc, pm_rmv, 
	pm_csm, 

	pm_bdl, pm_bdl2, 
	pm_erw, pm_mcal, 
	
	pm_h2, pm_h3, 
	pm_per, pm_snco,

	pm_r1i,
	pm_pt,

	pm_count
};

/*static const char* pm_spelling[pm_count] = {
	"z_is_good", "pm_ga", 
	"pm_fea", "pm_ns0", 

	"pm_pco", "pm_zr5", 
	"pm_zr6", "pm_ndi", 

	"pm_oer", "pm_r0i", 
	"pm_h0", "pm_f1e", 

	"pm_erc", "pm_rmv", 
	"pm_csm", 

	"pm_bdl", "pm_bdl2", 
	"pm_erw", "pm_mcal", 
	
	"pm_h2", "pm_h3", 
	"pm_per", "pm_snco",

	 "pm_r1i",
	"pm_pt",
};

*/


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

/*static const byte max_er_repetions = 50;
static const byte max_erw_count = 100;
static const byte max_modnat_repetions = 15;
static const byte max_consecutive_s0_incr = 30;
static const byte max_consecutive_s1_incr = 30;
static const byte max_consecutive_small_modnats = 200;
static const byte max_bdl_er_repetions = 25;
static const byte max_consecutive_h0_bouts = 10;
static const byte max_consecutive_h2_bouts = 30;
static const byte max_consecutive_h3_bouts = 30;
static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;
static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;*/

static byte* positions = NULL; 

struct prune_info {
	nat pm;
	nat ttp;
};

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

int main(void) {
	srand((unsigned) time(0));

	positions = calloc(hole_count, 1);
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	// char filename[4096] = {0};
	// byte* array = calloc(stage0_array_size + 1, sizeof(byte));

	byte* graph = calloc(1, graph_count);
	memcpy(graph, R ? _63R : _36R, 20);
	byte pointer = 0;
	nat display_counter = 0, good_count = 0, bad_count = 0;

	nat* histogram = calloc(40 + 1, sizeof(nat));

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	goto init;
loop:
	if (graph[positions[pointer]] < (positions[pointer] & 3 ? operation_count - 1 : 4)) goto increment;
	if (pointer < hole_count - 1) goto reset_;
	goto done;

increment:
	graph[positions[pointer]]++;
init:  	pointer = 0;

	u16 was_utilized = 0;
	byte at = 0;

	for (byte index = 20; index < graph_count - 4; index += 4) {
		if (graph[index] > graph[index + 4]) { at = index + 4; goto bad; } 
	}

	for (byte index = operation_count; index--;) {

		if (graph[4 * index + 3] == index) {  
			at = 4 * index + 3; goto bad; 
		}
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  
			at = 4 * index; goto bad; 
		}
		if (graph[4 * index] == six   and graph[4 * index + 2]) {  
			at = 4 * index; goto bad; 
		}
		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  
			at = 4 * index + 2 * (index == two); goto bad; 
		}
		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  
			at = 4 * index + 1 * (index == three); goto bad; 
		}	

		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == one) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == one) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * graph[4 * index + 3]] == five) {
			if (index == six) { at = 4 * index + 3; goto bad; } 
			const byte tohere = graph[4 * index + 3];
			if (tohere == five) { at = 4 * index; goto bad; }
			at = 4 * (index < tohere ? index : tohere); goto bad; 
		}
 
		const byte l = graph[4 * index + 1], g = graph[4 * index + 2], e = graph[4 * index + 3];

		if (graph[4 * index] == one and graph[4 * e] == one) {    // DELETE THIS!?!?!?! soon!!
			if (index == one) { at = 4 * index + 3; goto bad; }
			if (e == one) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad;
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

		if (graph[4 * index] == two)
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == six) {
					if (index == two) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == six) { at = 4 * index; goto bad; }
					at = 4 * (index < tohere ? index : tohere); goto bad; 
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

		if (graph[4 * index] == three) 
			for (byte offset = 1; offset < 4; offset++) 
				if (graph[4 * graph[4 * index + offset]] == three) { 
					if (index == three) { at = 4 * index + offset; goto bad; } 
					const byte tohere = graph[4 * index + offset];
					if (tohere == three) { at = 4 * index; goto bad; } 
					at = 4 * (index < tohere ? index : tohere); goto bad; 
				}

		
		if (l != index) was_utilized |= 1 << l;
		if (g != index) was_utilized |= 1 << g;
		if (e != index) was_utilized |= 1 << e;
	}

	for (byte index = 0; index < operation_count; index++) 
		if (not ((was_utilized >> index) & 1)) goto loop;


	for (byte i = 4 * (operation_count - 1); i >= 20; i -= 4) {
		for (byte j = 0; j < i; j += 4) {
			if (not memcmp(graph + i, graph + j, 4)) {
				if (j >= 20) { at = j; goto bad; } 
				for (byte h = 0; h < initial; h++) 
					if (positions[h] >= j) { at = positions[h]; goto bad; }
			}
		}
	}

	
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

	at = 0;
	if (not (display_counter & ((1 << display_rate) - 1))) {
		printf("\033[%dm%s:  z = ", at ? 31 : 32, at ? "PRUNED" : " FOUND"); 
		print_graph_raw(graph); 
		printf("    was pruned at = %hhu\033[0m\n", at);
		fflush(stdout);
		display_counter = 1;
	} else display_counter++;


	if (not at) {
		// TODO:   push this good z value to a candidates buffer, for stage2 to process later. 
		// append_to_file(filename, sizeof filename, graph, origin);
		good_count++;
		goto loop;
	} else bad_count++;



	histogram[at]++;


bad:
	
	///////////////////////////////////////////////////// this code can be so much better. just realize that if at >= 20 then we know that the value of at setting was valid, and we can just do simple arith  to turn that into a pointer value ("pointer = i" code would be replaced with simple arith) ANDDD if its NOT geq 20 then we can just do a for loop like the below code, butttt nottttt going over all holes,   just the ones part of the 0sp partial graph.  not the dol. 

	/////////////////////////////////////////////////////////////////////////////////////////////////
	for (byte i = 0; i < hole_count; i++) {
		if (positions[i] == at) { pointer = i; goto loop; } else graph[positions[i]] = 0;
	}
	//////////////////////////////////////////////////////////////////////////////////////////




	printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
	abort();

reset_:
	graph[positions[pointer]] = 0; 
	pointer++;
	goto loop;
done:;
	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec);
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	printf("su: found  %llu zv / %llu zv  in (%hhu,%hhu) search space!\n in %10.2lfs [%s:%s].\n", 
		good_count, bad_count, D, R, seconds,  time_begin_dt,  time_end_dt
	);

	const nat screen_width = 120;
	nat max_tally = 0;
	for (nat i = 0; i < 40 + 1; i++) {
		if (histogram[i] > max_tally) max_tally = histogram[i];
	}

	nat tallys_per_char = max_tally / screen_width;
	if (tallys_per_char == 0) tallys_per_char = 1;

	for (nat i = 0; i < 40 + 1; i++) {

		nat value = histogram[i];
		value /= tallys_per_char;
		printf(" %5llu :   %5llu : ", i, histogram[i]);

		if (histogram[i] and not value) value = 1;

		for (nat j = 0; j < value; j++) {
			printf("#");
		}
		puts("");
	}
}






























































	//puts("done");
	//abort();

			//printf("info: trying double combination PA @ i = %hhu, PA @ j = %hhu\n", i, j);
			//getchar();
			



	/*
	for (byte i = 20; i < 4 * operation_count; i += 4) 
		for (byte j = 20; j < i; j += 4) 
			if (not memcmp(graph + i, graph + j, 4)) { at = j; goto bad; } 
		
	*/




/*
static byte execute_graph_starting_at(byte origin, byte* graph, byte* array, byte* zskip_at) {

	const nat n = stage0_array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0, pointer = 0, mcal_index = 0;
	byte 	mcal_path = 0;

	byte ip = origin;
	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

	for (nat e = 0; e < stage0_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) abort();
			if (not array[pointer]) return pm_ns0; 
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
	
			did_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) {
		
			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6;

			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pm_snco;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three)  return pm_ndi;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

			array[pointer]++;
		}

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	return pm_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pm_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pm_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pm_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pm_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pm_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pm_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pm_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pm_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;

		if (*zskip_at > 3) {
			byte found_at = 0;
			for (nat i = 0; i < hole_count; i++) {
				if (I + state == positions[i]) { found_at = 1; break; }      /// change this to not be so crappy lol. 
			}

			if (*zskip_at > I + state and found_at) *zskip_at = I + state;
		}
		ip = graph[I + state];
	}
	return z_is_good;
}

static byte execute_graph(byte* graph, byte* array, byte* origin) {
	byte zskip_at = 255;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		const nat pm = execute_graph_starting_at(o, graph, array, &zskip_at);
		if (not pm) { *origin = o; return 0; } 
	}
	return zskip_at;
}

*/

















/*




	//puts("done");






printf("\033[%dm%s:  z = ", at ? 31 : 32, at ? "PRUNED" : " FOUND"); 
	print_graph_raw(graph); 
	printf("    was pruned at = %hhu\033[0m\n", at);
	fflush(stdout); 









puts("entering loop");
	
	printf("operation_count = %hhu\n", operation_count);




//printf("info: trying double combination PA @ i = %hhu, PA @ j = %hhu\n", i, j);
			//getchar();
//printf("i = %hhu\n", i);
		//printf("operation_count = %hhu\n", operation_count);

		//puts("");

		//printf("info: trying first combination PA @ i = %hhu\n", i);
		//getchar();


3sp

	
	0127 1000 2843 3650 4002    1000 2000 2100 4001

	0128 1076 2543 3100 4001    1000 2000 2100 4001

	0128 1076 2543 3100 4001    1000 1000 1000 1000

	0121 1053 2440 3405 4005  { 2044 3245 4124 }   D


	                            ^    ^    ^                i 


	^    ^    ^    ^    ^       ^    ^    ^                j


	(i < D):
		(j < i) :
			if (not memcmp(graph + i, graph + j)) {
				// set the value of at to SOMETHINGGGG?!?!
			}	
	
	
*/

/*
	at = 255;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;

		if (at > 4 * o + 1) at = 4 * o + 1;

		if (graph[4 * graph[4 * o + 1]] == five) continue;
		if (graph[4 * graph[4 * o + 1]] == six) continue;

		const byte eq_case = graph[4 * graph[4 * o + 1] + 3];

		if (graph[4 * graph[4 * o + 1]] == one) {
			if (at > 4 * graph[4 * o + 1] + 3) at = 4 * graph[4 * o + 1] + 3;

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
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
			}

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == three) continue;
				if (graph[4 * graph[4 * eq_case + 2]] == five) continue;
			}
		}
		goto run_graph_ega;
	}	
	goto bad;
*/






//run_graph_ega:;
//	byte origin;
//	at = execute_graph(graph, array, &origin);





	// if (graph[4 * 2 + 1] == 0) goto loop; // we are missing out on whether the graph is good or bad.... can't do this.









/*
	const nat screen_width = 120;
	nat max_tally = 0;
	for (nat i = 0; i < stage0_execution_limit + 1; i++) {
		if (histogram[i] > max_tally) max_tally = histogram[i];
	}

	const nat tallys_per_char = max_tally / screen_width;

	for (nat i = 0; i < 100 + 1; i++) {

		nat value = histogram[i];
		value /= tallys_per_char;
		printf(" %5llu :   %5llu : ", i, histogram[i]);

		if (histogram[i] and not value) value = 1;

		for (nat j = 0; j < value; j++) {
			printf("#");
		}
		puts("");
	}



printf("info: redundant original operation at j: "
					"PA @ i = %hhu, PA @ j = %hhu\n", i, j
				);





*/





/*



	//struct prune_info info[100];
	//nat info_count = 0;




	if (display_counter and not (display_counter & ((1 << display_rate) - 1))) {
		//printf("processing z = "); print_graph_raw(graph); 
		//printf("\n"); 
		//fflush(stdout);
		display_counter = 0;
	} 
	//for (nat i = 0; i < info_count; i++) {
		//	printf("(%s:%llu) ", pm_spelling[info[i].pm], info[i].ttp);
		//}







*/









/*



		 3 ----(<)---> 5
		 3 ----(<)---> 6


	3   1   1 



		0124 1035 2041 3606 4005 2000 2044
		          ^              ^    ^

		*0 == 0    (and the rest is too)
		 i == 0
		*n == 0

		*i++     // *0 == 1

		*n(0)  < *i(1)     --->    1
		*n(0)  > *i(1)    0
		*n(0) == *i(1)    0
		
		i++     //     *0 == 1     i == 1   *i == 0

		*n(0)  < *i(0)    0
		*n(0)  > *i(0)    0
		*n(0) == *i(0)      --->    1

		if (1 --(=)--> 6)   prune via pm_zr6.


		








36R:

		*i++     // *0 == 1

		*n(0)  < *i(1)     --->    1
		*n(0)  > *i(1)    0
		*n(0) == *i(1)    0
		
		*n++     //     *0 == 1     i == 0     *i == 1    *n == 1

		*n(1)  < *i(1)    0
		*n(1)  > *i(1)    0
		*n(1) == *i(1)      --->    1

		if (2 --(=)--> 5)   prune via pm_zr5.





*/













































/*






		//zthroughput = (double) space_size / seconds;

	//"\n\tspace_size=%llu"


	//"\n\tat %10.2lf z/s."
 //  space_size,  
		// BLAH,



	//      space_size = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);



static nat expn(nat base, nat exponent) {
	nat result = 1;
	for (nat i = 0; i < exponent; i++) result *= base;
	return result;
}


static nat adc64(nat* d, nat s) {
	*d += s;
	return *d < s;
}


static nat adc128(nat* dl, nat* dh, nat sl, nat sh, nat cin) {
	nat cl = adc64(dl, sl, cin);
	nat c = adc64(dh, sh, cl);
	return c;
}


chunk_count


mul512



3 * 6





	0 1 1 
	1 1 0  ---->   ( (1 0 0)   +    (0 1 0)  )
	----------

	( (0 1 1 )*(1 0 0)   +    (0 1 1 )*(0 1 0)  )





























static void mul128(nat* dl, nat* dh, const nat rl, const nat rh, const nat sl) {
	for (nat i = 0; i < sl; i++) 
		adc128(dl, dh, rl, rh, 0);
}

static void print_binary64(nat x) {
	for (nat i = 0; i < 64; i++) {
		printf("%llu", (x >> i) & 1);
	}
}

static void debug_zi(const char* s, nat l, nat h) {
	printf("%s = ", s);
	print_binary64(l); 
	putchar(' '); 
	print_binary64(h); 
	puts("");
}








		//nat copy[chunk_count] = {0};
		//memcpy(copy, total, chunk_count * sizeof(nat));
		//for (nat i = 0; i < 5 - 1; i++)  adc(total, copy, 0);




*/













/*int main(void) {
	const nat  D  =  0  ; 
 
	nat space_size_64 = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	printf("64-bit space size = %llu\n", space_size_64);

	const nat K = expn(5 + D, 3);
	nat total_l = expn(5 + D, 9);
	nat total_h = 0;
	
	for (nat _ = 0; _ < D; _++) {
		mul128(   &total_l, &total_h,    total_l,total_h,    5);
		mul128(   &total_l, &total_h,    total_l,total_h,    K);
	}
	
	print128("space_size", total_l, total_h);
}*/








//nat space_size_64 = expn(5 + D, 9) * expn(5 * expn(5 + D, 3), D);
	//printf("64-bit space size = %llu\n", space_size_64);






// if (array[n] >= 65535) { *out_e = e; return pm_snm; }




			//debug_zi("div1", div1);
			//debug_zi("rem2", rem2);
			//debug_zi("p_begin", p_begin);
			//getchar();
	

			//debug_zi("div1", div1);
			//debug_zi("rem2", rem2);
			//debug_zi("p_end", p_end);
			//getchar();






	//debug_zi("negative_divisor", negative_divisor);
	//debug_zi("total", total);







		//printf("---------------------job #%llu------------------------\n", job);
		//debug_zi("range_begin", range_begin);
		//debug_zi("range_end", range_end);
		//puts("---------------------------------------------");






// terminate_thread:
//printf("worker_thread[%llu]: max_pointer = %llu\n", thread_index, max_pointer);














/*

202408084.172603:

---------------- no sync ---------------------



using [D=2, R=0]:
	thread_count=6
	job_count_per_core=40
	display_rate=0
	stage1_execution_limit=10000000
	stage2_execution_limit=0
	array_size=100000


	searched [2:0] zvs
	using 6 threads, each with 40 jobs
	in     214.00s [1202408084.163115:1202408084.163449],


pm counts:
z_is_good: 76      		 pm_ga: 0       		
pm_fea: 0       		pm_ns0: 85769549		
pm_pco: 101688352		pm_zr5: 1330613485		
pm_zr6: 1829675457		pm_ndi: 359227372		
pm_oer: 17991   		pm_r0i: 649667  		
 pm_h0: 1099549 		pm_f1e: 41873   		
pm_erc: 11802431		pm_rmv: 36188   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 1980    		pm_bdl2: 144     		
pm_erw: 0       		pm_mcal: 527318490		
pm_snl: 0       		 pm_h1: 0       		
 pm_h2: 12      		 pm_h3: 0       		
pm_per: 1328144 		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 1214    		
[done_pm]









----------------- bsp -----------------------



using [D=2, R=0]:
	space_size=118689037748575
	thread_count=6
	cache_line_size=100
	minimum_split_size=6
	display_rate=0
	fea_execution_limit=5000
	execution_limit=10000000
	array_size=100000


	searched 118689037748575 zvs
	using 6 threads
	in     186.00s [1202408084.163932:1202408084.164238],
	at 638113106175.13 z/s.


pm counts:
z_is_good: 0       		 pm_ga: 118684830700923		
pm_fea: 0       		pm_ns0: 84018547		
pm_pco: 100895566		pm_zr5: 1317636407		
pm_zr6: 1816694631		pm_ndi: 353166042		
pm_oer: 17280   		pm_r0i: 647137  		
 pm_h0: 1086497 		pm_f1e: 41668   		
pm_erc: 11215047		pm_rmv: 36132   		
 pm_ot: 0       		pm_csm: 0       		
 pm_mm: 0       		pm_snm: 0       		
pm_bdl: 1980    		pm_bdl2: 144     		
pm_erw: 0       		pm_mcal: 520275382		
pm_snl: 0       	  	 pm_h1: 0       		
 pm_h2: 12      		 pm_h3: 0       		
pm_per: 1313974 		pm_snco: 0       		
pmf_fea: 0       		pmf_ns0: 0       		
pmf_pco: 0       		pmf_zr5: 0       		
pmf_zr6: 0       		pmf_ndi: 0       		
pmf_per: 0       		pmf_mcal: 0       		
pmf_snco: 0       		pm_r1i: 1206    		
[done_pm]



  0  1  2  2  

  1  0  5  0

  2  1  4  3

  3  1  1  6

  4  0  0  1


  1  1  1  1

  2  4  1  0
















































//  0122 1050 2143 3156 4002 1313 2403

// 0122 1050 2143 3156 4002 1313 2403
// 0122 1050 2143 3566 4005 1010 2455
// 012_ 10__ 2_4_ 3___ 400_ ____ ____









*/






// static const byte max_sn_loop_iterations = 100 * 2;






			
		/*	
			if (last_op == six) SNL_counter++;
			else if (last_op != two) SNL_counter = 0;

			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;
		*/





// SNL_counter = 0,  





		/*	if (last_op == two) SNL_counter++; 
			else SNL_counter = 0;
			if (SNL_counter >= max_sn_loop_iterations) return pm_snl;
		*/





/*


su: found  240256 zv / 66546056 zv  in (1,0) search space!
 in       5.00s [1202408224.213911:1202408224.213916].
     0 :       0 : 
     1 :   40881744 : ########################################################################################################################
     2 :   20375524 : ###########################################################
     3 :   12799228 : #####################################
     4 :   2837340 : ########
     5 :   3404454 : #########
     6 :   2198586 : ######
     7 :   854474 : ##
     8 :   238149 : #
     9 :   826241 : ##
    10 :       0 : 








su: found  240256 zv / 39440192 zv  in (1,0) search space!
 in       4.00s [1202408224.214525:1202408224.214529].
     0 :       0 : 
     1 :   8952000 : ####################################################
     2 :   20375524 : ########################################################################################################################
     3 :   12799228 : ###########################################################################
     4 :   2837340 : ################
     5 :   3404454 : ####################
     6 :   2198586 : ############
     7 :   854474 : #####
     8 :   238149 : #
     9 :   826241 : ####
    10 :       0 : 





su: found  625772 zv / 66160540 zv  in (1,1) search space!
 in       6.00s [1202408224.215035:1202408224.215041].
     0 :       0 : 
     1 :   9135320 : #############################
     2 :   21698752 : #####################################################################
     3 :   37521302 : ########################################################################################################################
     4 :   6629986 : #####################
     5 :   5490166 : #################
     6 :   1288702 : ####
     7 :   896368 : ##
     8 :   172148 : #
     9 :   1406120 : ####
    10 :       0 : 


su: found  625772 zv / 48907316 zv  in (1,1) search space!
 in       6.00s [1202408224.224233:1202408224.224239].
     0 :       0 : 
     1 :   6935078 : ######################
     2 :   2682626 : ########
     3 :   37521302 : ########################################################################################################################
     4 :   6629986 : #####################
     5 :   5490166 : #################
     6 :   1288702 : ####
     7 :   896368 : ##
     8 :   172148 : #
     9 :   1406120 : ####
    10 :       0 : 







su: found  240256 zv / 39440192 zv  in (1,0) search space!
 in       4.00s [1202408224.222758:1202408224.222802].
     0 :       0 : 
     1 :   8952000 : ####################################################
     2 :   20375524 : ########################################################################################################################
     3 :   12799228 : ###########################################################################
     4 :   2837340 : ################
     5 :   3404454 : ####################
     6 :   2198586 : ############
     7 :   854474 : #####
     8 :   238149 : #
     9 :   826241 : ####
    10 :       0 : 


su: found  240256 zv / 28118974 zv  in (1,0) search space!
 in       4.00s [1202408224.222827:1202408224.222831].
     0 :       0 : 
     1 :   6676212 : ##############################################################
     2 :   8383356 : ##############################################################################
     3 :   12799228 : ########################################################################################################################
     4 :   2837340 : ##########################
     5 :   3404454 : ###############################
     6 :   2198586 : ####################
     7 :   854474 : ########
     8 :   238149 : ##
     9 :   826241 : #######
    10 :       0 : 













su: found  240256 zv / 21674254 zv  in (1,0) search space!
 in       3.00s [1202408224.224206:1202408224.224209].
     0 :       0 : 
     1 :   4397796 : #########################################
     2 :   1255596 : ###########
     3 :   12799228 : ########################################################################################################################
     4 :   2837340 : ##########################
     5 :   3404454 : ###############################
     6 :   2198586 : ####################
     7 :   854474 : ########
     8 :   238149 : ##
     9 :   826241 : #######
    10 :       0 : 

















 3 0    1 1     2 2      1 3                        




ns0:

1=2>1
1=2>5
1=3<6
2=1>1
2=2>3
2=2>5



3 1 3 5








3 <    1 =    1

3 <    1 =    2 >    1         NEW (ns0)
3 <    1 =    2 >    2         ok
3 <    1 =    2 >    3         ok
3 <    1 =    2 >    5         NEW zr5
3 <    1 =    2 >    6


3 <    1 =    3 <    1         NEW (mcal)
3 <    1 =    3 <    2         ok
3 <    1 =    3 <    3
3 <    1 =    3 <    5         ok
3 <    1 =    3 <    6         NEW (zr6)

3 <    1 =    5
3 <    1 =    6
3 <    2 =    1 >    1         NEW (ns0)
3 <    2 =    1 >    2         ok
3 <    2 =    1 >    3         ok
3 <    2 =    1 >    5
3 <    2 =    1 >    6         ok

3 <    2 =    2 >    1	       ok
3 <    2 =    2 >    2         ok                                    <------- we should be pruning this... 
3 <    2 =    2 >    3         NEW (ndi)
3 <    2 =    2 >    5         NEW (zr5)
3 <    2 =    2 >    6 
3 <    2 =    3 
3 <    2 =    6
3 <    2 =    5
3 <    3
3 <    5
3 <    6







2 5    2 3   1 6 













3 <    2 =    2












3 <    3

3 <    5

3 <    6






















//



1= 2>1
1= 2>5
1= 3<6

2= 1>1
2= 2>3
2= 2>5



// 0140 1035 2044 3121 4202 0313 
//    ^       ^




	if (graph[4 * eq_case] == one) {
				if (pa + 3 < at) at = pa + 3;
				continue;
			}



			//...













su: found  240256 zv / 15403524 zv  in (1,0) search space!
 in       2.00s [1202408235.001119:1202408235.001121].
     0 :       0 : 
     1 :   3370932 : ###############################################################
     2 :   1070828 : ####################
     3 :   6411672 : ########################################################################################################################
     4 :   2837340 : #####################################################
     5 :   3404454 : ###############################################################
     6 :   2198586 : #########################################
     7 :   854474 : ###############
     8 :   238149 : ####
     9 :   826241 : ###############
    10 :       0 : 


su: found  678020 zv / 9976120 zv  in (1,0) search space!
 in       3.00s [1202408235.002155:1202408235.002158].
     0 :       0 : 
     1 :   2179128 : #############################################################################
     2 :   668156 : #######################
     3 :   152660 : #####
     4 :   3097116 : ##############################################################################################################
     5 :   3371982 : ########################################################################################################################
     6 :   2147826 : ############################################################################
     7 :   908070 : ################################
     8 :   301857 : ##########
     9 :   1288105 : #############################################
    10 :       0 : 
two_stage_search_ut




su: found  625772 zv / 32547606 zv  in (1,1) search space!
 in       6.00s [1202408235.001144:1202408235.001150].
     0 :       0 : 
     1 :   4653398 : ###########################
     2 :   2296674 : #############
     3 :   20671338 : ########################################################################################################################
     4 :   6629986 : ######################################
     5 :   5490166 : ###############################
     6 :   1288702 : #######
     7 :   896368 : #####
     8 :   172148 : #
     9 :   1406120 : ########
    10 :       0 : 






su: found  1713184 zv / 13322984 zv  in (1,1) search space!
 in       5.00s [1202408235.002107:1202408235.002112].
     0 :       0 : 
     1 :   2453156 : ####################################################
     2 :   1520574 : ################################
     3 :   315004 : ######
     4 :   5579074 : ########################################################################################################################
     5 :   4923382 : #########################################################################################################
     6 :   1366558 : #############################
     7 :   1054556 : ######################
     8 :   334368 : #######
     9 :   2625552 : ########################################################
    10 :       0 : 





su: found  678020 zv / 9976120 zv  in (1,0) search space!
 in       3.00s [1202408235.002552:1202408235.002555].
     0 :       0 : 
     1 :   2179128 : #############################################################################
     2 :   668156 : #######################
     3 :   152660 : #####
     4 :   3097116 : ##############################################################################################################
     5 :   3371982 : ########################################################################################################################
     6 :   2147826 : ############################################################################
     7 :   908070 : ################################
     8 :   301857 : ##########
     9 :   1288105 : #############################################
    10 :       0 : 








su: found  34693331 zv / 605218853 zv  in (2,0) search space!
 in     180.00s [1202408235.002820:1202408235.003120].
     0 :       0 : 
     1 :   214679252 : ##############################################################################################################
     2 :   55444484 : ############################
     3 :   16223985 : ########
     4 :   158642636 : #################################################################################
     5 :   232238844 : ########################################################################################################################
     6 :   158625066 : #################################################################################
     7 :   54135142 : ###########################
     8 :   17147959 : ########
     9 :   64765813 : #################################
    10 :       0 




















	*/



			


































/*
static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, nat* ttp) {

	const nat n = stage0_array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	e = 0,  xw = 0,  pointer = 0,  
		er_count = 0, 
		mcal_index = 0, 
		bout_length = 0, 
		walk_ia_counter = 0, 
		RMV_value = 0, 
		OER_er_at = 0,
		BDL_er_at = 0,
		BDL2_er_at = 0,
		pointer_incr_timeout = 0;

	byte 	mcal_path = 0, R1I_counter = 0,
		ERW_counter = 0, OER_counter = 0,  BDL_counter = 0,  
		BDL2_counter = 0,  R0I_counter = 0, 
		H0_counter = 0, H2_counter = 0, H3_counter = 0, 
		RMV_counter = 0, CSM_counter = 0;

	byte ip = origin;
	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

	for (; e < stage0_execution_limit; e++) {

		*ttp = e;

		if (e == expansion_check_timestep2) { 
			for (byte i = 0; i < 5; i++) {
				if (array[i] < required_s0_increments) return pm_f1e;
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return pm_erc;
		}
		
		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return pm_fea;
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
			pointer_incr_timeout = 0;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_er_repetions) return pm_oer;

			if (BDL_er_at and pointer == BDL_er_at - 1) { BDL_counter++; BDL_er_at--; }
			else { BDL_er_at = pointer; BDL_counter = 0; }
			if (BDL_counter >= max_bdl_er_repetions)  return pm_bdl;

			if (BDL2_er_at > 1 and pointer == BDL2_er_at - 2) { BDL2_counter++; BDL2_er_at -= 2; }
			else { BDL2_er_at = pointer; BDL2_counter = 0; }
			if (BDL2_counter >= max_bdl_er_repetions) return pm_bdl2;

			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_modnat_repetions) return pm_rmv;
			}

			if (walk_ia_counter == 1) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			did_ier_at = pointer;
			walk_ia_counter = 0;
			er_count++;
			pointer = 0;
		}

		else if (op == two) {
			// if (pointer_incr_timeout >= stage0_execution_limit >> 2) return pm_pt;
			// else pointer_incr_timeout++;

			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6;

			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pm_snco;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three)  return pm_ndi;

			if (last_mcal_op == five) {
				R0I_counter++;
				if (R0I_counter >= max_consecutive_s0_incr) return pm_r0i; 
			}

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (bout_length == 2) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (bout_length == 3) {
				H3_counter++;
				if (H3_counter >= max_consecutive_h3_bouts) return pm_h3; 
			} else H3_counter = 0;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

			bout_length = 0;
			walk_ia_counter++;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	return pm_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pm_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pm_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pm_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pm_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pm_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pm_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pm_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pm_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good;
}
*/





/*


static nat execute_operation_sequence(byte* os, nat* ttp, byte* trichotomy_outcomes) {

	const nat n = array_size;
	byte* array = calloc(n + 1, sizeof(byte));
	nat pointer = 0, mcal_index = 0;
	byte mcal_path = 0;

	byte last_op = 255, last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

	for (nat e = execution_limit + 1; e--;) {

		const byte op = os[e];
		*ttp = e;

		if (op == one) {
			if (pointer == n) return pm_fea;
			if (not array[pointer]) return pm_ns0;
			pointer++;
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			did_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) {


			//   todo:     determine if this piece of code is sensical to have in the utility...  plz


			// 			if (last_op == two) return pm_sndi;








			array[n]++;
		}
		else if (op == six) {  
			if (not array[n]) return pm_zr6;

			if (	last_op != one and 
				last_op != three and 
				last_op != five
			) return pm_snco;

			array[n] = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three)  return pm_ndi;

			if (did_ier_at != (nat) ~0) {
				if (pointer >= did_ier_at) return pm_per; 
				did_ier_at = (nat) ~0;
			}

			array[pointer]++;
		}

		if (op == three or op == one or op == five) { last_mcal_op = op; mcal_index++; }
		last_op = op;

		if (mcal_index == 1  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 2  and last_mcal_op != one) 	return pm_mcal;
		if (mcal_index == 3  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 4  and last_mcal_op != five) 	return pm_mcal;
		if (mcal_index == 5  and last_mcal_op != three) return pm_mcal;
		if (mcal_index == 6  and last_mcal_op != one) 	return pm_mcal;

		if (mcal_index == 7) {
			if (last_mcal_op == five) return pm_mcal;
			mcal_path = last_mcal_op == three ? 1 : 2;
		}

		if (mcal_index == 8 and mcal_path == 1 and last_mcal_op != one)  	return pm_mcal;
		if (mcal_index == 8 and mcal_path == 2 and last_mcal_op != three)  	return pm_mcal;

		if (mcal_index == 9 and mcal_path == 1 and last_mcal_op != three)  	return pm_mcal;
		if (mcal_index == 9 and mcal_path == 2 and last_mcal_op != five)  	return pm_mcal;

		if (mcal_index == 10 and mcal_path == 1 and last_mcal_op != five)  	return pm_mcal;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		trichotomy_outcomes[e] = state;
	}
	return z_is_good;
}







	at = 255;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (at > 4 * o + 1) at = 4 * o + 1;

		if (graph[4 * graph[4 * o + 1]] == five) continue;
		if (graph[4 * graph[4 * o + 1]] == six) continue;

		const byte eq_case = graph[4 * graph[4 * o + 1] + 3];

		if (graph[4 * graph[4 * o + 1]] == one) {
			if (at > 4 * graph[4 * o + 1] + 3) at = 4 * graph[4 * o + 1] + 3;

			if (graph[4 * eq_case] == six) continue;

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
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
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
			}

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == three) continue;
				if (graph[4 * graph[4 * eq_case + 2]] == five) continue;
			}
		}
		goto check_if_all_ops_are_used;
	}	
	goto bad;



*/













/*
static nat execute_graph(byte* graph, nat* array, byte* origin, struct prune_info* info, nat* info_count) {
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		nat e = 0;
		const nat pm = execute_graph_starting_at(o, graph, array, &e);
		info[(*info_count)++] = (struct prune_info) {.pm = pm, .ttp = e };
		if (not pm) { *origin = o; return z_is_good; } 
	}
	return 1;
}











	at = 255;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		if (at > 4 * o + 1) at = 4 * o + 1;

		if (graph[4 * graph[4 * o + 1]] == five) continue;
		if (graph[4 * graph[4 * o + 1]] == six) continue;

		const byte eq_case = graph[4 * graph[4 * o + 1] + 3];

		if (graph[4 * graph[4 * o + 1]] == one) {
			if (at > 4 * graph[4 * o + 1] + 3) at = 4 * graph[4 * o + 1] + 3;

			if (graph[4 * eq_case] == six) continue;

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
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
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
			}

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == three) continue;
				if (graph[4 * graph[4 * eq_case + 2]] == five) continue;
			}
		}
		goto check_if_all_ops_are_used;
	}	
	goto bad;












// 0125 1032 2140 3100 4001 1000
	for (nat i = 0; i < info_count; i++) {
		histogram[info[i].ttp]++;
	}

	if (info_count == 1 and info[0].ttp == 2) {

		printf("\033[%dm%s:  z = ", bad ? 31 : 32, bad ? "PRUNED" : "FOUND"); 
		print_graph_raw(graph); 
		printf("    was pruned by:  { ");
		for (nat i = 0; i < info_count; i++) {
			printf("(%s:%llu) ", pm_spelling[info[i].pm], info[i].ttp);
		}
		printf("}\033[0m\n"); 
		fflush(stdout);
		
		puts("something is problematic.lolololol");

		abort();
	}








*/










/*	at = 255;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;

		if (at > 4 * o + 1) at = 4 * o + 1;

		if (graph[4 * graph[4 * o + 1]] == five) continue;
		if (graph[4 * graph[4 * o + 1]] == six) continue;

		const byte eq_case = graph[4 * graph[4 * o + 1] + 3];

		if (graph[4 * graph[4 * o + 1]] == one) {
			if (at > 4 * graph[4 * o + 1] + 3) at = 4 * graph[4 * o + 1] + 3;

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
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == one) continue;
			}

			if (graph[4 * eq_case] == two) {
				if (at > 4 * eq_case + 2) at = 4 * eq_case + 2;
				if (graph[4 * graph[4 * eq_case + 2]] == three) continue;
				if (graph[4 * graph[4 * eq_case + 2]] == five) continue;
			}
		}
		goto check_if_all_ops_are_used;
	}	
	goto bad;



	//struct prune_info info[100];
	//nat info_count = 0;




	if (display_counter and not (display_counter & ((1 << display_rate) - 1))) {
		//printf("processing z = "); print_graph_raw(graph); 
		//printf("\n"); 
		//fflush(stdout);
		display_counter = 0;
	} 
	//for (nat i = 0; i < info_count; i++) {
		//	printf("(%s:%llu) ", pm_spelling[info[i].pm], info[i].ttp);
		//}







*/
































/*
NED:  z = 01241072264337524007100010001766    was pruned by:  { (pm_mcal:9) }
PRUNED:  z = 01241065214335624007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01271063254734724007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01221037254130044007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01251055274337144007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01221037254335244007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01241002274335744007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01211073254332054007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01241037254536054007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01221073254437254007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01251037254632454007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01221056264332554007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01271020254330654007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01221073254534654007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01261037254737654007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01211003264134754007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01241073254630164007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01271037254035164007100010001766    was pruned by:  { (pm_mcal:4) }
PRUNED:  z = 01221075214336264007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01211030254531464007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01241073254735464007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01221003264435564007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01221030254636664007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01261073254032764007100010001766    was pruned by:  { (pm_mcal:4) }
PRUNED:  z = 01241003264531074007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01241030254732174007100010001766    was pruned by:  { (pm_mcal:6) }
PRUNED:  z = 01271073254137174007100010001766    was pruned by:  { (pm_mcal:7) }
PRUNED:  z = 01251003264636274007100010001766    was pruned by:  { (pm_mcal:5) }
PRUNED:  z = 01241030254030574007100010001766    was pruned by:  { (pm_mcal:4) }
PRUNED:  z = 01261073254334574007100010001766    was pruned by:  { (pm_mcal:7) }
PRUNED:  z = 01271037204535674007100010001766    was pruned by:  { (pm_ns0:4) }
PRUNED:  z = 01251030254135774007100010001766    was pruned by:  { (pm_mcal:8) }
su: found  1383457067 zv / 25478631319 zv  in (3,0) search space!
 in   13645.00s [1202408235.003532:1202408235.042257].
     0 :       0 : 
     1 :   13252399259 : ########################################################################################################################
     2 :   2974803091 : ##########################
     3 :   871902597 : #######
     4 :   5978507822 : ######################################################
     5 :   10683321378 : ################################################################################################
     6 :   7409723487 : ###################################################################
     7 :   2214153731 : ####################
     8 :   653544567 : #####
     9 :   2396549931 : #####################
    10 :       0 : 
two_stage_search_utility: 

13645 / 3600 = 3.7902777778
















			paste the results here plz lol 


 z = 01211030214735444002100010003566    was pruned at = 7
PRUNED:  z = 01221073214331044005100020003566    was pruned at = 3
PRUNED:  z = 01211023274130664002200020003566    was pruned at = 3
PRUNED:  z = 01271053204536154005100010001666    was pruned at = 3
PRUNED:  z = 01261037204035204002100010002666    was pruned at = 3
PRUNED:  z = 01211037204634564005100010002666    was pruned at = 3
PRUNED:  z = 01251032204536244007100010002666    was pruned at = 3
PRUNED:  z = 01211005214337054005100010001766    was pruned at = 7
su: found  1383457067 zv / 9850859938 zv  in (3,0) search space!
 in   13674.00s [1202409113.202417:1202409124.001211].
     0 :       0 : 
     1 :       0 : 
     2 :       0 : 
     3 :   7716408329 : ########################################################################################################################
     4 :       0 : 
     5 :       0 : 
     6 :   1187355494 : ##################
     7 :   830035868 : ############
     8 :       0 : 
     9 :   117060247 : #
    10 :       0 : 
    11 :       0 : 
    12 :       0 : 
    13 :       0 : 
    14 :       0 : 
    15 :       0 : 


13674 / 3600 = 3.7983333333



nice lol

same amount of time 












PRUNED:  z = 01251032204536244007100010002666    was pruned at = 3
PRUNED:  z = 01211005214337054005100010001766    was pruned at = 7
su: found  1383457067 zv / 9850859938 zv  in (3,0) search space!
 in   13674.00s [1202409113.202417:1202409124.001211].
     0 :       0 : 
     1 :       0 : 
     2 :       0 : 
     3 :   7716408329 : ########################################################################################################################
     4 :       0 : 
     5 :       0 : 
     6 :   1187355494 : ##################
     7 :   830035868 : ############
     8 :       0 : 
     9 :   117060247 : #
    10 :       0 : 
    11 :       0 : 
    12 :       0 : 
    13 :       0 : 
    14 :       0 : 
    15 :       0 : 
    16 :       0 : 
    17 :       0 : 
    18 :       0 : 
    19 :       0 : 
    20 :       0 : 
    21 :       0 : 
    22 :       0 : 
    23 :       0 : 
    24 :       0 : 
    25 :       0 : 
    26 :       0 : 
    27 :       0 : 
    28 :       0 : 
    29 :       0 : 
    30 :       0 : 
    31 :       0 : 
    32 :       0 : 
    33 :       0 : 
    34 :       0 : 
    35 :       0 : 
    36 :       0 : 
    37 :       0 : 
    38 :       0 : 
    39 :       0 : 
    40 :       0 : 
    41 :       0 : 
    42 :       0 : 
    43 :       0 : 
    44 :       0 : 
    45 :       0 : 
    46 :       0 : 
    47 :       0 : 
    48 :       0 : 
    49 :       0 : 
    50 :       0 : 
    51 :       0 : 
    52 :       0 : 
    53 :       0 : 
    54 :       0 : 
    55 :       0 : 
    56 :       0 : 
    57 :       0 : 
    58 :       0 : 
    59 :       0 : 
    60 :       0 : 
    61 :       0 : 
    62 :       0 : 
    63 :       0 : 
    64 :       0 : 
    65 :       0 : 
    66 :       0 : 
    67 :       0 : 
    68 :       0 : 
    69 :       0 : 
    70 :       0 : 
    71 :       0 : 
    72 :       0 : 
    73 :       0 : 
    74 :       0 : 
    75 :       0 : 
    76 :       0 : 
    77 :       0 : 
    78 :       0 : 
    79 :       0 : 
    80 :       0 : 
    81 :       0 : 
    82 :       0 : 
    83 :       0 : 
    84 :       0 : 
    85 :       0 : 
    86 :       0 : 
    87 :       0 : 
    88 :       0 : 
    89 :       0 : 
    90 :       0 : 
    91 :       0 : 
    92 :       0 : 
    93 :       0 : 
    94 :       0 : 
    95 :       0 : 
    96 :       0 : 
    97 :       0 : 
    98 :       0 : 
    99 :       0 : 
   100 :       0 : 




*/








