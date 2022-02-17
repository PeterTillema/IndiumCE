#include "ast.h"
#include "expression.h"

#include <stdlib.h>

struct NODE *node_alloc(enum etype type) {
    struct NODE *new_node;

    // Allocate
    new_node = calloc(1, sizeof(struct NODE));
    if (new_node == NULL) parse_error("Memory error");

    new_node->data.type = type;

    return new_node;
}
