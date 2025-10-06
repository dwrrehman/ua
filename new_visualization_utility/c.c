// a visualization utility for looking at a z list, and using the binary lifetime or unary array state viz methods on it. 
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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

static const byte D = 2;        // the duplication count (operation_count = 5 + D)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#include <SDL.h>
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"

static const bool should_deduplicate_z_list = true;

static const nat execution_limit  = (nat) -1;
static const nat pre_run_duration = 1000000000;

static const nat array_size = 16384; // (must be divisible by 8)
static const nat lifetime_length = 16384;

static const nat generating_display_rate = 1;







enum operations { one, two, three, five, six };

static const size_t max_height = 4096, max_width = 4096;
static const int default_window_size_width = 1000;
static const int default_window_size_height = 1000;

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static nat* erp_tallys = NULL;
static nat* array = NULL;

struct z_value {
	byte** lifetime;
	byte* value;
	nat origin;
	nat unique;
	nat xw;
};

static void print_graph_raw(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); }
static void print_graph(byte* graph) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); puts(""); }

static void print_bytes(byte* v, nat l) {
	for (nat i = 0; i < l; i++) printf("%hhu", v[i]);
}

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

static void append_to_file(char* filename, size_t filename_size, byte* graph, nat origin, nat score) {
	
	char z[64] = {0};    get_graphs_z_value(z, graph); 
	char o[16] = {0};    snprintf(o, sizeof o, "%hhu", (byte) origin);
	char dt[32] = {0};   get_datetime(dt); 
	char sc[32] = {0};   snprintf(sc, sizeof sc, "%llu", score);
	
	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;

try_open:;
	const int file = open(filename, flags, permissions);
	if (file < 0) {
		if (permissions) {
			perror("create openat file");
			printf("[%s]: [z=%s]: failed to create filename = \"%s\"\n", dt, z, filename);
			fflush(stdout);
			abort();		}
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
	write(file, sc, strlen(sc));
	write(file, "\n", 1);
	close(file);

	//printf("[%s]: write: %s z = %s to file \"%s\"\n",
	//	dt, permissions ? "created" : "wrote", z, filename
	//);
}

static byte* graph = NULL;

static void init_graph_from_string(const char* string) {
	for (byte i = 0; i < graph_count; i++) 
		graph[i] = (byte) (string[i] - '0');
}

static void set_graph(byte* z) { memcpy(graph, z, graph_count); }

static void print_z_list(struct z_value* list, nat count) {
	printf("printing z list: (%llu z values): \n", count);
	for (nat i = 0; i < count; i++) {
		printf("z #%llu: ", i);
		print_bytes(list[i].value, graph_count);
		printf(", origin = %llu, lifetime = %p, xw = %llu\n", list[i].origin, (void*) list[i].lifetime, list[i].xw);
	}	
}

/*



----------------------------------------------------------

	uint32_t* array = calloc(count, sizeof(uint32_t));

	array[i] = 1;
	array[i] = 0;

	if (array[i]) { ... } 

----------------------------------------------------------


	byte* array = calloc(count / 8 + 1, 1);

	array[i / 8] |= (1 << (i % 8));       // array[i] = 1;
	array[i / 8] &= ~(1 << (i % 8));       // array[i] = 0;

	if ((array[i / 8] >> (i % 8)) & 1) { ... }    // if (array[i]) { ... }
	
*/




static void generate_lifetime(struct z_value* z) {
	const nat n = array_size;
	const nat width = n + 1;
	nat pointer = 0;
	nat timestep = 1;
	byte ip = (byte) z->origin;

	memset(array, 0, sizeof(nat) * (array_size + 1));
	set_graph(z->value);
	nat xw = 0;

	z->lifetime = calloc(4, sizeof(byte*));
	z->lifetime[0] = calloc(width * lifetime_length / 8 + 1, 1);
	z->lifetime[1] = calloc(width * lifetime_length / 8 + 1, 1);
	z->lifetime[2] = calloc(width * lifetime_length / 8 + 1, 1);
	z->lifetime[3] = calloc(width * lifetime_length / 8 + 1, 1);

	memset(erp_tallys, 0, sizeof(nat) * (array_size + 1));

	for (nat e = 0; e < execution_limit; e++) {
		const byte I = ip * 4, op = graph[I];

		if (op == one) { 
			if (pointer == n) { goto done; }  //  puts("fea pointer overflow"); 
			pointer++; 
			if (pointer > xw) xw = pointer;

		} else if (op == five) {
			if (e >= pre_run_duration) {
				timestep++;
				if (timestep >= lifetime_length) break;
			}

			erp_tallys[pointer]++;
			pointer = 0;
		}
		else if (op == two) { array[n]++; }

		else if (op == six) { array[n] = 0; }

		else if (op == three) { 
			array[pointer]++; 
			if (e >= pre_run_duration) { 
				const nat i = width * timestep + pointer;
				z->lifetime[0][i / 8] |= (1 << (i % 8));
			}
		}

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}

done:
	z->xw = xw;
	for (nat h = 0; h < n + 1 and h < lifetime_length - 1; h++) {
		if (h == xw + 1) break;		
		for (nat w = 0; w < width; w++) {
			if (w < array[h]) {
				const nat i = width * (h + 1) + w;
				z->lifetime[1][i / 8] |= (1 << (i % 8));
			}
			if (w < erp_tallys[h]) {
				const nat i = width * (h + 1) + w;
				z->lifetime[2][i / 8] |= (1 << (i % 8));
			}
		}
	}			
}

static struct z_value* load_zlist(const char* filename, nat* list_count) {
	FILE* file = fopen(filename, "r");
	if (not file) { perror("fopen"); exit(1); }
	
	struct z_value* list = NULL;
	nat count = 0;

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof buffer, file)) {
		char* zend = strchr(buffer, ' ');
		if (not zend) { puts("zend: could not z value..."); abort(); }
		buffer[zend - buffer] = 0;
		if (graph_count != strlen(buffer)) { puts("graph count or duplication_count mismatch!"); abort(); }
		init_graph_from_string(buffer);
		byte* g = calloc(graph_count, 1);
		memcpy(g, graph, graph_count);

		char* oend = strchr(zend + 1, ' ');
		if (not oend) { puts("oend: could not origin..."); abort(); }
		buffer[oend - buffer] = 0;
		const byte o = (byte) atoi(zend + 1);
		
		list = realloc(list, sizeof(struct z_value) * (count + 1));
		list[count++] = (struct z_value) {.value = g, .origin = o};
	}
	fclose(file);
	*list_count = count;
	return list;
}

static int comparison_function(const void* raw_a, const void* raw_b) {
	const struct z_value* a = (const struct z_value*) raw_a;
	const struct z_value* b = (const struct z_value*) raw_b;
	return (int) (a->xw - b->xw);

}

int main(int argc, const char** argv) {

	srand((unsigned) time(NULL));

	erp_tallys = calloc(array_size + 1, sizeof(nat));
	array = calloc(array_size + 1, sizeof(nat));
	graph = calloc(graph_count, 1);

	printf("using [D=%hhu]\n", D);

	if (argc < 2) return puts("give input z list filename!");

	nat count = 0;
	struct z_value* list = load_zlist(argv[1], &count);

	for (nat i = 0; i < count; i++) {
		if (i % generating_display_rate == 0) {
			printf("\r  %1.5lf [%3llu / %3llu] : [z = ", ((double)i) / ((double) count), i, count); 
			print_graph_raw(list[i].value); 
			printf(", o = %2llu] generating lifetime...", list[i].origin);
			fflush(stdout);
		} 
		generate_lifetime(list + i);
	}

	printf("loading lifetime data for zlist...\n");
	print_z_list(list, count);

	if (should_deduplicate_z_list) {

		nat* equivalent_count = calloc(count, sizeof(nat));
		nat* equivalent_z = calloc(count * count, sizeof(nat));
		
		const nat lifetime_byte_count = (array_size + 1) * lifetime_length / 8 + 1;

		puts("finding all equivalent lifetimes...");

		nat dupl_count = 0;
		nat* duplicates = NULL;

		puts("deduplicating...");
		const nat zvs_per_dot = count / 128 + 1;
		nat dot_counter = 0;
	
		for (nat i = 0; i < count; i++) { 

			if (dot_counter >= zvs_per_dot) {
				putchar('.'); fflush(stdout);
				dot_counter = 0; 
			} else dot_counter++;

			for (nat d = 0; d < dupl_count; d++) {
				if (duplicates[d] == i) goto next_i;
			}

			for (nat j = i + 1; j < count; j++) {
				// printf("testing i=%llu and j=%llu... ", i, j);

				if (not memcmp(list[i].lifetime[0], list[j].lifetime[0], lifetime_byte_count)) {
					equivalent_z[count * i + equivalent_count[i]] = j;
					equivalent_count[i]++;

					duplicates = realloc(duplicates, sizeof(nat) * (dupl_count + 1));
					duplicates[dupl_count++] = j;

					//printf("[%llu IS A DUPLICATE].\n", j);

				} else {
					//puts("different.");
				}
			}
			next_i: continue;
		}
		puts("");



#define high_profile_count_threshold 50
#define max_hp_count 10000

		nat hp_score[max_hp_count] = {0};
		nat hp_z[max_hp_count] = {0};
		nat hp_count = 0;

		puts("list has these empirical lifetime equivalencies: ");

		for (nat i = 0; i < count; i++) {
			printf("%llu: ", i);

			for (nat j = 0; j < equivalent_count[i]; j++) 
				printf(" %llu ", equivalent_z[count * i + j]);
			puts("");

			if (equivalent_count[i] >= high_profile_count_threshold) {
				if (hp_count < max_hp_count) { hp_z[hp_count] = i; hp_score[hp_count++] = equivalent_count[i]; }
			}
		}

		byte* seen = calloc(count, 1);
		for (nat i = 0; i < count; i++) {
			if (seen[i]) continue;
			for (nat j = 0; j < equivalent_count[i]; j++) {
				const nat index = equivalent_z[count * i + j];
				seen[index] = true;
			}
		}
		for (nat i = 0; i < count; i++) list[i].unique = not seen[i];

		puts("de-deuplicating z list...");
		nat unique_count = 0;
		for (nat i = 0; i < count; i++) {
			if (not list[i].unique) continue;
			list[unique_count++] = list[i];
		}
		count = unique_count;
		
		puts("--------------------- printing subset -------------------");
		puts("");


		char hp_filename[4096] = {0};

		for (nat i = 0; i < hp_count; i++) { 
			print_bytes(list[hp_z[i]].value, graph_count);
			printf(" %llu %llu\n", list[hp_z[i]].origin, hp_score[i]);
			append_to_file(hp_filename, sizeof hp_filename, list[hp_z[i]].value, list[hp_z[i]].origin, hp_score[i]);
		}
		puts("");
		
	}


	puts("sorting list...");

	qsort(list, count, sizeof(struct z_value), comparison_function);
	print_z_list(list, count);

	uint8_t* is_good = calloc(count, 1);
	
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
	nat speed = 64;
	nat current = 0, initial_y[4] = {0}, initial_x[4] = {0};

	int delay_ms_per_frame = 1, ers_per_frame = 1;

	nat timestep = 1;
	nat viz_method = 0;
	byte pc = 0;
	nat pointer = 0;

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		if (viz_method == 3) { 
		const nat n = array_size;
		//const nat array_width = n + 1;

		for (int _ = 0; _ < ers_per_frame; _++) {

			for (nat e = 0; e < execution_limit; e++) {
			const byte I = pc * 4, op = list[current].value[I];
	
			if (op == one) { 
				if (pointer == n) {
					pointer = 0;
					timestep = 1;
					pc = (byte) list[current].origin; 
					memset(array, 0, sizeof(nat) * (array_size + 1));
					memset(list[current].lifetime[3], 0, (array_size) * lifetime_length / 8 + 1);
					goto outside_of_loop;
				}
				pointer++; 
	
			} else if (op == five) {
				if (timestep < height - 1) timestep++;
				else {
					memmove(
						list[current].lifetime[3],
						list[current].lifetime[3] + (array_size / 8),
						(height - 1) * (array_size / 8)
					);

					memset(
						list[current].lifetime[3] + (height - 1) * (array_size / 8), 
						0, 
						array_size / 8
					);
				}
				pointer = 0;	
			}
			else if (op == two) array[n]++; 
			else if (op == six) array[n] = 0; 
			else if (op == three) { 
				array[pointer]++;
				const nat i = array_size * timestep + pointer;
				list[current].lifetime[3][i / 8] |= (1 << (i % 8));
			}
	
			byte state = 0;
			if (array[n] < array[pointer]) state = 1;
			if (array[n] > array[pointer]) state = 2;
			if (array[n] == array[pointer]) state = 3;
			pc = list[current].value[I + state];

			if (op == five) break;

		} } }  outside_of_loop:;
		
		if (lifetime_length < height or array_size + 1 < width) {
			height = default_window_size_height >> 1;
			width = default_window_size_width >> 1;
			while (width > 1 and height > 1 and (lifetime_length < height or array_size + 1 < width)) {
				width >>= 1;
				height >>= 1;
			}

			screen_size = width * height * 4;
			screen = realloc(screen, screen_size);
			memset(screen, 0x00, screen_size);
			SDL_DestroyTexture(texture);

			texture = SDL_CreateTexture(renderer, 
					SDL_PIXELFORMAT_ARGB8888, 
					SDL_TEXTUREACCESS_STREAMING, 
					(int) width, (int) height);
			printf("width = %lu, height = %lu\n", width, height);
		} 			

		if ((int64_t) initial_y[viz_method] > (int64_t) lifetime_length - (int64_t) height) initial_y[viz_method] = lifetime_length - height;
		if ((int64_t) initial_x[viz_method] > (int64_t) array_size + 1 - (int64_t) width) initial_x[viz_method] = array_size + 1 - width;

		nat h_l = initial_y[viz_method];
		for (nat h = 0; h < height; h++) {
			nat w_l = initial_x[viz_method];
			for (nat w = 0; w < width; w++) {

				const nat lifetime_width = array_size + (viz_method != 3);
				const nat i = lifetime_width * h_l + w_l;
				if ((list[current].lifetime[viz_method][i / 8] >> (i % 8)) & 1)
					screen[width * h + w] = 0xFFFFFFFF;
				else 	screen[width * h + w] = 0x00000000;

				if (w == 50 and is_good[current]) screen[width * h + w] = 0xFFFFFFFF;
				w_l++;
			}
			h_l++;
		}

		const double ratio = (double) current / (double) count;
		const nat progress = (nat) (ratio * (double) width);
		for (nat i = 0; i < progress; i++) screen[i] = (uint32_t) ~0;
		for (nat i = 0; i < width - progress; i++) screen[progress + i] = 0;	

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

				if (not command and event.wheel.y < 0) { 
					if ((int64_t) initial_y[viz_method] + (int64_t) speed < (int64_t) lifetime_length - (int64_t) height) initial_y[viz_method] += speed; 
					else initial_y[viz_method] = lifetime_length - height;
				}

				if (not command and event.wheel.y > 0) { 
					if (initial_y[viz_method] >= speed) initial_y[viz_method] -= speed; else initial_y[viz_method] = 0;
				}
	
				if (not command and event.wheel.x > 0) { 
					if ((int64_t) initial_x[viz_method] + (int64_t) speed < (int64_t) array_size + 1 - (int64_t) width) initial_x[viz_method] += speed; 
					else initial_x[viz_method] = array_size + 1 - width;
				}

				if (not command and event.wheel.x < 0) { 
					if (initial_x[viz_method] >= speed) initial_x[viz_method] -= speed; else initial_x[viz_method] = 0;
				}
	
				if (command and event.wheel.y > 0) {
					if (width > speed and height > speed) { 
						width -= speed; 
						height -= speed;

						if (viz_method == 3) { 
							timestep = 1;
							memset(list[current].lifetime[3], 0, (array_size) * lifetime_length / 8 + 1);
						}

						goto resize1; 
					}
				} 

				if (command and event.wheel.y < 0) {

					if (width >= array_size + 1)   goto done_resizing;
					if (height >= lifetime_length) goto done_resizing;

					width += speed; 
					height += speed;
					if (width > max_width) width = max_width;
					if (height > max_height) height = max_height;

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
				if (key[SDL_SCANCODE_ESCAPE] or key[SDL_SCANCODE_Q]) quit = true;

				if (key[SDL_SCANCODE_0]) { initial_x[viz_method] = 0; initial_y[viz_method] = 0; }
				if (key[SDL_SCANCODE_1]) viz_method = 0;
				if (key[SDL_SCANCODE_2]) viz_method = 1;
				if (key[SDL_SCANCODE_3]) viz_method = 2;

				if (key[SDL_SCANCODE_W]) { is_good[current] = 0; printf("MARKED zi#%llu and BAD\n", current); } 
				if (key[SDL_SCANCODE_E]) { is_good[current] = 1; printf("MARKED zi#%llu and GOOD\n", current); } 

				if (key[SDL_SCANCODE_X]) {					
					if (delay_ms_per_frame > 0) delay_ms_per_frame--; 
					else ers_per_frame++;
					printf("delay_ms_per_frame = %d, ers_per_frame = %d\n", delay_ms_per_frame, ers_per_frame); 
				}
				if (key[SDL_SCANCODE_C]) { 
					if (ers_per_frame > 1) ers_per_frame--;
					else delay_ms_per_frame++;
					printf("delay_ms_per_frame = %d, ers_per_frame = %d\n", delay_ms_per_frame, ers_per_frame); 
				}

				if (key[SDL_SCANCODE_F]) { 
					if (current < count - 1) current++;
					if (viz_method == 3) goto reset_dynamic_state;
				} 

				if (key[SDL_SCANCODE_A]) { 
					if (current) current--;
					if (viz_method == 3) goto reset_dynamic_state;
				}

				if (key[SDL_SCANCODE_K]) { if (speed > 1) speed >>= 1; printf("speed = %llu\n", speed); }
				if (key[SDL_SCANCODE_L]) { if (speed < 16 * 4096) speed <<= 1; printf("speed = %llu\n", speed); }

				if (key[SDL_SCANCODE_4]) {
					viz_method = 3;
					reset_dynamic_state: 
					pointer = 0;
					timestep = 1;
					pc = (byte) list[current].origin; 
					memset(array, 0, sizeof(nat) * (array_size + 1));
					memset(list[current].lifetime[3], 0, (array_size) * lifetime_length / 8 + 1);
				}

				if (key[SDL_SCANCODE_V]) {					
					if (viz_method == 3) { 
						timestep = 1;
						memset(list[current].lifetime[3], 0, (array_size) * lifetime_length / 8 + 1);
					}
				}

				if (key[SDL_SCANCODE_Z]) {
					printf("[index in list = %llu]: current displaying: origin = %llu,  ", current, list[current].origin);
					print_graph(list[current].value);
					puts("");
				}
			}
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = (viz_method == 3 ? delay_ms_per_frame : 16) - (int32_t) time; 
		if (sleep > 0) SDL_Delay((uint32_t) sleep);
		}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	const bool output_zlist_override = false;

	char good_zlist_filename[4096] = {0};
	nat good_count = 0;
	for (nat i = 0; i < count; i++) {
		if (is_good[i] or output_zlist_override) {
			append_to_file(
				good_zlist_filename, 
				sizeof good_zlist_filename, 
				list[i].value, list[i].origin, i
			); good_count++;
		}
	}

	printf("info: wrote good zlist of good_count = %llu : %s\n", good_count, good_zlist_filename);
}

































		//if (not (counter & ((1 << 6) - 1))) {
		//	//double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
		//	// printf("fps = %.5lf\n", fps);
		//}














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








/*static void print_z(struct z_value* list, nat i) {
	printf("z #%llu: ", i);
	print_bytes(list[i].value, graph_count);
	printf(", origin = %llu, lifetime = %p\n", list[i].origin, (void*) list[i].lifetime);
}*/











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
static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}
*/


















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












/*



		we can make the equivalence finder faster by leverging the transitive nature of equality!


		simply stop looking at zv which are already equiv to ones you've checked!

			only chechk against "currently still unique zv"


			yay





*/






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









/*
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

*/







	//puts("--------------------- generating new z list...---------------------");

	//puts("generate_good_origins_z_list: origin-pruning this list: ");
	//print_z_list(list, count);
	//getchar();






//printf(", [trying origin=%hhu]\n", o); //printf("trying z value index %llu... ", i);






/*

                                 v 

	[0]	[3]	[2]	[3]
         U	 U	 D	 U

                         ^






	[0] [3] 

*/







/*
##########################################     <--------- *0
##############################    <----- *1
##################### 
#######################
##################
########           <--- *5
###########
#########







202405013.180150:

	loooked back through the 2 space z value

	0122102521433062400106614201                     ie        0122 1025 2143 3062 4001 0661 4201


		its actually even better than the previous z values we were looking at!!!


			i want to actually write it out  on paper and see what the graph looks like!  like with nodes and arrows

				should be interesting!



				one interseting thing is that it does still     have a  DOL   that falls within spec of the NDH!!


									the nested DOL hypothesis!   


					its duplicating    {  0   4  }   



					which means that its a strict superset of 1 space! so NDH is still   possiblyyyyyyyy valid loll
								not definitely, just possibly. 


								lol 
					so yeah 



		





static void generate_lifetime(struct z_value* z) {
	const nat n = array_size;
	const nat width = n + 1;
	nat pointer = 0;
	nat timestep = 1;
	byte ip = (byte) z->origin;

	memset(array, 0, sizeof(nat) * (array_size + 1));
	set_graph(z->value);
	nat xw = 0;

	z->lifetime = calloc(3, sizeof(uint32_t*));
	z->lifetime[0] = calloc(width * lifetime_length, 4);
	z->lifetime[1] = calloc(width * lifetime_length, 4);
	z->lifetime[2] = calloc(width * lifetime_length, 4);

	memset(erp_tallys, 0, sizeof(nat) * (array_size + 1));

	for (nat e = 0; e < execution_limit; e++) {
		const byte I = ip * 4, op = graph[I];

		if (op == one) { 
			if (pointer == n) { goto done; }  //  puts("fea pointer overflow"); 
			pointer++; 
			if (pointer > xw) xw = pointer;

		} else if (op == five) {
			if (e >= pre_run_duration) {
				timestep++;
				if (timestep >= lifetime_length) break;
			}

			erp_tallys[pointer]++;
			pointer = 0;
		}
		else if (op == two) { array[n]++; }

		else if (op == six) { array[n] = 0; }

		else if (op == three) { 
			array[pointer]++; 
			if (e >= pre_run_duration) z->lifetime[0][width * timestep + pointer] = (uint32_t) ~0; 
		}

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}

done:
	z->xw = xw;
	for (nat h = 0; h < n + 1 and h < lifetime_length - 1; h++) {

		if (h == xw + 1) break;
		
		for (nat w = 0; w < width; w++) {

			if (w < array[h]) 
				z->lifetime[1][width * (h + 1) + w] = (uint32_t) ~0;

			if (w < erp_tallys[h]) 
				z->lifetime[2][width * (h + 1) + w] = (uint32_t) ~0;
		}
	}			
}



*/

























// 0122 1025 2143 3062 4001 0661 4201





//  todo: do something to make it so that the view can be larger than the image... somehow... (fill the larger image in  with zeros maybe?...)




