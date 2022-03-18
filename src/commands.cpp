#include "commands.h"
#include "ast.h"
#include "evaluate.h"
#include "globals.h"

#include <cstdio>
#include <fontlibc.h>
#include <tice.h>

void commandDisp(struct NODE *node) {
    struct NODE *node2;
    char buf[27];
    char num_buf[27];

    while (node != nullptr) {
        struct NODE *result = evalNode(node);
        enum etype type = result->data.type;

        if (type == ET_NUMBER) {
            if (globals.fixNr == 255) {
                sprintf(num_buf, "%f", result->data.operand.num->num);
            } else {
                sprintf(num_buf, "%10.*f", globals.fixNr, result->data.operand.num->num);
            }

            sprintf(buf, "%26s", num_buf);
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
