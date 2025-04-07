#pragma once

#include <stdint.h>

enum LogFuncError
{
    LOG_FUNC_ERROR_OK = 0,
    LOG_FUNC_ERROR_CALLOC = 1,
    LOG_FUNC_ERROR_FOPEN = 2,
    LOG_FUNC_ERROR_STACK = 3,
    LOG_FUNC_ERROR_TYPE = 4
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
    LOG_VOID = 9,
    NUMBER_LOG_FUNCTIONS
};

void logDefUseInt1(bool value, const char *instName, ...);
void logDefUseInt8(uint8_t value, const char *instName, ...);
void logDefUseInt16(uint16_t value, const char *instName, ...);
void logDefUseInt32(uint32_t value, const char *instName, ...);
void logDefUseInt64(uint64_t value, const char *instName, ...);
void logDefUseFloat(float value, const char *instName, ...);
void logDefUseDouble(double value, const char *instName, ...);
void logDefUsePointer(const void *value, const char *instName, ...);
void logDefUseVoid(const void *value, const char *instName, ...);
void logFuncPrintError(enum LogFuncError error);
const char *logFuncGetError(enum LogFuncError error);
void logInit();
void logFinish();
void beforeCall(const char *instName);
void processOperands(const char *instName, void *value, enum LogFunctionNumber valueType, va_list *args);
void printDefUseValue(void *value, enum LogFunctionNumber valueType);
void pushInstruction(const char *instName, enum LogFunctionNumber valueType, void *value);
