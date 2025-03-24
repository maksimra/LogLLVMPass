.PHONY: all clean

all: build build/LogPass.so build/main_opt DefUseGraph.png

# set llvm-config, cc, cxx as a variables, so user can set correct versions or custom paths of them
build/LogPass.so: LogPass.cpp
	@clang -shared -fPIC LogPass.cpp -o build/LogPass.so `llvm-config-19 --cxxflags --ldflags --system-libs --libs core`

build:
	@mkdir build

build/main_opt: main.c build/LogPass.so # Why? Is it some sort of test? Or I should add all of my programms I want compile with your pass to your makefile?
	@clang -fpass-plugin=./build/LogPass.so -O1 main.c -o build/main_opt -lstdc++

DefUseGraph.png: build/DefUseGraph.dot # Why? Wouldn't it be better to write some sort of script that compiles and plots graph of USER's program?
	@dot -Tpng build/DefUseGraph.dot -o build/DefUseGraph.png

clean:
	@rm -rf build


