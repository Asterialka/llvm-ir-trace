#ifndef PASS_H
#define PASS_H

#include <llvm-c/Core.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t instructions;
    uint32_t instrumented;
    uint32_t phi_skipped;
    uint64_t uses;
} PassStats;

#ifdef __cplusplus
extern "C" {
#endif
int instrument_module(LLVMModuleRef module, FILE *catalog, PassStats *stats);
#ifdef __cplusplus
}
#endif

#endif
