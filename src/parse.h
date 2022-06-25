#ifndef PARSE_H
#define PARSE_H

#include <fileioc.h>

struct NODE *parseProgram(ti_var_t slot, bool expectEnd, bool expectElse);

#endif
