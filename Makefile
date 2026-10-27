LOCAL_TOOLCHAIN := $(CURDIR)/.tools/toolchain/usr
ifneq ($(wildcard $(LOCAL_TOOLCHAIN)/bin/clang),)
export PATH := $(LOCAL_TOOLCHAIN)/bin:$(PATH)
export LD_LIBRARY_PATH := $(LOCAL_TOOLCHAIN)/lib:$(LD_LIBRARY_PATH)
CLANG ?= $(LOCAL_TOOLCHAIN)/bin/clang
LLVM_CONFIG ?= $(LOCAL_TOOLCHAIN)/bin/llvm-config
LLVM_OPT ?= $(LOCAL_TOOLCHAIN)/bin/opt
endif

CLANG ?= clang
LLVM_CONFIG ?= llvm-config
LLVM_OPT ?= opt
export CLANG LLVM_OPT CC
PKG_CONFIG ?= pkg-config
OPT ?= O2
FRAMES ?= 64
CFLAGS := -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
LLVM_CXXFLAGS = $(patsubst -I%,-isystem %,$(shell $(LLVM_CONFIG) --cxxflags))
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --link-shared --ldflags --libs core passes --system-libs)
SDL_CFLAGS = $(shell $(PKG_CONFIG) --cflags sdl2)
SDL_LIBS = $(shell $(PKG_CONFIG) --libs sdl2)

.PHONY: all run check stats clean ir
.SECONDARY:

all: build/game-O2 build/game-Os build/analyze

build:
	mkdir -p $@

build/pass.o: pass.c pass.h opcode.h trace_format.h | build
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -fPIC -c $< -o $@

build/plugin.o: plugin.cpp pass.h | build
	$(CXX) $(LLVM_CXXFLAGS) -O2 -Wall -Wextra -Werror -fPIC -c $< -o $@

build/IRTrace.so: build/plugin.o build/pass.o
	$(CXX) -shared $^ $(LLVM_LDFLAGS) -o $@

build/analyze: analyze.c opcode.h trace_format.h | build
	$(CC) $(CFLAGS) analyze.c -o $@

build/main.o: main.c sim.h sim_control.h | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

build/sim.o: sim.c sim.h sim_control.h | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

build/trace_runtime.o: trace_runtime.c trace_format.h | build
	$(CC) $(CFLAGS) -c $< -o $@

# Clang loads the pass into its own optimization pipeline. No IR parser/driver.
build/%/app.o build/%/sites.tsv &: app.c sim.h build/IRTrace.so
	mkdir -p build/$*
	IR_TRACE_SITES=build/$*/sites.tsv $(CLANG) $(filter-out -O2,$(CFLAGS)) -$* -fpass-plugin=$(CURDIR)/build/IRTrace.so -c app.c -o build/$*/app.o

build/%/plain.o: app.c sim.h
	mkdir -p $(@D)
	$(CLANG) $(filter-out -O2,$(CFLAGS)) -$* -c $< -o $@

# Optional human-readable snapshots; these are not inputs to the build.
build/%/app.ll: app.c sim.h
	mkdir -p $(@D)
	$(CLANG) $(filter-out -O2,$(CFLAGS)) -$* -S -emit-llvm $< -o $@

build/%/instrumented.ll: app.c sim.h build/IRTrace.so
	mkdir -p $(@D)
	IR_TRACE_SITES=build/$*/ir-sites.tsv $(CLANG) $(filter-out -O2,$(CFLAGS)) -$* -fpass-plugin=$(CURDIR)/build/IRTrace.so -S -emit-llvm $< -o $@

ir: build/O2/app.ll build/Os/app.ll build/O2/instrumented.ll build/Os/instrumented.ll

build/game-%: build/%/app.o build/main.o build/sim.o build/trace_runtime.o
	$(CC) $^ $(SDL_LIBS) -o $@

build/plain-%: build/%/plain.o build/main.o build/sim.o
	$(CC) $^ $(SDL_LIBS) -o $@

run: build/game-$(OPT)
	IR_TRACE_FILE=build/$(OPT)/trace.bin $<

check: all build/plain-O2 build/plain-Os
	sh tests/check.sh

stats: all build/plain-O2 build/plain-Os
	mkdir -p stats/O2 stats/Os
	$(LLVM_CONFIG) --version > stats/llvm-version.txt
	$(CLANG) --version | sed '/^InstalledDir:/d' > stats/clang-version.txt
	$(PKG_CONFIG) --modversion sdl2 > stats/sdl-version.txt
	sha256sum app.c sim.h sim.c sim_control.h main.c pass.c pass.h plugin.cpp trace_runtime.c trace_format.h analyze.c opcode.h Makefile tests/check.sh tests/app.ll tests/driver.c tests/verify_patterns.c > stats/source-sha256.txt
	printf 'frames\t$(FRAMES)\ngrid\t384x192\ncanvas\t1536x768\nants\t32\nsteps_per_frame\t128\ncolours\t6\n' > stats/workload.tsv
	IR_TRACE_FILE=build/O2/trace.bin build/game-O2 --headless --frames $(FRAMES) > stats/O2/result.txt
	build/plain-O2 --headless --frames $(FRAMES) > build/O2/plain-result.txt
	cmp stats/O2/result.txt build/O2/plain-result.txt
	cp build/O2/sites.tsv stats/O2/sites.tsv
	build/analyze stats/O2/sites.tsv build/O2/trace.bin stats/O2
	gzip -n -c build/O2/trace.bin > stats/O2/trace.bin.gz
	IR_TRACE_FILE=build/Os/trace.bin build/game-Os --headless --frames $(FRAMES) > stats/Os/result.txt
	build/plain-Os --headless --frames $(FRAMES) > build/Os/plain-result.txt
	cmp stats/Os/result.txt build/Os/plain-result.txt
	cmp stats/O2/result.txt stats/Os/result.txt
	cp build/Os/sites.tsv stats/Os/sites.tsv
	build/analyze stats/Os/sites.tsv build/Os/trace.bin stats/Os
	gzip -n -c build/Os/trace.bin > stats/Os/trace.bin.gz

clean:
	rm -rf build
