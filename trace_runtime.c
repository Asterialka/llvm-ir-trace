#include "trace_format.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { BUFFER_SIZE = 65536 };

static FILE *trace_file;
static unsigned char buffer[BUFFER_SIZE];
static size_t used;
static int initialized;

static void trace_error(void)
{
    perror("IR trace");
    _Exit(EXIT_FAILURE);
}

static void flush_trace(void)
{
    if (used && fwrite(buffer, 1, used, trace_file) != used)
        trace_error();
    used = 0;
}

static void finish_trace(void)
{
    flush_trace();
    if (fclose(trace_file))
        trace_error();
}

void __irtrace_record(uint32_t site)
{
    if (!initialized) {
        const char *path = getenv("IR_TRACE_FILE");
        if (!path || !*path)
            path = "trace.bin";
        trace_file = fopen(path, "wb");
        if (!trace_file)
            trace_error();
        if (fwrite(TRACE_MAGIC, 1, TRACE_MAGIC_SIZE, trace_file)
            != TRACE_MAGIC_SIZE)
            trace_error();
        if (atexit(finish_trace)) {
            fputs("Cannot register the trace finalizer\n", stderr);
            _Exit(EXIT_FAILURE);
        }
        initialized = 1;
    }

    if (used == sizeof(buffer))
        flush_trace();
    buffer[used++] = (unsigned char)site;
    buffer[used++] = (unsigned char)(site >> 8);
    buffer[used++] = (unsigned char)(site >> 16);
    buffer[used++] = (unsigned char)(site >> 24);
}
