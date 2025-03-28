#include <fstream>
#include <iostream>

#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

enum LogError
{
    ERROR_OK = 0,
    ERROR_UNKNOW_TYPE = 1,
    ERROR_OPEN_FILE = 2
};

enum LogFunctionNumber
{
    LOG_UNKNOW = 0,
    LOG_INT1 = 1,
    LOG_INT8 = 2,
    LOG_INT16 = 3,
    LOG_INT32 = 4,
    LOG_INT64 = 5,
    LOG_FLOAT = 6,
    LOG_DOUBLE = 7,
    LOG_POINTER = 8,
    NUMBER_LOG_FUNCTIONS
};

enum LogIntType
{
    I1_TYPE = 1,
    I8_TYPE = 1 << 3,
    I16_TYPE = 1 << 4,
    I32_TYPE = 1 << 5,
    I64_TYPE = 1 << 6
};

struct LogFunction
{
    LogFunctionNumber func_number;
    const char *func_name;
};

const char *get_error_msg(LogError error)
{
    switch (error)
    {
    case ERROR_OK:
        return "No error.";
    case ERROR_UNKNOW_TYPE:
        return "Unknow operand type.";
    case ERROR_OPEN_FILE:
        return "Cannot open file.";
    default:
        return "Unknow error type.";
    }
}

void print_error(LogError error)
{
    std::cerr << get_error_msg(error) << std::endl;
}

namespace
{
class DefUseInstrumentationPass : public llvm::PassInfoMixin<DefUseInstrumentationPass>
{
    static constexpr LogFunction FUNCTIONS[NUMBER_LOG_FUNCTIONS] = {{LOG_UNKNOW},
                                                         {LOG_INT1, "logDefUseInt1"},
                                                         {LOG_INT8, "logDefUseInt8"},
                                                         {LOG_INT16, "logDefUseInt16"},
                                                         {LOG_INT32, "logDefUseInt32"},
                                                         {LOG_INT64, "logDefUseInt64"},
                                                         {LOG_FLOAT, "logDefUseFloat"},
                                                         {LOG_DOUBLE, "logDefUseDouble"},
                                                         {LOG_POINTER, "logDefUsePointer"}};

  public:
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM)
    {
        llvm::errs() << "Running YourPass on function: " << F.getName() << "\n";
        std::string funcName = F.getName().str();
        if (funcName.find("logDefUse") != std::string::npos)
        {
            return llvm::PreservedAnalyses::all();
        }

        loggingPass(F);

        LogError error = buildDefUseGraph(F);
        if (error)
            print_error(error);

        return llvm::PreservedAnalyses::none();
    }

  private:
    static void loggingPass(llvm::Function &F)
    {
        for (auto &BB : F)
        {
            for (auto &I : BB)
            {
                LogError error = ERROR_OK;
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
                    llvm::Value *valueToLog = builder.CreateLoad(loadInst->getType(), pointerOperand);
                    error = insertLoggingCall(builder, valueToLog, "use");
                }

                if (error)
                {
                    print_error(error);
                    return;
                }
            }
        }
    }

    static LogError buildDefUseGraph(llvm::Function &F)
    {
        std::ofstream dotFile("DefUseGraph.dot");
        if (!dotFile.is_open())
        {
            std::cerr << "Cannot open dotFile.\n";
            return ERROR_OPEN_FILE;
        }
        dotFile << "digraph G {\n";

        for (auto &BB : F)
        {
            for (auto &I : BB)
            {
                if (auto *storeInst = llvm::dyn_cast<llvm::StoreInst>(&I))
                {
                    llvm::Value *value = storeInst->getValueOperand();
                    llvm::Value *pointer = storeInst->getPointerOperand();

                    std::string valueStr = getStrValue(value);
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
        return ERROR_OK;
    }

    static std::string getStrValue(llvm::Value *value)
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
            return std::string("temp_value");

        return std::string("error_value");
    }

    static LogError insertLoggingCall(llvm::IRBuilder<> &builder, llvm::Value *valueToLog, const std::string &type)
    {
        llvm::Module *M = builder.GetInsertBlock()->getModule();
        llvm::Type *valueType = valueToLog->getType();

        if (valueType->isStructTy())
        {
            llvm::StructType *structTy = llvm::cast<llvm::StructType>(valueType);
            for (unsigned elem_number = 0; elem_number < structTy->getNumElements(); ++elem_number)
            {
                llvm::Value *field = builder.CreateExtractValue(valueToLog, {elem_number});
                insertLoggingCall(builder, field, type + ".field" + std::to_string(elem_number));
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

        llvm::FunctionCallee logFunc = M->getOrInsertFunction(
            FUNCTIONS[functionNum].func_name, llvm::Type::getVoidTy(builder.getContext()),
            llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(builder.getContext())), valueType);

        builder.CreateCall(logFunc, {builder.CreateGlobalStringPtr(type), valueToLog});

        return ERROR_OK;
    }

    static LogFunctionNumber getLogFuncNum(llvm::Type *type)
    {
        if (type->isIntegerTy())
        {
            unsigned bitWidth = type->getIntegerBitWidth();
            switch (bitWidth)
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
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "LoggingPass",
        LLVM_VERSION_STRING,
        [](llvm::PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](llvm::ModulePassManager &MPM, llvm::OptimizationLevel) {
                    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(DefUseInstrumentationPass()));
                }
            );
        }
    };
}
