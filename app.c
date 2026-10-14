#include "app.h"

static unsigned char cells[APP_HEIGHT][APP_WIDTH];
static unsigned char next_cells[APP_HEIGHT][APP_WIDTH];

void app_init(uint32_t seed)
{
    uint32_t state = seed ? seed : 1;

    for (int y = 0; y < APP_HEIGHT; ++y) {
        for (int x = 0; x < APP_WIDTH; ++x) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            cells[y][x] = state % 5 == 0;
        }
    }
}

void app_step(void)
{
    for (int y = 0; y < APP_HEIGHT; ++y) {
        int above = (y + APP_HEIGHT - 1) % APP_HEIGHT;
        int below = (y + 1) % APP_HEIGHT;

        for (int x = 0; x < APP_WIDTH; ++x) {
            int left = (x + APP_WIDTH - 1) % APP_WIDTH;
            int right = (x + 1) % APP_WIDTH;
            unsigned neighbors = cells[above][left] + cells[above][x]
                + cells[above][right] + cells[y][left] + cells[y][right]
                + cells[below][left] + cells[below][x] + cells[below][right];
            next_cells[y][x] = neighbors == 3
                || (cells[y][x] && neighbors == 2);
        }
    }

    for (int y = 0; y < APP_HEIGHT; ++y)
        for (int x = 0; x < APP_WIDTH; ++x)
            cells[y][x] = next_cells[y][x];
}

void app_pixels(uint32_t *pixels)
{
    for (int y = 0; y < APP_HEIGHT; ++y)
        for (int x = 0; x < APP_WIDTH; ++x)
            pixels[y * APP_WIDTH + x] = cells[y][x]
                ? UINT32_C(0xff83d483) : UINT32_C(0xff121820);
}

uint64_t app_checksum(void)
{
    uint64_t hash = UINT64_C(14695981039346656037);

    for (int y = 0; y < APP_HEIGHT; ++y) {
        for (int x = 0; x < APP_WIDTH; ++x) {
            hash ^= cells[y][x];
            hash *= UINT64_C(1099511628211);
        }
    }
    return hash;
}
