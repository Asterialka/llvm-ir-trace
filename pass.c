#include "pass.h"
#include "opcode.h"
#include "trace_format.h"

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    LLVMValueRef instruction;
    uint32_t id;
} SiteSlot;

static size_t site_hash(LLVMValueRef instruction, size_t mask)
{
    uint64_t value = (uintptr_t)instruction;
    value ^= value >> 33;
    value *= UINT64_C(0xff51afd7ed558ccd);
    value ^= value >> 33;
    return (size_t)value & mask;
}

static SiteSlot *find_site(SiteSlot *table, size_t capacity,
                           LLVMValueRef instruction)
{
    size_t index = site_hash(instruction, capacity - 1);
    while (table[index].instruction && table[index].instruction != instruction)
        index = (index + 1) & (capacity - 1);
    return &table[index];
}

static void remove_attributes(LLVMValueRef value, int call)
{
    static const char *const names[] = {
        "memory", "readnone", "readonly", "writeonly", "argmemonly",
        "inaccessiblememonly", "inaccessiblemem_or_argmemonly",
        "nosync", "nofree", "speculatable", "willreturn", "mustprogress"
    };

    for (size_t i = 0; i < sizeof(names) / sizeof(*names); ++i) {
        unsigned kind = LLVMGetEnumAttributeKindForName(names[i], strlen(names[i]));
        if (!kind)
            continue;
        if (call)
            LLVMRemoveCallSiteEnumAttribute(value, LLVMAttributeFunctionIndex, kind);
        else
            LLVMRemoveEnumAttributeAtIndex(value, LLVMAttributeFunctionIndex, kind);
    }
}

int instrument_module(LLVMModuleRef module, FILE *catalog, PassStats *stats)
{
    size_t source_length;
    const char *source = LLVMGetSourceFileName(module, &source_length);
    const char *basename = source;

    for (size_t i = 0; i < source_length; ++i)
        if (source[i] == '/' || source[i] == '\\')
            basename = source + i + 1;
    if ((size_t)(source + source_length - basename) != 5
        || memcmp(basename, "app.c", 5)) {
        fputs("Only the app.c module may be instrumented\n", stderr);
        return -1;
    }
    if (LLVMGetNamedMetadata(module, TRACE_MARKER, strlen(TRACE_MARKER))
        || LLVMGetNamedFunction(module, TRACE_HOOK)) {
        fputs("The module already contains trace instrumentation\n", stderr);
        return -1;
    }

    size_t count = 0;
    for (LLVMValueRef function = LLVMGetFirstFunction(module); function;
         function = LLVMGetNextFunction(function)) {
        for (LLVMBasicBlockRef block = LLVMGetFirstBasicBlock(function); block;
             block = LLVMGetNextBasicBlock(block)) {
            for (LLVMValueRef instruction = LLVMGetFirstInstruction(block);
                 instruction; instruction = LLVMGetNextInstruction(instruction)) {
                LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
                if (!opcode_name(opcode) || opcode == LLVMLandingPad
                    || opcode == LLVMCatchPad || opcode == LLVMCleanupPad
                    || opcode == LLVMCatchSwitch || opcode == LLVMInvoke
                    || opcode == LLVMCallBr) {
                    fputs("Unsupported instruction in app.c\n", stderr);
                    return -1;
                }
                ++count;
            }
        }
    }
    if (!count || count > UINT32_MAX || count > SIZE_MAX / (2 * sizeof(SiteSlot))) {
        fputs("Invalid instruction count\n", stderr);
        return -1;
    }

    size_t capacity = 16;
    while (capacity < count * 2)
        capacity *= 2;
    LLVMValueRef *instructions = malloc(count * sizeof(*instructions));
    SiteSlot *table = calloc(capacity, sizeof(*table));
    if (!instructions || !table) {
        free(instructions);
        free(table);
        fputs("Cannot allocate the instruction catalog\n", stderr);
        return -1;
    }

    size_t index = 0;
    for (LLVMValueRef function = LLVMGetFirstFunction(module); function;
         function = LLVMGetNextFunction(function)) {
        for (LLVMBasicBlockRef block = LLVMGetFirstBasicBlock(function); block;
             block = LLVMGetNextBasicBlock(block)) {
            for (LLVMValueRef instruction = LLVMGetFirstInstruction(block);
                 instruction; instruction = LLVMGetNextInstruction(instruction)) {
                instructions[index] = instruction;
                SiteSlot *slot = find_site(table, capacity, instruction);
                slot->instruction = instruction;
                slot->id = (uint32_t)++index;
            }
        }
        if (LLVMCountBasicBlocks(function))
            remove_attributes(function, 0);
    }

    *stats = (PassStats){ .instructions = (uint32_t)count };
    fputs("id\topcode\tname\tfunction\tblock\toperands\n", catalog);
    for (size_t i = 0; i < count; ++i) {
        LLVMValueRef instruction = instructions[i];
        LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
        LLVMBasicBlockRef block = LLVMGetInstructionParent(instruction);
        LLVMValueRef function = LLVMGetBasicBlockParent(block);
        size_t name_length;
        const char *name = LLVMGetValueName2(function, &name_length);
        unsigned block_index = 0;
        for (LLVMBasicBlockRef current = LLVMGetFirstBasicBlock(function);
             current != block; current = LLVMGetNextBasicBlock(current))
            ++block_index;
        if (name_length > INT_MAX) {
            free(instructions);
            free(table);
            fputs("Function name is too long\n", stderr);
            return -1;
        }
        fprintf(catalog, "%zu\t%u\t%s\t%.*s\t%u\t", i + 1,
                (unsigned)opcode, opcode_name(opcode), (int)name_length,
                name, block_index);
        int first = 1;
        int operand_count = LLVMGetNumOperands(instruction);
        for (int operand_index = 0; operand_index < operand_count; ++operand_index) {
            LLVMValueRef operand = LLVMGetOperand(instruction, (unsigned)operand_index);
            if (!LLVMIsAInstruction(operand))
                continue;
            uint32_t id = find_site(table, capacity, operand)->id;
            if (!id) {
                free(instructions);
                free(table);
                fputs("Instruction operand is absent from the catalog\n", stderr);
                return -1;
            }
            fprintf(catalog, "%s%d:%" PRIu32, first ? "" : ",", operand_index, id);
            first = 0;
            if (opcode != LLVMPHI)
                ++stats->uses;
        }
        if (first)
            fputc('-', catalog);
        fputc('\n', catalog);
    }

    LLVMContextRef context = LLVMGetModuleContext(module);
    LLVMTypeRef argument_type = LLVMInt32TypeInContext(context);
    LLVMTypeRef hook_type = LLVMFunctionType(LLVMVoidTypeInContext(context),
                                            &argument_type, 1, 0);
    LLVMValueRef hook = LLVMAddFunction(module, TRACE_HOOK, hook_type);
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);

    for (size_t i = 0; i < count; ++i) {
        LLVMValueRef instruction = instructions[i];
        LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
        if (opcode == LLVMPHI) {
            ++stats->phi_skipped;
            continue;
        }
        if (opcode == LLVMCall) {
            remove_attributes(instruction, 1);
            LLVMSetTailCall(instruction, 0);
        }
        LLVMPositionBuilderBefore(builder, instruction);
        LLVMValueRef argument = LLVMConstInt(argument_type, i + 1, 0);
        LLVMBuildCall2(builder, hook_type, hook, &argument, 1, "");
        ++stats->instrumented;
    }

    LLVMMetadataRef marker = LLVMMDStringInContext2(context, "1", 1);
    LLVMMetadataRef node = LLVMMDNodeInContext2(context, &marker, 1);
    LLVMAddNamedMetadataOperand(module, TRACE_MARKER,
                                LLVMMetadataAsValue(context, node));
    LLVMDisposeBuilder(builder);
    free(instructions);
    free(table);
    return ferror(catalog) ? -1 : 0;
}
