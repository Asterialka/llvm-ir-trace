#define _POSIX_C_SOURCE 200809L

#include "pass.h"

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/IRReader.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE *temporary_file(const char *path, char **temporary_path)
{
    size_t length = strlen(path) + sizeof(".XXXXXX");
    *temporary_path = malloc(length);
    if (!*temporary_path)
        return NULL;
    snprintf(*temporary_path, length, "%s.XXXXXX", path);
    int descriptor = mkstemp(*temporary_path);
    if (descriptor < 0)
        return NULL;
    FILE *file = fdopen(descriptor, "w");
    if (!file)
        close(descriptor);
    return file;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s input.ll output.bc sites.tsv\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!strcmp(argv[1], argv[2]) || !strcmp(argv[1], argv[3])
        || !strcmp(argv[2], argv[3])) {
        fputs("Input and output paths must be different\n", stderr);
        return EXIT_FAILURE;
    }

    LLVMContextRef context = LLVMContextCreate();
    LLVMModuleRef module = NULL;
    LLVMMemoryBufferRef buffer;
    char *message = NULL;
    int status = EXIT_FAILURE;
    FILE *catalog = NULL;
    FILE *output = NULL;
    char *catalog_path = NULL;
    char *output_path = NULL;

    if (LLVMCreateMemoryBufferWithContentsOfFile(argv[1], &buffer, &message)) {
        fprintf(stderr, "%s\n", message);
        LLVMDisposeMessage(message);
        goto cleanup;
    }
    if (LLVMParseIRInContext(context, buffer, &module, &message)) {
        fprintf(stderr, "%s\n", message);
        LLVMDisposeMessage(message);
        goto cleanup;
    }
    if (LLVMVerifyModule(module, LLVMReturnStatusAction, &message)) {
        fprintf(stderr, "%s\n", message);
        LLVMDisposeMessage(message);
        goto cleanup;
    }
    LLVMDisposeMessage(message);

    catalog = temporary_file(argv[3], &catalog_path);
    if (!catalog) {
        perror(argv[3]);
        goto cleanup;
    }
    PassStats stats;
    if (instrument_module(module, catalog, &stats))
        goto cleanup;
    if (fclose(catalog)) {
        catalog = NULL;
        perror(argv[3]);
        goto cleanup;
    }
    catalog = NULL;
    if (LLVMVerifyModule(module, LLVMReturnStatusAction, &message)) {
        fprintf(stderr, "%s\n", message);
        LLVMDisposeMessage(message);
        goto cleanup;
    }
    LLVMDisposeMessage(message);
    output = temporary_file(argv[2], &output_path);
    if (!output) {
        perror(argv[2]);
        goto cleanup;
    }
    if (LLVMWriteBitcodeToFD(module, fileno(output), 0, 0)) {
        fprintf(stderr, "Cannot write %s\n", argv[2]);
        goto cleanup;
    }
    if (fclose(output)) {
        output = NULL;
        perror(argv[2]);
        goto cleanup;
    }
    output = NULL;
    if (rename(output_path, argv[2]) || rename(catalog_path, argv[3])) {
        perror("Cannot save the instrumentation outputs");
        goto cleanup;
    }
    fprintf(stderr, "instructions=%" PRIu32 " instrumented=%" PRIu32
            " phi_skipped=%" PRIu32 " static_uses=%" PRIu64 "\n",
            stats.instructions, stats.instrumented, stats.phi_skipped, stats.uses);
    status = EXIT_SUCCESS;

cleanup:
    if (catalog)
        fclose(catalog);
    if (output)
        fclose(output);
    if (module)
        LLVMDisposeModule(module);
    LLVMContextDispose(context);
    if (output_path)
        remove(output_path);
    if (catalog_path)
        remove(catalog_path);
    free(output_path);
    free(catalog_path);
    return status;
}
