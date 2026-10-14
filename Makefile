LOCAL_TOOLCHAIN := $(CURDIR)/.tools/toolchain/usr
ifneq ($(wildcard $(LOCAL_TOOLCHAIN)/bin/clang),)
export PATH := $(LOCAL_TOOLCHAIN)/bin:$(PATH)
export LD_LIBRARY_PATH := $(LOCAL_TOOLCHAIN)/lib:$(LD_LIBRARY_PATH)
CLANG ?= $(LOCAL_TOOLCHAIN)/bin/clang
LLVM_CONFIG ?= $(LOCAL_TOOLCHAIN)/bin/llvm-config
endif

CLANG ?= clang
LLVM_CONFIG ?= llvm-config
export CLANG
PKG_CONFIG ?= pkg-config
OPT ?= O2
FRAMES ?= 64
SEED ?= 42
CFLAGS := -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --link-shared --ldflags --libs core irreader bitwriter analysis --system-libs)
SDL_CFLAGS = $(shell $(PKG_CONFIG) --cflags sdl2)
SDL_LIBS = $(shell $(PKG_CONFIG) --libs sdl2)

.PHONY: all run stats clean
.SECONDARY:

all: build/game-O2 build/game-Os build/analyze

build:
	mkdir -p $@

build/instrument: instrument.c pass.c pass.h opcode.h trace_format.h | build
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) instrument.c pass.c $(LLVM_LDFLAGS) -o $@

build/analyze: analyze.c opcode.h trace_format.h | build
	$(CC) $(CFLAGS) analyze.c -o $@

build/main.o: main.c app.h | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

build/trace_runtime.o: trace_runtime.c trace_format.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%/app.ll: app.c app.h
	mkdir -p $(@D)
	$(CLANG) $(filter-out -O2,$(CFLAGS)) -$* -S -emit-llvm $< -o $@

build/%/instrumented.bc build/%/sites.tsv &: build/%/app.ll build/instrument
	build/instrument $< build/$*/instrumented.bc build/$*/sites.tsv

build/%/app.o: build/%/instrumented.bc
	$(CLANG) -O0 -c $< -o $@

build/%/plain.o: build/%/app.ll
	$(CLANG) -O0 -c $< -o $@

build/game-%: build/%/app.o build/main.o build/trace_runtime.o
	$(CC) $^ $(SDL_LIBS) -o $@

build/plain-%: build/%/plain.o build/main.o
	$(CC) $^ $(SDL_LIBS) -o $@

run: build/game-$(OPT)
	IR_TRACE_FILE=build/$(OPT)/trace.bin $<

stats: all build/plain-O2 build/plain-Os
	mkdir -p stats/O2 stats/Os
	$(LLVM_CONFIG) --version > stats/llvm-version.txt
	$(CLANG) --version | sed '/^InstalledDir:/d' > stats/clang-version.txt
	$(PKG_CONFIG) --modversion sdl2 > stats/sdl-version.txt
	sha256sum app.c app.h main.c pass.c pass.h instrument.c trace_runtime.c trace_format.h analyze.c opcode.h Makefile > stats/source-sha256.txt
	printf 'frames\t$(FRAMES)\nseed\t$(SEED)\ngrid\t64x48\n' > stats/workload.tsv
	IR_TRACE_FILE=build/O2/trace.bin build/game-O2 --headless --frames $(FRAMES) --seed $(SEED) > stats/O2/result.txt
	build/plain-O2 --headless --frames $(FRAMES) --seed $(SEED) > build/O2/plain-result.txt
	cmp stats/O2/result.txt build/O2/plain-result.txt
	cp build/O2/sites.tsv stats/O2/sites.tsv
	build/analyze stats/O2/sites.tsv build/O2/trace.bin stats/O2
	gzip -n -c build/O2/trace.bin > stats/O2/trace.bin.gz
	IR_TRACE_FILE=build/Os/trace.bin build/game-Os --headless --frames $(FRAMES) --seed $(SEED) > stats/Os/result.txt
	build/plain-Os --headless --frames $(FRAMES) --seed $(SEED) > build/Os/plain-result.txt
	cmp stats/Os/result.txt build/Os/plain-result.txt
	cmp stats/O2/result.txt stats/Os/result.txt
	cp build/Os/sites.tsv stats/Os/sites.tsv
	build/analyze stats/Os/sites.tsv build/Os/trace.bin stats/Os
	gzip -n -c build/Os/trace.bin > stats/Os/trace.bin.gz

clean:
	rm -rf build
