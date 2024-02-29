// rmtrc: 000000C0000000000000000050003006

// rmtrc+Ft6,IAL+563 (without Et) :  00F0E0C0006030000000000051003026

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h> 
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>


/// 	XFG  search and run  utility.  used for finding the XFG, a special case of the UA.
/// 		written by Daniel W. R. Rehman , started on 2110317.093242 .
///                               edited on 2112046.134422

///	NOTE:  this is a new version of the utility, which replaces the notion of an option from being   "{source-br}{side(0/1)}{operation}{destination-br}" to being "{source-instruction}{destination-instruction}".  ie, more general- allowing for direct extensions, and true direct branches, which arent using the instruction 4 at all. furthermore, having a blacklist is harder in the new system, but thats okay.

// description: a test framework to brute force search the possibility space and search it manually, 
// and make it easier to see what the graphs do, ie their computation/lifetime after some number of ins.


typedef unsigned long long nat;
typedef int8_t byte;


// -------------- constants -------------------

static const nat array_display_limit = 15;
// how many cells you print, when displaying the arrray. must be ≤ array size.

static const nat array_size = 4096; 	
// finite size of the arrray, used to simulate n = infinity.    make this reasonably big.

static const nat max_stack_size = 128; 			
// how many stack frames at maximum.

static const nat required_er_count = 10;


struct options {
	byte* options;   // an array of destinations.
	nat count;
};

struct stack_frame {
	struct options options;
	nat pointer_state;
	nat zero_reset_happened;
	nat er_count;
	nat array_state[array_size];
	nat try;
	int padding2;
	byte source;
	byte side;         // always   0/false    for operations.
	byte padding0;
	byte padding1;
};

enum expansion_type {

	no_expansion,            
	// eg,    0 0 0 0 0 0 0 0 0 0 0 .. y   ie, no expansion at all.

	constant_expansion,      
	// eg,    1 1 1 1 1 1 1 1 1 0 0 .. y   ie, no internal structure, just expansion and never ER.

	good_expansion,          
	// eg,    4 2 6 1 5 2 1 1 0 0 0 .. y   ie, what we are looking for. does ER, and has structure.

	hole_expansion,          
	// eg,    3 4 2 0 1 1 0 0 0 0 0 .. y   ie, has a hole, which means that it double skipped on LE. bad.

	short_expansion,  	 
	// eg,    3 4 1 0 0 0 0 0 0 0 0 .. y   ie, is good, but very short, and thus bad. 

	firstone_expansion,
	// eg,    1 3 4 2 1 1 x x x x x .. y   ie, has *0 == 1, after execution_limit timesteps.

	expansion_type_count
};

static const char* expansion_type_spelling[] = {
	"none",
	"const",
	"good",
	"hole",
	"short",
	"first",
};

struct candidate {
	nat expansion_type;
	nat is_complete;
	byte graph[32];
	nat lifetime[64];
	nat zero_reset_happened;
	nat er_count;
};

struct edge {
	byte source;
	bool side;
	byte destination;
};

static nat blacklist_count = 0;
static struct edge* blacklist = NULL;

static const char* blacklisted_edges[] = {

	"EtF", "FtE",    	// vanilla trich red

	"EtC", "FtC",    	// C trichotomy reducabilities

	"6fF",           	// nerdp reduc.

	"1f5", "2f6",	 	// increment-reset reduc.

	"3f5",           	// this makes the IA mode impossible. 

	"6f5", "2f5",    	// we require that on our walk star n is reset and incr without saying 5.

	"5f1",           	// we need to ER to the b:beginning; of the array.

	"Ct5", "Et5", "Ft5",    // if these, then we dont ER at the FEA, which is required.

	
0};



static void clear_screen() { printf("\033[2J\033[H"); }

static char hex(byte i) { return i < 10 ? '0' + i : i - 10 + 'A'; }

static char nonzero_hex(byte i) { if (not i) return ' '; else return hex(i); }

static int read_hex(int c) { return isalpha(c) ? toupper(c) - 'A' + 10 : c - '0'; }

static bool is(const char* c[8], const char* _long, const char* _short) {
	return c[0] and (not strcmp(c[0], _long) or not strcmp(c[0], _short));
}

static void get_datetime(char datetime[16]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 15, "%y%m%d%u.%H%M%S", tm_info);
}

static void print_datetime() {
	char dt[16] = {0};
	get_datetime(dt);
	printf("%s\n", dt);
}

static char* hex_string(byte* graph) {
	static char string[33] = {0};
	nat s = 0;
	for (nat i = 0; i < 32; i++) string[s++] = hex(graph[i]);
	string[s] = 0;
	return string;
}

static void display_state(nat* array, nat n, nat pointer) {
	printf("\n");
	for (nat i = 0; i < array_display_limit and i < n; i++) { 
		printf(" [ %2llu ] ", array[i]);
	} 
	printf("   ...  *n:[ %2llu ]\n    ", array[n]);
	for (nat i = 0; i < pointer and i < array_display_limit; i++) {
		printf("        ");
	}
	printf("^\n. \n");
}

static void display_state_compact(nat* array, nat n) {
	for (nat i = 0; i < array_display_limit and i < n; i++) { 
		printf("%2llu ", array[i]);
	} 
	printf(".. %2llu\n", array[n]);
}

static void print_as_adjacency_list(byte* graph) {
	printf("graph:\n\n");
	for (byte i = 0; i < 8; i++) {
		printf("\t%c: %c, %c    ", hex(i), nonzero_hex(graph[i * 2 + 0]), nonzero_hex(graph[i * 2 + 1]));
		printf("%c: %c, %c\n", hex(i + 8), nonzero_hex(graph[(i + 8) * 2 + 0]), nonzero_hex(graph[(i + 8) * 2 + 1]));
	}
	printf("\n");
}


static void print_as_xfg_adjacency_list(byte* graph) {
	printf("xfg:\n\n"

		"\t1: %c\t5: %c\n"
		"\t2: %c\t6: %c\n"
		"\t3: %c\n"
		"\n"
		"\tC: %c, %c\n"
		"\tE: %c, %c\n"
		"\tF: %c, %c\n"
		"\n",

		nonzero_hex(graph[1 * 2]),  nonzero_hex(graph[5 * 2]), 
		nonzero_hex(graph[2 * 2]),  nonzero_hex(graph[6 * 2]),
		nonzero_hex(graph[3 * 2]), 

		nonzero_hex(graph[0xC * 2]), nonzero_hex(graph[0xC * 2 + 1]),
		nonzero_hex(graph[0xE * 2]), nonzero_hex(graph[0xE * 2 + 1]),
		nonzero_hex(graph[0xF * 2]), nonzero_hex(graph[0xF * 2 + 1])
	);
}

static void print_stack(struct stack_frame* stack, nat stack_count) {
	printf("printing stack: [sf count=%llu]\n{\n", stack_count);

	for (nat i = 0; i < stack_count; i++) {
		printf("\tframe #%llu:  [%hhX%c]  "
			"{.try=%llu, .source=%d, .side=%d, .options.count=%llu .pointer=%llu, .zeroreset=%llu} :: ", 
		i, stack[i].source, stack[i].side ? 't':'f', 
		stack[i].try, stack[i].source, stack[i].side, stack[i].options.count, 
		stack[i].pointer_state, stack[i].zero_reset_happened);
		display_state_compact(stack[i].array_state, array_size - 1);
	}
	printf("}\n");
}

static void print_z_list(byte** list, nat list_count) {
	printf("printing list of z values:\n");
	for (nat i = 0; i < list_count; i++) {
		printf("#%llu:   %s\n", i, hex_string(list[i]));
	}
	printf("list done.\n");
}

static void initialize_graph_from_string(byte* graph, const char* string) {
	nat s = 0;
	for (nat i = 0; i < 32; i++) {
		const char c = string[s++];
		const int g = isalpha(c) ? toupper(c) - 'A' + 10 : isdigit(c) ? c - '0' : 0;
		graph[i] = (byte) g;
	}
}

static byte** read_z_list_from_file(const char* filename, nat* result_count) {
	FILE* file = fopen(filename, "r");
	if (not file) { perror("fopen"); return NULL; }
	
	byte** list = NULL;
	nat list_count = 0;
	char buffer[1024] = {0};

	while (fgets(buffer, sizeof buffer, file)) {
		byte* new_graph = malloc(32);
		initialize_graph_from_string(new_graph, buffer);
		list = realloc(list, sizeof(byte*) * (list_count + 1));
		list[list_count++] = new_graph;
	}
	fclose(file);
	*result_count = list_count;
	return list;
	
}

static void write_candidates_to_file(const char* filename, struct candidate* candidates, nat count) {
	FILE* file = fopen(filename, "w+");
	if (not file) { perror("fopen"); return; }
	
	for (nat i = 0; i < count; i++) 
		fprintf(file, "%s\n", hex_string(candidates[i].graph));
	

	fclose(file);
}

static void read_graph_edit(byte* graph, const char* string) {  // [format: {source}{false}{true} ]

	const int source = read_hex(string[0]), dest0 = read_hex(string[1]), dest1 = read_hex(string[2]);
	if (source < 0 or source >= 16) { printf("error: source out of bounds\n"); return; }
	if (dest0 < 0 or dest0 >= 16) { printf("error: false_destination out of bounds\n"); return; }
	if (dest1 < 0 or dest1 >= 16) { printf("error: true_destination out of bounds\n"); return; }

	graph[source * 2 + 0] = (byte) dest0;
	graph[source * 2 + 1] = (byte) dest1;

}

static void parse_command(const char* arguments[8], char* buffer) {	
	nat length = strlen(buffer) - 1;
	buffer[length] = 0;

	for (nat i = 0, a = 0; a < 8; a++) {	
		while (buffer[i] == ' ' and i < length) i++;
		arguments[a] = buffer + i;
		while (buffer[i] != ' ' and i < length) i++; 
		buffer[i++] = 0;
	}
}

static byte read_origin(char c) {
	byte origin = (byte) read_hex(c);
	if (origin < 0 or origin >= 16) { 
		printf("error: origin out of bounds, received (%d)\n", c); 
		return 0; 
	} else return origin;
}




static void run(const nat m, const nat n, byte* graph, const byte start) {
	
	nat* a = calloc(n + 1, sizeof(nat));
	nat p = 0;

	byte i = start;

	display_state(a, n, p);

	while (i) {
		
		printf("[%c]", hex(i));
		
		if (i == 9) {
			i = graph[i * 2 + (p != n)];
			printf("[%c]\n", p != n ? 't' : 'f');

		} else if (i == 0xB) {
			i = graph[i * 2 + (a[p] != m)]; 
			printf("[%c]\n", a[p] != m ? 't' : 'f');

		} else if (i == 0xC) {
			i = graph[i * 2 + (a[n] != a[p])];
			printf("[%c]\n", a[n] != a[p] ? 't' : 'f');
		
		} else if (i == 0xE) {
			i = graph[i * 2 + (a[n] < a[p])];
			printf("[%c]\n", a[n] < a[p] ? 't' : 'f');

		} else if (i == 0xF) {
			i = graph[i * 2 + (a[p] < a[n])];
			printf("[%c]\n", a[p] < a[n] ? 't' : 'f');
		}

		else if (i == 1) { p++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 2) { a[n]++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 3) { a[p]++; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 5) { p = 0; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 6) { a[n] = 0; 			i = graph[i * 2];  printf("\n"); }
		else if (i == 7) { a[p] = 0; 			i = graph[i * 2];  printf("\n"); }
		else abort();

		display_state(a, n, p);
		printf(": "); 
		if (getchar() == 'q') break;
	}

	printf("[HALT]\n");

	free(a);
}



static struct termios terminal = {0};


static void restore_terminal() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) < 0) {
		perror("tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal))");
		abort();
	}
}

static void configure_terminal() {
	if (tcgetattr(STDIN_FILENO, &terminal) < 0) { perror("tcgetattr(STDIN_FILENO, &terminal)"); abort(); }
	atexit(restore_terminal);
	struct termios raw = terminal;
	raw.c_lflag &= (unsigned long) ~(ECHO | ICANON);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
		perror("tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)");
		abort();
	}
}

static char get_character() {
	char c = 0;
	const ssize_t n = read(STDIN_FILENO, &c, 1);
	if (n < 0) {
		printf("n < 0 : ");
		perror("read(STDIN_FILENO, &c, 1) syscall");
		abort();
	} else if (n == 0) {
		printf("n == 0 : ");
		perror("read(STDIN_FILENO, &c, 1) syscall");
		abort();
	} else return c;
}

static void visualize_set(const char* filename, byte* graph, byte start, nat width, nat height) {
	nat list_count = 0;
	byte** list = read_z_list_from_file(filename, &list_count);
	if (not list) return;
	print_z_list(list, list_count);
	printf("loaded %llu z values. continue? ", list_count);
	getchar();

	configure_terminal();

	const nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));
	nat pointer = 0;

	for (nat z = 0; z < list_count; z++) {

		// initialize the graph to the z vlaue.
		memcpy(graph, list[z], 32);

		// reset the array back to the void.
		memset(array, 0, array_size * sizeof(nat));
		pointer = 0;

		// reset the origin back to start.
		byte i = start;
		nat timestep = 0, executed_count = 0, max_timesteps = height;
		const nat execution_limit = 3000;

		clear_screen();
		printf("[ ( %llu / %llu ) ] visualizing:   %s  \n", z, list_count, hex_string(graph));
		
		while (i) {
			
			if (i == 5) {
				// printf("[@%c] : ", hex(i));
				for (nat j = 0; j < width; j++) { 
					if (not array[j]) printf("    ");
					else printf(" %2llu ", array[j]);
				} 
				printf(" .. %2llu\n", array[n]);

				timestep++;
			}


			if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[pointer])];
			} else if (i == 0xC) {
				i = graph[i * 2 + (array[n] != array[pointer])];
			} else if (i == 0xF) {
				i = graph[i * 2 + (array[n] > array[pointer])];
			}

			else if (i == 1) { pointer++; 			i = graph[i * 2];  }
			else if (i == 2) { array[n]++; 			i = graph[i * 2];  }
			else if (i == 3) { array[pointer]++; 		i = graph[i * 2];  }
			else if (i == 5) { pointer = 0; 		i = graph[i * 2];  }
			else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  }
			
			executed_count++;
			if (executed_count < execution_limit and timestep < max_timesteps) continue;

		re_input:
			printf("(qtrhnu)> "); 
			int c = get_character();

			if (c == 'q') goto done;
			else if (c == 't') break;
			else if (c == 'r') { if (z) z -= 2; break; }
			else if (c == 'h' or c == '\n') { 
				timestep = 0;
				executed_count = 0;
				clear_screen(); 
				printf("[ ( %llu / %llu ) ] visualizing:   %s  \n", z, list_count, hex_string(graph));
				continue; 
			} else if (c == 'n') { printf("unimplemented\n "); /* blacklist z value */ }
			else if (c == 'u') { printf("unimplemented\n ");/* save z value */ }
			else {
				printf("error: unknown input, please use: t r h q to control visualization\n");
				goto re_input;
			}
		}
	}
done:
	restore_terminal();
	printf("exiting visualize utility...\n");
	free(array);
}

/*

	VISUALIZER TODO:  
---------------------------------------


	x	fix input so that you dont have to press enter. very annoying. 

	x	make the prompt less noisy. add a help menu to it. 

		add the visulize utility to the general command help menu. 

	x	make there print new lines, for height timesteps, emptyts, if no say 5 

	x	clear screen every new z value. 

	x	have a command to print the z adjacnceyc list for the z vlaue. 

*/



static void generate_blacklist() {

	for (nat i = 0; blacklisted_edges[i]; i++) {

		const char* string = blacklisted_edges[i];

		struct edge edge = {
			.source = (byte) read_hex(string[0]),
			.side = string[1] == 't',
			.destination = (byte) read_hex(string[2]),
		};
		
		blacklist = realloc(blacklist, sizeof(struct edge) * (blacklist_count + 1));
		blacklist[blacklist_count++] = edge;
	}
}


static void print_blacklist() {
	printf("current blacklist (%llu edges) = {\n\n", blacklist_count);
	for (nat i = 0; i < blacklist_count; i++) {
		struct edge edge = blacklist[i];
		printf("%llu.\t%hhX%c%hhX\n\n", i, edge.source, edge.side ? 't' : 'f', edge.destination);
	}
	printf("}\n");
}


static bool is_blacklisted(byte source, bool side, byte destination) {
	for (nat i = 0; i < blacklist_count; i++) {
		if (	blacklist[i].source == source and 
			blacklist[i].side == side and 
			blacklist[i].destination == destination) 
				return true;
	}
	return false;
}


static struct options generate(byte source, bool side) {

	byte* options = malloc(8); 
	nat count = 0;

	byte destinations[] = {1, 2, 3, 5, 6, 0xE, 0xC, 0xF};

	for (int i = 0; i < (int) sizeof destinations; i++) {
		if (destinations[i] == source) continue;

		if (not is_blacklisted(source, side, destinations[i])) options[count++] = destinations[i];
	}

	struct options result = {.options = options, .count = count};
	return result;
}

static nat determine_expansion_type(nat* lifetime, nat n) { // classifications that are based on the lifetime's end-state alone.


	// ------------- firstone expansion test ----------------      eg        1 2 4 3 2 2 1 0 0 0 0 0 0 0 

	if (lifetime[0] == 1) return firstone_expansion;


	// ------------ no expansion test: -------------            eg        0 0 0 0 0 0 0 0 0 0 0 0 0 0 

	bool is_no_expansion = true;
	for (nat i = 0; i < n; i++) {
		if (lifetime[i]) { is_no_expansion = false; break; }
	}
	if (is_no_expansion) return no_expansion;


	// --------------- hole expansion ----------------         eg         1 2 4 2 0 3 2 3 0 0 0 0 0 
	//									      ^
	//                                                                           (hole)
	nat first = 0, last = n;
	for (;last--;)
		if (lifetime[last]) break;     // go from the last modnat, to the first, looking for where the first nonzero cell is.

	// we are now looking at the first positive cell. move forwards 1 back to where the most recent zero cell is.
	last++;
	
	for (;first < last; first++)
		if (not lifetime[first]) break; // go from the first modnat to the last, looking for where the first zero cell is.
	
	// if the left most zero cell (ie, last) is the same cell as the first zero cell that we found going forwards, then we say its good. if not, then there must be a hole.
	if (last != first) return hole_expansion;
	

	// ------------------ constant expansion -------------------   eg   1 1 1 1 1 1 1 1 1 1 0 0 0 0 0

	bool is_constant_expansion = true;
	for (nat i = 0; i < last; i++) {
		if (lifetime[i] != 1) { is_constant_expansion = false; break; }
	}
	
	if (is_constant_expansion) return constant_expansion;


	// ------------------ short expansion -----------------------  eg  2 1 1 0 0 0 0 0 0 0 0 0 0 0 0
	if (last < 5) return short_expansion;


	// everything else:
	return good_expansion;
}


static bool is_complete(byte* graph) {

	if (not graph[2 * 0x1]) return false;

	if (not graph[2 * 0x2]) return false;

	if (not graph[2 * 0x3]) return false;

	if (not graph[2 * 0x5]) return false;

	if (not graph[2 * 0x6]) return false;

	if (not graph[2 * 0xC + 0]) return false;
	if (not graph[2 * 0xC + 1]) return false;

	if (not graph[2 * 0xE + 0]) return false;
	if (not graph[2 * 0xE + 1]) return false;

	if (not graph[2 * 0xF + 0]) return false;
	if (not graph[2 * 0xF + 1]) return false;

	return true;
}


static void print_statistics(nat tried, nat counts[expansion_type_count][2][2][2]) {
	printf("statistics:\n\n");

	for (nat xp = 0; xp < expansion_type_count; xp++) { 
		for (nat clt = 0; clt < 2; clt++) {
			for (nat zr = 0; zr < 2; zr++) {
				for (nat com = 0; com < 2; com++) {
					printf("\t%s:%s:%s:%s = %5llu (%3.3lf%%)   ",
						expansion_type_spelling[xp], 
						com ? "complete" : "incomplete", 
						zr ? "zeroreset" : "!zeroreset",
						clt ? "longclt" : "shortclt",
						counts[xp][com][zr][clt], 
						(double)counts[xp][com][zr][clt] / (double)tried * 100.0
					);
				}
				puts("");
			}
			puts("");
		}
		puts("");
	}
	puts("");
}

static void search(byte* graph, byte origin, nat execution_limit) {

	printf("searching: [graph: %s, origin = %hhX, limit = %llu]\n", 
		hex_string(graph), origin, execution_limit);

	nat tried = 0;
	nat counts[expansion_type_count][2][2][2];
	memset(counts, 0, sizeof(nat) * 2 * 2 * 2 * expansion_type_count);

	struct candidate* candidates = NULL;
	nat candidate_count = 0;
	nat candidate_capacity = 0;

	nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));
	nat pointer = 0;
	nat zero_reset_happened = false;
	nat er_count = 0;

	byte i = origin;     // instruction pointer.

	// const nat history_count = 6;
	// byte instruction_history[history_count] = {0};
	// byte side_history[history_count] = {0};

	byte parent = 0;      // previous iteration's value of the instruction pointer. ("parent")
	bool parent_side = false;
	
	struct stack_frame* stack = calloc(max_stack_size, sizeof(struct stack_frame));
	nat stack_count = 0;
	nat executed_count = 0;

begin:
	while (executed_count < execution_limit) {
	
		// printf("info: [instruction pointer = %hhX]\n", i);
		if (i == 0) {
			// printf("found hole at %hhX%c (parent=%hhX, parent_side=%d) generating a list of options to fill it...\n", parent, parent_side ? 't' : 'f', parent, parent_side);
			struct options options = generate(parent, parent_side);
			// print_options(options, parent, parent_side); // debug.
			struct stack_frame frame;
			frame.try = 0; // start with the first option.
			frame.source = parent;      // keep track of the origin/parent of the hole.
			frame.side = parent_side;
			frame.options = options;

			memcpy(frame.array_state, array, sizeof(nat) * array_size);
			frame.pointer_state = pointer;
			frame.zero_reset_happened = zero_reset_happened;
			frame.er_count = er_count;

			stack[stack_count++] = frame;
			if (stack_count == max_stack_size) { printf("error: stack overflow\n"); abort(); }
			// printf("pushed new stack frame [sf count=%llu]: {.try=%llu, .source=%d, .side=%d, .options.count=%llu}\n", 
				// stack_count, frame.try, frame.source, frame.side, frame.options.count);
			// printf("state of array before push:\n"); 
			// display_state(array, n);
			// print_stack(stack, stack_count);
			// printf("[info: executed %llu / %llu ins before generate() call.]\n", executed_count, execution_limit);
			executed_count = 0;
			graph[2 * parent + parent_side] = options.options[0];
			// printf("current graph is now: \n");
			// print_as_adjacency_list(graph);
			// printf("reverting instruction pointer to go back before we had executed the hole, %hhX...\n", parent);
			i = options.options[0];
			// printf("generate: continue? > "); 
			// if (getchar() == 'q') return;
		} else {
			// printf("[%llu / %llu] executing %hhX...\n", executed_count, execution_limit, i);
			executed_count++;
			parent = i;
			parent_side = false; // overwritten by branch instructions, not overwritten by operations.
			// execute instruction.
			// printf("[%c]", hex(i));
		
			if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[pointer])];
				parent_side = (array[n] < array[pointer]);
				// printf("[%c]\n", array[n] < array[array[0]] ? 't' : 'f');

			} else if (i == 0xC) {
				i = graph[i * 2 + (array[n] != array[pointer])];
				parent_side = (array[n] != array[pointer]);
				// printf("[%c]\n", array[array[0]] < array[n] ? 't' : 'f');

			} else if (i == 0xF) {
				i = graph[i * 2 + (array[n] > array[pointer])];
				parent_side = (array[n] > array[pointer]);
				// printf("[%c]\n", array[array[0]] < array[n] ? 't' : 'f');
			}


			else if (i == 1) { pointer++; 				i = graph[i * 2];  }
			else if (i == 2) { array[n]++; 				i = graph[i * 2];  }
			else if (i == 3) { array[pointer]++; 			i = graph[i * 2];  }

			else if (i == 5) { 
				if (not pointer) zero_reset_happened = true;
				er_count++;
				pointer = 0; 
				i = graph[i * 2];

			} else if (i == 6) { 
				if (not array[n]) zero_reset_happened = true;
				array[n] = 0;
				i = graph[i * 2];
			}
			
			else abort();
			// printf("search: printing array state:\n");
			// display_state(array, n);
			// printf("execute: continue? > "); 
			// if (getchar() == 'q') return;
		} // else 
	} // while()
	// printf("state of the stack: \n");
	// print_stack(stack, stack_count);
	// printf("tried candidate graph: \n");
	// print_as_adjacency_list(graph);
	// puts("");
	
	struct candidate new = {determine_expansion_type(array, n), is_complete(graph), {0}, {0}, zero_reset_happened, er_count};
	memcpy(new.graph, graph, 32);
	
	if (new.is_complete and new.expansion_type == good_expansion 
		and not new.zero_reset_happened  and new.er_count >= required_er_count ) {
		if (candidate_count + 1 > candidate_capacity) {
			candidates = realloc(candidates, sizeof(struct candidate) * 
				(candidate_capacity = 4 * (candidate_capacity + candidate_count + 1)));
			// printf("expanded array: %llu\n", candidate_capacity);
		}
		candidates[candidate_count++] = new;
	}
	
	tried++;
	counts[new.expansion_type][new.is_complete][new.zero_reset_happened][new.er_count >= required_er_count]++;
	// if (getchar() == 'q') {
	// 	printf("info: ending graph search early...\n");
	// 	return;
	// }


	if (not (tried & 4095)) { // 65535
	// if ((1)) {
		// printf("\rtried: %llu                 ", tried);
		clear_screen();
		print_as_adjacency_list(graph);
		printf("searching: [graph: %s, origin = %hhX, limit = %llu]\n", 
			hex_string(graph), origin, execution_limit);
		printf("----> tried %llu control flow graphs.\n", tried);
		print_statistics(tried, counts);
		print_stack(stack, stack_count);
		fflush(stdout);
	}

backtrack:

	if (stack_count == 0) {
		printf("error: no hole was found in the partial graph, after %llu instructions executed.\n", executed_count);
		return;
	}

	// printf("backtracking!...\n");
	// print_stack(stack, stack_count);
	memcpy(array, stack[stack_count - 1].array_state, array_size * sizeof(nat));
	pointer = stack[stack_count - 1].pointer_state;
	zero_reset_happened = stack[stack_count - 1].zero_reset_happened;
	er_count = stack[stack_count - 1].er_count;

	// printf("reverted state of the array back to:\n");
	// display_state(array, n);
	// struct stack_frame top = stack[stack_count - 1];

	if (stack[stack_count - 1].try < stack[stack_count - 1].options.count - 1) {
		
		stack[stack_count - 1].try++;
		// printf("incremented stack[%llu].try to be %llu... instantiating..\n", stack_count - 1, stack[stack_count - 1].try);
		// printf("before graph was: \n");
		// print_as_adjacency_list(graph);	
		graph[2 * stack[stack_count - 1].source + stack[stack_count - 1].side] 
			= stack[stack_count - 1].options.options[stack[stack_count - 1].try];

		// printf("current graph is now: \n");
		// print_as_adjacency_list(graph);
		// printf("previous i value = %hhX...\n", i);
		i = stack[stack_count - 1].options.options[stack[stack_count - 1].try];
		// printf("trying this option starting from [i = %hhX]...\n", i);
		// printf("reseting the executed count (which was %llu...\n", executed_count);
		executed_count = 0;
		goto begin;
	} else {
		// printf("reverting last option...\n");
		// printf("before graph was: \n");
		// print_as_adjacency_list(graph);
		graph[2 * stack[stack_count - 1].source + stack[stack_count - 1].side] = 0;
		// printf("current graph is now: \n");
		// print_as_adjacency_list(graph);
		if (stack_count <= 1) goto done;
		// printf("popping...\n");
		stack_count--; 
		goto backtrack;
	}
done:
	printf("finished searching: [graph: %s, origin = %hhX, limit = %llu]\n", hex_string(graph), origin, execution_limit);
	printf("success: [exited search function successfully: exhausted all graph extension possibilities.]\n");
	printf("----> tried %llu control flow graphs.\n", tried);
	print_statistics(tried, counts);

	const char* dest_filename = "good.txt";
	printf("writing good:complete:!zeroreset:longclt z values to a file: \"%s\": (%llu total candidates).\n", dest_filename, candidate_count);
	write_candidates_to_file(dest_filename, candidates, candidate_count);
	free(array);
}






// void generate_lifetime_image(const char* filename, nat begin_timestep, nat begin_slice, nat end_slice, graph) {
            
//     const nat cell_count = p.s, timesteps = p.t;
    
//     FILE* file = fopen(filename, "wb");
    
//     if (!file) {
//         fprintf(stderr, "error: %s: could not open file for writing: %s\n", filename, strerror(errno));
//         return;
//     }
    
//     fprintf(file, "P6\n%llu %llu\n255\n", cell_count, timesteps);
    
//     mode = running;
    
    
//     for (nat timestep = 0; timestep < begin_timestep + timesteps && mode != stopped; timestep++) {
        
//         for (nat cell = 0; cell < cell_count; cell++) {
            
//             // fill_balanced_neighbors(n, read_cells, cell, neighborhood, cell_count, sidelength);
//             // write_cells[cell] = hgrid[unreduce(neighborhood, m, n)];
            
//             if (timestep >= begin_timestep) {
//                 double x = (double) a[i] / (double) maximum;   // what would this be?...
//                 unsigned char r = x * 255, g = x * 255, b = x * 255;
//                 fwrite(&r, 1, 1, file);
//                 fwrite(&g, 1, 1, file);
//                 fwrite(&b, 1, 1, file);
//             }
//         }
//     }

//     fclose(file);
// }

// void generate_lifetime_images(char** input, struct context* context) {
    
//     // const nat m = context->parameters.m, n = context->parameters.n, H = to(m,n);
    
//     const char* destination_dir = input[5];
//     nat z_count = 0, begin = atoll(input[3]), end = atoll(input[4]);
//     vector z_values = read_nats_from_file(input[2], &z_count);
    
//     printf("generate: generating %llu lifetimes .ppm's...\n", z_count);
    
//     mode = running;
    
//     for (nat i = 0; i < z_count; i++) {
//         nat z = z_values[i];
//         printf("\r [  %llu  /  %llu  ] : %llu                  ", i, z_count, z);
//         fflush(stdout);
        
//         char path[4096] = {0}, filename[4096] = {0};
//         sprintf(filename, "z_%llu.ppm", z);
//         strcpy(path, destination_dir);
//         strcat(path, "/");
//         strcat(path, filename);
        
//         generate_lifetime_image(path, time_begin, time_count, space_begin, space_count);
//     }
//     printf("generate: generated all images.\n");
// }




















static void generate_lifetime_images(
	const char* z_list_filename, 
	const char* destination_dir, 
	byte* graph, byte start, 
	nat begin_timestep, nat end_timestep,
	nat begin_cell, nat end_cell,
	nat maximum
) {
	nat list_count = 0;
	byte** list = read_z_list_from_file(z_list_filename, &list_count);
	if (not list) return;

	printf("generate: generating %llu lifetimes .ppm's...\n", list_count);

	const nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));
	nat pointer = 0;

	for (nat z = 0; z < list_count; z++) {

		memcpy(graph, list[z], 32);
		memset(array, 0, array_size * sizeof(nat));
		pointer = 0;

		byte i = start;

		nat timestep = 0;
		
		printf("\r [ ( %llu / %llu ) ] generating lifetime:  %s            ", 
				z, list_count, hex_string(graph));

		fflush(stdout);

		
		char path[4096] = {0}, filename[4096] = {0};
		sprintf(filename, "z_%s.ppm", hex_string(graph));
		strcpy(path, destination_dir);
		strcat(path, "/");
		strcat(path, filename);

		FILE* file = fopen(path, "wb");

		if (not file) {
			fprintf(stderr, "error: %s: could not open file for writing: %s\n", 
				path, strerror(errno));
			return;
		}

		fprintf(file, "P6\n%llu %llu\n255\n", end_cell - begin_cell, end_timestep - begin_timestep);

		do {
			
			if (i == 5) {
				if (timestep >= begin_timestep and 
				    timestep < end_timestep) {
					for (nat j = begin_cell; j < end_cell; j++) {
						double x = (double) array[j] / (double) maximum;  

						// nat x = array[j];
				   //              unsigned char 
							// r = x ? 255 : 0, 
							// g = x ? 255 : 0, 
							// b = x ? 255 : 0;

						unsigned char 
							r = (unsigned char)(x * 255.0), 
							g = (unsigned char)(x * 255.0), 
							b = (unsigned char)(x * 255.0);
				                fwrite(&r, 1, 1, file);
				                fwrite(&g, 1, 1, file);
				                fwrite(&b, 1, 1, file);
					}
				}
				timestep++;
			}


			if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[pointer])];
			} else if (i == 0xC) {
				i = graph[i * 2 + (array[n] != array[pointer])];
			} else if (i == 0xF) {
				i = graph[i * 2 + (array[n] > array[pointer])];
			}

			else if (i == 1) { pointer++; 			i = graph[i * 2];  }
			else if (i == 2) { array[n]++; 			i = graph[i * 2];  }
			else if (i == 3) { array[pointer]++; 		i = graph[i * 2];  }
			else if (i == 5) { pointer = 0; 		i = graph[i * 2];  }
			else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  }
			
		} while (timestep < end_timestep);

		fclose(file);


	}
	printf("generate: generated all images.\n");
	free(array);
}


















static void print_help_menu() {

printf("available commands:\n"
"\n"
	"\t- init(i) <hex32_string> : initialize the graph via a 32 digit hex string.\n"
	"\t- dump(d) : dump the current graph as a hex string.\n"
	"\t- empty(0) : equivalent to init of all zeros.\n"
"\n"
	"\t- show(ls) : display the current graph as an adjacency list. (spaces are the value 0)\n"
	"\t- show_xfg(x) : display the current graph as an xfg specialized adjacency list. (only 1, 2, 3, 5, 6, E, C.)\n"
	"\t- show_blacklist(sb) : display the current list of disallowed (reducable) edges. these won't be generated by generate().\n"
	"\t- show_origin(so) : show which instruction execution is starting at.\n"
	"\t- show_limit(sl) : show the execution limit which the search function uses to stop trying an option.\n"
"\n"
	"\t- edit(e) <edge> : add, remove or change an edge in the graph. \n"
	"\t\t edge format: {source: hex}{false_destination: hex}{true_destination: hex}\n"
	"\t- edit_origin(eo) <hex>: edit which instruction execute should start at.\n"
	"\t- edit_limit(el) <nat>: edit the execution limit, the number of instructions to try per option, before giving up.\n"
"\n"	
	"\t- run(r) <m> <n> : run the current graph, using the full UA ISA and DS.\n"
	"\t- search(s) : search over all possible extensions of the current partial graph starting execution from origin. 0's represent unknowns to be searched over.\n"
	"\t- visualize(v) <file> <width> <height>: display the lifetimes of a set of given z values from a file in order, to manually inspect them.\n"
	"\t- generate(g) <z_list_file> <dest_image_dir> <begin_cell> <end_cell> <begin_ts> <end_ts> <maximum_whitepoint>: generate a set of images for the lifetimes of a set of given z values from a file in order, to manually inspect them, in a better visualization medium.\n"
"\n"
	"\t- rmtrc+ft6(~) : print the z value for the RMTRC (revised mode trichotomy correspondence) partial graph, including the Ft6 constraint.\n"
	"\t- datetime(dt) : print the current time and date.\n"
	"\t- clear(o) : clear the screen.\n"
	"\t- help(?) : this help menu.\n"
	"\t- quit(q) : quit the XFG utility.\n"

"\n");
}



int main() {

	printf("this is a program to help with finding the XFG, in the UA theory.\ntype help for more info.\n");

	byte origin = 0x3;
	nat execution_limit = 3000;
	byte* graph = calloc(32, 1);

	char buffer[4096] = {0};

	printf("pre-generating blacklist... ");
	generate_blacklist();
	printf("using %llu blacklist edges.\n", blacklist_count);

	while (1) {
		printf(":: ");
		fgets(buffer, sizeof buffer, stdin);
		const char* command[8] = {0};
		parse_command(command, buffer);

		if (is(command, "", "")) {}
		else if (is(command, "quit", "q")) break;

		// else if (is(command, "debug_command", "dc")) {
		// 	printf("\n{ \n\t");
		// 	for (nat a = 0; a < 8; a++) {
		// 		printf("[%llu]:\"%s\", ", a, command[a]);
		// 	}
		// 	printf("\n}\n\n");
		// }

		else if (is(command, "help", "?")) print_help_menu();
		else if (is(command, "clear", "o")) clear_screen();
		else if (is(command, "datetime", "dt")) print_datetime();
		else if (is(command, "rmtrc+ft6", "~")) printf("\n\t000000C0000000000000000050003006\n\n");

		else if (is(command, "init", "i")) initialize_graph_from_string(graph, command[1]);
		else if (is(command, "empty", "0")) memset(graph, 0, 32);
		else if (is(command, "dump", "d")) printf("graph: %s\n", hex_string(graph));
		
		else if (is(command, "show", "ls"))  print_as_adjacency_list(graph);
		else if (is(command, "show_xfg", "x"))  print_as_xfg_adjacency_list(graph);
		else if (is(command, "show_limit", "sl")) printf("\n\texecution limit = %llu\n\n", execution_limit);
		else if (is(command, "show_origin", "so"))  printf("\n\torigin instruction = %hhX\n\n", origin);
		else if (is(command, "show_blacklist", "sb")) print_blacklist(); 

		else if (is(command, "edit", "e")) read_graph_edit(graph, command[1]);
		else if (is(command, "edit_origin", "eo")) origin = read_origin(command[1][0]);
		else if (is(command, "edit_limit", "el")) execution_limit = (nat) atoi(command[1]);
		
		else if (is(command, "run", "r")) run((nat)atoi(command[1]), (nat)atoi(command[2]), graph, origin);
		else if (is(command, "search", "s")) search(graph, origin, execution_limit);

		else if (is(command, "visualize", "v")) 
			visualize_set(
				command[1], 
				graph, origin, 
				(nat)atoi(command[2]), (nat)atoi(command[3])
			);

		else if (is(command, "generate", "g")) 
			generate_lifetime_images(
				command[1], command[2], 
				graph, origin, 
				(nat)atoi(command[3]), (nat)atoi(command[4]), 
				(nat)atoi(command[5]), (nat)atoi(command[6]),
				(nat)atoi(command[7])
			);

		else printf("error: unknown command.\n");
	}
	printf("quitting utility...\n");
}













// static void test_reduc_finder() {

	// fact: we want to find reduc edges which go on true (or false) for all inputs. --->   thats what it means to "reduce". 

	// we need to look at single edges reducing, given several possible previously executed instructions. 


	// these histories will never contain a cycle, i think..?? yikes...

	

		// so i just found out that    actually     we need to look at the +    the =    and the minus. 


//			ie, we need to stress test the branches in every possible way, without looking at "particular" values. 


						// ie, we need to make a system which can kind of fuzzily run the branches,  and see if they reduce.  note: this only applies to branches. not operations. 


			// in order to prove that operations are not allowed,   i think we just need a blacklist.   so yeah. we need a combination of these two approaches. 


					// this only works for branches. 












// trichotomy reducability principle:


///             ------>     these are still easily possible to implement, because they are technically just a direct branch. very easy to implement in the new system. 

	// "At4D", "Dt4A",
	// "Et4F", "Ft4E",


///           --->  these are all still possible to implement easily. 

	// "At2D", "Af2D",
	// "Dt1A", "Df1A",

	// "Et2F", "Ef2F",
	// "Ft3E", "Ff3E",
	

///     --> TODO: NOTE: i need to add 4 more:       Et3F      Ft2E       and also               Dt2A   At1D


// negative reset destination reducability principle:

///           --->  these are all still possible to implement easily. 


	// "Ft5A", "Ff5A",
	// "Et5A", "Ef5A",
	// "Dt5A", "Df5A",

	// "Ft5E", "Ff5E",
	// "At5E", "Af5E",
	// "Dt5E", "Df5E",

	// "Ft6D","Ff6D",
	// "At6D","Af6D",
	// "Et6D","Ef6D",

	// "Dt6F","Df6F",
	// "At6F","Af6F",
	// "Et6F","Ef6F",


// 0};
/*


	E :   *n < *i

	F :   *i < *n

	5 :   i = 0      

	6 :   *n = 0







x	A :   *n < *0

x	D :   *0 < *n



*/

