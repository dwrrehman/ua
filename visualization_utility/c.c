// UDP client for my multiplayer game.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"

#include <SDL.h>

#pragma clang diagnostic pop
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"


static const int delay_ms_per_frame = 16;

static const int display_rate = 5;

int main(const int argc, const char** argv) {

	if (SDL_Init(SDL_INIT_VIDEO)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));
	SDL_Window *window = SDL_CreateWindow("visualization utility", SDL_WINDOWPOS_CENTERED, 
				SDL_WINDOWPOS_CENTERED, 640, 400, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 20, 15);
	
	const size_t window_height = 25;
	const size_t window_width = 25;

	uint32_t* buffer = malloc(window_height * window_width * 4);
	uint32_t* pixels = NULL;
	int pitch = 0;
	bool quit = false, fullscreen = false;
	int counter = 0;

	int timestep = 0;

	memset(buffer, 0xff, window_height * window_width * 4);

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		if (not (counter & ((1 << display_rate) - 1))) {
			timestep++;
			buffer[timestep * window_width * 4 + timestep] = 0x00;
		}

		SDL_LockTexture(texture, NULL, (void**) &pixels, &pitch);

		memcpy(pixels, buffer, window_height * window_width * 4);

		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_GRAVE]) SDL_SetWindowFullscreen(window, (fullscreen = !fullscreen) ? SDL_WINDOW_FULLSCREEN : 0);
				if (key[SDL_SCANCODE_0] or key[SDL_SCANCODE_ESCAPE] or key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_0]) {}
				if (key[SDL_SCANCODE_1]) {}
				if (key[SDL_SCANCODE_2]) {}

				
				if (key[SDL_SCANCODE_T]) {}
				if (key[SDL_SCANCODE_F]) {}
				if (key[SDL_SCANCODE_G]) {}
				if (key[SDL_SCANCODE_H]) {}

				if (key[SDL_SCANCODE_I]) {}
				if (key[SDL_SCANCODE_J]) {}
				if (key[SDL_SCANCODE_K]) {}
				if (key[SDL_SCANCODE_L]) {}
			}
		}

		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = delay_ms_per_frame - (int32_t) time; 
		if (sleep > 0) SDL_Delay((uint32_t) sleep);

		counter++;
	
		if (not (counter & ((1 << 5) - 1))) {
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


