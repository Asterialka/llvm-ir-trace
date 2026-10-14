#ifndef APP_H
#define APP_H

#include <stdint.h>

enum { APP_WIDTH = 64, APP_HEIGHT = 48 };

void app_init(uint32_t seed);
void app_step(void);
void app_pixels(uint32_t *pixels);
uint64_t app_checksum(void);

#endif
