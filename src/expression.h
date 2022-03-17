#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <fileioc.h>

struct NODE *expressionLine(ti_var_t slot, int token, bool stopAtComma, bool stopAtParen);

struct NODE *tokenExpression(ti_var_t slot, int token);

#endif
