#include "commands.h"
#include "ast.h"
#include "evaluate.h"
#include "main.h"

#include <cstdio>
#include <cstring>
#include <fontlibc.h>
#include <tice.h>
#include <ti/tokens.h>

void commandDisp(struct NODE *node) {
    static char buf[27] = {0};

    while (node != nullptr) {
        BaseType *result = evalNode(node);

        if (result != nullptr) {
            if (result->type() == TypeType::MATRIX) {
                uint8_t maxColLengths[12] = {0};

                auto elements = dynamic_cast<Matrix &>(*result).elements;

                // Get the max lengths of each column, 12 max
                unsigned int rowIndex = 0;
                for (auto &row: elements) {
                    unsigned int colIndex = 0;

                    for (auto &col: row) {
                        char *out = col.toString();
                        uint24_t length = strlen(out);

                        if (length > maxColLengths[colIndex]) maxColLengths[colIndex] = length;
                        colIndex++;

                        if (colIndex >= 12) break;
                    }

                    rowIndex++;
                    if (rowIndex >= 10) break;
                }

                // Get the total width of all columns combined
                unsigned int totalWidth = 3;
                for (auto maxColLength: maxColLengths) {
                    if (maxColLength) totalWidth += maxColLength + 1;
                }

                // Get the start position
                unsigned int beginWidth = 0;
                if (totalWidth < 26) {
                    beginWidth = 26 - totalWidth;
                }

                // Draw the first [
                fontlib_SetCursorPosition(beginWidth * GLYPH_WIDTH + HOMESCREEN_X, fontlib_GetCursorY());
                fontlib_DrawString("\xC1");

                // Display each column
                rowIndex = 0;
                for (auto &row: elements) {
                    unsigned int colIndex = 0;
                    unsigned int cumSumColX = beginWidth + 2;

                    fontlib_SetCursorPosition((beginWidth + 1) * GLYPH_WIDTH + HOMESCREEN_X, fontlib_GetCursorY());
                    fontlib_DrawString("\xC1");

                    // Display a space to leave space for the big bracket
                    for (auto &col: row) {
                        char *out = col.toString();

                        fontlib_SetCursorPosition(cumSumColX * GLYPH_WIDTH + HOMESCREEN_X, fontlib_GetCursorY());
                        fontlib_DrawString(out);
                        cumSumColX += maxColLengths[colIndex] + 1;
                        colIndex++;

                        if (cumSumColX >= 26) break;
                    }

                    // Draw end of line
                    rowIndex++;
                    fontlib_DrawString("]");

                    if (rowIndex == elements.size()) fontlib_DrawString("]");
                    if (rowIndex == 10 && elements.size() != 10) fontlib_DrawString("\x1F");

                    fontlib_Newline();

                    if (rowIndex >= 10) break;
                }
            } else {
                char *out = result->toString();

                if (result->type() == TypeType::STRING) {
                    fontlib_DrawStringL(out, 26);
                } else {
                    sprintf(buf, "%26s", out);
                    fontlib_DrawString(buf);
                }
                fontlib_Newline();
            }

            delete result;
        }

        // Get the next child
        node = node->next;
    }
}

void evalCommand(struct NODE *node) {
    unsigned int command = node->data.operand.command;

    if (command == OS_TOK_DISP) commandDisp(node->child);
}
