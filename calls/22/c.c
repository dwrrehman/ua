// major revision to the seach utility, 
// written on 1202602275.170743 by dwrr
// made to be faster for the UO execution path, 
// and also use the PAS system in some ways, including LTRDO
// but still use a zv for the central SU DS. 

// code for testing the network logic, in 
// the SU version 4, which uses the networked 
// peer-to-peer job distribution system!
// 1202606173.125913 by dwrr
// pasted from network logic test on 1202607061.185658


/*  ---------------- new implementation of MP, which we will add in, 
	once we get the SU_version_4 done!)  ----------------------

	if (e == 1000000) {
		if (xw == 0) abort();
		for (nat i = 0; i < xw - 1; i++) {
			if (array[i] < array[i + 1] and array[i + 1] - array[i] > 30) return pm_mp;
		}
	}

*/

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <iso646.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <time.h>
#include <unistd.h>
#include <signal.h>
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
typedef uint8_t byte;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t nat;

#define D 2

#define machine_count 10
#define thread_count 10
#define job_digit_count 6

#define execution_limit 100000000LLU
#define array_size 1000000LLU
#define pg0  0x0003000200010010
#define pg1  0x0000000000000404
#define pg2  0x0
#define required_difference  		2
#define redistribution_delay 		1
#define maximum_number_of_total_jobs 	10000000
#define server_port 		32768
#define max_transfer_amount     64

static const char* ip_addresses[10] = {
	"fe80::1c3f:ebba:990e:2e30%bridge0", 	// 0
	"fe80::cb4:c1b3:d25e:9241%bridge0",  	// 1
	"fe80::dd:4db5:440c:6274%bridge0",  	// 2
	"fe80::cbb:f438:fde:6242%bridge0",  	// 3
	"fe80::d8:eb7d:813e:7f5d%bridge0",  	// 4
	"fe80::449:29c3:416a:6061%bridge0",  	// 5
	"fe80::b9:c498:c86a:1d57%bridge0",  	// 6
	"fe80::186a:9d50:d08d:a5f5%bridge0",  	// 7
	"fe80::1077:f45b:174b:83e4%bridge0",  	// 8
	"fe80::4e7:9372:195a:8ad6%bridge0",  	// 9
};

static nat jobs[maximum_number_of_total_jobs] = {0};
static nat master_list_g0[maximum_number_of_total_jobs] = {0};
static nat master_list_g1[maximum_number_of_total_jobs] = {0};
static nat master_list_g2[maximum_number_of_total_jobs] = {0};
static nat total_job_count = 0;

static char** filenames = NULL;
static int server_socket = 0;
static nat machine_index = (nat) -1;
static char output_filename[16384] = {0};
static char output_string[16384] = {0};
static char pm_string[16384] = {0};

static _Atomic nat running = 0;
static _Atomic nat job_count = 0;
static _Atomic nat global_min_index = 0;
static _Atomic nat global_max_index = 0;
static _Atomic nat global_job_counts[machine_count] = {0};
static _Atomic nat progress[thread_count * 3] = {0};

enum operations { one, two, three, five, six };
#define operation_count (5 + D)
#define graph_count (operation_count * 4)
#define packet_size_in_bytes    (8 * (max_transfer_amount + 5))
#define packet_size_in_nats    (max_transfer_amount + 5)

enum pruning_metrics {
	z_is_good,

	pm_zr5, pm_zr6, pm_ndi, pm_sndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi, pm_mcal,
	pm_h0, pm_h0s, pm_h1, pm_h2, 
	pm_rmv, pm_csm, pm_pair,
	pm_bdl1, pm_bdl2, pm_bdl3, pm_xw,

	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good",

	"pm_zr5", "pm_zr6", "pm_ndi", "pm_sndi",
	"pm_pco", "pm_per", "pm_ns0",
	"pm_oer", "pm_rsi", "pm_mcal",
	"pm_h0", "pm_h0s", "pm_h1", "pm_h2", 
	"pm_rmv", "pm_csm", "pm_pair",
	"pm_bdl1", "pm_bdl2", "pm_bdl3", "pm_xw",
};

static const byte loops[4 * 15] = {
	five, 3, two, 2,
	three, 3, two, 2,
	two, 3, three, 1,
	six, 3, three, 1,
	one, 1, three, 1,
	five, 3, three, 1,
	three, 1, five, 1, 
	three, 1, six, 1,
	three, 2, six, 1,
	three, 3, six, 1,
	five, 1, six, 1,
	five, 2, six, 1,
	five, 3, six, 1,
	one, 1, six, 1,
	three, 1, two, 1, 
};

#define g(x)   (gi(g0, g1, g2, x))

#define lsepa 2
__attribute__((always_inline)) static bool editable(nat p) {
	if (p < 20 and not (p & 3)) return false;
	if (p == 1 or p == 5 or p == 18) return false;	
	return true;
}

__attribute__((always_inline))
static byte gi(nat g0, nat g1, nat g2,  byte pa) {
	return ((pa < 16 ? g0 : pa < 32 ? g1 : g2) >> ((pa & 15) << 2)) & 0xf;
}

__attribute__((always_inline))
static u16 gi16(nat g0, nat g1, nat g2,  byte pa) {
	return ((pa < 16 ? g0 : pa < 32 ? g1 : g2) >> ((pa & 15) << 2)) & 0xffff;
}

static void print_graph_raw(nat g0, nat g1, nat g2) { 
	for (byte i = 0; i < graph_count; i++) 
		printf("%hhu", gi(g0, g1, g2, i)); 
}

static void get_graphs_z_value(char* string, nat g0, nat g1, nat g2) {
	for (byte i = 0; i < graph_count; i++) 
		string[i] = (char) (gi(g0, g1, g2, i) + '0');
	string[graph_count] = 0;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static nat pull_job_if_available(void) {
	nat local = atomic_load(&job_count);
	retry_loop:; if (not local) return (nat) -1;
	const nat d = local - 1;
	bool b = atomic_compare_exchange_weak(&job_count, &local, d);
	if (not b) goto retry_loop;
	return jobs[d];
}

static void publish(void) {
	char dt[32] = {0};   get_datetime(dt);
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;
try_open:;
	const int file = open(output_filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("print: [%s]: failed to create output_filename = \"%s\"\n", dt, output_filename);
			fflush(stdout);
			abort();
		}
		snprintf(output_filename, sizeof output_filename, "%s_D%u_M%llu_%08x%08x%08x%08x_output.txt", dt, D, machine_index,
			rand(), rand(), rand(), rand()
		);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		goto try_open;
	}
	dt[17] = ' '; dt[18] = ':';
	dt[19] = ':'; dt[20] = ' ';
	write(file, dt, 21);
	write(file, output_string, strlen(output_string));
	close(file);
}

static void publish_error(const char* message) {
	char buffer[4096] = {0};
	snprintf(buffer, sizeof buffer, "%d: %s", errno, strerror(errno));
	snprintf(output_string, sizeof output_string, 
		"[MI%llu]: [%s:%llu]: [PID%d]: %s: %s\n",
		machine_index, 
		ip_addresses[machine_index],
		server_port + machine_index, 
		getpid(), message, buffer
	); 
	publish();
}

static char* get_command_output(const char* input_command) {
	char command[4096] = {0};
	snprintf(command, sizeof command, "%s 2>&1", input_command);
	FILE* f = popen(command, "r");
	if (not f) { publish_error("popen"); abort(); } 
	char* string = NULL;
	size_t length = 0;
	char line[2048] = {0};
	while (fgets(line, sizeof line, f)) {
		size_t l = strlen(line);
		string = realloc(string, length + l);
		memcpy(string + length, line, l);
		length += l;
	}
	pclose(f);
	return strndup(string, length);
}

static nat translate_hostname_to_machine_index(char* s) {
	if (not strcmp(s, "dwrr-mini0.local\n")) return 0;
	if (not strcmp(s, "dwrr-mini1.local\n")) return 1;
	if (not strcmp(s, "dwrr-mini2.local\n")) return 2;
	if (not strcmp(s, "dwrr-mini3.local\n")) return 3;
	if (not strcmp(s, "dwrr-mini4.local\n")) return 4;
	if (not strcmp(s, "dwrr-mini5.local\n")) return 5;
	if (not strcmp(s, "dwrr-mini6.local\n")) return 6;
	if (not strcmp(s, "dwrr-mini7.local\n")) return 7;
	if (not strcmp(s, "dwrr-mini8.local\n")) return 8;
	if (not strcmp(s, "dwrr-mini9.local\n")) return 9;

	if (not strcmp(s, "dwrr.local\n")) return 1;

	puts(s); 
	publish_error("translate_hostname_to_machine_index"); 
	abort(); 
}

static void signal_handler(int sig) {
	char progress_string[4096] = {0};
	int progress_string_length = 0;
	for (nat thread = 0; thread < thread_count; thread++) {
		const nat g0 = atomic_load_explicit(progress + 3 * thread + 0, memory_order_relaxed);
		const nat g1 = atomic_load_explicit(progress + 3 * thread + 1, memory_order_relaxed);
		const nat g2 = atomic_load_explicit(progress + 3 * thread + 2, memory_order_relaxed);
		progress_string_length += snprintf(progress_string + progress_string_length, sizeof progress_string, 
			" %3llu : ", thread
		); 
		get_graphs_z_value(progress_string + progress_string_length, g0, g1, g2);
		progress_string_length += graph_count;
		progress_string[progress_string_length++] = 10; 
		progress_string[progress_string_length] = 0;
	}
	char job_count_string[4096] = {0};
	int job_count_string_length = 0;
	nat total_amount_of_jobs_remaining = 0;
	for (nat machine = 0; machine < machine_count; machine++) {
		const nat value = atomic_load_explicit(global_job_counts + machine, memory_order_relaxed);
		job_count_string_length += snprintf(job_count_string + job_count_string_length, 
			sizeof job_count_string, " %3llu : %llu\n", machine, value
		); 
		total_amount_of_jobs_remaining += value;
	}	
	snprintf(output_string, sizeof output_string, 
		"------------ [MACHINE %llu] on signal %d: file %s------------\n"
		"thread_count = %u, machine_count = %u\n"
		"job_digit_count = %u, total_job_count = %llu\n"
		"graph_count = %u, operation_count = %u\n"
		"array_size = %llu, execution_limit = %llu\n"
		"redistribution_delay = %u, max_transfer_amount = %u\n"
		"required_difference = %u, UNUSED = %u\n"
		"our ip address = %s, port = %llu\n"
		"\n"
		"total job count remaining : jobs %llu / %llu : %1.3lf%% remaining\n"
		"%s\n"
		"%s\n"
		"SU: [PID = %d]: searching [D = %u] space...\n"
	,
		machine_index, sig, output_filename, 
		thread_count, machine_count, 
		job_digit_count, total_job_count, 
		graph_count, operation_count, 
		array_size, execution_limit,
		redistribution_delay, max_transfer_amount, 
		required_difference, 0,
		ip_addresses[machine_index], server_port + machine_index, 
		total_amount_of_jobs_remaining, total_job_count, 
		100.0 * ((double) total_amount_of_jobs_remaining / (double) total_job_count),
		job_count_string, 
		progress_string,
		getpid(), D
	); 
	publish();
}

static void append_to_file(
	char* filename, size_t filename_size, 
	nat g0, nat g1, nat g2, 
	byte origin, nat cdm
) {
	char dt[32] = {0};   get_datetime(dt);
	char z[128] = {0};   get_graphs_z_value(z, g0, g1, g2);
	char o[128] = {0};   snprintf(o, sizeof o, " %hhu %llu ", origin, cdm);
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
	write(file, o, strlen(o));
	write(file, dt, 17);
	write(file, "\n", 1);
	close(file);
}

#define max_rsi_count 512
#define max_oer_repetions 50
#define max_rmv_modnat_repetions 15
#define max_consecutive_small_modnats 230
#define max_consecutive_s0_incr 30
#define max_consecutive_h0_bouts 10
#define max_consecutive_h1_bouts 16
#define max_consecutive_h2_bouts 24
#define max_consecutive_h0s_bouts 7
#define max_consecutive_pairs 8

static nat execute_graph_starting_at(
	nat g0, nat g1, nat g2, 
	const byte origin, 
	nat* array, 
	nat* output_cdm
) {
	nat comparator = 0;
	const nat N = array_size;
	array[0] = 0; 
	nat 	xw = 0,  pointer = 0,  bout_length = 0, 
		OER_ier_at = 0,
		BDL_ier_at = 0,
		PER_ier_at = (nat) ~0;

	byte	H0_counter = 0,  H0S_counter = 0, SNDI_counter = 0,
		H1_counter = 0, H2_counter = 0, OER_counter = 0,
		BDL1_counter = 0, BDL2_counter = 0, BDL3_counter = 0, 
		pair_index = 0, pair_count = 0;	
	byte ip = origin;
	byte last_mcal_op = 255;
	byte rsi_counter[max_rsi_count];   rsi_counter[0] = 0;
	byte has_executed_5 = 0;
	byte has_executed_6 = 0;
	nat cdm = 0;

	for (nat e = 0; e < execution_limit; e++) {

		if (e == 100000 and xw >= 150) return pm_xw;
		if (e == 20000 and xw < 8) return pm_xw;

		const byte op = g(4 * ip);
		
		if (op == one) {
			if (pointer == N) {
				puts("FEA condition violated by a z value: "); 
				print_graph_raw(g0, g1, g2);
				puts(""); 
				publish_error("execute_graph_starting_at : FEA condition violated by a z value"); 
				abort(); 
			}
  
			if (not array[pointer]) return pm_ns0; 
			if (last_mcal_op == one)  H0_counter = 0;
			if (last_mcal_op == one)  H0S_counter = 0;

			if (pointer < max_rsi_count) {
				if (last_mcal_op == three) {
					rsi_counter[pointer]++;
					if (rsi_counter[pointer] >= max_consecutive_s0_incr) return pm_rsi;
				} else rsi_counter[pointer] = 0;
			}

			if (pair_index == 1) pair_index = 2;
			else if (pair_index == 3) pair_index = 4;
			else if (pair_index == 4) { pair_index = 0; pair_count++; if (pair_count >= max_consecutive_pairs) return pm_pair; } 
			else if (pair_index) { pair_count = 0; pair_index = 0; }

			SNDI_counter = 0;
			bout_length++;
			pointer++;

			if (pointer > xw and pointer < N) { 
				xw = pointer; 
				array[pointer] = 0; 
				if (pointer < max_rsi_count) rsi_counter[pointer] = 0;
			}
		}

		else if (op == five) {
			if (not has_executed_5) { if (pointer > 1) return pm_mcal; }
			has_executed_5 = 1;
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (pointer == OER_ier_at or pointer == OER_ier_at + 1) {
				OER_counter++;
				if (OER_counter >= max_oer_repetions) return pm_oer;
			} else { OER_ier_at = pointer; OER_counter = 0; }

			byte CSM_counter = 0;
			byte RMV_counter = 0;
			nat RMV_value = (nat) -1;

			for (nat i = 0; i < xw; i++) {
				if (array[i] < 8) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter >= max_consecutive_small_modnats) return pm_csm;
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_rmv_modnat_repetions) return pm_rmv;

				if (i and array[i] > array[i - 1]) {
					const nat k = array[i] - array[i - 1];
					cdm += k;
				}
			}

			if (pointer + 1 == BDL_ier_at) {
				BDL1_counter++; 
				if (BDL1_counter >= 8) return pm_bdl1; 
			} else BDL1_counter = 0;

			if (pointer + 2 == BDL_ier_at) {
				BDL2_counter++; 
				if (BDL2_counter >= 8) return pm_bdl2; 
			} else BDL2_counter = 0;

			if (pointer + 3 == BDL_ier_at) {
				BDL3_counter++;
				if (BDL3_counter >= 30) return pm_bdl3; 
			} else BDL3_counter = 0;

			if (pair_index == 3) { pair_index = 0; pair_count++; if (pair_count >= max_consecutive_pairs) return pm_pair; } 
			else if (pair_index) { pair_count = 0; pair_index = 0; }

			SNDI_counter = 0;
			BDL_ier_at = pointer;
			PER_ier_at = pointer;
			pointer = 0;
		}
		else if (op == two) {
			SNDI_counter++;
			if (SNDI_counter >= 10) return pm_sndi;
			comparator++;
		}
		else if (op == six) {
			if (not has_executed_6) { if (comparator > 1) return pm_mcal; }
			has_executed_6 = 1;
			if (not comparator) return pm_zr6;
			SNDI_counter = 0;
			comparator = 0;
		}
		else if (op == three) {
			if (last_mcal_op == three) return pm_ndi;

			if (last_mcal_op == one) {
				H0_counter++;
				if (H0_counter >= max_consecutive_h0_bouts) return pm_h0; 
			}

			if (last_mcal_op == one) {
				H0S_counter++;
				if (H0S_counter >= max_consecutive_h0s_bouts and e >= 100000) return pm_h0s; 
			}

			if (bout_length == 2) {
				H1_counter++;
				if (H1_counter >= max_consecutive_h1_bouts) return pm_h1; 
			} else H1_counter = 0;

			if (bout_length == 3) {
				H2_counter++;
				if (H2_counter >= max_consecutive_h2_bouts) return pm_h2; 
			} else H2_counter = 0;

			if (PER_ier_at != (nat) ~0) {
				if (pointer >= PER_ier_at) return pm_per; 
				PER_ier_at = (nat) ~0;
			}

			if (not pair_index) pair_index = 1;
			else if (pair_index == 2) pair_index = 3;
			else { pair_count = 0; pair_index = 0; }

			SNDI_counter = 0;
			bout_length = 0;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;
		byte state = 0;
		if (comparator < array[pointer]) state = 1;
		if (comparator > array[pointer]) state = 2;
		if (comparator == array[pointer]) state = 3;
		ip = g(4 * ip + state);
	}
	if (execution_limit == 100000000 and xw >= 1000) return pm_xw;
	*output_cdm = cdm;
	return z_is_good;
}
		
static byte execute_graph(
	nat g0, nat g1, nat g2, 
	byte* origin, 
	nat* array, 
	nat* counts, 
	const nat thread_index,
	nat* output_cdm
) {
	for (byte o = 0; o < operation_count; o++) {
		byte op = g(4 * o);
		if (op != three) continue;
		nat local_cdm = (nat) -1;
		const nat pm = execute_graph_starting_at(g0, g1, g2, o, array, &local_cdm);
		atomic_store_explicit(progress + 3 * thread_index + 0, g0, memory_order_relaxed);
		atomic_store_explicit(progress + 3 * thread_index + 1, g1, memory_order_relaxed);
		atomic_store_explicit(progress + 3 * thread_index + 2, g2, memory_order_relaxed);
		counts[pm]++;
		if (not pm) { *origin = o; *output_cdm = local_cdm; return false; }
		continue;
	}
	return true;
}

static void* server_thread_function(void* unused) {
	server_socket = socket(AF_INET6, SOCK_STREAM, 0);
	if (server_socket < 0) { publish_error("socket"); abort(); }
	int opt = 1;
	int r = setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof opt);
	if (r) { publish_error("setsockopt(SO_REUSEPORT)"); abort(); }
	struct sockaddr_in6 client_address = {0};
	client_address.sin6_family = AF_INET6;
	client_address.sin6_addr = in6addr_any;
	client_address.sin6_port = htons((int) (server_port + machine_index));
	client_address.sin6_flowinfo = 0;
	client_address.sin6_scope_id = 0;
	r = bind(server_socket, (struct sockaddr *) &client_address, sizeof client_address);
	if (r < 0) { publish_error("bind"); abort(); }
	r = listen(server_socket, machine_count);
	if (r < 0) { publish_error("listen"); exit(1); }
	struct sockaddr_in6 client = client_address;
	int length = sizeof client_address;
	printf("[server thread running on port %llu]\n", server_port + machine_index);
try_accepting_clients:; 
	const nat is_running = atomic_load_explicit(&running, memory_order_relaxed);
	if (not is_running) goto done;
	int connection = accept(server_socket, (struct sockaddr *) &client, (socklen_t*) &length);
	if (connection < 0) { publish_error("accept"); goto done; } 
	char ip[INET6_ADDRSTRLEN] = {0};
	inet_ntop(AF_INET6, &client.sin6_addr, ip, sizeof ip);
	//int port = ntohs(client.sin6_port);
	//printf("[connected to [%s:%d]\n", ip, port);
	nat data[packet_size_in_nats] = {0};
	ssize_t n = read(connection, data, packet_size_in_bytes);
	if (n <= 0 and errno == EPIPE) goto done;
	else if (n <= 0) { publish_error("read"); goto close_connection; } 
	if (n != packet_size_in_bytes) {
		publish_error("n != packet_size_in_bytes : read did not receive all bytes from packet."); 
		goto close_connection; 
	}
	atomic_store_explicit(global_job_counts + data[0], data[1], memory_order_relaxed);
	const nat amount = data[2];
	if (not amount) goto close_connection;
	const nat min_index = atomic_load_explicit(&global_min_index, memory_order_relaxed);
	const nat max_index = atomic_load_explicit(&global_max_index, memory_order_relaxed);
	if (data[3] != min_index or data[4] != max_index) {
		byte ack = 0;
		n = write(connection, &ack, 1);
		if (n <= 0 and errno == EPIPE) goto done;
		else if (n <= 0) { publish_error("write"); goto close_connection; }
	} else {
		for (nat j = 0; j < amount; j++) {
			const nat k = atomic_fetch_add_explicit(&job_count, 1, memory_order_relaxed);
			jobs[k] = data[5 + j];
		}
		byte ack = 1;
		n = write(connection, &ack, 1);
		if (n <= 0 and errno == EPIPE) goto done;
		else if (n <= 0) { publish_error("write"); goto close_connection; }
		puts("ACCEPTED JOB, SENT ACK");
	}
	close_connection: close(connection); goto try_accepting_clients;
	done: return unused;
}

static void* worker_thread(void* raw_thread_index) {

	pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

	const nat thread_index = *(nat*) raw_thread_index;

	nat* counts = calloc(pm_count, sizeof(nat));
	nat* array = calloc(array_size + 1, sizeof(nat));

	register nat g0 = 0;  
	register nat g1 = 0;
	register nat g2 = 0;

	register byte pointer = 0;

pull_job_from_queue:;
	const nat is_running = atomic_load_explicit(&running, memory_order_relaxed);
	const nat job = pull_job_if_available();
	if (job == (nat) -1) { if (not is_running) goto terminate; sleep(1); goto pull_job_from_queue; } 

	g0 = master_list_g0[job];
	g1 = master_list_g1[job];
	g2 = master_list_g2[job];
	goto init;

bad:	if (pointer + job_digit_count >= graph_count) goto pull_job_from_queue;
	for (byte i = 0; i < pointer; i++) {
		if (not editable(i)) continue;
		const nat mask = ~(0xfLLU << ((i & 15) << 2));
		     if (i < 16) g0 &= mask;
		else if (i < 32) g1 &= mask;
		else             g2 &= mask;
	}
loop: 	if (pointer + job_digit_count >= graph_count) goto pull_job_from_queue;
	if (g(pointer) < ((pointer & 3) ? operation_count - 1 : 4)) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto pull_job_from_queue;
reset_:; const nat mask = ~(0xfLLU << ((pointer & 15) << 2));
	     if (pointer < 16) g0 &= mask;
	else if (pointer < 32) g1 &= mask;
	else                   g2 &= mask;
	do pointer++; while (not editable(pointer));
	if (pointer + job_digit_count >= graph_count) goto pull_job_from_queue;
	goto loop;
increment:;
	const nat addend = 1LLU << ((pointer & 15) << 2);
	     if (pointer < 16) g0 += addend;
	else if (pointer < 32) g1 += addend;
	else                   g2 += addend;
init:   pointer = lsepa;
	for (byte i = D; i--;) {
		const byte pa = 4 * (5 + i);
		const byte op = g(pa);
		byte side = 0;	
		if (op == six and g(4 * g(pa + 3)) == one) { side = 3; goto prune_edge; } // ns0.6e1
		if (op == six and g(4 * g(pa + 3)) == five) { side = 3; goto prune_edge; } // ns0.6e5
		if (op == six and g(4 * g(pa + 3)) == six) { side = 3; goto prune_edge; }  // zr6.e
		if (op == six and g(pa + 2) != six)        { side = 2; goto prune_edge; }  // 6g
		if (op == six and g(4 * g(pa + 1)) == six) { side = 1; goto prune_edge; }  // zr6.l 
		if (op == five and g(4 * g(pa + 3)) == five) { side = 3; goto prune_edge; } // zr5.e
		if (op == five and g(4 * g(pa + 2)) == five) { side = 2; goto prune_edge; } // zr5.g
		if (op == five and g(4 * g(pa + 1)) == five) { side = 1; goto prune_edge; } // zr5.l
		if (op == three and g(4 * g(pa + 3)) == three) { side = 3; goto prune_edge; } // ndi.e
		if (op == three and g(4 * g(pa + 2)) == three) { side = 2; goto prune_edge; } // ndi.g
		if (op == three and g(4 * g(pa + 1)) == three) { side = 1; goto prune_edge; } // ndi.l
		if (op == two and g(4 * g(pa + 3)) == six) { side = 3; goto prune_edge; } // snco.e
		if (op == two and g(4 * g(pa + 2)) == six) { side = 2; goto prune_edge; } // snco.g
		if (op == two and g(4 * g(pa + 1)) == six) { side = 1; goto prune_edge; } // snco.l
		if (g(4 * g(pa + 2)) == two) { side = 2; goto prune_edge; } // sci
		if (op == one and g(4 * g(pa + 3)) == five) { side = 3; goto prune_edge; } // pco.e
		if (op == one and g(4 * g(pa + 2)) == five) { side = 2; goto prune_edge; } // pco.g
		if (op == one and g(4 * g(pa + 1)) == five) { side = 1; goto prune_edge; } // pco.l
		if (op == one and g(4 * g(pa + 3)) == one) { side = 3; goto prune_edge; } // ns0.1e1
		if (op == one and g(4 * g(pa + 2)) == one) { side = 2; goto prune_edge; } // ns0.1g1 
		if (op == two and g(pa + 1) == pa >> 2) { side = 1; goto prune_edge; } // sndi
		if (op == one and g(pa + 1) == pa >> 2) { side = 1; goto prune_edge; } // lb
		
		for (byte opi = 5; opi--;) {
			if ((opi != one and opi != two) or g(4 * opi + 1) == g(pa + 1)) {
				if (op == opi and gi16(g0,g1,g2, 4 * opi) >= gi16(g0,g1,g2, pa)) { 
					pointer = 4 * (opi + 1);
					while (not editable(pointer)) pointer++;
				}
			}
		}

		for (byte j = 0; j < i; j++) {
			if (op == g(4 * (5 + j)) and gi16(g0,g1,g2, 4 * (5 + j)) >= gi16(g0,g1,g2, pa)) {
				pointer = 4 * (5 + j + 1);
				while (not editable(pointer)) pointer++;
			} 
		}

		continue; prune_edge:;
		const byte k = 4 * g(pa + side);
		pointer = graph_count;
		if (editable(pa + side) and pointer > pa + side) pointer = pa + side; 
		if (editable(pa) and pointer > pa) pointer = pa; 
		if (editable(k) and pointer > k) pointer = k;
		goto bad;
	}
	if (g(4 * g(4 * six   + 3)) == one)   { pointer = 4 * six + 3; goto bad; } 	// ns0.6e1
	if (g(4 * g(4 * six   + 3)) == five)  { pointer = 4 * six + 3; goto bad; } 	// ns0.6e5
	if (g(4 * g(4 * six   + 3)) == six)   { pointer = 4 * six + 3; goto bad; } 	// zr6.e
	if (g(4 * g(4 * six   + 1)) == six)   { pointer = 4 * six + 1; goto bad; }  	// zr6.l
	if (g(4 * g(4 * five  + 3)) == five)  { pointer = 4 * five + 3; goto bad; }	// zr5.e
	if (g(4 * g(4 * five  + 2)) == five)  { pointer = 4 * five + 2; goto bad; } 	// zr5.g
	if (g(4 * g(4 * five  + 2)) == two)   { pointer = 4 * five + 2; goto bad; }	// sci.5g
	if (g(4 * g(4 * five  + 1)) == five)  { pointer = 4 * five + 1; goto bad; }	// zr5.l
	if (g(4 * g(4 * three + 3)) == three) { pointer = 4 * three + 3; goto bad; }	// ndi.e
	if (g(4 * g(4 * three + 2)) == three) { pointer = 4 * three + 2; goto bad; }	// ndi.g
	if (g(4 * g(4 * three + 2)) == two)   { pointer = 4 * three + 2; goto bad; }	// sci.3g
	if (g(4 * g(4 * three + 1)) == three) { pointer = 4 * three + 1; goto bad; }	// ndi.l
	if (g(4 * g(4 * two   + 3)) == six)   { pointer = 4 * two + 3; goto bad; } 	// snco.e
	if (g(4 * g(4 * two   + 2)) == six)   { pointer = 4 * two + 2; goto bad; } 	// snco.g
	if (g(4 * g(4 * two   + 2)) == two)   { pointer = 4 * two + 2; goto bad; } 	// sci.2g
	if (g(4 * g(4 * one   + 3)) == one)   { pointer = 4 * one + 3; goto bad; } 	// ns0.1e1
	if (g(4 * g(4 * one   + 2)) == one)   { pointer = 4 * one + 2; goto bad; } 	// ns0.1g1
	if (g(4 * g(4 * one   + 3)) == five)  { pointer = 4 * one + 2; goto bad; } 	// pco.e
	if (g(4 * g(4 * one   + 2)) == five)  { pointer = 4 * one + 2; goto bad; } 	// pco.g
	if (g(4 * g(4 * one   + 2)) == two)   { pointer = 4 * one + 2; goto bad; } 	// sci.1g

	u16 was_utilized = 0;
	{ byte pa = graph_count; 
	GA_loop: 
		pa -= 4;
		const byte op = g(pa);
		const byte l = g(pa + 1);
		const byte g = g(pa + 2);
		const byte e = g(pa + 3);
		for (byte i = 0; i < 4 * 15; i += 4) {
			const byte A = loops[i + 0];
			const byte x = loops[i + 1];
			const byte B = loops[i + 2];
			const byte y = loops[i + 3];
			const byte K = g(pa + x);
			const byte E = 4 * K + y;

			if (	op == A and
				g(4 * K) == B and
				g(E) == pa >> 2
			) {
				pointer = graph_count;
				if (editable(pa) and pointer > pa) pointer = pa;
				if (editable(pa + x) and pointer > pa + x) pointer = pa + x;
				if (editable(4 * K) and pointer > 4 * K) pointer = 4 * K;
				if (editable(E) and pointer > E) pointer = E;
				goto bad;
			}
		}	
		if (l != pa >> 2) 			was_utilized |= 1 << l;
		if (g != pa >> 2 and op != six) 	was_utilized |= 1 << g;
		if (e != pa >> 2) 			was_utilized |= 1 << e;
		if (pa) goto GA_loop; 
	} 
	for (byte la = 0; la < operation_count; la++) {
		if (not ((was_utilized >> la) & 1)) { pointer = lsepa; goto bad; } 
	}

	for (byte pa = 20; pa < graph_count; pa += 4) if (g(pa) == six) goto skip_6_2_check;
	if (	g(4 * g(4 * six + 1)) == two and
		g(4 * g(4 * six + 2)) == two and
		g(4 * g(4 * six + 3)) == two
	) { pointer = 4 * six + 1; goto bad; } skip_6_2_check:; 

	for (byte pa = 20; pa < graph_count; pa += 4) if (g(pa) == five) goto skip_5_1_check;
	if (	g(4 * g(4 * five + 1)) == one and
		g(4 * g(4 * five + 2)) == one and
		g(4 * g(4 * five + 3)) == one
	) { pointer = 4 * five + 1; goto bad; } skip_5_1_check:; 

	for (byte pa = 20; pa < graph_count; pa += 4) if (g(pa) == three) goto skip_3_15_check;
	if (	g(4 * g(4 * three + 1)) == one and
		g(4 * g(4 * three + 2)) == one and
		g(4 * g(4 * three + 3)) == one 
	) {pointer = 4 * three + 1; goto bad; }
	if (	g(4 * g(4 * three + 1)) == five and
		g(4 * g(4 * three + 2)) == five and
		g(4 * g(4 * three + 3)) == five 
	) {pointer = 4 * three + 1; goto bad; }
	skip_3_15_check:; 

	byte origin = 255;
	nat cdm = 0;
	const byte is_bad = execute_graph(g0, g1, g2, &origin, array, counts, thread_index, &cdm);
	if (not is_bad) append_to_file(filenames[thread_index], 4096, g0, g1, g2, origin, cdm);
	goto loop;

terminate: free(array);
	atomic_store_explicit(progress + 3 * thread_index + 0, g0, memory_order_relaxed);
	atomic_store_explicit(progress + 3 * thread_index + 1, g1, memory_order_relaxed);
	atomic_store_explicit(progress + 3 * thread_index + 2, g2, memory_order_relaxed);
	return counts;
}


static void periodically_transfer_jobs(void) {

mainloop:;
	sleep(redistribution_delay);

	const nat our_job_count = atomic_load_explicit(&job_count, memory_order_relaxed);
	if (not our_job_count) goto done;
	atomic_store_explicit(global_job_counts + machine_index, our_job_count, memory_order_relaxed); 
	
	nat transfer_amount = 0;
	nat 	min_index = (nat) -1,
		min_value = (nat) -1,
		max_index = (nat) -1,
		max_value = 0;
	for (nat i = 0; i < machine_count; i++) {
		const nat g = atomic_load_explicit(global_job_counts + i, memory_order_relaxed);
		if (g == (nat) -1) continue;
		if (min_value > g) { min_value = g; min_index = i; }
	}
	for (nat i = 0; i < machine_count; i++) {
		const nat g = atomic_load_explicit(global_job_counts + i, memory_order_relaxed);
		if (g == (nat) -1) continue;
		if (max_value < g) { max_value = g; max_index = i; }
	}
	
	atomic_store_explicit(&global_min_index, min_index, memory_order_relaxed);
	atomic_store_explicit(&global_max_index, max_index, memory_order_relaxed);

	if (max_value > min_value and max_value - min_value > required_difference and machine_index == max_index) {
		const nat ideal = (max_value + min_value) / 2;
		const nat diff = max_value - ideal;
		if ((int64_t) diff < 0) { publish_error("transfer amount diff < 0"); transfer_amount = 0; goto connect_to_peers; } 
		transfer_amount = diff;
		if (transfer_amount > max_transfer_amount) transfer_amount = max_transfer_amount;
		printf("WARING: computed non-zero transfer amount of %llu, initiating transfer!\n", transfer_amount);
	} else transfer_amount = 0; 

	connect_to_peers: for (nat i = 0; i < machine_count; i++) {
		if (i == machine_index) continue;

		int connection = socket(AF_INET6, SOCK_STREAM, 0);
		if (connection < 0) { publish_error("socket"); abort(); }

		struct sockaddr_in6 server_address = {0};
		memset(&server_address, 0, sizeof server_address);
		server_address.sin6_family = AF_INET6;
		server_address.sin6_port = htons((int) (server_port + i));
		int r = inet_pton(AF_INET6, ip_addresses[i], &server_address.sin6_addr);
		if (r <= 0) { publish_error("inet_pton"); abort(); }

		r = connect(connection, (struct sockaddr *) &server_address, sizeof server_address); 
		if (r < 0) { publish_error("connect"); goto skip_to_next; } 

		char ip[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &server_address.sin6_addr, ip, sizeof ip);
		//const int port = ntohs(server_address.sin6_port);

		const nat local_job_count = atomic_load_explicit(&job_count, memory_order_relaxed);
		const nat suggested_amount = i == min_index ? transfer_amount : 0;
		nat amount = 0;
		nat command[packet_size_in_nats] = { machine_index, local_job_count, 0, min_index,  max_index };
		for (nat j = 0; j < suggested_amount; j++) {
			const nat job = pull_job_if_available();
			if (job == (nat) -1) break;
			command[5 + j] = job;
			amount++;
		}
		command[2] = amount;
		ssize_t n = write(connection, command, packet_size_in_bytes);
		if (n <= 0 and errno == EPIPE) goto done;
		else if (n <= 0) { publish_error("write"); goto skip_to_next; }
		if (not amount) goto skip_to_next;

		byte ack = 0;
		n = read(connection, &ack, 1);
		if (n <= 0 and errno == EPIPE) goto done;
		else if (n <= 0) { publish_error("read"); goto skip_to_next; }
		else if (n == 1 and ack == 1) {
		} else if (n == 1 and ack != 1) {
			for (nat j = 0; j < amount; j++) {
				const nat k = atomic_fetch_add_explicit(&job_count, 1, memory_order_relaxed);
				jobs[k] = command[5 + j];
			}
		}
		skip_to_next: close(connection);
	}
	goto mainloop; done:;
}

static void handle_sigalarm(int __attribute__((unused)) _) { }

int main(void) {
	machine_index = translate_hostname_to_machine_index(get_command_output("hostname"));
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, handle_sigalarm);
	signal(SIGUSR1, signal_handler);
	srand(20000000);
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	nat counts[pm_count] = {0};
	atomic_init(&job_count, 0);
	atomic_init(&running, true);
	atomic_init(&global_min_index, (nat) -1);
	atomic_init(&global_min_index, (nat) -1);
	for (nat i = 0; i < machine_count; i++) atomic_init(global_job_counts + i, (nat) -1);
	for (nat i = 0; i < thread_count * 3; i++) atomic_init(progress + i, 0);

{	byte pointer = 0;
	register nat g0 = pg0;
	register nat g1 = pg1;
	register nat g2 = pg2;
	goto init;
loop: 	if (g(pointer) < ((pointer & 3) ? operation_count - 1 : 4)) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto done;
reset_:; const nat mask = ~(0xfLLU << ((pointer & 15) << 2));
	     if (pointer < 16) g0 &= mask;
	else if (pointer < 32) g1 &= mask;
	else                   g2 &= mask;
	do pointer++; while (not editable(pointer));
	goto loop;
increment:; const nat addend = 1LLU << ((pointer & 15) << 2);
	     if (pointer < 16) g0 += addend;
	else if (pointer < 32) g1 += addend;
	else                   g2 += addend;
init:	pointer = graph_count - job_digit_count;
	while (not editable(pointer)) pointer++;
	const nat intended_machine = (nat) (rand() % machine_count);
	atomic_fetch_add_explicit(global_job_counts + intended_machine, 1, memory_order_relaxed);
	if (intended_machine == machine_index) {
		const nat n = atomic_fetch_add_explicit(&job_count, 1, memory_order_relaxed);
		jobs[n] = total_job_count;
	}	
	master_list_g0[total_job_count] = g0;
	master_list_g1[total_job_count] = g1;
	master_list_g2[total_job_count] = g2;
	total_job_count++;
	goto loop; done:; }

	srand((unsigned) time(0));
	filenames = calloc(thread_count, sizeof(char*));
	for (nat i = 0; i < thread_count; i++) {
		filenames[i] = calloc(4096, 1);
		char dt[32] = {0};
		get_datetime(dt);
		snprintf(filenames[i], 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
	}
	printf("SU: [PID %d]: [D %u]: [machine %llu]: starting search call...\n", getpid(), D, machine_index);
	snprintf(output_string, sizeof output_string, 
	     "SU: [PID %d]: [D %u]: [machine %llu]: starting search call...\n", getpid(), D, machine_index
	);
	publish();

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);
	pthread_t server_thread;
	int r = pthread_create(&server_thread, NULL, server_thread_function, NULL);
	if (r) { publish_error("pthread_create"); abort(); } 

	for (nat i = 0; i < thread_count; i++) {
		nat* arg = malloc(sizeof(nat)); *arg = i;
		pthread_create(threads + i, NULL, worker_thread, arg);
	}

	puts("[search call running]\n");

	periodically_transfer_jobs();

	atomic_store_explicit(&running, false, memory_order_relaxed);
	shutdown(server_socket, SHUT_RD);
	close(server_socket);

	puts("joining server thread...");
	pthread_join(server_thread, NULL);

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

	signal_handler(0);

	int length = 0;
	length += snprintf(pm_string + length, sizeof pm_string - (size_t) length, "\npm counts:\n");
        for (nat i = 0; i < pm_count; i++) {
                if (i and not (i % 2)) pm_string[length++] = 10;
		length += snprintf(pm_string + length, sizeof pm_string - (size_t) length, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
        }
        length += snprintf(pm_string + length, sizeof pm_string - (size_t) length, "\n[done]\n");
	snprintf(output_string, sizeof output_string,
		"SU: finished searching [D = %u] space:\n"
		"\t search took %10.2lf seconds\n"
		"\t from %s\n"
		"\t to   %s\n"
		"\n"
		"%s\n"
		"\n",
		D, seconds, time_begin_dt, time_end_dt,
		pm_string
	);
	publish();
	exit(0);
}





























































































































































































/*
	
int main(void) {
	machine_index = translate_hostname_to_machine_index(get_command_output("hostname"));
	signal(SIGUSR1, signal_handler);
	srand(20000000);
	pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
	nat counts[pm_count] = {0};
	memset(queue, 0, sizeof *queue * 65536);
	atomic_store(&queue_count, 0);
	for (nat i = 0; i < thread_count * 3; i++) atomic_store_explicit(progress + i, 0, memory_order_relaxed);






{	byte pointer = 0;
	register nat g0 = pg0;
	register nat g1 = pg1;
	register nat g2 = pg2;
	goto init;
loop: 	if (g(pointer) < ((pointer & 3) ? operation_count - 1 : 4)) goto increment;
	if (pointer < graph_count - 1) goto reset_;
	goto done;
reset_:; const nat mask = ~(0xfLLU << ((pointer & 15) << 2));
	     if (pointer < 16) g0 &= mask;
	else if (pointer < 32) g1 &= mask;
	else                   g2 &= mask;
	do pointer++; while (not editable(pointer));
	goto loop;
increment:; const nat addend = 1LLU << ((pointer & 15) << 2);
	     if (pointer < 16) g0 += addend;
	else if (pointer < 32) g1 += addend;
	else                   g2 += addend;
init:	pointer = graph_count - job_digit_count;
	while (not editable(pointer)) pointer++;
	if ((nat) rand() % machine_count != machine_index) goto loop; 
	const nat n = atomic_fetch_add_explicit(&queue_count, 1, memory_order_relaxed);
	queue[3 * n + 0] = g0;
	queue[3 * n + 1] = g1;
	queue[3 * n + 2] = g2;
	total_job_count++;
	goto loop;
done:; }










	srand((unsigned) time(0));

	filenames = calloc(thread_count, sizeof(char*));
	for (nat i = 0; i < thread_count; i++) {
		filenames[i] = calloc(4096, 1);
		char dt[32] = {0};
		get_datetime(dt);
		snprintf(filenames[i], 4096, "%s_%08x%08x%08x%08x_z.txt", dt, 
			rand(), rand(), rand(), rand()
		);
	}
	printf("SU: [PID %d]: [D %u]: [machine %llu]: starting search call...\n", getpid(), D, machine_index);
	snprintf(output_string, sizeof output_string, 
	     "SU: [PID %d]: [D %u]: [machine %llu]: starting search call...\n", getpid(), D, machine_index
	);
	publish();

	struct timeval time_begin = {0};
	gettimeofday(&time_begin, NULL);
	for (nat i = 0; i < thread_count; i++) {
		nat* arg = malloc(sizeof(nat)); *arg = i;
		pthread_create(threads + i, NULL, worker_thread, arg);
	}



	puts("[search call running]\n");



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
	signal_handler(0);
	int length = 0;
	length += snprintf(pm_string + length, sizeof pm_string - (size_t) length, "\npm counts:\n");
        for (nat i = 0; i < pm_count; i++) {
                if (i and not (i % 2)) pm_string[length++] = 10;
		length += snprintf(pm_string + length, sizeof pm_string - (size_t) length, "%6s: %-8lld\t\t", pm_spelling[i], counts[i]);
        }
        length += snprintf(pm_string + length, sizeof pm_string - (size_t) length, "\n[done]\n");
	snprintf(output_string, sizeof output_string,
		"SU: finished searching [D = %u] space:\n"
		"\t search took %10.2lf seconds\n"
		"\t from %s\n"
		"\t to   %s\n"
		"\n"
		"%s\n"
		"\n",
		D, seconds, time_begin_dt, time_end_dt,
		pm_string
	);
	publish();	
} // main







*/





































/*static void* worker_thread_function(void* unused) {

loop:; 	
	const nat is_running = atomic_load_explicit(&running, memory_order_relaxed);
	const nat job = pull_job_if_available();
	if (job != (nat) -1)  master_job_list[job]++; 
	else if (not is_running) goto ret;


	

	goto loop;



ret:	return unused;
}*/
















