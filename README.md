# Logging pass for LLVM
## Description
This program adds instrumentation to LLVM IR and generates a .png def-use graph. 
## Requirements
- *OC*: Linux
- *Compiler*: Clang 18.1.3
- *LLVM*: Version 18.1.3 (built with RTTI support)
## Build
(for the code in main.c)
1) Clone the repository:
```
git clone https://github.com/maksimra/LogLLVMPass.git
```
2) Go to the folder with project:
```
cd LogLLVMPass
```
3) Compile the project:
```
make
```
- To run:
```
make run
```
- To build def-use graph (only after run):
```
make graph
```
- To build pure LLVM IR
```
make pure_llvm_ir
```
- To build instrumentated LLVM IR (only after compilation)
```
make instr_llvm_ir
```

***

After completing these steps, you will have:

- The def-use graph: build/ResultGraph.svg