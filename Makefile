ifeq ($(origin LDFLAGS), default)
	LDFLAGS = -lstdc++
endif

CFLAGS ?= -O0
CXXFLAGS ?= `llvm-config-18 --cxxflags`
LLVM_FLAGS ?= `llvm-config-18 --cxxflags --ldflags --system-libs --libs core`
LLVM_IR_FLAG = -S -emit-llvm
OUT_O_DIR ?= build
DOT_FOLDER := dot
COMMONINC = -I./include -I./include/utils
SRC = ./source

override CFLAGS += $(COMMONINC)

TEST_FILES = test_files/recursive.c
FUNCTION_SRC = logFunctions.c compare_doubles.c void_stack.c
FUNCTION_OBJ = $(FUNCTION_SRC:%.c=$(OUT_O_DIR)/%.o)

.PHONY: all
all: $(OUT_O_DIR)/LogPass.so $(DOT_FOLDER)

$(OUT_O_DIR)/LogPass.so: $(SRC)/LogPass.cpp $(FUNCTION_OBJ)
	clang -shared -fPIC $^ -o $@ $(LLVM_FLAGS) $(COMMONINC)

$(OUT_O_DIR)/%.o: $(SRC)/%.c $(OUT_O_DIR)
	clang -c $< -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR)/%.o: $(SRC)/utils/%.c $(OUT_O_DIR)
	clang -c $< -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR):
	@mkdir -p $@

$(DOT_FOLDER):
	@mkdir -p $@

.PHONY: graph
graph:
	@cat dot/ControlFlow.dot dot/DefUse.dot > dot/ResultGraph.dot
	dot -Tsvg dot/ResultGraph.dot -o $(OUT_O_DIR)/ResultGraph.svg

.PHONY: clean
clean:
	rm -rf $(OUT_O_DIR) $(DOT_FOLDER) logs