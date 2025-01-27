// edit distance search utility, based on the D general SU, 
// written on 2412146.212420, by dwrr
/* tries to use these 4 zv's as a partial graph:
	014415352131354442420020 2 dt
	014415352131354542420020 2 dt
	012115252033300442420040 2 dt
	014415252133310442420021 2 dt
and modifies K different PAs inside that partial graph, 
along with searching over all possible values for 
the new second duplicate operation, (in the case of 2sp)
*/

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

#define D 2
#define K 3

#define execution_limit 1000000000LLU
#define array_size 1000000LLU
#define display_rate 11

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good,
	pm_zr5, pm_zr6, pm_ndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi,
	pm_h0, pm_h1, pm_erw,
	pm_rmv, pm_imv, pm_csm,

	pm_ga_dummy,

	pm_ga_26dol,  pm_ga_sdol, 
	pm_ga_6g,  pm_ga_ns0, 
	pm_ga_zr5, pm_ga_pco, 
	pm_ga_ndi,  pm_ga_sndi, 
	pm_ga_snco,  pm_ga_sn1, 
	pm_ga_zr6,  pm_ga_rdo, 
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

	"pm_ga_dummy",

	"pm_ga_26dol",  "pm_ga_sdol", 
	"pm_ga_6g",  "pm_ga_ns0", 
	"pm_ga_zr5", "pm_ga_pco", 
	"pm_ga_ndi",  "pm_ga_sndi", 
	"pm_ga_snco",  "pm_ga_sn1", 
	"pm_ga_zr6",  "pm_ga_rdo", 
	"pm_ga_uo", 
};

#define operation_count (5 + D)
#define graph_count (operation_count * 4)
#define max_oer_repetions 50
#define max_rmv_modnat_repetions 30
#define max_imv_modnat_repetions 80
#define max_consecutive_small_modnats 230
#define max_consecutive_s0_incr 30
#define max_consecutive_h0_bouts 12
#define max_consecutive_h1_bouts 24
#define max_erw_count 200

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

static void append_to_file(char* filename, size_t filename_size, byte* graph, byte origin) {
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
		snprintf(filename, filename_size, "%s_%08x%08x%08x%08x_z.txt", dt, 
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

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0,  pointer = 0,  
		bout_length = 0, 
		RMV_value = 0, 
		IMV_value = 0,
		OER_er_at = 0,
		ERW_counter = 0,
		walk_ia_counter = 0;

	byte	H0_counter = 0,  H1_counter = 0, 
		OER_counter = 0, RMV_counter = 0, 
		IMV_counter = 0, CSM_counter = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

#define max_rsi_count 20
	byte rsi_counter[max_rsi_count] = {0};


	for (nat e = 0; e < execution_limit; e++) {

		if (e and not (e & ((1 << 28) - 1))) {
			printf("taking a significant amount of time on e = %llu, z = ", e); 
			print_graph_raw(graph); puts(""); 
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

			if (walk_ia_counter < (e < 500000 ? 3 : 6)) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			walk_ia_counter = 0;
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

			walk_ia_counter++;
			bout_length = 0;
			array[pointer]++;
		}
		if (op == three or op == one or op == five) last_mcal_op = op;
		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;

		//  if (*zskip_at > I + state) *zskip_at = I + state;

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
	return 1;
}




static byte** editable_pa_combinations = NULL;
static nat combination_count = 0;



// ((pa < 20 and pa % 4 == 0) or pa == 18)    AND   not in   {1, 2, 3}




static byte one_of_K_designated_editable_pas(byte pa, nat this) {

	//printf("\tnoneditable(pa = %hhu, this = %llu)\n", pa, this);

	//puts("\tgoing through k elements in this combination");
	for (byte i = 0; i < K; i++) {

		//printf("\t\tnoneditable:   looking at   pa = %hhu,  [i] = %hhu...\n", pa, editable_pa_combinations[this][i]);

		if (editable_pa_combinations[this][i] == pa) {
			//puts("\t\t\t---> FOUND!!!!");
			return 1;
		}
	}
	//puts("NOT FOUND. in editable PA list.");
	return 0;
}


static byte editable(byte pa, nat this) {
	if (pa == 18) return 0;
	if (pa < 20 and pa % 4 == 0) return 0;
	if (pa >= 24) return 1;
	return one_of_K_designated_editable_pas(pa, this);
}

static byte noneditable(byte pa, nat this) { return not editable(pa, this); }



static void index_combination(byte* editable_pas, const byte editable_count) {
	if (not K) return;
	int* pointers = calloc(K, sizeof(int));	
	int r = 0, i = 0;
	const int n = (int) editable_count;
	while (r >= 0) {
		if (i <= (n + (r - K))) {
			pointers[r] = i;
			if (r == K - 1) {

				editable_pa_combinations = realloc(editable_pa_combinations, sizeof(byte*) * (combination_count + 1));
				editable_pa_combinations[combination_count] = malloc(K);
				for (byte e = 0; e < K; e++) {
					editable_pa_combinations[combination_count][e] = editable_pas[pointers[e]];
				}
				combination_count++;
				
				i++;
			} else {
				i = pointers[r] + 1;
				r++;
			}
		} else {
			r--;
			if (r >= 0) i = pointers[r] + 1;
		}			
	}
}


static void init_graph_from_string(byte* graph, const char* string) {
	for (byte i = 0; i < strlen(string); i++) 
		graph[i] = (byte) (string[i] - '0');
}


static byte graph_analysis(byte* graph, nat c, nat* counts) {

	u16 was_utilized = 0;
	byte at = 255;

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

		if (graph[4 * index] == six and g != 4) {
			at = graph_count;
			if (editable(4 * index + 2, c) and at > 4 * index + 2) at = 4 * index + 2;
			if (editable(4 * index, c) and at > 4 * index) at = 4 * index;
			if (at == graph_count) continue;
			counts[pm_ga_6g]++;
			//puts(pm_spelling[pm_ga_6g]);
			goto bad;
		}

		if (graph[4 * index] == six and graph[4 * e] == one) {
			at = graph_count;
			if (editable(4 * index + 3, c) and at > 4 * index + 3) at = 4 * index + 3;
			if (editable(4 * index, c) and at > 4 * index) at = 4 * index;
			if (editable(4 * e, c) and at > 4 * e) at = 4 * e;
			if (at == graph_count) continue;
			counts[pm_ga_ns0]++;
			//puts(pm_spelling[pm_ga_ns0]);
			goto bad;
		}
 
		if (graph[4 * index] == six and graph[4 * e] == five) {
			at = graph_count;
			if (editable(4 * index + 3, c) and at > 4 * index + 3) at = 4 * index + 3;
			if (editable(4 * index, c) and at > 4 * index) at = 4 * index;
			if (editable(4 * e, c) and at > 4 * e) at = 4 * e;
			if (at == graph_count) continue;
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
					if (editable(4 * index + offset, c) and at > 4 * index + offset) at = 4 * index + offset;
					if (editable(4 * index, c) and at > 4 * index) at = 4 * index;
					if (editable(4 * dest, c) and at > 4 * dest) at = 4 * dest;
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

		/*const byte j = 4 * index;
		for (byte i = graph_count - 4; i >= 20 and j < i; i -= 4) {
			if (not memcmp(graph + i, graph + j, 4)) { 
				at = j + (j < 20); 
				counts[pm_ga_rdo]++; 
				// puts("pm_ga_rdo"); 
				goto bad;
			}
		}*/
	}

	for (byte index = 0; index < operation_count; index++) {
		if (not ((was_utilized >> index) & 1)) { 
			at = 0; do at++; while (noneditable(at, c));
			counts[pm_ga_uo]++; 
			//puts("pm_ga_uo"); 
			goto bad; 
		} 
	}

	return 0;
bad: 	return at;
}


static void nf(
	byte* graph, 
	nat* array, 
	nat* counts, 
	char* filename, 
	nat* good_count, 
	nat* bad_count, 
	nat* exg_bad_count,
	nat combination
) {
	nat display_counter = 0;
	byte pointer = 1;

	goto init;
loop:
	if (graph[pointer] < ((pointer % 4) ? operation_count - 1 : 4)) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto done;

increment:
	graph[pointer]++;
init:  	pointer = 0;
	do pointer++; while (noneditable(pointer, combination));

	const byte debug = not (display_counter & ((1 << display_rate) - 1));
	if (debug) display_counter = 1; else display_counter++;

	if (debug) {
		printf("GA trying: z = "); print_graph_raw(graph); puts(""); fflush(stdout);
	}
	
	byte at = graph_analysis(graph, combination, counts);
	if (at) goto bad;

	byte origin = 0;
	byte is_bad = execute_graph(graph, array, &origin, counts);

	if (debug) {
		printf("EXECUTED:  z = "); 
		print_graph_raw(graph); 
		puts(""); fflush(stdout);
	}

	if (not is_bad) {
		printf("\033[32m  ---> GOOD: origin = %hhu, z = ", origin); 
		print_graph_raw(graph);
		printf("\033[0m\n");
		fflush(stdout);

		append_to_file(filename, 4096, graph, origin);
		usleep(100000);
		(*good_count)++;
	} else 
		(*exg_bad_count)++;

	goto loop;

bad:	(*bad_count)++;
	if (noneditable(at, combination)) {
		printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
		abort();
	}
	for (byte i = 1; i < at; i++) if (editable(i, combination)) graph[i] = 0;
	pointer = at; goto loop;
reset_:
	graph[pointer] = 0; 
	do pointer++; while (noneditable(pointer, combination));
	goto loop;
done:;
	return;
}



static void select_partial_graph(byte* graph, byte zv_index) {
	const char* zv_string = NULL;
	if (zv_index == 0) zv_string = "014415352131354442420020";
	if (zv_index == 1) zv_string = "014415352131354542420020";
	if (zv_index == 2) zv_string = "012115252033300442420040";
	if (zv_index == 3) zv_string = "014415252133310442420021";
	init_graph_from_string(graph, zv_string);
}


int main(void) {

	srand((unsigned) time(0));

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	byte* editable_pas_1sp = calloc(graph_count, 1);
	byte editable_count_1sp = 0;

	for (byte pa = 0; pa < 24; pa++) {
		if ((pa < 20 and pa % 4 == 0) or pa == 18) continue;
		editable_pas_1sp[editable_count_1sp++] = pa;
	}

	index_combination(editable_pas_1sp, editable_count_1sp); 


	printf("editable_pas_1sp: (%hhu){", editable_count_1sp);
	for (byte i = 0; i < editable_count_1sp; i++) {
		printf("%hhu ", editable_pas_1sp[i]);
	}
	printf("}\n");

	printf("editable_pa_combinations: (%llu)\n", combination_count);
	for (nat i = 0; i < combination_count; i++) {
		printf("%llu: { ", i);
		for (byte j = 0; j < K; j++) {
			printf("%hhu ", editable_pa_combinations[i][j]);
		}
		puts("}\n");
	}
	puts("");


	char filename[4096] = {0};
	byte graph[graph_count] = {0};
	nat* array = calloc(array_size + 1, sizeof(nat));
	nat* counts = calloc(pm_count, sizeof(nat));
	
	nat 
		good_count = 0, 
		bad_count = 0, 
		exg_bad_count = 0;
		
	for (byte zv_index = 0; zv_index < 4; zv_index++) {

		printf("trying zv index = %hhu...\n", zv_index);

		for (nat this_combination = 0; this_combination < combination_count; this_combination++) {

			printf("trying combination index = %llu / %llu : { ", this_combination, combination_count);

			for (byte i = 0; i < K; i++) {
				printf("%hhu ", editable_pa_combinations[this_combination][i]);
			}
			puts("}");
			
			
			select_partial_graph(graph, zv_index);

			//puts("using zv: ");
			//print_graph_raw(graph); puts("");
			//getchar();

			for (byte i = 0; i < graph_count; i++)  {
				//puts("\n");

				const byte pa_is_editable = editable(i, this_combination);
				//printf("checking PA = %hhu: is_editable = %u\n", i, pa_is_editable);
				if (pa_is_editable)  { 
					//puts("zeroing..."); 
					graph[i] = 0; 
				} else { 
					//puts("not zeroing..."); 
				} 
			}


			//puts("using initial parial graph: ");
			//print_graph_raw(graph); puts("");
			//getchar();

			nf(
				graph, array, counts, filename, 
				&good_count, &bad_count, &exg_bad_count,
				this_combination
			);
			
			//printf("final graph state: ");
			//print_graph_raw(graph); 
			//puts("");
			// getchar();


			//printf("trying combination index = %llu : { ", this_combination);

			/*for (byte i = 0; i < K; i++) {
				printf("%hhu ", editable_pa_combinations[this_combination][i]);
			}
			puts("}");
			

			for (byte i = 0; i < graph_count; i++)  {
				puts("\n");

				const byte pa_is_editable = editable(i, this_combination);
				printf("checking PA = %hhu: is_editable = %u\n", i, pa_is_editable);
				if (pa_is_editable)  { 
					puts("zeroing..."); 
				} else { 
					puts("not zeroing..."); 
				} 
			}

			puts("using initial parial graph: ");
			print_graph_raw(graph); puts("");
			// getchar();

			*/
		}

		printf("finished trying zv %hhu...\n", zv_index);
		// getchar();
	}








	struct timeval time_end = {0};
	gettimeofday(&time_end, NULL);

	const double seconds = difftime(time_end.tv_sec, time_begin.tv_sec);
	char time_begin_dt[32] = {0}, time_end_dt[32] = {0};
	strftime(time_end_dt,   32, "1%Y%m%d%u.%H%M%S", localtime(&time_end.tv_sec));
	strftime(time_begin_dt, 32, "1%Y%m%d%u.%H%M%S", localtime(&time_begin.tv_sec));

	printf("su: D = %u, K = %u, space:\n"
		"\t good    %llu zv\n"
		"\t bad     %llu zv\n"
		"\t exg_bad %llu zv\n"
		"\t execution_limit = %llu\n"
                "\t array_size = %llu\n"
		"\t in %10.2lfs [%s:%s]\n"
		"\n",
		D, K, good_count, bad_count, exg_bad_count,
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
	for (byte index = graph_count - 4; index >= 20; index -= 4) {
		if (graph[index] != two and graph[index] != six) {       // for now
			at = index; 
			counts[pm_ga_26dol]++; 
			// puts(pm_spelling[pm_ga_26dol]); 
			goto bad; 
		}  
	}
*/


/*


ttp4  (aka ttp3)    pruning table  for creating    ct ega    source code:


------------------------------------------------------------------------

H	[1]                                   ..... [pm_ns0]


H	2 >     [1]                           ..... [pm_ns0]
H	2 >     2 >     [1]                   ..... [pm_ns0]
H	2 >     2 >     2 >     [1]           ..... [pm_ns0]
H	2 >     2 >     2 >     [5]           ..... [pm_pco]
H	2 >     2 >     3 >     [5]           ..... [pm_zr5]
H	2 >     2 >     [5]                   ..... [pm_pco]

H	2 >     2 >     6 =     [1]           ..... [pm_ns0]
H	2 >     2 >     6 =     [5]           ..... [pm_pco]
H	2 >     2 >     6 =     [6]           ..... [pm_zr6]
H	2 >     3 =     1 >     [1]           ..... [pm_ns0]
H	2 >     3 =     1 >     [5]           ..... [pm_pco]
H	2 >     3 =     2 >     [5]           ..... [pm_zr5]
H	2 >     3 =     3 <     [5]           ..... [pm_zr5]
H	2 >     3 =     [5]                   ..... [pm_zr5]
H	2 >     3 =     6 <     [5]           ..... [pm_zr5]
H	2 >     3 =     6 <     [6]           ..... [pm_zr6]
H	2 >     [5]                           ..... [pm_pco]

H	2 >     6 =     [1]                   ..... [pm_ns0]
H	2 >     6 =     2 >     [1]           ..... [pm_ns0]
H	2 >     6 =     2 >     [5]           ..... [pm_pco]
H	2 >     6 =     3 <     [5]           ..... [pm_zr5]
H	2 >     6 =     3 <     [6]           ..... [pm_zr6]
H	2 >     6 =     [5]                   ..... [pm_pco]
H	2 >     6 =     [6]                   ..... [pm_zr6]





	3 <     1 =     [1]                   ..... [pm_ns0]
	3 <     1 =     2 >     [1]           ..... [pm_ns0]
	3 <     1 =     2 >     [5]           ..... [pm_pco]
	3 <     1 =     3 <     [6]           ..... [pm_zr6]
	3 <     1 =     [5]                   ..... [pm_pco]
	3 <     1 =     [6]                   ..... [pm_zr6]
	3 <     2 =     1 >     [1]           ..... [pm_ns0]
	3 <     2 =     1 >     [5]           ..... [pm_pco]
	3 <     2 =     2 >     [5]           ..... [pm_zr5]
	3 <     2 =     3 <     [5]           ..... [pm_zr5]
	3 <     2 =     [5]                   ..... [pm_zr5]
	3 <     2 =     6 <     [5]           ..... [pm_zr5]
	3 <     2 =     6 <     [6]           ..... [pm_zr6]
	3 <     3 <     1 =     [1]           ..... [pm_ns0]
	3 <     3 <     1 =     [5]           ..... [pm_pco]
	3 <     3 <     1 =     [6]           ..... [pm_zr6]
	3 <     3 <     2 <     [5]           ..... [pm_zr5]
	3 <     3 <     3 <     [5]           ..... [pm_zr5]
	3 <     3 <     3 <     [6]           ..... [pm_zr6]
	3 <     3 <     [5]                   ..... [pm_zr5]
	3 <     3 <     [6]                   ..... [pm_zr6]
	3 <     [5]                           ..... [pm_zr5]
	3 <     [6]                           ..... [pm_zr6]

H	[5]                                   ..... [pm_pco]
H	[6]                                   ..... [pm_zr6]

*/











/*

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
		

		*/




	
	/*
	at = graph_count;
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














		//if (graph[4 * index] == one and graph[4 * index + 2] == index) {       // removed this ga pm too.
		//	at = 4 * index + 2; goto bad; 
		//}







//printf("trying combination: [index = %u, i = %hhu]\n", j, i);



// print_graph_raw(graph); printf("at = %hhu\n", at); getchar(); 




// const nat debug = not memcmp(graph, (byte[]) {0, 1, 1, 2, 1 ,1 ,2 ,0 ,2 ,1 ,3 ,4 ,3 ,0 ,2 ,0 ,4 ,0 ,0 ,1}, graph_count);



// 01121120213430204001
// 01121120213430204001





/*

if (debug) {
			printf("origin = %hhu, pm = %s\n", o, pm_spelling[pm]);
			getchar();
		}
*/




















/*


with ct-ega:
=-------------

PRUNED:  origin = 0, z = 00411120202432244302 ... at = 1
PRUNED:  origin = 0, z = 01411423234030444302 ... at = 1
PRUNED:  origin = 0, z = 01041040200032444302 ... at = 3
PRUNED:  origin = 0, z = 00011022241134444302 ... at = 6
su: 0 space:
	 good    12 zv
	 bad     16188009 zv
	 exg_bad 4197998 zv
	 execution_limit = 10000000000
	 array_size = 1000000
	 in    1450.00s [1202410222.201110:1202410222.203520]


pm counts:
z_is_good: 12      		 pm_ga: 0       		
pm_zr5: 1501823 		pm_zr6: 1204877 		
pm_pco: 813413  		pm_per: 402525  		
pm_ns0: 773358  		pm_oer: 216     		
pm_r0i: 22278   		pm_r1i: 452     		
 pm_h0: 2413552 		 pm_h1: 45      		
pm_erw: 3909    		pm_rmv: 4922    		
pm_imv: 156     		 pm_pt: 1254482 		[done]
D_general_search_utility: 

1202410222.201418_73a9b4c20a9a67e11f19ea375e5fd0d4_z.txt


02421020210331024001 2 1202410222.201418
02421020211331024001 2 1202410222.201452
02421020212331024001 2 1202410222.201526
02421020210331124001 2 1202410222.201605
02421020211331124001 2 1202410222.201639
02421020212331124001 2 1202410222.201713
02421020210331224001 2 1202410222.201755
02421020211331224001 2 1202410222.201829
02421020212331224001 2 1202410222.201903
02421020210331424001 2 1202410222.201954
02421020211331424001 2 1202410222.202028
02421020212331424001 2 1202410222.202102






without ct-ega: (but with rt-ega)
=-------------

su: 0 space:
	 good    50 zv
	 bad     17948095 zv
	 exg_bad 6510145 zv
	 execution_limit = 10000000000
	 array_size = 1000000
	 in    2973.00s [1202410222.183934:1202410222.192907]


pm counts:
z_is_good: 50      		 pm_ga: 0       		
pm_zr5: 2164468 		pm_zr6: 1842803 		
pm_pco: 2022018 		pm_per: 521677  		
pm_ns0: 2283862 		pm_oer: 258     		
pm_r0i: 34582   		pm_r1i: 816     		
 pm_h0: 2838664 		 pm_h1: 76      		
pm_erw: 6229    		pm_rmv: 6112    		
pm_imv: 286     		 pm_pt: 1298489 		[done]
D_general_search_utility: 







*/












		/*if (e == index) {  
			at = 4 * index + 3; goto bad; 
		}




if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == one) { 
			// when we duplicate a five, this is not quite sound. 
			if (index == five) { at = 4 * index + 1; goto bad; } 
			if (e == one) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad; 
		}

		if (graph[4 * index] == five and l == g and l == e and graph[4 * e] == three) {
			if (index == five) { at = 4 * index + 1; goto bad; } 
			if (e == three) { at = 4 * index; goto bad; }
			at = 4 * (index < e ? index : e); goto bad; 
		}













	at = graph_count;
	for (byte o = 0; o < operation_count; o++) {
		const byte t0 = 4 * o;
		if (graph[t0] == one) continue;
		else if (graph[t0] == five) continue;
		else if (graph[t0] == six) continue;
		else if (graph[t0] == two) {  if (at > t0 + 2) at = t0 + 2;
			const byte t1 = 4 * graph[t0 + 2];
			if (graph[t1] == one) continue;
			else if (graph[t1] == five) continue;
			else if (graph[t1] == two) {  if (at > t1 + 2) at = t1 + 2;
				const byte t2 = 4 * graph[t1 + 2];
				if (graph[t2] == one) continue;
				else if (graph[t2] == five) continue;
				else if (graph[t2] == two) {  if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == three) { if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == six) { if (at > t2 + 3) at = t2 + 3;
					const byte t3 = 4 * graph[t2 + 3];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
					if (graph[t3] == six) continue;
				}
			}
			if (graph[t1] == three) {  if (at > t1 + 3) at = t1 + 3;
				const byte t2 = 4 * graph[t1 + 3];
				if (graph[t2] == five) continue;
				else if (graph[t2] == one) {  if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == two) {  if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == three) { if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == six) { if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == five) continue;
					if (graph[t3] == six) continue;
				}
			}
			else if (graph[t1] == six) { if (at > t1 + 3) at = t1 + 3;
				const byte t2 = 4 * graph[t1 + 3];
				if (graph[t2] == one) continue;
				else if (graph[t2] == five) continue;
				else if (graph[t2] == six) continue;
				else if (graph[t2] == two) {   if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
				} 
				else if (graph[t2] == three) {   if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == five) continue;
					if (graph[t3] == six) continue;
				}
			}
		} 
		else if (graph[t0] == three) { if (at > t0 + 1) at = t0 + 1;
			const byte t1 = 4 * graph[t0 + 2];
			if (graph[t1] == five) continue;
			else if (graph[t1] == six) continue;
			else if (graph[t1] == one) { if (at > t1 + 3) at = t1 + 3;
				const byte t2 = 4 * graph[t1 + 3];
				if (graph[t2] == one) continue;
				else if (graph[t2] == five) continue;
				else if (graph[t2] == six) continue;
				else if (graph[t2] == two) { if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
				}
				if (graph[t2] == three) { if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == six) continue;
				}
			}
			else if (graph[t1] == two) { if (at > t1 + 3) at = t1 + 3;
				const byte t2 = 4 * graph[t1 + 3];

				if (graph[t2] == five) continue;

				else if (graph[t2] == one) {  if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == two) { if (at > t2 + 2) at = t2 + 2;
					const byte t3 = 4 * graph[t2 + 2];
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == three) { if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == five) continue;
				}
			}
			else if (graph[t1] == three) { if (at > t1 + 1) at = t1 + 1;
				const byte t2 = 4 * graph[t1 + 1];

				if (graph[t2] == five) continue;
				else if (graph[t2] == six) continue;

				else if (graph[t2] == one) {  if (at > t2 + 3) at = t2 + 3;
					const byte t3 = 4 * graph[t2 + 3];
					if (graph[t3] == one) continue;
					if (graph[t3] == five) continue;
					if (graph[t3] == six) continue;
				}
				else if (graph[t2] == two) { if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == five) continue;
				}
				else if (graph[t2] == three) { if (at > t2 + 1) at = t2 + 1;
					const byte t3 = 4 * graph[t2 + 1];
					if (graph[t3] == five) continue;
					if (graph[t3] == six) continue;
				}
			}
		}
		goto exit_ega;
	}

	// printf("CT-EGA ga prune: at = %hhu, goto bad;\n", at);

	goto bad;
exit_ega:;








*/





			//if (pointer_incr_timeout >= 100000) return pm_pt;
			//else pointer_incr_timeout++;






//if (pointer_incr_timeout >= 100000) return pm_pt;
			//else pointer_incr_timeout++;






 //pm_pt,//"pm_pt",






// pointer_incr_timeout = 0,


/*

if ((at < 20 and at % 4 == 0) or at == 5 or at == 1) {
		printf("internal programming error: at was set to the value of %hhu, which is not an valid hole\n", at);
	}












					if (index == source) { at = 4 * index + offset; goto bad; } 


					if (dest == destination) { at = 4 * index; goto bad; }

					at = 4 * (index < dest ? index : dest); goto bad; 














		if (graph[4 * index] == two and g == index) {  
			at = 4 * index + 2 * (index == two); goto bad; 
		}

		if (graph[4 * index] == three and l == index) {  
			at = 4 * index + 1 * (index == three); goto bad; 
		}













	{const byte pairs[3 * 3] = {
			three, three, pm_ga_ndi,
			five, one, pm_ga_pco,
			five, five, pm_ga_zr5,
		};

		for (nat i = 0; i < 3 * 3; i += 3) {
			const byte source = pairs[i + 1], destination = pairs[i + 0];
			if (graph[4 * index] == source) {
				for (byte offset = 1; offset < 4; offset++) {
					const byte middle = graph[4 * index + offset];
					if (graph[4 * middle] != two and graph[4 * middle] != six) continue;
					for (byte offset2 = 1; offset2 < 4; offset2++) {
						const byte dest = graph[4 * middle + offset2];
						if (graph[4 * dest] != destination) continue;
						at = graph_count;
						if (4 * index < 20 and at > 4 * index + offset) at = 4 * index + offset;
						if (4 * middle < 20 and at > 4 * middle + offset2) at = 4 * middle + offset2;
						if (4 * index >= 20 and at > 4 * index) at = 4 * index;
						if (4 * middle >= 20 and at > 4 * middle) at = 4 * middle;
						if (4 * dest >= 20 and at > 4 * dest) at = 4 * dest;
						counts[pairs[i + 2]]++;
						// puts(pm_spelling[pairs[i + 2]]);
						goto bad;
					}
				}
			} 
		} }








*/


/*


		if (graph[4 * index] == one and l == g and g == e and graph[4 * l] == one) {
			at = graph_count;
			if (index < 5 and at > 4 * index + 1) at = 4 * index + 1;
			if (index >= 5 and at > 4 * index) at = 4 * index;
			if (l >= 5 and at > 4 * l) at = 4 * l;
			counts[pm_ga_ns0]++;
			//puts(pm_spelling[pm_ga_ns0]);
			goto bad;
		}













if (pointer == 1) {
				if (last_mcal_op == three) {
					R1I_counter++;
					if (R1I_counter >= max_consecutive_s0_incr) return pm_r1i;
				} else R1I_counter = 0;
			}

			if (pointer == 2) {
				if (last_mcal_op == three) {
					R2I_counter++;
					if (R2I_counter >= max_consecutive_s0_incr) return pm_r2i;
				} else R2I_counter = 0;
			}

			if (pointer == 3) {
				if (last_mcal_op == three) {
					R3I_counter++;
					if (R3I_counter >= max_consecutive_s0_incr) return pm_r3i;
				} else R3I_counter = 0;
			}








*/









/*
			if (pointer == 1) {
				if (last_mcal_op == three) {
					R1I_counter++;
					if (R1I_counter >= max_consecutive_s0_incr) return pm_r1i;
				} else R1I_counter = 0;
			}

			if (pointer == 2) {
				if (last_mcal_op == three) {
					R2I_counter++;
					if (R2I_counter >= max_consecutive_s0_incr) return pm_r2i;
				} else R2I_counter = 0;
			}

			if (pointer == 3) {
				if (last_mcal_op == three) {
					R3I_counter++;
					if (R3I_counter >= max_consecutive_s0_incr) return pm_r3i;
				} else R3I_counter = 0;
			}
*/











/*if (graph[4 * index] == six and l != g) { //  
			counts[pm_ga_6g]++;
			// puts(pm_spelling[pm_ga_6g]);
			at = 4 * index + 2 * (index == six); goto bad;
		}*/






/*




static nat execute_graph_starting_at(byte origin, byte* graph, nat* array, byte* zskip_at) {

	const nat n = array_size;
	array[0] = 0; 
	array[n] = 0;

	nat 	xw = 0,  pointer = 0,  
		bout_length = 0, 
		RMV_value = 0, 
		IMV_value = 0,
		OER_er_at = 0,
		ERW_counter = 0,
		walk_ia_counter = 0;

	byte	H0_counter = 0,  H1_counter = 0, 
		OER_counter = 0, RMV_counter = 0, 
		IMV_counter = 0, CSM_counter = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;
	nat did_ier_at = (nat)~0;

#define max_rsi_count 20
	byte rsi_counter[max_rsi_count] = {0};


	for (nat e = 0; e < execution_limit; e++) {

		if (e and not (e & ((1 << 28) - 1))) {
			printf("taking a significant amount of time on e = %llu, z = ", e); 
			print_graph_raw(graph); puts(""); 
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

			if (walk_ia_counter < (e < 1000000 ? 3 : 6)) {
				ERW_counter++;
				if (ERW_counter >= max_erw_count) return pm_erw;
			} else ERW_counter = 0;

			walk_ia_counter = 0;
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

			walk_ia_counter++;
			bout_length = 0;
			array[pointer]++;
		}
		if (op == three or op == one or op == five) last_mcal_op = op;
		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		// if (*zskip_at > I + state) *zskip_at = I + state;
		ip = graph[I + state];
	}
	return z_is_good;
}






*/







		/*

		{const byte pairs[3 * 3] = {
			three, three, pm_ga_ndi,
			five, one, pm_ga_pco,
			five, five, pm_ga_zr5,
		};

		for (nat i = 0; i < 3 * 3; i += 3) {
			const byte source = pairs[i + 1], destination = pairs[i + 0];
			if (graph[4 * index] == source) {
				for (byte offset = 1; offset < 4; offset++) {
					const byte middle = graph[4 * index + offset];
					if (graph[4 * middle] != two and graph[4 * middle] != six) continue;
					for (byte offset2 = 1; offset2 < 4; offset2++) {
						const byte dest = graph[4 * middle + offset2];
						if (graph[4 * dest] != destination) continue;
						at = graph_count;
						if (4 * index < 20 and at > 4 * index + offset) at = 4 * index + offset;
						if (4 * middle < 20 and at > 4 * middle + offset2) at = 4 * middle + offset2;
						if (4 * index >= 20 and at > 4 * index) at = 4 * index;
						if (4 * middle >= 20 and at > 4 * middle) at = 4 * middle;
						if (4 * dest >= 20 and at > 4 * dest) at = 4 * dest;
						counts[pairs[i + 2]]++;
						// puts(pm_spelling[pairs[i + 2]]);
						goto bad;
					}
				}
			} 
		} }
		*/












	//const byte debug = not (display_counter & ((1 << display_rate) - 1));
	//if (debug) display_counter = 1; else display_counter++;
	//if (debug) {
	//	printf("GA trying: z = "); print_graph_raw(graph); puts(""); fflush(stdout);
	//	display_counter = 1;
	//}



/*

0144153521313544424200200000
0000153521313544424200200000




0144153521313544424200200000
0044153521313544424200200006
0104153521313544424200200000
0104153521313544424200200006








0144 1535 2131 3544424200200000


*/




// D-general version of the 0 space search util  
// written on 202410163.164303 dwrr
// size of raw 0 space is: (5^15) 	                 		=              30,517,578,125
// size of raw 1 space is: (6^15) * (5 * (6 ^ 3)) 	                =         507,799,783,342,080
// size of raw 2 space is: (7^15) * (5 * (7 ^ 3)) * (5 * (7 ^ 3)) 	=  13,963,646,602,082,100,175

// rewritten kinda  on 202411144.202807 dwrr


