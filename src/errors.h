#ifndef ERRORS_H
#define ERRORS_H

#include <new>

void forceExit() __attribute__((noreturn));

void parseError(const char *string) __attribute__((noreturn));

void memoryError() __attribute__((noreturn));

void typeError() __attribute__((noreturn));

void divideBy0Error() __attribute__((noreturn));

void dimensionError() __attribute__((noreturn));

void dimensionMismatch() __attribute__((noreturn));

void overflowError() __attribute__((noreturn));

void domainError() __attribute__((noreturn));

void argumentsError() __attribute__((noreturn));

#endif
