#pragma once

#include <stdint.h>

void logDefUseInt1   (const char *type, bool value);
void logDefUseInt8   (const char *type, uint8_t value);
void logDefUseInt16  (const char *type, uint16_t value);
void logDefUseInt32  (const char *type, uint32_t value);
void logDefUseInt64  (const char *type, uint64_t value);
void logDefUseFloat  (const char *type, float value);
void logDefUseDouble (const char *type, double value);
void logDefUsePointer(const char *type, const void* value);