ifeq ($(origin LDFLAGS), default)
	LDFLAGS = -lstdc++
endif

CFLAGS ?= -O0
CXXFLAGS ?= `llvm-config-18 --cxxflags`
LLVM_FLAGS ?= `llvm-config-18 --cxxflags --ldflags --system-libs --libs core`
LLVM_IR_FLAG = -S -emit-llvm
OUT_O_DIR ?= build
COMMONINC = -I./include
SRC = ./source

override CFLAGS += $(COMMONINC)

CSRC = main.c
DEPS = $(CSRC:%.c=$(OUT_O_DIR)/%.d)

.PHONY: all
all: $(DEPS) $(OUT_O_DIR)/LogPass.so $(OUT_O_DIR)/main

$(OUT_O_DIR)/LogPass.so: $(SRC)/LogPass.cpp
	clang -shared -fPIC $^ -o $@ $(LLVM_FLAGS)

$(OUT_O_DIR)/logFunctions.o: $(SRC)/logFunctions.c
	clang -c $^ -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR)/main: $(OUT_O_DIR)/LogPass.so $(CSRC) $(OUT_O_DIR)/logFunctions.o
	clang -O1 -disable-llvm-passes -fpass-plugin=$< $(CSRC) $(OUT_O_DIR)/logFunctions.o -o $@ $(LDFLAGS)

$(DEPS): $(OUT_O_DIR)/%.d: %.c
	@mkdir -p $(@D)
	clang -E $< -MM -MT $(@:%.d=%) > $@

.PHONY: pure_llvm_ir
pure_llvm_ir:
	clang -O0 $(CSRC) $(LLVM_IR_FLAG) -o $(OUT_O_DIR)/main.ll

.PHONY: graph
graph:
	dot -Tsvg DefUseGraph.dot -o $(OUT_O_DIR)/DefUseGraph.svg

.PHONY: run
run:
	@$(OUT_O_DIR)/main

.PHONY: clean
clean:
	rm -rf *.dot $(DEPS) $(OUT_O_DIR)