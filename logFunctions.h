#pragma once

// TODO: Why are these functions defined in header? It can easily lead you to ODR violation. Either define them as inline, "weak", or more preferably just move them to source file since you don't need their prototypes.

void logDefUseInt1(const char *type, bool value)
{
	printf("%s (i1): (as unsigned) %u, "
		   "(as signed) %d\n", type, value, value);
}

void logDefUseInt8(const char *type, uint8_t value)
{
	printf("%s (i8): (as unsigned) %hhu, "
		   "(as signed) %hhd\n", type, value, value);
}

void logDefUseInt16(const char *type, uint16_t value)
{
	printf("%s (i16): (as unsigned) %hu, "
		   "(as signed) %hd\n", type, value, value);
}

void logDefUseInt32(const char *type, uint32_t value)
{
	printf("%s (i32): (as unsigned) %u, "
		   "(as signed) %d\n", type, value, value);
}

void logDefUseInt64(const char *type, uint64_t value)
{
	printf("%s (i64): (as unsigned) %lu, "
		   "(as signed) %ld\n", type, value, value);
}

void logDefUseFloat(const char *type, float value)
{
	printf("%s (float): %g\n", type, value);
}

void logDefUseDouble(const char *type, double value)
{
	printf("%s (double): %g\n", type, value);
}

void logDefUsePointer(const char *type, const void* value)
{
	printf("%s (pointer): %p\n", type, value);
}
