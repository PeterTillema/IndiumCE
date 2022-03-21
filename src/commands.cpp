#include "commands.h"
#include "ast.h"
#include "evaluate.h"

#include <cstdio>
#include <fontlibc.h>
#include <tice.h>

void commandDisp(struct NODE *node) {
    static char buf[27] = {0};

    while (node != nullptr) {
        BaseType *result = evalNode(node);

        sprintf(buf, "%26s", result->toString());
        fontlib_DrawString(buf);
        fontlib_Newline();

        delete result;

        // Get the next child
        node = node->next;
    }
}

void evalCommand(struct NODE *node) {
    unsigned int command = node->data.operand.command;

    if (command == tDisp) commandDisp(node->child);
}
