#ifndef COMMANDS_H
#define COMMANDS_H

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

void evaluate_command(struct NODE *func_node);

#ifdef __cplusplus
}
#endif

#endif
