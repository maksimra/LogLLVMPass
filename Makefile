.PHONY: all clean

all: build build/LogPass.so build/main_opt DefUseGraph.png

build/LogPass.so: LogPass.cpp
	@clang -shared -fPIC LogPass.cpp -o build/LogPass.so `llvm-config --cxxflags --ldflags --system-libs --libs core`

build:
	@mkdir build

build/main_opt: main.c build/LogPass.so
	@clang -fpass-plugin=./build/LogPass.so -O1 main.c -o build/main_opt -lstdc++

DefUseGraph.png: build/DefUseGraph.dot
	@dot -Tpng build/DefUseGraph.dot -o build/DefUseGraph.png

clean:
	@rm -rf build


