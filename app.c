#include "sim.h"

#define CELL_SIZE 4
#define GRID_WIDTH (SIM_X_SIZE / CELL_SIZE)
#define GRID_HEIGHT (SIM_Y_SIZE / CELL_SIZE)
#define ANT_COUNT 32
#define STEPS_PER_FRAME 128
#define COLOURS 6

_Static_assert(sizeof(int) == 4, "The sim interface requires 32-bit int");
_Static_assert(SIM_X_SIZE % CELL_SIZE == 0, "Width must fit whole cells");
_Static_assert(SIM_Y_SIZE % CELL_SIZE == 0, "Height must fit whole cells");

static void paint_cell(int x, int y, int argb) {
  int base_x = x * CELL_SIZE;
  int base_y = y * CELL_SIZE;
  for (int dy = 0; dy < CELL_SIZE; ++dy) {
    for (int dx = 0; dx < CELL_SIZE; ++dx) {
      simPutPixel(base_x + dx, base_y + dy, argb);
    }
  }
}

void app(void) {
  int cells[GRID_HEIGHT][GRID_WIDTH] = {{0}};
  int ant_x[ANT_COUNT];
  int ant_y[ANT_COUNT];
  int direction[ANT_COUNT];
  int phase[ANT_COUNT];
  const int turn[COLOURS] = {1, -1, -1, 1, 1, -1};
  const int palette[COLOURS] = {
      0xFF000000u, 0xFF293C83u, 0xFF398BB6u,
      0xFF78D4BEu, 0xFFF3CA7Du, 0xFFEF7185u};

  for (int i = 0; i < ANT_COUNT; ++i) {
    ant_x[i] = (i * 101 + 37) % GRID_WIDTH;
    ant_y[i] = (i * 73 + 19) % GRID_HEIGHT;
    direction[i] = (i * 7) & 3;
    phase[i] = i % COLOURS;
  }

  while (1) {
    for (int step = 0; step < STEPS_PER_FRAME; ++step) {
      for (int i = 0; i < ANT_COUNT; ++i) {
        int x = ant_x[i];
        int y = ant_y[i];
        int old_colour = cells[y][x];
        int new_colour = old_colour + 1;
        int rule_index = old_colour + phase[i];
        if (new_colour == COLOURS) {
          new_colour = 0;
        }
        if (rule_index >= COLOURS) {
          rule_index -= COLOURS;
        }

        direction[i] = (direction[i] + turn[rule_index] + 4) & 3;
        cells[y][x] = new_colour;
        paint_cell(x, y, palette[new_colour]);

        ant_x[i] += (direction[i] == 1) - (direction[i] == 3);
        ant_y[i] += (direction[i] == 2) - (direction[i] == 0);
        if (ant_x[i] < 0) {
          ant_x[i] = GRID_WIDTH - 1;
        }
        if (ant_x[i] == GRID_WIDTH) {
          ant_x[i] = 0;
        }
        if (ant_y[i] < 0) {
          ant_y[i] = GRID_HEIGHT - 1;
        }
        if (ant_y[i] == GRID_HEIGHT) {
          ant_y[i] = 0;
        }
      }
    }

    for (int i = 0; i < ANT_COUNT; ++i) {
      int px = ant_x[i] * CELL_SIZE + 1;
      int py = ant_y[i] * CELL_SIZE + 1;
      simPutPixel(px, py, 0xFFFFFFFFu);
      simPutPixel(px + 1, py, 0xFFFFFFFFu);
      simPutPixel(px, py + 1, 0xFFFFFFFFu);
      simPutPixel(px + 1, py + 1, 0xFFFFFFFFu);
    }
    simFlush();
  }
}
