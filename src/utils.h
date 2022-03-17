#ifndef UTILS_H
#define UTILS_H

#include <fileioc.h>

bool is2ByteTok(int token);

void seekPrev(ti_var_t slot);

int tokenNext(ti_var_t slot);

int tokenCurrent();

int tokenPeek();

bool endOfLine(int token);

#endif
