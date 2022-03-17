#include "ast.h"

#include <cstdlib>

void nodeFree(struct NODE *node) {
    switch (node->data.type) {

        case ET_NUMBER:
            delete node->data.operand.num;
            break;

        case ET_COMPLEX:
            delete node->data.operand.cplx;
            break;

        case ET_TEMP_STRING:
            delete node->data.operand.string;
            break;

        case ET_TEMP_LIST:
            delete node->data.operand.list;
            break;

        case ET_TEMP_LIST_COMPLEX:
            delete node->data.operand.complexList;
            break;

        case ET_TEMP_MATRIX:
            delete node->data.operand.matrix;
            break;

        default:
            break;
    }

    free(node);
}
