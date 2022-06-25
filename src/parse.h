#ifndef PARSE_H
#define PARSE_H

#include "ast.h"

#include <fileioc.h>

#define EXPRESSION(func) (reinterpret_cast<NODE *(*)(ti_var_t, int)>(((unsigned int*)(&func) - 0x800000)))
#define UNEXPRESSION(func) (reinterpret_cast<void (*)(ti_var_t, int)>(((unsigned int *)(func) + 0x800000)))

struct NODE *parseProgram(ti_var_t slot, bool expectEnd, bool expectElse);

#endif
