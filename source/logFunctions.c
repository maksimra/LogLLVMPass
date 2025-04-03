#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "logFunctions.hpp"

#define MAX_NUMBER_INSTRUCTIONS 100000 // Пока не придумал, как без этого сделать

static const char *DEF_INSTRUCTIONS[MAX_NUMBER_INSTRUCTIONS] = {};
static size_t NUMBER_INSTRUCTIONS = 0;

void logDefUseInt1(bool value, const char *instName, ...)
{
	assert(instName);
	
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;

	FILE* defUseFile = fopen("dot/DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = (as unsigned) %u or "
		                      "(as signed) %d\n", instName, value, value);
	fclose(runtimeValueFile); 
}

void logDefUseInt8(uint8_t value, const char *instName, ...)
{
	assert(instName);
	
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = (as unsigned) %hhu or "
		                      "(as signed) %hhd\n", instName, value, value);
	fclose(runtimeValueFile); 
}

void logDefUseInt16(uint16_t value, const char *instName, ...)
{
	assert(instName);
	
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = (as unsigned) %hu or "
		"(as signed) %hd\n", instName, value, value);
	fclose(runtimeValueFile); 
}

void logDefUseInt32(uint32_t value, const char *instName, ...)
{
	assert(instName);
	
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = (as unsigned) %u or "
		"(as signed) %d\n", instName, value, value);
	fclose(runtimeValueFile); 
}

void logDefUseInt64(uint64_t value, const char *instName, ...)
{
	assert(instName);
	
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = (as unsigned) %lu or "
		"(as signed) %ld\n", instName, value, value);
	fclose(runtimeValueFile); 
}

void logDefUseFloat(float value, const char *instName, ...)
{
	assert(instName);
	
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = %g\n", instName, value);
	fclose(runtimeValueFile); 
}

void logDefUseDouble(double value, const char *instName, ...)
{
	assert(instName);
	
	fprintf(stderr, "ЛОГГИРУЮ ДАБЛ\n");
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);
	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = %g\n", instName, value);
	fclose(runtimeValueFile); 

	fprintf(stderr, "PERVII: %s\n", DEF_INSTRUCTIONS[0]);
}

void logDefUsePointer(const void* value, const char *instName, ...)
{
	assert(instName);

	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);

	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = %p\n", instName, value);
	fclose(runtimeValueFile); 
}

void logDefUseVoid(void* value, const char *instName, ...)
{
	DEF_INSTRUCTIONS[NUMBER_INSTRUCTIONS] = instName;
	fprintf(stderr, "number instr == %zu\n", NUMBER_INSTRUCTIONS);
	fprintf(stderr, "VOID: %s\n", instName);

	FILE* defUseFile = fopen("DefUseGraph.dot", "a");
	if (!defUseFile)
	{
		fprintf(stderr, "error open file\n");
		return;
	}
	fprintf(defUseFile, " -> \"%s\"", instName);

	FILE* defUseChain = fopen("DefUseChain.dot", "a"); //TODO: может с помощью флага управлять открытием файла
	if (!defUseChain)
	{
		fprintf(stderr, "error open file\n");
		return;
	}

	va_list args;
	va_start(args, instName);

	const char *operandInstr = NULL;
	
	while (operandInstr = va_arg(args, const char*))
	{
		for (int numberDefInst = 0; numberDefInst < NUMBER_INSTRUCTIONS; numberDefInst++)
		{
			if (strcmp(operandInstr, DEF_INSTRUCTIONS[numberDefInst]) == 0)
				fprintf(defUseChain, "\n\"%s\" -> \"%s\";", operandInstr, instName);
		}
	}
	va_end(args);
	fclose(defUseChain);

	NUMBER_INSTRUCTIONS++;
	fclose(defUseFile);

	FILE* runtimeValueFile = fopen("RuntimeValue.txt", "a");
	fprintf(runtimeValueFile, "%s | VALUE = %p\n", instName, value);
	fclose(runtimeValueFile); 
}