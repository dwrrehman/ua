// a utility to test out the behavior of all possible two loops to be able to construct more GA (graph analysis) checks for use in the 3sp search utiltiy wihch has an emphasis on GA lol. 
// written 1202508262.225239 by dwrr

// 00071273200534544041000010001767

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#define execution_limit 1000
#define array_size 10000


typedef uint8_t byte;
typedef uint64_t nat;

enum operations { one, two, three, five, six };

enum pruning_metrics {
	z_is_good,  pm_infinite_loop,
	pm_zr5, pm_zr6, pm_ndi, pm_sndi,
	pm_pco, pm_per, pm_ns0,
	pm_oer, pm_rsi,
	pm_h0, pm_h0s, pm_h1, pm_h2,
	pm_pair, 
	pm_count
};

static const char* pm_spelling[pm_count] = {
	"z_is_good", "pm_infinite_loop", 
	"pm_zr5", "pm_zr6", "pm_ndi", "pm_sndi",
	"pm_pco", "pm_per", "pm_ns0",
	"pm_oer", "pm_rsi",
	"pm_h0", "pm_h0s", "pm_h1", "pm_h2",
	"pm_pair", 
};

enum { 
	loop_undefined = ' ', 
	loop_redundant = '/', 
	loop_pointer_modifying = '.', 
	loop_good = '-', 
};

static char pm_notation[pm_count] = {
	'-', 'i', 
	'5', '6', 'n', 's',
	'p', 'e', '0',
	'o', 'r',
	'h', 'k', '1', '2',
	'a', 
};


static void print_bytes(byte* array, byte count) {
	printf("%hhu) { ", count);
	for (byte i = 0; i < count; i++) {
		printf("%hhu ", array[i]);
	}
	printf("} ");
}

static bool is_in(byte* this, byte* array, nat count) {
	for (nat i = 0; i < 4 * count; i += 4) {
		if (
			this[0] == array[i + 1] and
			this[1] == array[i + 0] and 
			this[2] == array[i + 3] and 
			this[3] == array[i + 2]
		) return true;
	}
	return false;
}



#define max_erp_count 20
#define max_rsi_count 512
#define max_oer_repetions 50

#define max_consecutive_s0_incr 30
#define max_consecutive_h0_bouts 10
#define max_consecutive_h1_bouts 16
#define max_consecutive_h2_bouts 24
#define max_consecutive_h0s_bouts 7

#define max_consecutive_pairs 8

static nat execute_graph_starting_at(
	byte* nfarray,
	byte origin, 
	nat initial_comp,
	nat initial_modnat,
	nat initial_first
) {

	nat pointer = 2; 

	nat 	bout_length = 0, 
		OER_ier_at = 0,
		PER_ier_at = (nat) ~0;

	byte	H0_counter = 0,  H0S_counter = 0, SNDI_counter = 0,
		H1_counter = 0, H2_counter = 0, OER_counter = 0,
		pair_index = 0, pair_count = 0;
	
	byte ip = origin;
	byte last_mcal_op = 255;

 	byte small_erp_array[max_erp_count]; small_erp_array[0] = 0;
	byte rsi_counter[max_rsi_count]; rsi_counter[0] = 0;

	nat array[array_size] = {0};
	array[0] = initial_first;
	array[pointer] = initial_modnat;
	nat comp = initial_comp;

	for (nat e = 0; e < execution_limit; e++) {

		const byte op = nfarray[ip];

		if (op == one) {
  
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
		}

		else if (op == five) {
			if (last_mcal_op != three) return pm_pco;
			if (not pointer) return pm_zr5; 
			
			if (pointer == OER_ier_at or pointer == OER_ier_at + 1) {
				OER_counter++;
				if (OER_counter >= max_oer_repetions) return pm_oer;
			} else { OER_ier_at = pointer; OER_counter = 0; }

			if (pair_index == 3) { pair_index = 0; pair_count++; if (pair_count >= max_consecutive_pairs) return pm_pair; } 
			else if (pair_index) { pair_count = 0; pair_index = 0; }

			SNDI_counter = 0;

			PER_ier_at = pointer;
			pointer = 0;
		}

		else if (op == two) {
			SNDI_counter++;
			if (SNDI_counter >= 10) return pm_sndi;
			comp++;
		}

		else if (op == six) {  
			if (not comp) return pm_zr6;
			SNDI_counter = 0;
			comp = 0;
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
		if (comp < array[pointer]) state = 0;
		if (comp > array[pointer]) state = 1;
		if (comp == array[pointer]) state = 2;		
		if (nfarray[2 + ip] != state) return z_is_good;
		ip = not ip;
	}

	return pm_infinite_loop;
}



static void print_table(byte* table) {
	
	printf("TABLE:\n\n");

	printf("     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    \n");


	for (nat i = 0; i < 225; i += 15) {

		if (i % 75 == 0 and i) {
			puts("\n");
			printf("     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    \n");
		}

		char* at = (char*) (table + i);

		printf("     "
			"| %c | %c | %c | %c | %c |    "
			"| %c | %c | %c | %c | %c |    "
			"| %c | %c | %c | %c | %c |    \n",

			at[0], at[1], at[2], at[3], at[4], 
			at[5 + 0], at[5 + 1], at[5 + 2], at[5 + 3], at[5 + 4], 
			at[10 + 0], at[10 + 1], at[10 + 2], at[10 + 3], at[10 + 4]

		);

		printf("     +---+---+---+---+---+    +---+---+---+---+---+    +---+---+---+---+---+    \n");


	}
	//getchar();

	usleep(10000);

}

static void set_value(byte* table, byte* nfarray, const byte value) { 

	const byte A = nfarray[0];
	const byte B = nfarray[1];
	const byte x = nfarray[2];
	const byte y = nfarray[3];
	table[5 * 5 * 3 * y + 3 * 5 * A + 5 * x + B] = value;
}

int main(void) {


	byte table[225] = {0};
	for (nat i = 0; i < 225; i++) {
		table[i] = loop_undefined;
	}

	nat pm = 0;

	byte total[1024] = {0};
	nat total_count = 0;
	byte modulus[4] = {4, 4, 2, 2};
	byte nfarray[4] = {0};
	byte nfpointer = 0;
	goto init;

loop:
	if (nfarray[nfpointer] < modulus[nfpointer]) goto increment;
	if (nfpointer < 3) goto reset_;
	goto done;

increment:
	nfarray[nfpointer]++;
init:	nfpointer = 0;

	if (is_in(nfarray, total, total_count)) { 
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4);
		printf(" is redundant\n");
		set_value(table, nfarray, loop_redundant);
		print_table(table);
		goto loop;
	}

	total[4 * total_count + 0] = nfarray[0];
	total[4 * total_count + 1] = nfarray[1];
	total[4 * total_count + 2] = nfarray[2];
	total[4 * total_count + 3] = nfarray[3];
	total_count++;

	printf("\n\n\n\n\n\n\n\n\n\n\n\n");

	printf("[%5llu]: is unique! : ", total_count);
	print_bytes(nfarray, 4);
	puts("");

	if ((nfarray[0] == one and nfarray[1] == five) or (nfarray[1] == one and nfarray[0] == five)) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, 'P');
		print_table(table);
		goto loop;
	}

	if ((nfarray[0] == two and nfarray[1] == six) or (nfarray[1] == two and nfarray[0] == six)) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, 'S');
		print_table(table);
		goto loop;
	}

	if (nfarray[0] == six and nfarray[1] == one and nfarray[2] == 2) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, '0');
		print_table(table);
		goto loop;
	}

	if (nfarray[1] == six and nfarray[0] == one and nfarray[3] == 2) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, '0');
		print_table(table);
		goto loop;
	}




	if (nfarray[0] == six and nfarray[1] == five and nfarray[2] == 2) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, '0');
		print_table(table);
		goto loop;
	}

	if (nfarray[1] == six and nfarray[0] == five and nfarray[3] == 2) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, '0');
		print_table(table);
		goto loop;
	}






	if (nfarray[0] == six and nfarray[2] == 1 and nfarray[1] != six) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, 'g');
		print_table(table);
		goto loop;
	}

	if (nfarray[1] == six and nfarray[3] == 1 and nfarray[0] != six) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4); puts("");
		set_value(table, nfarray, 'g');
		print_table(table);
		goto loop;
	}



	if (nfarray[0] == one or nfarray[1] == one) {
		printf("[%5llu]: tried: ", total_count);
		print_bytes(nfarray, 4);
		printf(" is pointer modifying\n");
		set_value(table, nfarray, loop_pointer_modifying);
		print_table(table);
		goto loop;
	}




	for (byte ip = 0; ip < 2; ip++) {
	for (byte addend = 0; addend < 5; addend++) {
	for (byte combination = 0; combination < 8; combination++) {
		
		nat a = !!(combination & 1);
		nat b = !!(combination & 2);
		nat c = !!(combination & 4);

		if (addend == 1)  { //    0 / 6
			if (a) a += 5; 
			if (b) b += 5;
			if (c) c += 5;

		} else if (addend == 2) { //   5 / 6
			a += 5; b += 5; c += 5;

		} else if (addend == 3) { // 5 / 7
			if (a) a += 6; else a += 5;
			if (b) b += 6; else b += 5;
			if (c) c += 6; else c += 5;

		} else if (addend == 4) { // 5 / 50
			if (a) a += 49; else a += 5;
			if (b) b += 49; else b += 5;
			if (c) c += 49; else c += 5;
		}
	
		pm = execute_graph_starting_at(nfarray, ip, a, b, c);
		printf("EXG: [pm = %s] (%hhu,%llu,%llu,%llu)\n", pm_spelling[pm], ip, a, b, c);
		if (pm) goto bad; 
	}}}
	printf("[%5llu]: ran fully: ", total_count);
	print_bytes(nfarray, 4);
	printf(" is good\n");
	set_value(table, nfarray, loop_good);
	print_table(table);
	goto loop;

bad:
	printf("[%5llu]: tried: ", total_count);
	print_bytes(nfarray, 4);
	printf(" is BAD\n");
	set_value(table, nfarray, (byte) pm_notation[pm]);
	print_table(table);
	goto loop;

reset_:
	nfarray[nfpointer] = 0;
	nfpointer++;
	goto loop;

done:
	puts("terminating...");
	printf("total_count = %llu\n", total_count);


	puts("LEGEND:");
	for (nat i = 0; i < pm_count; i++) {
		printf("   %c  :   %s\n", pm_notation[i], pm_spelling[i]);
	}
	puts("");


}











		//nat init_array[array_size] = {16, 14, 14, 13, 10, 9, 5, 3, 2, 2, 1, 0, 0};
		//memcpy(array, init_array, sizeof(nat) * 15);	printf("[%5llu]: ran fully: ", total_count);



