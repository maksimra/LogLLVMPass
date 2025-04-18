ifeq ($(origin LDFLAGS), default)
	LDFLAGS = -lstdc++
endif

CFLAGS ?= -O0
LLVM_FLAGS ?= `llvm-config-18 --cxxflags --ldflags --system-libs --libs core`
OUT_O_DIR ?= build
DOT_FOLDER = dot
LOG_FOLDER = logs 
COMMONINC = -I./include -I./include/utils
SRC = ./source

override CFLAGS += $(COMMONINC) -MMD -MP

FUNCTION_SRC = logFunctions.c compare_doubles.c void_stack.c
FUNCTION_OBJ = $(FUNCTION_SRC:%.c=$(OUT_O_DIR)/%.o)

DEPS = $(FUNCTION_OBJ:%.o=%.d)
-include $(DEPS)

.PHONY: install
install:
	@sudo cp shlang /usr/local/bin/shlang 
	@sudo chmod +x /usr/local/bin/shlang   

.PHONY: all
all: $(OUT_O_DIR)/LogPass.so $(OUT_O_DIR)/liblog.a $(DOT_FOLDER) $(LOG_FOLDER)

$(OUT_O_DIR)/LogPass.so: $(SRC)/LogPass.cpp $(OUT_O_DIR)
	clang -shared -fPIC $< -o $@ $(LLVM_FLAGS) $(COMMONINC)

$(OUT_O_DIR)/liblog.a: $(FUNCTION_OBJ)
	ar rcs $@ $^

$(OUT_O_DIR)/%.o: $(SRC)/%.c $(OUT_O_DIR)
	clang -c $< -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR)/%.o: $(SRC)/utils/%.c $(OUT_O_DIR) 
	clang -c $< -o $@ -fPIC $(CFLAGS)

$(OUT_O_DIR):
	@mkdir -p $@

$(LOG_FOLDER):
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