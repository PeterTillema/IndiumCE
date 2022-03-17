#ifndef ERRORS_H
#define ERRORS_H

void forceExit() __attribute__((noreturn));

void parseError(const char *string) __attribute__((noreturn));

void memoryError();

void typeError();

void divideBy0Error();

#endif
