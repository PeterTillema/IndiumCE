#ifndef EVALUATE_H
#define EVALUATE_H

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

void evaluate_consecutive_nodes(struct NODE *node);

#ifdef __cplusplus
}
#endif

#endif
