#ifndef OPCODE_H
#define OPCODE_H

#include <stddef.h>
#include <stdint.h>

static inline const char *opcode_name(uint32_t opcode)
{
    static const char *const names[] = {
        [1] = "ret", [2] = "br", [3] = "switch", [4] = "indirectbr",
        [5] = "invoke", [7] = "unreachable", [8] = "add", [9] = "fadd",
        [10] = "sub", [11] = "fsub", [12] = "mul", [13] = "fmul",
        [14] = "udiv", [15] = "sdiv", [16] = "fdiv", [17] = "urem",
        [18] = "srem", [19] = "frem", [20] = "shl", [21] = "lshr",
        [22] = "ashr", [23] = "and", [24] = "or", [25] = "xor",
        [26] = "alloca", [27] = "load", [28] = "store",
        [29] = "getelementptr", [30] = "trunc", [31] = "zext",
        [32] = "sext", [33] = "fptoui", [34] = "fptosi",
        [35] = "uitofp", [36] = "sitofp", [37] = "fptrunc",
        [38] = "fpext", [39] = "ptrtoint", [40] = "inttoptr",
        [41] = "bitcast", [42] = "icmp", [43] = "fcmp", [44] = "phi",
        [45] = "call", [46] = "select", [47] = "userop1",
        [48] = "userop2", [49] = "va_arg", [50] = "extractelement",
        [51] = "insertelement", [52] = "shufflevector",
        [53] = "extractvalue", [54] = "insertvalue", [55] = "fence",
        [56] = "cmpxchg", [57] = "atomicrmw", [58] = "resume",
        [59] = "landingpad", [60] = "addrspacecast", [61] = "cleanupret",
        [62] = "catchret", [63] = "catchpad", [64] = "cleanuppad",
        [65] = "catchswitch", [66] = "fneg", [67] = "callbr",
        [68] = "freeze", [69] = "ptrtoaddr"
    };

    return opcode < sizeof(names) / sizeof(*names) ? names[opcode] : NULL;
}

#endif
