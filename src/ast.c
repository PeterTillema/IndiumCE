#include "ast.h"

#include <string.h>
#include <stdlib.h>

struct NODE *node_alloc(enum etype type) {
    struct NODE *new_node;

    // Allocate
    new_node = calloc(1, sizeof(struct NODE));
    if (new_node == NULL) {
        return NULL;
    }

    new_node->data.type = type;

    return new_node;
}

void node_free(struct NODE *node) {
    if (node == NULL) {
        return;
    }

    if (node->next != NULL) {
        node_free(node->next);
    }

    if (node->child != NULL) {
        node_free(node->child);
    }

    free(node);
}
