#pragma once

#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

// TODO: This file is unused. Do you really need this headr?

void              loggingPass      (llvm::Function &F);
void              buildDefUseGraph (llvm::Function &F);
std::string       getStrValue      (llvm::Value *value);
LogError          insertLoggingCall(llvm::IRBuilder<> &builder, llvm::Value *valueToLog, const std::string &type);
LogFunctionNumber getLogFuncNum    (llvm::Type *type);
void              print_error      (LogError error);
const char*       get_error_msg    (LogError error);
