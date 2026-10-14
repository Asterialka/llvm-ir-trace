#include "app.h"

#include <SDL.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *text, uint32_t *value)
{
    char *end;
    errno = 0;
    unsigned long parsed = strtoul(text, &end, 10);

    if (!*text || text[0] == '-' || *end || errno || parsed > UINT32_MAX)
        return -1;
    *value = (uint32_t)parsed;
    return 0;
}

int main(int argc, char **argv)
{
    uint32_t frames = 600;
    uint32_t seed = 42;
    int headless = 0;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--headless")) {
            headless = 1;
        } else if ((!strcmp(argv[i], "--frames") || !strcmp(argv[i], "--seed"))
                   && i + 1 < argc) {
            uint32_t *value = !strcmp(argv[i], "--frames") ? &frames : &seed;
            if (parse_number(argv[++i], value)) {
                fprintf(stderr, "Invalid number: %s\n", argv[i]);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Usage: %s [--frames N] [--seed N] [--headless]\n",
                    argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (!frames || frames > 1000000) {
        fprintf(stderr, "Frames must be between 1 and 1000000\n");
        return EXIT_FAILURE;
    }
    if (headless && SDL_setenv("SDL_VIDEODRIVER", "dummy", 1)) {
        fprintf(stderr, "Cannot select the dummy video driver\n");
        return EXIT_FAILURE;
    }
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow("Conway's Game of Life",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        APP_WIDTH * 10, APP_HEIGHT * 10,
        headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = window
        ? SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE) : NULL;
    SDL_Texture *texture = renderer
        ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            APP_WIDTH, APP_HEIGHT) : NULL;
    int status = EXIT_FAILURE;

    if (!texture) {
        fprintf(stderr, "SDL setup: %s\n", SDL_GetError());
        goto cleanup;
    }

    uint32_t pixels[APP_WIDTH * APP_HEIGHT];
    uint32_t completed = 0;
    int quit = 0;
    app_init(seed);

    while (completed < frames && !quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT
                || (event.type == SDL_KEYDOWN
                    && event.key.keysym.sym == SDLK_ESCAPE))
                quit = 1;
        }
        if (quit)
            break;
        app_step();
        app_pixels(pixels);
        if (SDL_UpdateTexture(texture, NULL, pixels,
                              APP_WIDTH * (int)sizeof(*pixels))
            || SDL_RenderClear(renderer)
            || SDL_RenderCopy(renderer, texture, NULL, NULL)) {
            fprintf(stderr, "SDL rendering: %s\n", SDL_GetError());
            goto cleanup;
        }
        SDL_RenderPresent(renderer);
        ++completed;
        if (!headless)
            SDL_Delay(16);
    }

    printf("frames=%" PRIu32 " seed=%" PRIu32 " checksum=%016" PRIx64 "\n",
           completed, seed, app_checksum());
    status = EXIT_SUCCESS;

cleanup:
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    return status;
}
