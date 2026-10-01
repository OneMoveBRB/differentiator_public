#ifndef STACK_DUMP_H
#define STACK_DUMP_H

#ifndef STACK_DEBUG
#define STACK_DEBUG
#endif /* STACK_DEBUG */

#include "../include/stack.h"

StackErr StackVerify(Stack* stack);
StackErr StackDump(Stack* stack, StackErr error);

size_t CalculateDataHash(const Stack* stack);
void FillPoison(void* data, size_t size);
const char* StackErrorMessage(StackErr error);

#endif /* STACK_DUMP_H */