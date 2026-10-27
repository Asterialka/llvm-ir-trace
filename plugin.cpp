#include "pass.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#if __has_include("llvm/Plugins/PassPlugin.h")
#include "llvm/Plugins/PassPlugin.h"
#else
#include "llvm/Passes/PassPlugin.h"
#endif
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

using namespace llvm;

namespace {
class IRTracePass : public PassInfoMixin<IRTracePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    const char *path = std::getenv("IR_TRACE_SITES");
    if (!path || !*path)
      report_fatal_error("Set IR_TRACE_SITES to the output catalog path");

    // Publish the catalog only after instrumentation and verification succeed.
    std::string pattern = std::string(path) + ".XXXXXX";
    std::vector<char> temporary(pattern.begin(), pattern.end());
    temporary.push_back('\0');
    int fd = mkstemp(temporary.data());
    if (fd < 0)
      report_fatal_error("Cannot create the instruction catalog");
    FILE *catalog = fdopen(fd, "w");
    if (!catalog) {
      close(fd);
      std::remove(temporary.data());
      report_fatal_error("Cannot open the instruction catalog");
    }
    PassStats stats{};
    bool failed = instrument_module(wrap(&M), catalog, &stats) != 0;
    failed |= std::fclose(catalog) != 0;
    if (!failed)
      failed = verifyModule(M, &errs());
    if (!failed)
      failed = std::rename(temporary.data(), path) != 0;
    if (failed) {
      std::remove(temporary.data());
      report_fatal_error("IR trace instrumentation failed");
    }
    errs() << "IRTrace: instructions=" << stats.instructions
           << " instrumented=" << stats.instrumented
           << " phi_skipped=" << stats.phi_skipped
           << " static_uses=" << stats.uses << '\n';
    return PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "IRTrace", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerOptimizerLastEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel, auto...) {
                  MPM.addPass(IRTracePass());
                });
            // Explicit opt entry point for small IR fixtures and diagnostics.
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name != "irtrace")
                    return false;
                  MPM.addPass(IRTracePass());
                  return true;
                });
          }};
}
