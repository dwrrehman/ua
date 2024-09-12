// the compiletime empirical graph analysis utility,  ct ega.
// used for generating the c code at compiletime that is used 
// for the ega checks in the actual search utility.
// written on 2409113.171734 by dwrr.

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

static const nat execution_limit = 5;   // max ttp.
static const nat array_size = 1000;

enum operations { one, two, three, five, six };
static const char* trichotomy_outcome_spelling[4] = { " ", "<", ">", "=", };
static const char* operation_numeric_spelling[5] = { "1", "2", "3", "5", "6" };

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

static const char* pm_spelling[pm_count] = {
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

static nat execute_operation_sequence(byte* os, nat* ttp, byte* trichotomy_outcomes) {

	const nat n = array_size;
	nat* array = calloc(n + 1, sizeof(nat));
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



int main(void) {
	/*puts("");
	puts("");
	puts("\tat = 255;");
	puts("\tfor (byte o = 0; o < operation_count; o++) {");
	puts("\t\tif (graph[4 * o] != three) continue;");
	puts("\t\tif (at > 4 * o + 1) at = 4 * o + 1;");

	puts("\n\n\n");*/

	nat pointer = 0;
	byte* array = calloc(execution_limit + 1, 1);
	array[execution_limit] = 2;
	byte* trichotomy_outcomes = calloc(execution_limit + 1, 1);
	goto init;
loop:
	if (array[pointer] < 5 - 1) goto increment;
	if (pointer < execution_limit - 1) goto reset_;
	goto done;
increment:
	array[pointer]++;
	pointer = 0;
init:;
	nat at = (nat) -1;
	const nat pm = execute_operation_sequence(array, &at, trichotomy_outcomes);
if (pm) {
	printf("\t"); 
	for (nat i = execution_limit + 1; i--;) {
		if (i == at) {
			printf("[%s]  ", operation_numeric_spelling[array[i]]); 
		} else if (i < at) {
			printf("   "); 
		} else {
			printf("%s %s", operation_numeric_spelling[array[i]], trichotomy_outcome_spelling[trichotomy_outcomes[i]]); 
		}
		printf("     ");
	}
	printf("    ..... [%s]\n", pm_spelling[pm]);
}
	if (not pm) goto loop; 

	for (nat i = 0; i < at; i++) array[i] = 0;
	pointer = at;
	goto loop;
reset_:
	array[pointer] = 0; 
	pointer++;
	goto loop;
done:;


	/*puts("\n\n\n");
	puts("\t\tgoto check_if_all_ops_are_used;");
	puts("\t}");
	puts("\tgoto bad;");
	puts("check_if_all_ops_are_used:");
	puts("");
	puts("");*/
}























































































































/*
static nat execute_graph_starting_at(byte* graph) {

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

			bout_length++;
			pointer_incr_timeout = 0;
			pointer++;
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
		
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
	puts("given os: { ");
	for (nat i = execution_limit; i--;) {


		printf("%llu ", os[i]);


	}
	puts("}");
*/
















/*








for (byte i = 0; i < execution_limit; i++) {
		if (positions[i] == at) { pointer = i; goto loop; } else 
	}
	printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
	abort();








	positions = calloc(hole_count, 1);
	for (byte i = 0; i < initial; i++) positions[i] = R ? _63R_hole_positions[i] : _36R_hole_positions[i];
	for (byte i = 0; i < 4 * D; i++) positions[initial + i] = 20 + i; 

	nat* array = calloc(stage0_array_size + 1, sizeof(nat));

	byte* graph = calloc(1, graph_count);
	memcpy(graph, R ? _63R : _36R, 20);
	byte pointer = 0;

	nat* histogram = calloc(stage0_execution_limit + 1, sizeof(nat));

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

	if (display_counter and not (display_counter & ((1 << display_rate) - 1))) {
		//printf("processing z = "); print_graph_raw(graph); 
		//printf("\n"); 
		//fflush(stdout);
		display_counter = 0;
	} else display_counter++;
	
	GA



	EGA


bad:
	for (byte i = 0; i < hole_count; i++) {
		if (positions[i] == at) { pointer = i; goto loop; } else graph[positions[i]] = 0;
	}
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












	if (display_counter and not (display_counter & ((1 << display_rate) - 1))) {
		//printf("processing z = "); print_graph_raw(graph); 
		//printf("\n"); 
		//fflush(stdout);
		display_counter = 0;
	} else display_counter++;


try_executing:;

	struct prune_info info[100];
	nat info_count = 0;
	byte origin;
	nat bad = execute_graph(graph, array, &origin, info, &info_count);
	
	if (display_counter and not (display_counter & ((1 << display_rate) - 1))) {
		printf("\033[%dm%s:  z = ", bad ? 31 : 32, bad ? "PRUNED" : "FOUND"); 
		print_graph_raw(graph); 
		printf("    was pruned by:  { ");
		for (nat i = 0; i < info_count; i++) {
			printf("(%s:%llu) ", pm_spelling[info[i].pm], info[i].ttp);
		}
		printf("}\033[0m\n"); 
		fflush(stdout);
	}
	if (bad) bad_count++; else good_count++;

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

// 0125 1032 2140 3100 4001 1000




// TODO:   push this good z value to a candidates array, for stage2 to process later. 
// append_to_file(filename, sizeof filename, graph, origin);
// 
// }




*/













/*



//while (BLAH) {
	//	if (was_pruned(combination)) {
	//		printf("if(x < y){ ... } \n");
	//	}
	//}





we want to generate the following c code:










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



202409113.181418:
generated by program    [ TTP = 3 ]:


	3 <     1 =     [1]                   ..... [pm_ns0]
	3 <     1 =     2 >     [1]           ..... [pm_ns0]
	3 <     1 =     2 >     [5]           ..... [pm_pco]
	3 <     1 =     2 >     [6]           ..... [pm_snco]
	3 <     1 =     3 <     [1]           ..... [pm_mcal]
	3 <     1 =     3 <     [3]           ..... [pm_ndi]
	3 <     1 =     3 <     [6]           ..... [pm_zr6]
	3 <     1 =     [5]                   ..... [pm_pco]
	3 <     1 =     [6]                   ..... [pm_zr6]
	3 <     2 =     1 >     [1]           ..... [pm_ns0]
	3 <     2 =     1 >     [5]           ..... [pm_pco]
	3 <     2 =     2 >     [3]           ..... [pm_ndi]
	3 <     2 =     2 >     [5]           ..... [pm_zr5]
	3 <     2 =     2 >     [6]           ..... [pm_snco]
	3 <     2 =     [3]                   ..... [pm_ndi]
	3 <     2 =     [5]                   ..... [pm_zr5]
	3 <     2 =     [6]                   ..... [pm_snco]
	3 <     [3]                           ..... [pm_ndi]
	3 <     [5]                           ..... [pm_zr5]
	3 <     [6]                           ..... [pm_zr6]




also, 

generated by program    [ TTP = 4 ]:



	3 <     1 =     [1]                           ..... [pm_ns0]
	3 <     1 =     2 >     [1]                   ..... [pm_ns0]
	3 <     1 =     2 >     2 >     [1]           ..... [pm_ns0]
	3 <     1 =     2 >     2 >     [5]           ..... [pm_pco]
	3 <     1 =     2 >     2 >     [6]           ..... [pm_snco]
	3 <     1 =     2 >     3 =     [1]           ..... [pm_mcal]
	3 <     1 =     2 >     3 =     [3]           ..... [pm_ndi]
	3 <     1 =     2 >     [5]                   ..... [pm_pco]
	3 <     1 =     2 >     [6]                   ..... [pm_snco]
	3 <     1 =     3 <     [1]                   ..... [pm_mcal]
	3 <     1 =     3 <     2 =     [1]           ..... [pm_mcal]
	3 <     1 =     3 <     2 =     [3]           ..... [pm_ndi]
	3 <     1 =     3 <     2 =     [6]           ..... [pm_snco]
	3 <     1 =     3 <     [3]                   ..... [pm_ndi]
	3 <     1 =     3 <     5 <     [1]           ..... [pm_mcal]
	3 <     1 =     3 <     5 <     [5]           ..... [pm_pco]
	3 <     1 =     3 <     5 <     [6]           ..... [pm_zr6]
	3 <     1 =     3 <     [6]                   ..... [pm_zr6]
	3 <     1 =     [5]                           ..... [pm_pco]
	3 <     1 =     [6]                           ..... [pm_zr6]
	3 <     2 =     1 >     [1]                   ..... [pm_ns0]
	3 <     2 =     1 >     2 >     [1]           ..... [pm_ns0]
	3 <     2 =     1 >     2 >     [5]           ..... [pm_pco]
	3 <     2 =     1 >     2 >     [6]           ..... [pm_snco]
	3 <     2 =     1 >     3 =     [1]           ..... [pm_mcal]
	3 <     2 =     1 >     3 =     [3]           ..... [pm_ndi]
	3 <     2 =     1 >     [5]                   ..... [pm_pco]
	3 <     2 =     1 >     6 =     [1]           ..... [pm_ns0]
	3 <     2 =     1 >     6 =     [5]           ..... [pm_pco]
	3 <     2 =     1 >     6 =     [6]           ..... [pm_zr6]
	3 <     2 =     2 >     1 >     [1]           ..... [pm_ns0]
	3 <     2 =     2 >     1 >     [5]           ..... [pm_pco]
	3 <     2 =     2 >     2 >     [3]           ..... [pm_ndi]
	3 <     2 =     2 >     2 >     [5]           ..... [pm_zr5]
	3 <     2 =     2 >     2 >     [6]           ..... [pm_snco]
	3 <     2 =     2 >     [3]                   ..... [pm_ndi]
	3 <     2 =     2 >     [5]                   ..... [pm_zr5]
	3 <     2 =     2 >     [6]                   ..... [pm_snco]
	3 <     2 =     [3]                           ..... [pm_ndi]
	3 <     2 =     [5]                           ..... [pm_zr5]
	3 <     2 =     [6]                           ..... [pm_snco]
	3 <     [3]                                   ..... [pm_ndi]
	3 <     [5]                                   ..... [pm_zr5]
	3 <     [6]                                   ..... [pm_zr6]


















	3 <    1 =    1                [ns0]
	3 <    1 =    2 >    1         NEW (ns0)
	3 <    1 =    2 >    2         ok
	3 <    1 =    2 >    3         ok
	3 <    1 =    2 >    5         NEW pco    (edited)
	3 <    1 =    2 >    6         [snco]
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
	3 <    2 =    2 >    6         [snco]
	3 <    2 =    3 
	3 <    2 =    5
	3 <    2 =    6
	3 <    3
	3 <    5
	3 <    6






















GA:














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



	// EGA




	goto try_executing;














*/





