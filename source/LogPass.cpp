#include <fstream>
#include <iostream>
#include <vector>

#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "logFunctions.h"

enum LogError
{
    ERROR_OK = 0,
    ERROR_UNKNOW_TYPE = 1,
    ERROR_OPEN_FILE = 2,
    ERROR_INVALID_TYPE = 3
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
    case ERROR_INVALID_TYPE:
        return "Invalid type.";
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
                                                                    {LOG_POINTER, "logDefUsePointer"},
                                                                    {LOG_VOID, "logDefUseVoid"}};

  public:
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM)
    {
        std::string funcName = F.getName().str();
        if (funcName.find("logDefUse") != std::string::npos)
        {
            return llvm::PreservedAnalyses::all();
        }

        LogError error = loggingPass(F);
        if (error)
            print_error(error);

        if (funcName == "main")
            insertInitAndFinishCall(F);

        return llvm::PreservedAnalyses::all();
    }

  private:
    static void insertInitAndFinishCall(llvm::Function &mainF)
    {
        llvm::Function::iterator BB = mainF.begin();
        llvm::BasicBlock::iterator firstInstr = BB->begin();

        llvm::IRBuilder<> builder(&*firstInstr);

        llvm::LLVMContext &context = mainF.getContext();
        llvm::Module *M = mainF.getParent();

        llvm::FunctionType *functionsType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);

        llvm::FunctionCallee initFunc = M->getOrInsertFunction("logInit", functionsType);
        builder.CreateCall(functionsType, initFunc.getCallee());

        llvm::FunctionCallee finishFunc = M->getOrInsertFunction("logFinish", functionsType);
        for (llvm::BasicBlock &block : mainF)
        {
            if (llvm::ReturnInst *ret =
                    llvm::dyn_cast<llvm::ReturnInst>(block.getTerminator())) // would we process exit()?
            {
                builder.SetInsertPoint(ret);
                builder.CreateCall(functionsType, finishFunc.getCallee());
            }
        }
    }

    static LogError loggingPass(llvm::Function &F)
    {
        LogError error = ERROR_OK;

        std::ofstream dotFile("dot/ControlFlow.dot");
        if (!dotFile.is_open())
        {
            std::cerr << "Cannot open dotFile.\n";
            return ERROR_OPEN_FILE;
        }
        dotFile << "digraph G {\n"
                << "rankdir = LR;\n"
                << "{rank = same;\n\"CFG\""; // graph head

        std::vector<llvm::Instruction *> instructions;
        for (auto &BB : F)
        {
            for (auto &I : BB)
            {
                if (!shouldSkipInstruction(&I))
                    instructions.push_back(&I);
            }
        }

        for (auto I : instructions)
        {
            std::vector<llvm::Instruction *> defOperands;
            for (llvm::Use &U : I->operands())
            {
                llvm::Value *value = U.get();
                if (llvm::Instruction *sourceInst = llvm::dyn_cast<llvm::Instruction>(value))
                {
                    defOperands.push_back(sourceInst);
                }
            }

            llvm::IRBuilder<> builder(I);
            if (auto *callInst = llvm::dyn_cast<llvm::CallInst>(I))
                insertInstrBeforeCall(builder, *I);

            builder.SetInsertPoint(I->getNextNode()); // for insert after "I"
            llvm::Value *computedValue = I;
            error = insertLoggingCall(builder, *I, computedValue, defOperands);

            if (error)
                return error;
        }

        return ERROR_OK;
    }

    static void insertInstrBeforeCall(llvm::IRBuilder<> &builder, llvm::Instruction &I)
    {
        llvm::FunctionType *funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(builder.getContext()),
                                    {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(builder.getContext()))}, false);

        llvm::Module *M = builder.GetInsertBlock()->getModule();
        llvm::FunctionCallee logBeforeCall = M->getOrInsertFunction("beforeCall", funcType);

        std::string instructionName;
        llvm::raw_string_ostream ostream(instructionName);
        I.print(ostream);

        builder.CreateCall(funcType, logBeforeCall.getCallee(), builder.CreateGlobalStringPtr(instructionName));
    }

    static bool shouldSkipInstruction(llvm::Instruction *I)
    {
        if (I->isTerminator())
        {
            return true;
        }
        else if (auto *CI = llvm::dyn_cast<llvm::CallInst>(I))
        {
            if (auto *F = CI->getCalledFunction())
            {
                llvm::Intrinsic::ID ID = (llvm::Intrinsic::ID)F->getIntrinsicID();
                if (ID == llvm::Intrinsic::lifetime_start || ID == llvm::Intrinsic::lifetime_end)
                {
                    return true;
                }
            }
        }

        return false;
    }

    static LogError insertLoggingCall(llvm::IRBuilder<> &builder, llvm::Instruction &I, llvm::Value *valueToLog,
                                      const std::vector<llvm::Instruction *> &defOperands)
    {
        llvm::Type *valueType = valueToLog->getType();

        if (valueType->isStructTy())
        {
            llvm::StructType *structTy = llvm::cast<llvm::StructType>(valueType);
            for (unsigned elem_number = 0; elem_number < structTy->getNumElements(); ++elem_number)
            {
                llvm::Value *field = builder.CreateExtractValue(valueToLog, {elem_number});
                insertLoggingCall(builder, I, field, defOperands);
            }
            return ERROR_OK;
        }
        else if (valueType->isArrayTy())
        {
            llvm::ArrayType *arrayTy = llvm::cast<llvm::ArrayType>(valueType);
            for (unsigned elem_number = 0; elem_number < arrayTy->getNumElements(); ++elem_number)
            {
                llvm::Value *element = builder.CreateExtractValue(valueToLog, {elem_number});
                insertLoggingCall(builder, I, element, defOperands);
            }
            return ERROR_OK;
        }

        LogFunctionNumber functionNum = getLogFuncNum(valueType);
        if (functionNum == LOG_UNKNOW)
            return ERROR_UNKNOW_TYPE;
        if (functionNum == LOG_VOID)
        {
            valueType = llvm::PointerType::getUnqual(builder.getContext()); // ptr null
            valueToLog = llvm::ConstantPointerNull::get(llvm::dyn_cast<llvm::PointerType>(valueType));
        }

        llvm::FunctionType *funcType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(builder.getContext()),
            {valueType, llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(builder.getContext()))},
            true // varargs
        );

        llvm::Module *M = builder.GetInsertBlock()->getModule();
        llvm::FunctionCallee logFunc = M->getOrInsertFunction(FUNCTIONS[functionNum].func_name, funcType);

        std::string instructionText;
        llvm::raw_string_ostream ostream(instructionText);
        I.print(ostream);

        std::vector<llvm::Value *> args;
        fillVectorArguments(builder, args, valueToLog, instructionText, defOperands);

        builder.CreateCall(funcType, logFunc.getCallee(), args);

        return ERROR_OK;
    }

    static void fillVectorArguments(llvm::IRBuilder<> &builder, std::vector<llvm::Value *> &args,
                                    llvm::Value *valueToLog, const std::string &instrText,
                                    const std::vector<llvm::Instruction *> &defOperands)
    {
        args.push_back(valueToLog);
        args.push_back(builder.CreateGlobalStringPtr(instrText));

        for (llvm::Instruction *instr : defOperands)
        {
            std::string defOperandsText;
            llvm::raw_string_ostream rso(defOperandsText);
            instr->print(rso);
            rso.flush();

            args.push_back(builder.CreateGlobalStringPtr(defOperandsText));
        }
        args.push_back(llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(builder.getContext())));
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
        else if (type->isVoidTy())
        {
            return LOG_VOID;
        }
        else
        {
            return LOG_UNKNOW;
        }
    }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "LoggingPass", LLVM_VERSION_STRING, [](llvm::PassBuilder &PB) {
                PB.registerPipelineStartEPCallback([](llvm::ModulePassManager &MPM, llvm::OptimizationLevel) {
                    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(DefUseInstrumentationPass()));
                });
            }};
}
