#ifndef OPERATORS_H
#define OPERATORS_H

#include <cstdint>

#define MAX_PRECEDENCE 11

uint8_t getOpPrecedence(uint8_t op);

bool isUnaryOp(uint8_t prec);

struct NODE *evalOperator(struct NODE *op_node);

#endif
