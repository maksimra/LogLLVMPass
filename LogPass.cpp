#include <fstream>
#include <iostream>
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

void insertLoggingCall(llvm::IRBuilder<> &builder, llvm::Value *valueToLog, const std::string &type)
{
    llvm::errs() << "Value type: " << *valueToLog->getType() << "\n";
    llvm::Module *M = builder.GetInsertBlock()->getModule();

    llvm::FunctionCallee logFunc = M->getOrInsertFunction("logDefUse",
                                                          llvm::Type::getVoidTy(builder.getContext()),
                                                          llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(builder.getContext())),
                                                          llvm::Type::getInt32Ty(builder.getContext()));
    
    llvm::Value *valueToLogInt32 = valueToLog;
    if (valueToLog->getType() != llvm::Type::getInt32Ty(builder.getContext()))
    {
        if (valueToLog->getType()->isIntegerTy(64))
        {
            valueToLogInt32 = builder.CreateTrunc(valueToLog, llvm::Type::getInt32Ty(builder.getContext()));
        }
        else if (valueToLog->getType()->isPointerTy())
        {
            valueToLogInt32 = builder.CreatePtrToInt(valueToLog, llvm::Type::getInt32Ty(builder.getContext()));
        }
        else
        {
            valueToLogInt32 = builder.CreateIntCast(valueToLog, llvm::Type::getInt32Ty(builder.getContext()), true);
        }
    }

    builder.CreateCall(logFunc, 
                       {builder.CreateGlobalStringPtr(type),
                        valueToLogInt32});
}

void buildDefUseGraph(llvm::Function &F)
{
    std::ofstream dotFile("def_use_graph.dot");
    dotFile << "digraph G {\n";

    for (auto &BB : F)
    {
        for (auto &I : BB)
        {
            if (auto *storeInst = llvm::dyn_cast<llvm::StoreInst>(&I))
            {
                llvm::Value *value   = storeInst->getValueOperand();
                llvm::Value *pointer = storeInst->getPointerOperand();

                std::string valueStr = "Value";
                if (auto *constant = llvm::dyn_cast<llvm::ConstantInt>(value))
                {
                    valueStr += " (" + std::to_string(constant->getZExtValue()) + ")";
                }

                dotFile << "\"" << valueStr << " " << value << "\" -> \"Store " << storeInst << "\" [label=\"def\"];\n";
                dotFile << "\"Store " << storeInst << "\" -> \"Pointer " << pointer << "\" [label=\"store\"];\n";
            }
            else if (auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(&I))
            {
                llvm::Value *pointer = loadInst->getPointerOperand();
                std::cerr << "Value " << "\n";
                dotFile << "\"Pointer " << pointer << "\" -> \"Load " << loadInst << "\" [label=\"use\"];\n";
            }
        }
    }

    dotFile << "}\n";
    dotFile.close();
}

void loggingPass (llvm::Function &F)
{
    for (auto &BB : F)
    {
        for (auto &I : BB)
        {
            if (auto *storeInst = llvm::dyn_cast<llvm::StoreInst>(&I))
            {
                llvm::IRBuilder<> builder(storeInst);
                llvm::Value *valueToLog = storeInst->getValueOperand();
                insertLoggingCall(builder, valueToLog, "def");
            }
            else if (auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(&I))
            {
                llvm::IRBuilder<> builder(loadInst);
                llvm::Value *pointerOperand = loadInst->getPointerOperand();
                llvm::Value * valueToLog = builder.CreateLoad(loadInst->getType(), pointerOperand);
                insertLoggingCall(builder, valueToLog, "use");
            }
        }
     }            
}

struct DefUseInstrumentationPass : public llvm::PassInfoMixin<DefUseInstrumentationPass>
{
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM)
    {
        if (F.getName() == "logDefUse")
        {
            return llvm::PreservedAnalyses::all();
        }

        loggingPass(F);
        buildDefUseGraph(F);        

        return llvm::PreservedAnalyses::none();
    }
};

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
    return
    {
        LLVM_PLUGIN_API_VERSION, "LogPass", "v1.0",
        [] (llvm::PassBuilder &PB)
        {
            PB.registerPipelineStartEPCallback([](llvm::ModulePassManager &MPM, llvm::OptimizationLevel)
                                               {
                                                   llvm::errs() << "LogPass: Adding Pass to pipeline!\n";
                                                   MPM.addPass(llvm::createModuleToFunctionPassAdaptor(DefUseInstrumentationPass()));
                                                   return true;
                                                });
        }
    };
}
