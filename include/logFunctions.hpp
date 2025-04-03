#pragma once

#include <stdint.h>

void logDefUseInt1(bool value, const char *instName, ...);
void logDefUseInt8(uint8_t value, const char *instName, ...);
void logDefUseInt16(uint16_t value, const char *instName, ...);
void logDefUseInt32(uint32_t value, const char *instName, ...);
void logDefUseInt64(uint64_t value, const char *instName, ...);
void logDefUseFloat(float value, const char *instName, ...);
void logDefUseDouble(double value, const char *instName, ...);
void logDefUsePointer(const void* value, const char *instName, ...);
void logDefUseVoid(void* value, const char *instName, ...);
