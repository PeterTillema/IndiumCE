#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#include <fileioc.h>

char *formatNum(float num);

bool is2ByteTok(int token);

void seekPrev(ti_var_t slot);

int tokenNext(ti_var_t slot);

int tokenCurrent();

int tokenPeek();

bool endOfLine(int token);

float cosfMode(float num);

float sinfMode(float num);

float atanfMode(float num);

Matrix *multiplyMatrices(Matrix &lhs, Matrix &rhs);

#endif
