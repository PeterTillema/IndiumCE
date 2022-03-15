#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

struct NODE *evaluate_function(struct NODE *func_node);

#ifdef __cplusplus
}
#endif

#endif
