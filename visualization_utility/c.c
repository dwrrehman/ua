// a visualization utility for looking at a z list, and using the binary lifetime or unary array state viz methods on it. 
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef uint8_t byte;
typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;

static const byte D = 1;        // the duplication count (operation_count = 5 + D)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#include <SDL.h>
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"



enum operations { one, two, three, five, six };


static const bool should_deduplicate_z_list = true;


static const int delay_ms_per_frame = 16;
static const int display_rate = 1;

static const int default_window_size_width = 800;
static const int default_window_size_height = 800;

static const nat fea_execution_limit = 5000;
static const nat execution_limit = 1000000;
static const nat array_size = 60000;
static const nat lifetime_length = 100000;
static const nat pre_run_ins = 0;

enum pruning_metrics {
	z_is_good, PM_ga, PM_fea, PM_ns0, 
	PM_pco, PM_zr5, PM_zr6, PM_ndi, 
	PM_oer, PM_r0i, PM_h, PM_f1e, 
	PM_erc, PM_rmv, PM_ot, PM_csm, 
	PM_mm, PM_snm, 
	PM_count
};

static const char* pm_spelling[] = {
	"z_is_good", "PM_ga", "PM_fea", "PM_ns0", 
	"PM_pco", "PM_zr5", "PM_zr6", "PM_ndi", 
	"PM_oer", "PM_r0i", "PM_h", "PM_f1e", 
	"PM_erc", "PM_rmv", "PM_ot", "PM_csm", 
	"PM_mm", "PM_snm"
};

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static const nat max_acceptable_er_repetions = 50;
static const nat max_acceptable_modnat_repetions = 15;
static const nat max_acceptable_consecutive_s0_incr = 30;
static const nat max_acceptable_run_length = 9;
static const nat max_acceptable_consequtive_small_modnats = 200;

static const nat expansion_check_timestep = 5000;
static const nat required_er_count = 25;

static const nat expansion_check_timestep2 = 10000;
static const nat required_s0_increments = 5;

static nat* array = NULL;
static nat* timeout = NULL;
static byte* graph = NULL;

struct z_value {
	uint32_t* lifetime;
	byte* value;
	nat origin;
	nat unique;
};


static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}

static void print_bytes(byte* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) printf("%2hhu ", v[i]);
	printf("]");
}


static void init_graph_from_string(const char* string) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}




static void set_graph(byte* z) { memcpy(graph, z, graph_count); }
static void print_graph(void) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }


static nat execute_graph_starting_at(byte origin) {

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
				if (array[i] < required_s0_increments) return PM_f1e; 
			}
		}

		if (e == expansion_check_timestep)  { 
			if (er_count < required_er_count) return PM_erc; 
		}
		
		const byte I = ip * 4, op = graph[I];

		for (nat i = 0; i < operation_count; i++) {
			if (timeout[i] >= execution_limit >> 1) return PM_ot; 
			timeout[i]++;
		}
		timeout[ip] = 0;

		if (op == one) {
			if (pointer == n) return PM_fea; 
			if (not array[pointer]) return PM_ns0; 

			if (last_mcal_op == one) H_counter = 0;
			if (last_mcal_op == five) R0I_counter = 0;

			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return PM_pco; 
			if (not pointer) return PM_zr5; 
			
			if (	pointer == OER_er_at or 
				pointer == OER_er_at + 1) OER_counter++;
			else { OER_er_at = pointer; OER_counter = 0; }
			if (OER_counter >= max_acceptable_er_repetions) return PM_oer; 
			
			CSM_counter = 0;
			RMV_value = (nat) -1;
			RMV_counter = 0;
			for (nat i = 0; i < xw; i++) {
				if (array[i] < 6) CSM_counter++; else CSM_counter = 0;
				if (CSM_counter > max_acceptable_consequtive_small_modnats) return PM_csm; 
				if (array[i] == RMV_value) RMV_counter++; else { RMV_value = array[i]; RMV_counter = 0; }
				if (RMV_counter >= max_acceptable_modnat_repetions) return PM_rmv; 
			}

			pointer = 0;
			er_count++;
		}

		else if (op == two) {
			array[n]++;
			if (array[n] >= 65535) return PM_snm; 
		}
		else if (op == six) {  
			if (not array[n]) return PM_zr6; 
			array[n] = 0;   
		}
		else if (op == three) {
			if (last_mcal_op == three) return PM_ndi; 

			if (last_mcal_op == one) {
				H_counter++;
				if (H_counter >= max_acceptable_run_length) return PM_h; 
			}

			if (last_mcal_op == five) {
				R0I_counter++; 
				if (R0I_counter >= max_acceptable_consecutive_s0_incr) return PM_r0i; 
			}

			if (array[pointer] >= 65535) return PM_mm; 
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good;
}

static nat fea_execute_graph_starting_at(byte origin) {

	const nat n = 5;
	array[n] = 0; 
	array[0] = 0; 

	byte ip = origin, last_mcal_op = 0;
	nat pointer = 0, e = 0, xw = 0;

	for (; e < fea_execution_limit; e++) {

		const byte I = ip * 4, op = graph[I];

		if (op == one) {
			if (pointer == n) return PM_fea;
			if (not array[pointer]) return PM_ns0;
			pointer++;

			if (pointer > xw and pointer < n) { 
				xw = pointer; 
				array[pointer] = 0; 
			}
		}

		else if (op == five) {
			if (last_mcal_op != three) return PM_pco;
			if (not pointer) return PM_zr5;
			pointer = 0;
		}

		else if (op == two) { array[n]++; }
		else if (op == six) {  
			if (not array[n]) return PM_zr6;
			array[n] = 0;   
		}

		else if (op == three) {
			if (last_mcal_op == three) return PM_ndi;
			array[pointer]++;
		}

		if (op == three or op == one or op == five) last_mcal_op = op;

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
	return z_is_good; 
}


/*
static nat fea_execute_graph(void) {
	nat pm = 0;
	for (byte o = 0; o < operation_count; o++) {
		if (graph[4 * o] != three) continue;
		pm = fea_execute_graph_starting_at(o);
		if (not pm) return z_is_good;
	}
	return pm;
}
*/



static nat graph_analysis(void) {

	u16 was_utilized = 0;
	nat a = 0;
	byte previous_op = graph[20];     // make this not use this temporary variable, by using   index and index + 4   
	for (byte index = 20; index < graph_count; index += 4) {// (except if index+4==graphcount, then we will  just say its index.. yeah)
		const byte op = graph[index];
		if (previous_op > op) { a = index; goto bad; }
		previous_op = op;
	}
	// constructive GA here

	for (byte index = operation_count; index--;) {
		if (graph[4 * index + 3] == index) {  a = 4 * index + 3; goto bad; }
		if (graph[4 * index] == one   and graph[4 * index + 2] == index) {  a = 4 * index; goto bad; } 			     
		if (graph[4 * index] == six   and graph[4 * index + 2])          {  a = 4 * index; goto bad; }
		if (graph[4 * index] == two   and graph[4 * index + 2] == index) {  a = 4 * index + 2 * (index == two); goto bad; }  
		if (graph[4 * index] == three and graph[4 * index + 1] == index) {  a = 4 * index + 1 * (index == three); goto bad; }

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
		if (not ((was_utilized >> index) & 1)) goto bad;

	return z_is_good;
bad:	return PM_ga;
}

static nat graph_at_origin_was_pruned_by(byte origin) {
	nat pm = graph_analysis();     				if (pm) return pm;
	    pm = fea_execute_graph_starting_at(origin);  	if (pm) return pm;
	    pm = execute_graph_starting_at(origin);      	return pm;
}


static void print_z_list(struct z_value* list, nat count) {
	printf("printing z list: (%llu z values): \n", count);
	for (nat i = 0; i < count; i++) {
		printf("z #%llu: ", i);
		print_bytes(list[i].value, graph_count);
		printf(", origin = %llu, lifetime = %p\n", list[i].origin, (void*) list[i].lifetime);
	}
	
}


static void print_unique_list(struct z_value* list, nat count) {
	nat unique_count = 0;
	printf("printing unique z list: \n");
	for (nat i = 0; i < count; i++) {
		if (not list[i].unique) continue;
		printf("z #%llu: ", i);
		print_bytes(list[i].value, graph_count);
		printf(", origin = %llu, lifetime = %p\n", list[i].origin, (void*) list[i].lifetime);
		unique_count++;
	}
	printf("printed %llu unique z values.\n", unique_count);
}

static struct z_value* generate_good_origins_z_list(struct z_value* list, nat count, nat* out_new_count) {

	//puts("generate_good_origins_z_list: origin-pruning this list: ");
	//print_z_list(list, count);
	//getchar();

	struct z_value* new = NULL;
	nat new_count = 0;

	//puts("--------------------- generating new z list...---------------------");
	
	for (nat i = 0; i < count; i++) {
		//printf("trying z value index %llu... ", i);
		for (byte o = 0; o < operation_count; o++) {
			if (graph[4 * o] != three) continue;
			//printf(", [trying origin=%hhu]\n", o);
		
			set_graph(list[i].value);
			const nat pm = graph_at_origin_was_pruned_by(o);

			if (not pm) {
				list[i].origin = o;
				list[i].unique = true;

				printf("generating z #%llu: ", new_count);
				print_bytes(list[i].value, graph_count);
				printf(", origin = %llu, lifetime = %p\n", list[i].origin, (void*) list[i].lifetime);

				new = realloc(new, sizeof(struct z_value) * (new_count + 1)); 
				new[new_count++] = list[i];
			} else {
				printf("pruned z #%llu using pm = %s...\n", i, pm_spelling[pm]);
			}
		}
	}

	*out_new_count = new_count;
	return new;
}

static void generate_lifetime(struct z_value* z) {
	const nat n = array_size;
	const nat width = n + 1;
	nat pointer = 0;
	nat timestep = 1;
	byte ip = (byte) z->origin;

	memset(array, 0, sizeof(nat) * (array_size + 1));
	set_graph(z->value);

	printf("info: generating lifetime for origin = %hhu, z = ", ip);
	print_graph();

	z->lifetime = calloc(width * lifetime_length, 4);

	while (1) {
		const byte I = ip * 4, op = graph[I];
		if (op == one) { 
			if (pointer == n) { puts("fea pointer overflow"); abort(); } 
			pointer++; 

		} else if (op == five) {
			timestep++;
			if (timestep >= lifetime_length) return;
			pointer = 0;
		}
		else if (op == two) { array[n]++; z->lifetime[width * timestep + n] = (uint32_t) ~0; }

		else if (op == six) { array[n] = 0; }

		else if (op == three) { array[pointer]++; z->lifetime[width * timestep + pointer] = (uint32_t) ~0; }

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}
}





static struct z_value* load_zlist(const char* filename, nat* list_count) {
	FILE* file = fopen(filename, "r");
	if (not file) { perror("fopen"); exit(1); }
	
	struct z_value* list = NULL;
	nat count = 0;

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {
		char* index = strchr(buffer, ' ');
		if (not index) abort();
		buffer[index - buffer] = 0;
		if (graph_count != strlen(buffer)) { puts("graph count or duplication_count mismatch!"); abort(); }
		init_graph_from_string(buffer);
		byte* g = calloc(graph_count, 1);
		memcpy(g, graph, graph_count);
		list = realloc(list, sizeof(struct z_value) * (count + 1));
		list[count++] = (struct z_value) {.value = g};

	}
	fclose(file);
	*list_count = count;
	return list;
}


int main(int argc, const char** argv) {

	array = calloc(array_size + 1, sizeof(nat));
	timeout = calloc(operation_count, sizeof(nat));
	graph = calloc(graph_count, 1);

	printf("using [D=%hhu]\n", D);

	if (argc < 2) return puts("give input z list filename!");

	nat raw_count = 0;
	struct z_value* raw_list = load_zlist(argv[1], &raw_count);

	nat count = 0;
	struct z_value* list = generate_good_origins_z_list(raw_list, raw_count, &count);

	for (nat i = 0; i < count; i++) {
		generate_lifetime(list + i);
	}

	printf("loading lifetime data for zlist...\n");
	print_z_list(list, count);



	//de-duplication of lifetime images:


	if (should_deduplicate_z_list) {

		nat* equivalent_count = calloc(count, sizeof(nat));
		nat* equivalent_z = calloc(count * count, sizeof(nat));
		
		
		const nat lifetime_byte_count = 4 * ((array_size + 1) * (lifetime_length));

		puts("finding all equivalent lifetimes...");


		for (nat i = 0; i < count; i++) { 
			for (nat j = 0; j < count; j++) {
				if (i == j) continue;
				printf("testing i=%llu and j=%llu...\n", i, j);

				if (not memcmp(list[i].lifetime, list[j].lifetime, lifetime_byte_count)) {
					equivalent_z[count * i + equivalent_count[i]] = j;
					equivalent_count[i]++;
				}
			}
		}


		puts("list has these empirical lifetime equivalencies: ");
		for (nat i = 0; i < count; i++) {
			printf("%llu: ", i);
			for (nat j = 0; j < equivalent_count[i]; j++) 
				printf(" %llu ", equivalent_z[count * i + j]);
			puts("");
		}





		byte* seen = calloc(count, 1);
		
		for (nat i = 0; i < count; i++) {
			if (seen[i]) continue;

			for (nat j = 0; j < equivalent_count[i]; j++) {
				const nat index = equivalent_z[count * i + j];
				seen[index] = true;
			}
		}



		for (nat i = 0; i < count; i++) {
			list[i].unique = not seen[i];
		}

		print_unique_list(list, count);



		puts("dedeuplicating z list...");
		nat unique_count = 0;
		for (nat i = 0; i < count; i++) {
			if (list[i].unique) unique_count++;
			else memmove(list + i, list + i + 1, count - i - 1);
		}
		count = unique_count;
		


	}








	size_t height = default_window_size_height >> 1, width = default_window_size_width >> 1;
	size_t screen_size = height * width * 4;

	if (SDL_Init(SDL_INIT_VIDEO)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));
	SDL_Window *window = SDL_CreateWindow("visualization utility", SDL_WINDOWPOS_CENTERED, 
				SDL_WINDOWPOS_CENTERED, default_window_size_width, default_window_size_height, 
				SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, (int) width, (int) height);

	uint32_t* screen = calloc(screen_size, 1);
	uint32_t* pixels = NULL;
	int pitch = 0;

	bool quit = false, fullscreen = false;
	nat counter = 0, speed = 1;
	nat current = 0, initial_y = 0, initial_x = 0;


	while (not quit) {
		uint32_t start = SDL_GetTicks();

		if (not (counter & ((1 << display_rate) - 1))) {

			if (lifetime_length < height) abort();
			if (array_size + 1 < width) abort();

			if ((int64_t) initial_y > (int64_t) lifetime_length - (int64_t) height) initial_y = lifetime_length - height;
			if ((int64_t) initial_x > (int64_t) array_size + 1 - (int64_t) width) initial_x = array_size + 1 - width;

			nat h_l = initial_y;
			for (nat h = 0; h < height; h++) {
				nat w_l = initial_x;
				for (nat w = 0; w < width; w++) {
					const nat lifetime_width = array_size + 1;
					screen[width * h + w] = list[current].lifetime[lifetime_width * h_l + w_l];
					w_l++;
				}
				h_l++;
			}



			const double ratio = (double) current / (double) count;
			const nat progress = (nat) (ratio * (double) width);

			for (nat i = 0; i < progress; i++) {
				screen[i] = (uint32_t) ~0;
			}
			for (nat i = 0; i < width - progress; i++) {
				screen[progress + i] = 0;
			}
		}

		SDL_LockTexture(texture, NULL, (void**) &pixels, &pitch);
		memcpy(pixels, screen, screen_size);
		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;

			if (event.type == SDL_MOUSEWHEEL) {

				const bool command = !!key[SDL_SCANCODE_LGUI];

				if (not command) {
				
					if (event.wheel.y < 0) { 
						if ((int64_t) initial_y + (int64_t) speed < (int64_t) lifetime_length - (int64_t) height) initial_y += speed; 
						else initial_y = lifetime_length - height;
					}

					if (event.wheel.y > 0) { 
						if (initial_y >= speed) initial_y -= speed; else initial_y = 0;
					}
		
					if (event.wheel.x > 0) { 
						if ((int64_t) initial_x + (int64_t) speed < (int64_t) array_size + 1 - (int64_t) width) initial_x += speed; 
						else initial_x = array_size + 1 - width;
					}

					if (event.wheel.x < 0) { 
						if (initial_x >= speed) initial_x -= speed; else initial_x = 0;
					}
				}
	
				if (command and event.wheel.y > 0) {
					if (width > speed and height > speed) { 
						width -= speed; 
						height -= speed; 
						goto resize1; 
					}
				} 


				if (command and event.wheel.y < 0) {

					if (width >= array_size + 1)   goto done_resizing;
					if (height >= lifetime_length) goto done_resizing;

					width += speed; 
					height += speed;

				resize1: 
					screen_size = width * height * 4;
					screen = realloc(screen, screen_size);
					memset(screen, 0x00, screen_size);
					SDL_DestroyTexture(texture);

					texture = SDL_CreateTexture(renderer, 
							SDL_PIXELFORMAT_ARGB8888, 
							SDL_TEXTUREACCESS_STREAMING, 
							(int) width, (int) height);

				done_resizing:
					printf("width = %lu, height = %lu\n", width, height);
				}
			} 

			if (event.type == SDL_MOUSEBUTTONDOWN) printf("user clicked!\n");

			if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_GRAVE]) SDL_SetWindowFullscreen(window, (fullscreen = !fullscreen) ? SDL_WINDOW_FULLSCREEN : 0);
				if (key[SDL_SCANCODE_0] or key[SDL_SCANCODE_ESCAPE] or key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_1]) { initial_x = 0; initial_y = 0; }
				if (key[SDL_SCANCODE_2]) {}

				if (key[SDL_SCANCODE_F]) { if (current < count - 1) current++; printf("current is now %llu.\n", current); } 
				if (key[SDL_SCANCODE_A]) { if (current) current--; printf("current is now %llu.\n", current); } 

				if (key[SDL_SCANCODE_K]) { if (speed > 1) speed >>= 1; printf("speed = %llu\n", speed); }
				if (key[SDL_SCANCODE_L]) { speed <<= 1; printf("speed = %llu\n", speed); }

				if (key[SDL_SCANCODE_Z]) {
					printf("current displaying: origin = %llu,  ", list[current].origin);
					set_graph(list[current].value);
					print_graph();
					puts("");
				}

			}
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = delay_ms_per_frame - (int32_t) time; 
		if (sleep > 0) SDL_Delay((uint32_t) sleep);
		counter++;
	
		if (not (counter & ((1 << 6) - 1))) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %.5lf\n", fps);
		}
	}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}






























/*
	0:  5 8 13 45
	1:  31 41 78 105
	2:  3
	3:  2
	4:  6 7 8 9
	5:  0 

*/











/*

if (key[SDL_SCANCODE_E]) { if (width and height) { width--; height--; goto resize; } }
				if (key[SDL_SCANCODE_G]) {
					width++; height++;
				resize:	screen_size = width * height * 4;
					screen = realloc(screen, screen_size);
					memset(screen, 0x00, screen_size);
					SDL_DestroyTexture(texture);
					texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, (int) width, (int) height);
				}

*/

















/*



	size_t timestep = 0, cell = 0;
	bool state = 0;


			if (not state) cell++; else cell--;
			timestep++;
			cell = (cell + width) % width;
			timestep = (timestep + height) % height;
			screen[width * timestep + cell] = (uint32_t) ~0;












		the idea is to send a resize command byte, and then a 5 byte packet of    { continutation_byte, w=u16, h=u16 }.

			thats it!
		just make sure that when we receive the 5 byte packet, that it says a continutation byte,    to verify its correct. 

		also make sure that the command receiver ignores the continutation byte packets. 

			thaats it!





s
*/









// ------------------------------------------------ work in progress:---------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static inline void zoom_in(SDL_Renderer* renderer) {
// 	int future_scaled_width = (int) ((float)window_width * (scale - 0.005f));
// 	int future_scaled_height = (int) ((float)window_height * (scale - 0.005f));
// 	if (not future_scaled_width or not future_scaled_height) return;
// 	scale -= 0.001f;
// 	rescale(renderer);
// }

// static inline void zoom_out(SDL_Renderer* renderer) {
// 	if (scale >= 0.99f) return;
// 	scale += 0.001f;
// 	rescale(renderer);
// }



// static inline void send_resize_command(int connection) {
// 	u8 command = view_resized, response = 0;
// 	write(connection, &command, 1);
// 	write(connection, &scaled_width, 2);
// 	write(connection, &scaled_height, 2);
// 	ssize_t n = read(connection, &response, 1); 
// 	check(n); if (response != 1) not_acked();
// }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




// static void interrupt_kill(int __attribute__((unused)) _) {
// 	kill(getpid(), SIGINT);
// }	




















/*


// #define reset "\x1B[0m"
 // #define red   "\x1B[31m"
#define green   "\x1B[32m"
#define yellow   "\x1B[33m"
#define bold    "\033[1m"

// #define lightblue "\033[38;5;67m"
// #define cyan     "\x1B[36m"
//#define blue   "\x1B[34m"




*/



		// SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  //   		SDL_RenderClear(renderer);

		// // SDL_SetRenderDrawColor(renderer, 67, 45, 234, 255);
		// // SDL_RenderDrawPoint(renderer, width_radius, height_radius);

		// SDL_SetRenderDrawColor(renderer, colors[3 + 0], colors[3 + 1], colors[3 + 2], 255);
		// for (u32 i = 0; i < screen_block_count; i += 2) {
		// 	SDL_RenderDrawPoint(renderer, screen[i], screen[i + 1]);
		// }





 // , SDL_Renderer* renderer, SDL_Texture** texture
// SDL_DestroyTexture(*texture);
	// *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);


// static inline void toggle_fullscreen(SDL_Window* window) { // , SDL_Renderer* renderer, SDL_Texture** texture
	
// }




// printf("width = %llu, timestep = %llu, n = %llu\n", width, timestep, n);fflush(stdout);

