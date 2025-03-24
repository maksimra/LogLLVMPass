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

enum LogError
{
    ERROR_OK          = 0,
    ERROR_UNKNOW_TYPE = 1
};

enum LogFunctionNumber
{
    LOG_UNKNOW  = 0,
    LOG_INT1    = 1,
    LOG_INT8    = 2,
    LOG_INT16   = 3,
    LOG_INT32   = 4,
    LOG_INT64   = 5,
    LOG_FLOAT   = 6,
    LOG_DOUBLE  = 7,
    LOG_POINTER = 8
};

enum LogIntType
{
    I1_TYPE  = 1,
    I8_TYPE  = 1 << 3,
    I16_TYPE = 1 << 4,
    I32_TYPE = 1 << 5,
    I64_TYPE = 1 << 6
};

struct LogFunction
{
    LogFunctionNumber func_number;
    const char* func_name;
};

// It would be better to move it inside pass' class 
const LogFunction FUNCTIONS[] = 
{
    {LOG_UNKNOW},
    {LOG_INT1,    "logDefUseInt1"},
    {LOG_INT8,    "logDefUseInt8"},
    {LOG_INT16,   "logDefUseInt16"},
    {LOG_INT32,   "logDefUseInt32"},
    {LOG_INT64,   "logDefUseInt64"},
    {LOG_FLOAT,   "logDefUseFloat"},
    {LOG_DOUBLE,  "logDefUseDouble"},
    {LOG_POINTER, "logDefUsePointer"}
};

LogFunctionNumber getLogFuncNum(llvm::Type *type)
{
    if (type->isIntegerTy())
    {
        unsigned bitWidth = type->getIntegerBitWidth();
        switch(bitWidth)
        {
            case I1_TYPE:
                return LOG_INT1;
            case I8_TYPE:
                return LOG_INT8;
            case I16_TYPE:
                return LOG_INT16;
            case I32_TYPE:
                return LOG_INT32;
            case I64_TYPE:
                return LOG_INT64;
            default:
                return LOG_UNKNOW;
        }
    }
    else if (type->isFloatTy())
    {
        return LOG_FLOAT;
    }
    else if (type->isDoubleTy())
    {
        return LOG_DOUBLE;
    }
    else if (type->isPointerTy())
    {
        return LOG_POINTER;
    }
    else 
    {
        return LOG_UNKNOW;
    }
}

const char* get_error_msg(LogError error)
{
    switch (error)
    {
        case ERROR_OK:
            return "No error.";
        case ERROR_UNKNOW_TYPE:
            return "Unknow operand type.";
        default:
            return "Unknow error type.";
    }
}

void print_error(LogError error)
{
    std::cerr << get_error_msg(error) << std::endl;
}

// TODO: Why is it a standalone function, not private method of DefUseInstrumentationPass?
//       Same question for almost all of the other functions in this file
LogError insertLoggingCall(llvm::IRBuilder<> &builder, llvm::Value *valueToLog, const std::string &type)
{
    llvm::Module *M = builder.GetInsertBlock()->getModule();
    llvm::Type *valueType = valueToLog->getType();

    if (valueType->isStructTy())
    {
        llvm::StructType *structTy = llvm::cast<llvm::StructType>(valueType);
        for (unsigned elem_number = 0; elem_number < structTy->getNumElements(); ++elem_number)
        {
            llvm::Value *field = builder.CreateExtractValue(valueToLog, {elem_number});
            insertLoggingCall(builder, field, type + ".field" + std::to_string(elem_number)); // TODO: I assume "type" param is not a type, but a name. So, what if you have two structs with fist value of type int in same basic block? Both of them will have the same name, I believe...
        }
        return ERROR_OK;
    }
    else if (valueType->isArrayTy())
    {
        llvm::ArrayType *arrayTy = llvm::cast<llvm::ArrayType>(valueType);
        for (unsigned elem_number = 0; elem_number < arrayTy->getNumElements(); ++elem_number)
        {
            llvm::Value *element = builder.CreateExtractValue(valueToLog, {elem_number});
            insertLoggingCall(builder, element, type + ".element" + std::to_string(elem_number));
        }
        return ERROR_OK;
    }

    LogFunctionNumber functionNum = getLogFuncNum(valueType);
    if (functionNum == LOG_UNKNOW)
        return ERROR_UNKNOW_TYPE;

    llvm::FunctionCallee logFunc = M->getOrInsertFunction(FUNCTIONS[functionNum].func_name,
                                                          llvm::Type::getVoidTy(builder.getContext()),
                                                          llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(builder.getContext())),
                                                          valueType);

    builder.CreateCall(logFunc, 
                       {builder.CreateGlobalStringPtr(type),
                        valueToLog});

    return ERROR_OK;
}

std::string getStrValue(llvm::Value *value)
{
    if (auto *constantInt = llvm::dyn_cast<llvm::ConstantInt>(value))
        return std::to_string(constantInt->getZExtValue());

    if (auto *constantFP = llvm::dyn_cast<llvm::ConstantFP>(value))
        return std::to_string(constantFP->getValueAPF().convertToDouble());

    if (value->getType()->isPointerTy())
    {
        if (auto *globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value))
            return std::string("0x") + std::to_string(reinterpret_cast<uintptr_t>(globalVar));

        if (auto *constantExpr = llvm::dyn_cast<llvm::ConstantExpr>(value))
            return std::string("0x") + std::to_string(reinterpret_cast<uintptr_t>(constantExpr));

        if (auto *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(value))
            return std::string("0x") + std::to_string(reinterpret_cast<uintptr_t>(allocaInst));
        
        return std::string("pointer");
    }

    if (auto *tempVal = llvm::dyn_cast<llvm::Instruction>(value))
        return std::string("temp_value"); // ? Please, levave comments to explain yourself. There are no "temp values" in IR spec.

    return std::string("error_value");
}

void buildDefUseGraph(llvm::Function &F)
{
    std::ofstream dotFile("build/DefUseGraph.dot");
    dotFile << "digraph G {\n";

    for (auto &BB : F)
    {
        for (auto &I : BB)
        {
            // TODO: What if it's not a store and a load instruction?
            //       for example, phi-node or add / sub / call / mul etc. instruction?
            
            if (auto *storeInst = llvm::dyn_cast<llvm::StoreInst>(&I))
            {
                llvm::Value *value   = storeInst->getValueOperand();
                llvm::Value *pointer = storeInst->getPointerOperand();

                std::string valueStr   = getStrValue(value);
                std::string pointerStr = getStrValue(pointer);

                dotFile << "\"val: " << valueStr << "\" -> \"Pointer " << pointerStr << "\" [label=\"store\"];\n";
            }
            else if (auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(&I))
            {
                llvm::Value *pointer = loadInst->getPointerOperand();
                std::string pointerStr = getStrValue(pointer);

                dotFile << "\"Pointer " << pointerStr << "\" -> \"Load " << loadInst << "\" [label=\"use\"];\n";
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
            LogError error = ERROR_OK;
            // TODO: Same as above. What if it's not steroe / load? For example, alloca inst, or gep inst?
            
            if (auto *storeInst = llvm::dyn_cast<llvm::StoreInst>(&I))
            {
                llvm::IRBuilder<> builder(storeInst);
                llvm::Value *valueToLog = storeInst->getValueOperand();
                error = insertLoggingCall(builder, valueToLog, "def");
            }
            else if (auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(&I))
            {
                llvm::IRBuilder<> builder(loadInst);
                llvm::Value *pointerOperand = loadInst->getPointerOperand();
                llvm::Value * valueToLog = builder.CreateLoad(loadInst->getType(), pointerOperand);
                error = insertLoggingCall(builder, valueToLog, "use");
            }

            if (error)
            {
                print_error (error);
                return;
            }
        }
     }            
}

struct DefUseInstrumentationPass : public llvm::PassInfoMixin<DefUseInstrumentationPass>
{
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM)
    {
        std::string funcName = F.getName().str();
        if (funcName.find("logDefUse") != std::string::npos)
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
                                                    // It's not really good idea to print something here. At least, put this into #ifndef NDEBUG 
                                                   llvm::errs() << "LogPass: Adding Pass to pipeline!\n";
                                                   MPM.addPass(llvm::createModuleToFunctionPassAdaptor(DefUseInstrumentationPass()));
                                                   return true;
                                                });
        }
    };
}
