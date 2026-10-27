#ifndef SIM_CONTROL_H
#define SIM_CONTROL_H

#include <stdint.h>

/* Runner controls live outside the unchanged first-assignment app.c. */
void simConfigure(uint32_t frames, int headless);

#endif
