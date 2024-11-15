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

static const size_t max_height = 4096, max_width = 4096;
static const int delay_ms_per_frame = 16;
static const int display_rate = 1;

static const int default_window_size_width = 800;
static const int default_window_size_height = 800;

static const nat execution_limit = 400000;
static const nat array_size = 20000;
static const nat lifetime_length = 20000;

static const byte operation_count = 5 + D;
static const byte graph_count = 4 * operation_count;

static nat* array = NULL;
static byte* graph = NULL;

struct z_value {
	uint32_t** lifetime;
	byte* value;
	nat origin;
	nat unique;
};

/*
static void print_nats(nat* v, nat l) {
	printf("(%llu)[ ", l);
	for (nat i = 0; i < l; i++) {
		printf("%llu ", v[i]);
	}
	printf("]");
}
*/

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

static void print_z_list(struct z_value* list, nat count) {
	printf("printing z list: (%llu z values): \n", count);
	for (nat i = 0; i < count; i++) {
		printf("z #%llu: ", i);
		print_bytes(list[i].value, graph_count);
		printf(", origin = %llu, lifetime = %p\n", list[i].origin, (void*) list[i].lifetime);
	}	
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

	z->lifetime = calloc(2, sizeof(uint32_t*));
	z->lifetime[0] = calloc(width * lifetime_length, 4);
	z->lifetime[1] = calloc(width * lifetime_length, 4);

	for (nat e = 0; e < execution_limit; e++) {

		// printf("executing instructions...\n");

		const byte I = ip * 4, op = graph[I];

		if (op == one) { 
			if (pointer == n) { puts("fea pointer overflow"); abort(); } 
			pointer++; 

		} else if (op == five) {
			timestep++;
			if (timestep >= lifetime_length) break;
			pointer = 0;
		}
		else if (op == two) { array[n]++; }

		else if (op == six) { array[n] = 0; }

		else if (op == three) { array[pointer]++; z->lifetime[0][width * timestep + pointer] = (uint32_t) ~0; }

		byte state = 0;
		if (array[n] < array[pointer]) state = 1;
		if (array[n] > array[pointer]) state = 2;
		if (array[n] == array[pointer]) state = 3;
		ip = graph[I + state];
	}

	for (nat h = 0; h < n + 1 and h < lifetime_length - 1; h++) 
		for (nat w = 0; w < width; w++) 
			if (w < array[h]) z->lifetime[1][width * (h + 1) + w] = (uint32_t) ~0;
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


int main(int argc, const char** argv) {

	array = calloc(array_size + 1, sizeof(nat));
	graph = calloc(graph_count, 1);

	printf("using [D=%hhu]\n", D);

	if (argc < 2) return puts("give input z list filename!");

	nat count = 0;
	struct z_value* list = load_zlist(argv[1], &count);

	print_z_list(list, count);

	for (nat i = 0; i < count; i++)
		generate_lifetime(list + i);
	
	printf("loading lifetime data for zlist...\n");
	print_z_list(list, count);

	if (should_deduplicate_z_list) {

		nat* equivalent_count = calloc(count, sizeof(nat));
		nat* equivalent_z = calloc(count * count, sizeof(nat));
		
		const nat lifetime_byte_count = 4 * ((array_size + 1) * (lifetime_length));

		puts("finding all equivalent lifetimes...");

		nat dupl_count = 0;
		nat* duplicates = NULL;
	
		for (nat i = 0; i < count; i++) { 

			for (nat d = 0; d < dupl_count; d++) {
				if (duplicates[d] == i) goto next_i;
			}

			for (nat j = i + 1; j < count; j++) {
				printf("testing i=%llu and j=%llu... ", i, j);

				if (not memcmp(list[i].lifetime[0], list[j].lifetime[0], lifetime_byte_count)) {
					equivalent_z[count * i + equivalent_count[i]] = j;
					equivalent_count[i]++;

					duplicates = realloc(duplicates, sizeof(nat) * (dupl_count + 1));
					duplicates[dupl_count++] = j;

					printf("[%llu IS A DUPLICATE].\n", j);

				} else {
					puts("different.");
				}
			}
			next_i: continue;
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
		for (nat i = 0; i < count; i++) list[i].unique = not seen[i];

		puts("de-deuplicating z list...");
		nat unique_count = 0;
		for (nat i = 0; i < count; i++) {
			if (not list[i].unique) continue;
			list[unique_count++] = list[i];
		}
		count = unique_count;
		print_z_list(list, count);
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
	nat counter = 0, speed = 64;
	nat current = 0, initial_y = 0, initial_x = 0;

	nat viz_method = 0;

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		if (not (counter & ((1 << display_rate) - 1))) {
			if (lifetime_length < height) abort(); //  todo: do something to make it so that the view can be larger than the image... somehow... (fill the larger image in  with zeros maybe?...)
			if (array_size + 1 < width) abort();

			if ((int64_t) initial_y > (int64_t) lifetime_length - (int64_t) height) initial_y = lifetime_length - height;
			if ((int64_t) initial_x > (int64_t) array_size + 1 - (int64_t) width) initial_x = array_size + 1 - width;

			nat h_l = initial_y;
			for (nat h = 0; h < height; h++) {
				nat w_l = initial_x;
				for (nat w = 0; w < width; w++) {
					const nat lifetime_width = array_size + 1;
					screen[width * h + w] = list[current].lifetime[viz_method][lifetime_width * h_l + w_l];
					w_l++;
				}
				h_l++;
			}
			const double ratio = (double) current / (double) count;
			const nat progress = (nat) (ratio * (double) width);
			for (nat i = 0; i < progress; i++) screen[i] = (uint32_t) ~0;
			for (nat i = 0; i < width - progress; i++) screen[progress + i] = 0;
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

				if (not command and event.wheel.y < 0) { 
					if ((int64_t) initial_y + (int64_t) speed < (int64_t) lifetime_length - (int64_t) height) initial_y += speed; 
					else initial_y = lifetime_length - height;
				}

				if (not command and event.wheel.y > 0) { 
					if (initial_y >= speed) initial_y -= speed; else initial_y = 0;
				}
	
				if (not command and event.wheel.x > 0) { 
					if ((int64_t) initial_x + (int64_t) speed < (int64_t) array_size + 1 - (int64_t) width) initial_x += speed; 
					else initial_x = array_size + 1 - width;
				}

				if (not command and event.wheel.x < 0) { 
					if (initial_x >= speed) initial_x -= speed; else initial_x = 0;
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

				if (key[SDL_SCANCODE_0]) { initial_x = 0; initial_y = 0; }
				if (key[SDL_SCANCODE_1]) viz_method = 0;
				if (key[SDL_SCANCODE_2]) viz_method = 1;

				if (key[SDL_SCANCODE_F]) { if (current < count - 1) current++; printf("current is now %llu.\n", current); } 
				if (key[SDL_SCANCODE_A]) { if (current) current--; printf("current is now %llu.\n", current); } 

				if (key[SDL_SCANCODE_K]) { if (speed > 1) speed >>= 1; printf("speed = %llu\n", speed); }
				if (key[SDL_SCANCODE_L]) { speed <<= 1; printf("speed = %llu\n", speed); }

				if (key[SDL_SCANCODE_Z]) {
					printf("[index in list = %llu]: current displaying: origin = %llu,  ", current, list[current].origin);
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



		







*/

























// 0122 1025 2143 3062 4001 0661 4201


