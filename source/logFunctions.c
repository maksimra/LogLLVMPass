#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logFunctions.h"
#include "void_stack.h"

union Value {
    bool i1_val;
    uint8_t i8_val;
    uint16_t i16_val;
    uint32_t i32_val;
    uint64_t i64_val;
    float float_val;
    double double_val;
    const void *pointer_val;
};

struct Instruction
{
    const char *name;
    enum LogFunctionNumber type;
    union Value value;
};

static FILE *logFile = NULL;

static struct Stack *INSTR_STACK = NULL;
static FILE *CONTROL_FLOW_FILE = NULL;
static FILE *DEF_USE_FILE = NULL;

void logInit()
{
    logFile = fopen("logs/logFile.txt", "w");
    if (!logFile)
    {
        fprintf(stderr, "cannot open log file.\n");
        logFile = stderr;
    }

    stack_set_log_file(logFile);

    INSTR_STACK = (struct Stack *)calloc(1, sizeof(struct Stack));
    if (!INSTR_STACK)
    {
        logFuncPrintError(LOG_FUNC_ERROR_CALLOC);
        exit(EXIT_FAILURE);
    }

    enum StkError error = stack_ctor(INSTR_STACK, sizeof(struct Instruction));
    if (error)
    {
        logFuncPrintError(LOG_FUNC_ERROR_STACK);
        exit(EXIT_FAILURE);
    }

    CONTROL_FLOW_FILE = fopen("dot/ControlFlow.dot", "a");
    DEF_USE_FILE = fopen("dot/DefUse.dot", "w");
    if ((!CONTROL_FLOW_FILE) || (!DEF_USE_FILE))
    {
        logFuncPrintError(LOG_FUNC_ERROR_FOPEN);
        exit(EXIT_FAILURE);
    }
}

void logFinish()
{
    enum StkError error = stack_dtor(INSTR_STACK);
    if (error)
    {
        logFuncPrintError(LOG_FUNC_ERROR_STACK);
        exit(EXIT_FAILURE);
    }

    fprintf(CONTROL_FLOW_FILE, " [label=\"cfg\"];}\n");
    fprintf(DEF_USE_FILE, "}");

    fclose(CONTROL_FLOW_FILE);
    fclose(DEF_USE_FILE);
    fclose(logFile);
}

void processOperands(const char *instName, void *value, enum LogFunctionNumber valueType, va_list *args)
{
    const char *operandInstr = NULL;

    while ((operandInstr = va_arg(*args, const char *)))
    {
        for (int numberDefInstr = 0; numberDefInstr < INSTR_STACK->size; numberDefInstr++)
        {
            if (strcmp(operandInstr, ((struct Instruction *)INSTR_STACK->data + numberDefInstr)->name) == 0)
            {
                fprintf(DEF_USE_FILE, "\"%s| VALUE = ", operandInstr);
                printDefUseValue(&(((struct Instruction *)INSTR_STACK->data + numberDefInstr)->value),
                                 ((struct Instruction *)INSTR_STACK->data + numberDefInstr)->type);
                fprintf(DEF_USE_FILE, "\" -> \"%s| VALUE = ", instName);
                printDefUseValue(value, valueType);
                fprintf(DEF_USE_FILE, "\" [label = \"user\"];\n");
                break;
            }
        }
    }
}

void printDefUseValue(void *value, enum LogFunctionNumber valueType)
{
    switch (valueType)
    {
    case LOG_INT1:
        fprintf(DEF_USE_FILE, "%d", *((bool *)value));
        break;
    case LOG_INT8:
        fprintf(DEF_USE_FILE, "%hhd", *((uint8_t *)value));
        break;
    case LOG_INT16:
        fprintf(DEF_USE_FILE, "%hd", *((uint16_t *)value));
        break;
    case LOG_INT32:
        fprintf(DEF_USE_FILE, "%d", *((uint32_t *)value));
        break;
    case LOG_INT64:
        fprintf(DEF_USE_FILE, "%ld", *((uint64_t *)value));
        break;
    case LOG_FLOAT:
        fprintf(DEF_USE_FILE, "%g", *((float *)value));
        break;
    case LOG_DOUBLE:
        fprintf(DEF_USE_FILE, "%g", *((double *)value));
        break;
    case LOG_POINTER:
    case LOG_VOID:
        fprintf(DEF_USE_FILE, "%p", *((const void **)value));
        break;
    default:
        logFuncPrintError(LOG_FUNC_ERROR_TYPE);
        exit(EXIT_FAILURE);
    }
}

void pushInstruction(const char *instName, enum LogFunctionNumber valueType, void *value)
{
    struct Instruction newInstruction = {.name = instName, .type = valueType};

    switch (valueType)
    {
    case LOG_INT1:
        newInstruction.value.i1_val = *((bool *)value);
        break;
    case LOG_INT8:
        newInstruction.value.i8_val = *((uint8_t *)value);
        break;
    case LOG_INT16:
        newInstruction.value.i16_val = *((uint16_t *)value);
        break;
    case LOG_INT32:
        newInstruction.value.i32_val = *((uint32_t *)value);
        break;
    case LOG_INT64:
        newInstruction.value.i64_val = *((uint64_t *)value);
        break;
    case LOG_FLOAT:
        newInstruction.value.float_val = *((float *)value);
        break;
    case LOG_DOUBLE:
        newInstruction.value.double_val = *((double *)value);
        break;
    case LOG_POINTER:
    case LOG_VOID:
        newInstruction.value.pointer_val = *((const void **)value);
        break;
    default:
        logFuncPrintError(LOG_FUNC_ERROR_TYPE);
        exit(EXIT_FAILURE);
    }

    enum StkError error = stack_push(INSTR_STACK, &newInstruction);
    if (error)
    {
        logFuncPrintError(LOG_FUNC_ERROR_STACK);
        exit(EXIT_FAILURE);
    }
}

void beforeCall(const char *instName)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s\"", instName);
}

void logDefUseInt1(bool value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %d\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_INT1, &args);
    va_end(args);

    pushInstruction(instName, LOG_INT1, &value);
}

void logDefUseInt8(uint8_t value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %hhd\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_INT8, &args);
    va_end(args);

    pushInstruction(instName, LOG_INT8, &value);
}

void logDefUseInt16(uint16_t value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %hd\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_INT16, &args);
    va_end(args);

    pushInstruction(instName, LOG_INT16, &value);
}

void logDefUseInt32(uint32_t value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %d\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_INT32, &args);
    va_end(args);

    pushInstruction(instName, LOG_INT32, &value);
}

void logDefUseInt64(uint64_t value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %ld\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_INT64, &args);
    va_end(args);

    pushInstruction(instName, LOG_INT64, &value);
}

void logDefUseFloat(float value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %g\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_FLOAT, &args);
    va_end(args);

    pushInstruction(instName, LOG_FLOAT, &value);
}

void logDefUseDouble(double value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %g\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_DOUBLE, &args);
    va_end(args);

    pushInstruction(instName, LOG_DOUBLE, &value);
}

void logDefUsePointer(const void *value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %p\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_POINTER, &args);
    va_end(args);

    pushInstruction(instName, LOG_POINTER, &value);
}

void logDefUseVoid(const void *value, const char *instName, ...)
{
    assert(instName);

    fprintf(CONTROL_FLOW_FILE, " -> \"%s| VALUE = %p\"", instName, value);

    va_list args;
    va_start(args, instName);
    processOperands(instName, &value, LOG_VOID, &args);
    va_end(args);

    pushInstruction(instName, LOG_VOID, &value);
}

void logFuncPrintError(enum LogFuncError error)
{
    fprintf(stderr, "%s\n", logFuncGetError(error));
}

const char *logFuncGetError(enum LogFuncError error)
{
    switch (error)
    {
    case LOG_FUNC_ERROR_OK:
        return "logFunc: Ошибок в работе функций не выявлено.";
    case LOG_FUNC_ERROR_CALLOC:
        return "logFunc: Ошибка в работе функции calloc.";
    case LOG_FUNC_ERROR_STACK:
        return "logFunc: Ошибка в работе stack.";
    case LOG_FUNC_ERROR_FOPEN:
        return "logFunc: Ошибка открытия файла.";
    case LOG_FUNC_ERROR_TYPE:
        return "logFunc: Ошибка распознования типа.";
    default:
        return "logFunc: Нужной ошибки не найдено...";
    }
}