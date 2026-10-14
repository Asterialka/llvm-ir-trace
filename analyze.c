#define _POSIX_C_SOURCE 200809L

#include "opcode.h"
#include "trace_format.h"

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    uint32_t index;
    uint32_t site;
} Operand;

typedef struct {
    uint16_t opcode;
    Operand *operands;
    size_t operand_count;
    uint64_t executed;
} Site;

typedef struct {
    uint16_t tokens[TRACE_MAX_WINDOW];
    unsigned length;
    uint64_t count;
} Pattern;

typedef struct {
    Pattern *slots;
    size_t capacity;
    size_t used;
    uint16_t history[TRACE_MAX_WINDOW];
    uint64_t total;
} Stream;

static void fail(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static void *allocate(size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size)
        fail("Allocation is too large");
    void *memory = calloc(count, size);
    if (!memory)
        fail("Cannot allocate memory");
    return memory;
}

static uint32_t parse_uint(const char *text)
{
    char *end;
    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);
    if (!*text || *text == '-' || *text == '+' || *end || errno || value > UINT32_MAX)
        fail("Invalid catalog number: %s", text);
    return (uint32_t)value;
}

static Site *read_catalog(const char *path, size_t *count)
{
    FILE *file = fopen(path, "r");
    if (!file)
        fail("Cannot open %s: %s", path, strerror(errno));
    char *line = NULL;
    size_t line_capacity = 0;
    if (getline(&line, &line_capacity, file) < 0
        || strcmp(line, "id\topcode\tname\tfunction\tblock\toperands\n"))
        fail("Invalid catalog header");

    size_t capacity = 256;
    Site *sites = allocate(capacity, sizeof(*sites));
    *count = 0;
    ssize_t length;

    while ((length = getline(&line, &line_capacity, file)) >= 0) {
        if (length && line[length - 1] == '\n')
            line[--length] = '\0';
        char *fields[6];
        fields[0] = line;
        for (size_t i = 1; i < 6; ++i) {
            char *tab = strchr(fields[i - 1], '\t');
            if (!tab)
                fail("Incomplete catalog row");
            *tab = '\0';
            fields[i] = tab + 1;
        }
        if (strchr(fields[5], '\t'))
            fail("Too many catalog columns");
        uint32_t id = parse_uint(fields[0]);
        uint32_t opcode = parse_uint(fields[1]);
        if (id != *count + 1 || !opcode_name(opcode)
            || strcmp(fields[2], opcode_name(opcode)) || !*fields[3])
            fail("Invalid catalog row for site %" PRIu32, id);
        parse_uint(fields[4]);
        if (id >= capacity) {
            size_t next_capacity = capacity * 2;
            Site *next = allocate(next_capacity, sizeof(*next));
            memcpy(next, sites, capacity * sizeof(*sites));
            free(sites);
            sites = next;
            capacity = next_capacity;
        }
        sites[id].opcode = (uint16_t)opcode;
        if (strcmp(fields[5], "-")) {
            size_t operands = 1;
            for (char *p = fields[5]; *p; ++p)
                operands += *p == ',';
            sites[id].operands = allocate(operands, sizeof(Operand));
            sites[id].operand_count = operands;
            char *token = fields[5];
            for (size_t i = 0; i < operands; ++i) {
                char *comma = strchr(token, ',');
                if (comma)
                    *comma = '\0';
                char *colon = strchr(token, ':');
                if (!colon)
                    fail("Invalid operand for site %" PRIu32, id);
                *colon = '\0';
                Operand *operand = &sites[id].operands[i];
                operand->index = parse_uint(token);
                operand->site = parse_uint(colon + 1);
                if (!operand->site
                    || (i && operand->index <= sites[id].operands[i - 1].index))
                    fail("Invalid operand ordering for site %" PRIu32, id);
                if (i + 1 < operands)
                    token = comma + 1;
            }
        }
        ++*count;
    }
    if (ferror(file))
        fail("Cannot read the catalog");
    fclose(file);
    free(line);
    if (!*count)
        fail("The catalog is empty");
    for (size_t i = 1; i <= *count; ++i)
        for (size_t j = 0; j < sites[i].operand_count; ++j)
            if (sites[i].operands[j].site > *count)
                fail("Operand references an unknown site");
    return sites;
}

static size_t pattern_hash(const uint16_t *tokens, unsigned length)
{
    uint64_t hash = UINT64_C(14695981039346656037) ^ length;
    for (unsigned i = 0; i < length; ++i) {
        hash ^= tokens[i];
        hash *= UINT64_C(1099511628211);
    }
    hash ^= hash >> 32;
    return (size_t)hash;
}

static Pattern *find_pattern(Stream *stream, const uint16_t *tokens,
                             unsigned length)
{
    size_t index = pattern_hash(tokens, length) & (stream->capacity - 1);
    while (stream->slots[index].count
           && (stream->slots[index].length != length
               || memcmp(stream->slots[index].tokens, tokens,
                         length * sizeof(*tokens))))
        index = (index + 1) & (stream->capacity - 1);
    return &stream->slots[index];
}

static void grow_stream(Stream *stream)
{
    Pattern *old = stream->slots;
    size_t old_capacity = stream->capacity;
    if (old_capacity > SIZE_MAX / 2)
        fail("Too many distinct patterns");
    stream->capacity = old_capacity ? old_capacity * 2 : 1024;
    stream->slots = allocate(stream->capacity, sizeof(*stream->slots));
    for (size_t i = 0; i < old_capacity; ++i)
        if (old[i].count)
            *find_pattern(stream, old[i].tokens, old[i].length) = old[i];
    free(old);
}

static void append_token(Stream *stream, uint16_t token)
{
    memmove(stream->history, stream->history + 1,
            (TRACE_MAX_WINDOW - 1) * sizeof(*stream->history));
    stream->history[TRACE_MAX_WINDOW - 1] = token;
    if (stream->total == UINT64_MAX)
        fail("The trace is too long");
    ++stream->total;
    unsigned max_length = stream->total < TRACE_MAX_WINDOW
        ? (unsigned)stream->total : TRACE_MAX_WINDOW;

    for (unsigned length = 1; length <= max_length; ++length) {
        if (!stream->capacity || stream->used >= stream->capacity / 2)
            grow_stream(stream);
        const uint16_t *tokens = stream->history + TRACE_MAX_WINDOW - length;
        Pattern *pattern = find_pattern(stream, tokens, length);
        if (!pattern->count) {
            memcpy(pattern->tokens, tokens, length * sizeof(*tokens));
            pattern->length = length;
            ++stream->used;
        }
        ++pattern->count;
    }
}

static int compare_patterns(const void *left, const void *right)
{
    const Pattern *a = left;
    const Pattern *b = right;
    if (a->length != b->length)
        return a->length < b->length ? -1 : 1;
    if (a->count != b->count)
        return a->count > b->count ? -1 : 1;
    for (unsigned i = 0; i < a->length; ++i)
        if (a->tokens[i] != b->tokens[i])
            return a->tokens[i] < b->tokens[i] ? -1 : 1;
    return 0;
}

static FILE *open_output(const char *directory, const char *name)
{
    size_t length = strlen(directory) + strlen(name) + 2;
    char *path = allocate(length, 1);
    snprintf(path, length, "%s/%s", directory, name);
    FILE *file = fopen(path, "w");
    if (!file)
        fail("Cannot open %s: %s", path, strerror(errno));
    free(path);
    return file;
}

static void close_output(FILE *file)
{
    if (ferror(file) || fclose(file))
        fail("Cannot write the analysis output");
}

static void print_pattern(FILE *file, const Pattern *pattern, int uses)
{
    for (unsigned i = 0; i < pattern->length; ++i) {
        if (i)
            fputs("; ", file);
        uint16_t token = pattern->tokens[i];
        if (uses)
            fprintf(file, "%s <- %s", opcode_name(token >> 8),
                    opcode_name(token & 255));
        else
            fputs(opcode_name(token), file);
    }
}

static void write_patterns(Stream *stream, const char *directory,
                           const char *name, int uses, FILE *report,
                           FILE *summary)
{
    Pattern *sorted = allocate(stream->used ? stream->used : 1, sizeof(*sorted));
    size_t count = 0;
    for (size_t i = 0; i < stream->capacity; ++i)
        if (stream->slots[i].count)
            sorted[count++] = stream->slots[i];
    qsort(sorted, count, sizeof(*sorted), compare_patterns);
    FILE *file = open_output(directory, name);
    fputs("length\tcount\tshare_percent\tpattern\n", file);
    fprintf(report, "## %s\n\n", uses ? "User <- Operand" : "Instructions");
    size_t position = 0;

    for (unsigned length = 1; length <= TRACE_MAX_WINDOW; ++length) {
        uint64_t windows = stream->total >= length ? stream->total - length + 1 : 0;
        size_t start = position;
        uint64_t sum = 0;
        while (position < count && sorted[position].length == length) {
            const Pattern *pattern = &sorted[position];
            sum += pattern->count;
            fprintf(file, "%u\t%" PRIu64 "\t%.8f\t", length,
                    pattern->count, 100.0 * (double)pattern->count / (double)windows);
            print_pattern(file, pattern, uses);
            fputc('\n', file);
            ++position;
        }
        if (sum != windows)
            fail("Window count mismatch");
        fprintf(summary, "%s_windows_%u\t%" PRIu64 "\n%s_distinct_%u\t%zu\n",
                uses ? "uses" : "instructions", length, windows,
                uses ? "uses" : "instructions", length, position - start);
        fprintf(report, "### Window %u\n\nTotal: %" PRIu64
                "; distinct: %zu.\n\n| Count | Share | Pattern |\n|---:|---:|:---|\n",
                length, windows, position - start);
        size_t limit = position - start < 10 ? position : start + 10;
        for (size_t i = start; i < limit; ++i) {
            fprintf(report, "| %" PRIu64 " | %.4f%% | `", sorted[i].count,
                    100.0 * (double)sorted[i].count / (double)windows);
            print_pattern(report, &sorted[i], uses);
            fputs("` |\n", report);
        }
        if (!uses || length < TRACE_MAX_WINDOW)
            fputc('\n', report);
    }
    close_output(file);
    free(sorted);
}

int main(int argc, char **argv)
{
    if (argc < 4 || argc > 5 || (argc == 5 && strcmp(argv[4], "--expand")))
        fail("Usage: %s sites.tsv trace.bin output_directory [--expand]", argv[0]);
    size_t site_count;
    Site *sites = read_catalog(argv[1], &site_count);
    if (mkdir(argv[3], 0755) && errno != EEXIST)
        fail("Cannot create %s: %s", argv[3], strerror(errno));
    FILE *input = fopen(argv[2], "rb");
    if (!input)
        fail("Cannot open %s: %s", argv[2], strerror(errno));
    unsigned char magic[TRACE_MAGIC_SIZE];
    if (fread(magic, 1, sizeof(magic), input) != sizeof(magic)
        || memcmp(magic, TRACE_MAGIC, sizeof(magic)))
        fail("Invalid trace header");
    FILE *expanded = argc == 5 ? open_output(argv[3], "trace.tsv") : NULL;
    if (expanded)
        fputs("sequence\tuser_id\tuser\toperand_index\toperand_id\toperand\n", expanded);

    Stream instructions = {0};
    Stream uses = {0};
    unsigned char buffer[65536];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), input))) {
        if (bytes % 4)
            fail("Truncated trace event");
        for (size_t offset = 0; offset < bytes; offset += 4) {
            uint32_t id = (uint32_t)buffer[offset]
                | (uint32_t)buffer[offset + 1] << 8
                | (uint32_t)buffer[offset + 2] << 16
                | (uint32_t)buffer[offset + 3] << 24;
            if (!id || id > site_count)
                fail("Unknown trace site: %" PRIu32, id);
            Site *site = &sites[id];
            if (site->opcode == 44)
                fail("A phi instruction was recorded as User");
            ++site->executed;
            append_token(&instructions, site->opcode);
            if (expanded && !site->operand_count)
                fprintf(expanded, "%" PRIu64 "\t%" PRIu32 "\t%s\t-\t-\t-\n",
                        instructions.total, id, opcode_name(site->opcode));
            for (size_t i = 0; i < site->operand_count; ++i) {
                Operand operand = site->operands[i];
                uint16_t operand_opcode = sites[operand.site].opcode;
                append_token(&uses, (uint16_t)(site->opcode << 8 | operand_opcode));
                if (expanded)
                    fprintf(expanded, "%" PRIu64 "\t%" PRIu32 "\t%s\t%" PRIu32
                            "\t%" PRIu32 "\t%s\n", instructions.total, id,
                            opcode_name(site->opcode), operand.index,
                            operand.site, opcode_name(operand_opcode));
            }
        }
    }
    if (ferror(input))
        fail("Cannot read the trace");
    fclose(input);
    if (expanded)
        close_output(expanded);

    FILE *summary = open_output(argv[3], "summary.tsv");
    FILE *report = open_output(argv[3], "report.md");
    FILE *frequencies = open_output(argv[3], "sites_frequency.tsv");
    size_t phi_count = 0;
    size_t executed_sites = 0;
    uint64_t static_uses = 0;
    fputs("id\topcode\tname\tcount\n", frequencies);
    for (size_t i = 1; i <= site_count; ++i) {
        phi_count += sites[i].opcode == 44;
        executed_sites += sites[i].executed != 0;
        if (sites[i].opcode != 44)
            static_uses += sites[i].operand_count;
        fprintf(frequencies, "%zu\t%u\t%s\t%" PRIu64 "\n", i,
                (unsigned)sites[i].opcode, opcode_name(sites[i].opcode), sites[i].executed);
    }
    close_output(frequencies);
    fprintf(summary, "metric\tvalue\nstatic_instructions\t%zu\nphi_skipped\t%zu\n"
            "static_uses\t%" PRIu64 "\nexecuted_sites\t%zu\ninstruction_events\t%" PRIu64
            "\nuse_events\t%" PRIu64 "\n", site_count, phi_count, static_uses,
            executed_sites, instructions.total, uses.total);
    fprintf(report, "# Runtime IR statistics\n\nStatic instructions: %zu; phi Users skipped: %zu; "
            "executed sites: %zu.\n\nInstruction events: %" PRIu64
            "; User <- Operand events: %" PRIu64 ".\n\n"
            "Windows overlap and continue across blocks, functions and frames. "
            "Instruction operands are listed in operand-index order. "
            "Constants, arguments, globals and basic blocks are excluded from use events.\n\n",
            site_count, phi_count, executed_sites, instructions.total, uses.total);
    write_patterns(&instructions, argv[3], "instructions.tsv", 0, report, summary);
    write_patterns(&uses, argv[3], "uses.tsv", 1, report, summary);
    close_output(report);
    close_output(summary);
    printf("instructions=%" PRIu64 " uses=%" PRIu64 "\n", instructions.total, uses.total);

    for (size_t i = 1; i <= site_count; ++i)
        free(sites[i].operands);
    free(sites);
    free(instructions.slots);
    free(uses.slots);
    return EXIT_SUCCESS;
}
