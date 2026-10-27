#include "sim.h"
#include "sim_control.h"
#include <SDL2/SDL.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static uint32_t pixels[SIM_Y_SIZE][SIM_X_SIZE];
static uint32_t frame_tick;
static int pending_click;
static int click_position;
static uint32_t frame_limit;
static uint32_t completed;
static int headless;
static uint64_t draw_hash = UINT64_C(14695981039346656037);

void simConfigure(uint32_t frames, int no_window) {
  frame_limit = frames;
  headless = no_window;
}

static void finish(void) {
  uint64_t checksum = UINT64_C(14695981039346656037);
  for (int y = 0; y < SIM_Y_SIZE; ++y)
    for (int x = 0; x < SIM_X_SIZE; ++x) {
      checksum ^= pixels[y][x];
      checksum *= UINT64_C(1099511628211);
    }
  printf("frames=%" PRIu32 " checksum=%016" PRIx64 " draws=%016" PRIx64 "\n",
         completed, checksum, draw_hash);
  exit(EXIT_SUCCESS);
}

static void check_sdl(int failed) {
  if (failed) {
    fprintf(stderr, "SDL error: %s\n", SDL_GetError());
    exit(1);
  }
}

void simInit(void) {
  if (headless)
    check_sdl(SDL_setenv("SDL_VIDEODRIVER", "dummy", 1) != 0);
  check_sdl(SDL_Init(SDL_INIT_VIDEO) != 0);
  if (atexit(simExit)) {
    fputs("Cannot register SDL cleanup\n", stderr);
    exit(EXIT_FAILURE);
  }
  window = SDL_CreateWindow("Colony of six-colour turmites",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SIM_X_SIZE, SIM_Y_SIZE,
                            headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
  check_sdl(window == NULL);
  renderer = SDL_CreateRenderer(window, -1, 0);
  check_sdl(renderer == NULL);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, SIM_X_SIZE,
                              SIM_Y_SIZE);
  check_sdl(texture == NULL);
  check_sdl(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE) != 0);
  frame_tick = SDL_GetTicks();
}

void simExit(void) {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

static void poll_events(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT
        || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
      finish();
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      pending_click = 1;
      click_position = (event.button.x << 16) | (event.button.y & 0xFFFF);
    }
  }
}

void simFlush(void) {
  poll_events();
  check_sdl(SDL_UpdateTexture(texture, NULL, pixels,
                              SIM_X_SIZE * (int)sizeof(uint32_t)) != 0);
  check_sdl(SDL_RenderClear(renderer) != 0);
  check_sdl(SDL_RenderCopy(renderer, texture, NULL, NULL) != 0);
  SDL_RenderPresent(renderer);
  ++completed;
  if (frame_limit && completed == frame_limit)
    finish();
  uint32_t elapsed = SDL_GetTicks() - frame_tick;
  if (!headless && elapsed < 33) {
    SDL_Delay(33 - elapsed);
  }
  frame_tick = SDL_GetTicks();
}

void simPutPixel(int x, int y, int argb) {
  if (x < 0 || x >= SIM_X_SIZE || y < 0 || y >= SIM_Y_SIZE) {
    fputs("simPutPixel: coordinates outside the canvas\n", stderr);
    exit(1);
  }
  pixels[y][x] = (uint32_t)argb;
  draw_hash = (draw_hash ^ (uint32_t)x) * UINT64_C(1099511628211);
  draw_hash = (draw_hash ^ (uint32_t)y) * UINT64_C(1099511628211);
  draw_hash = (draw_hash ^ (uint32_t)argb) * UINT64_C(1099511628211);
}

int simRand(void) { return rand(); }

int simHasClick(void) {
  poll_events();
  return pending_click;
}

int simGetClick(void) {
  poll_events();
  if (pending_click) {
    pending_click = 0;
    return click_position;
  }
  return 0;
}
