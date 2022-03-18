#include "commands.h"
#include "ast.h"
#include "evaluate.h"

#include <cstdio>
#include <fontlibc.h>
#include <tice.h>

void commandDisp(struct NODE *node) {
    struct NODE *node2;
    static char buf[27] = {0};

    while (node != nullptr) {
        struct NODE *result = evalNode(node);
        enum etype type = result->data.type;

        if (type == ET_NUMBER) {
            sprintf(buf, "%26s", result->data.operand.num->toString());
            fontlib_DrawString(buf);
            fontlib_Newline();
        } else if (type == ET_COMPLEX) {
            sprintf(buf, "%26s", result->data.operand.cplx->toString());
            fontlib_DrawString(buf);
            fontlib_Newline();
        }

        // Get the next child
        node2 = node->next;
        node = node2;
    }
}

void evalCommand(struct NODE *node) {
    unsigned int command = node->data.operand.command;

    if (command == tDisp) commandDisp(node->child);
}
