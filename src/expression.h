#ifndef EXPRESSION_H
#define EXPRESSION_H

#ifdef __cplusplus
extern "C" {
#endif


#include <fileioc.h>

extern unsigned int parse_line;
extern unsigned int parse_col;

void parse_error(char *string) __attribute__((noreturn));

struct NODE *token_expression(ti_var_t slot, int token);

struct NODE *parse_expression_line(ti_var_t slot, int token, bool stop_at_comma, bool stop_at_paren);

#ifdef __cplusplus
}
#endif

#endif
