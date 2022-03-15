#ifndef OPERATORS_H
#define OPERATORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define MAX_PRECEDENCE (11)

uint8_t get_operator_precedence(uint8_t op);

bool is_unary_op(uint8_t prec);

const char *get_op_string(uint8_t op);

struct NODE *evaluate_operator(struct NODE *func_node);

#ifdef __cplusplus
}
#endif

#endif
