#include "sim.h"
#include "sim_control.h"

#include <errno.h>
#include <stdint.h>
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
    uint32_t frames = 0;
    int headless = 0;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--headless")) {
            headless = 1;
        } else if (!strcmp(argv[i], "--frames") && i + 1 < argc) {
            if (parse_number(argv[++i], &frames) || !frames || frames > 1000000) {
                fputs("Frames must be between 1 and 1000000\n", stderr);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Usage: %s [--frames N] [--headless]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (headless && !frames)
        frames = 64;
    simConfigure(frames, headless);
    simInit();
    app();
    return EXIT_SUCCESS;
}
