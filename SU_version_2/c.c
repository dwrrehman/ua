// SU version 2, using possibility nf-array states (PAS) 
// system for searching, instead of z-values
// written on 1202512151.174454 by dwrr

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <iso646.h>

enum operations { one, two, three, five, six };
typedef uint8_t byte;
typedef uint16_t u16;
typedef uint64_t nat;

#define D 1





#define dol   ((one << 0) | (two << 4) | (five << 8))



/* const byte pas_pa = pa_map[pa] */


const byte value = ((decode[pas_pa] >> (nfarray[pas_pa] << 2)) & 0xf;



pas_pa = 0  MEANS    opi0.g
pas_pa = 1  MEANS    opi0.e
pas_pa = 2  MEANS    opi1.g
pas_pa = 3  MEANS    opi1.e
pas_pa = 4  MEANS    opi2.l

...

pas_pa = 22   MEANS   addr8.opi4.e




/*   DOL = {  1, 2, 5, 6,  }    */



static const nat decode[] = {


0123456789ABCDEFGHIJKLM


--------------------
	PA_MAP:
--------------------
ad | opi  <   >   =
--------------------
 0 | 0 |  /   0   1 
--------------------
 1 | 1 |  /   2   3 
--------------------
 2 | 2 |  4   5   6
--------------------
 3 | 3 |  7   8   9
--------------------
 4 | 4 |  A   /   B
--------------------
 5 | 0 |  C   D   E
--------------------
 6 | 1 |  F   G   H
--------------------
 7 | 3 |  I   J   K
--------------------
 8 | 4 |  L   /   M
--------------------

23 digits, aka  23 nat's   each nat encodes up to 16 possible address



opi0:
	pas[pa=0] = 0    :  MEANS     address 0  --(g)-->  address 2   





                                                   
/* paspa 0 */	{   

	0 out [ns0],   
	1 out [sci],   
	2 in,   
	3 out [pco],  
	4 in,   
	5 out  [ns0],   
	6 





/* paspa 1 */	{3, 5, 6, 7, 8}
/* paspa 2 */	{3, 5, 6, 7, 8}
/* paspa 3 */	{3, 5, 6, 7, 8}
/* paspa 4 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}
/* paspa 2 */	{}


};





#define machine_count 1
#define machine_index 0
#define thread_count 10

#define machine0_throughput 1
#define machine1_throughput 1

#define execution_limit 100000000LLU
#define array_size 1000000LLU
#define display_rate 2

#define operation_count (5 + D)
#define graph_count (operation_count * 4)

static u16 queue[4096] = {0};
static _Atomic nat queue_count = 0;
static _Atomic nat progress[thread_count * 2] = {0};
static char** filenames = NULL;


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


	pm_ga_ns0,  pm_ga_pco, 
	pm_ga_rdo,  pm_ga_uo, 
	pm_ga_5u1,  pm_ga_6u2,
	pm_ga_3u5,  pm_ga_3u1,
	pm_ga_sndi, pm_ga_h,
	pm_ga_6e,

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

	"pm_ga_sdol", 

	"pm_ga_6g",    "pm_ga_ns0", 
	"pm_ga_zr5",   "pm_ga_pco", 
	"pm_ga_ndi",   "pm_ga_snco",  
	"pm_ga_zr6",   "pm_ga_rdo", 

	"pm_ga_uo",    "pm_ga_il",

	"pm_ga_5u1",   "pm_ga_6u2",
	"pm_ga_3u5",   "pm_ga_3u1",

	"pm_ga_sndi",  "pm_ga_h",
	
	"pm_ga_sci",   "pm_ga_6e",

	"pm_ga_lb",
};

static void print_binary(nat x) {
	for (nat i = 0; i < 16; i++) {
		if (not (i & 3)) putchar('_');
		printf("%llu", (x >> i) & 1);
	}
}

__attribute__((always_inline))
static byte gi(nat graph0, nat graph1, byte pa) {
	return (
		(pa < 16 ? graph0 : graph1) 
		>> 
		((pa & 15) << 2)
	) & 0xf;
}


static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void append_to_file(
	char* filename, size_t filename_size, 
	nat g0, nat g1, byte origin
) {
	char dt[32] = {0};   get_datetime(dt);
	char z[64] = {0};    get_graphs_z_value(z, g0, g1);
	char o[16] = {0};    snprintf(o, sizeof o, " %hhu ", origin);

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

	write(file, z, graph_count);
	write(file, o, 3);
	write(file, dt, 17);
	write(file, "\n", 1);
	close(file);

	printf("[%s]: write: %s z = %s to file \"%s\"\n",
		dt, permissions ? "created" : "wrote", z, filename
	);
}




































int main(void) {
	srand((unsigned) time(0));
	
	static char output_filename[4096] = {0};
	static char output_string[4096] = {0};
	
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	atomic_init(&queue_count, 0);
	for (nat i = 0; i < thread_count; i++) {
		atomic_init(progress + 2 * i + 0, 0);
		atomic_init(progress + 2 * i + 1, 0);
	}

	filenames = calloc(thread_count, sizeof(char*));
	for (nat i = 0; i < thread_count; i++) {
		filenames[i] = calloc(4096, 1);
		char dt[32] = {0};
		get_datetime(dt);
		snprintf(filenames[i], 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
	}








	nat total_job_count = 0;

{	byte mi = 0, pointer = 0;
	u16 g0 = 0;
	goto init;
loop:
	if (gi(g0, 0, pointer) < (pointer ? operation_count - 1 : 4)) goto increment;
	if (pointer < 4 - 1) goto reset_;
	goto done;
increment:
	g0 += 1LLU << ((pointer & 15LLU) << 2LLU);
init:	pointer = 0;

	const byte op = gi(g0, 0, 0);
	const byte l  = gi(g0, 0, 1);
	const byte g  = gi(g0, 0, 2);
	const byte e  = gi(g0, 0, 3);

	if (g == two) goto loop;

	if (op == one and l == 7) goto loop;
	if (op == one and g == 7) goto loop;
	if (op == one and e == 7) goto loop;
	if (op == two and l == 7) goto loop;
	if (op == two and g == 7) goto loop;
	if (op == three and l == 7) goto loop;
	if (op == three and g == 7) goto loop;
	if (op == three and e == 7) goto loop;
	if (op == five and l == 7) goto loop;
	if (op == five and g == 7) goto loop;
	if (op == five and e == 7) goto loop;
	if (op == six and l == 7) goto loop;
	if (op == six and e == 7) goto loop;
	if (op == six and e == one) goto loop;
	if (op == six and e == five) goto loop;
	if (op == one and l == one) goto loop;
	if (op == one and g == one) goto loop;
	if (op == one and e == one) goto loop;
	if (op == one and l == five) goto loop;
	if (op == one and g == five) goto loop;
	if (op == one and e == five) goto loop;
	if (op == two and l == six) goto loop;
	if (op == two and g == six) goto loop;
	if (op == two and e == six) goto loop;
	if (op == three and l == three) goto loop;
	if (op == three and g == three) goto loop;
	if (op == three and e == three) goto loop;
	if (op == five and l == five) goto loop;
	if (op == five and g == five) goto loop;
	if (op == five and e == five) goto loop;
	if (op == six and l == six) goto loop;
	if (op == six and g != six) goto loop;
	if (op == six and e == six) goto loop;
	

	mi = (mi + 1) % machine_count;
	if (mi != machine_index) goto loop;
	const nat n = atomic_fetch_add_explicit(&queue_count, 1, memory_order_relaxed);
	queue[n] = g0;
	total_job_count++;

	goto loop;
reset_:
	g0 &= ~(0xfLLU << ((pointer & 15LLU) << 2LLU));
	pointer++;
	goto loop;

done:; }
	printf("printing jobs: (%llu total jobs)\n", total_job_count);
	for (nat i = 0; i < total_job_count; i++) {
		if (not (i % 2)) putchar(' ');
		if (not (i % 4)) putchar(' ');
		if (not (i % 8)) putchar(10);
		printf("%04hx ", queue[i]);
	}
	puts("");
	getchar();

	snprintf(output_string, 4096, "SU: searching [D=%u] space....\n", D);
	print(output_filename, 4096, output_string);

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);

	for (nat i = 0; i < thread_count; i++) {
		nat* arg = malloc(sizeof(nat));
		*arg = i;
		pthread_create(threads + i, NULL, worker_thread, arg);
	}

	nat counts[pm_count] = {0};
	const bool disable_main = 0;
	while (1) {

		const nat amount_remaining = atomic_load_explicit(&queue_count, memory_order_relaxed);
		if ((int64_t) amount_remaining <= 0 or disable_main) goto terminate;

		printf("\033[H\033[2J");
		printf("----------------- jobs remaining %llu / %llu -------------------\n", 
			amount_remaining, total_job_count
		);
		printf("\n\t complete %1.10lf%%\n\n", (double) (total_job_count - amount_remaining) / (double) total_job_count);
		for (nat i = 0; i < thread_count; i++) {
			const nat g0 = atomic_load_explicit(progress + 2 * i + 0, memory_order_relaxed);
			const nat g1 = atomic_load_explicit(progress + 2 * i + 1, memory_order_relaxed);
			printf(" %5llu : ", i);
			print_graph_raw(g0, g1); puts("");
		}
		puts("");
		sleep(1 << display_rate);
	}

terminate:
	puts("\nmain: joining threads...\n");
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
		"\t thread_count = %u\n"
		"\t machine_index = %u\n"
		"\t machine0_throughput = %u\n"
		"\t machine1_throughput = %u\n"
		"\t display_rate = %u\n"
		"\t execution_limit = %llu\n"
                "\t array_size = %llu\n"
		"\t in %10.2lfs [%s:%s]\n"
		"\n",
		D, thread_count,
		machine_index,
		machine0_throughput,
		machine1_throughput,
		display_rate,
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

} // main














