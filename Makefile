ifeq ($(origin LDFLAGS), default)
	LDFLAGS = -lstdc++
endif

CFLAGS ?= -O0
CXXFLAGS ?= `llvm-config-18 --cxxflags`
LLVM_FLAGS ?= `llvm-config-18 --cxxflags --ldflags --system-libs --libs core`
LLVM_IR_FLAG = -S -emit-llvm
OUT_O_DIR ?= build
COMMONINC = -I./include -I./include/utils
SRC = ./source

override CFLAGS += $(COMMONINC)

CSRC = main.c
FUNCTION_SRC = logFunctions.c compare_doubles.c void_stack.c
FUNCTION_OBJ = $(FUNCTION_SRC:%.c=$(OUT_O_DIR)/%.o)
DEPS = $(CSRC:%.c=$(OUT_O_DIR)/%.d)

.PHONY: all
all: $(DEPS) $(OUT_O_DIR)/LogPass.so $(OUT_O_DIR)/main

$(OUT_O_DIR)/LogPass.so: $(SRC)/LogPass.cpp
	echo "hello $(FUNCTION_OBJ)"
	clang -shared -fPIC $^ -o $@ $(LLVM_FLAGS) $(COMMONINC)

$(OUT_O_DIR)/%.o: $(SRC)/%.c
	clang -c $< -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR)/%.o: $(SRC)/utils/%.c
	clang -c $< -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR)/main: $(OUT_O_DIR)/LogPass.so $(CSRC) $(FUNCTION_OBJ)
	clang -O1 -fpass-plugin=$< $(CSRC) $(FUNCTION_OBJ) -o $@ $(LDFLAGS)

$(DEPS): $(OUT_O_DIR)/%.d: %.c # TODO: не все зависимости обрабатываю
	@mkdir -p $(@D) 
	clang -E $< -MM -MT $(@:%.d=%) > $@

.PHONY: pure_llvm_ir
pure_llvm_ir:
	clang -O0 $(CSRC) $(LLVM_IR_FLAG) -o $(OUT_O_DIR)/main.ll

.PHONY: log_llvm_ir
log_llvm_ir: $(OUT_O_DIR)/LogPass.so $(OUT_O_DIR)/logFunctions.o
	clang -O1 -fpass-plugin=$< $(LLVM_IR_FLAG) $(CSRC) -o $(OUT_O_DIR)/main_log.ll 

.PHONY: graph
graph:
	@cat dot/ControlFlow.dot dot/DefUse.dot > dot/ResultGraph.dot
	dot -Tsvg dot/ResultGraph.dot -o $(OUT_O_DIR)/ResultGraph.svg

.PHONY: run
run:
	@$(OUT_O_DIR)/main

.PHONY: clean
clean:
	rm -rf *.dot $(DEPS) $(OUT_O_DIR)

include $(DEPS)