#include "console.h"

#include <graphx.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t row = 0;
static uint8_t col = 0;

void console_clear(void) {
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(55, 79, CONSOLE_WIDTH * 8 + 2, CONSOLE_HEIGHT * 8 + 2);
    row = 0;
    col = 0;
}

void console_newline(void) {
    if (row == CONSOLE_HEIGHT) {
        gfx_CopyRectangle(gfx_screen, gfx_screen, CONSOLE_X, CONSOLE_Y + 8, CONSOLE_X, CONSOLE_Y, CONSOLE_WIDTH * 8, (CONSOLE_HEIGHT - 1) * 8);
        gfx_FillRectangle_NoClip(CONSOLE_X, CONSOLE_Y + 9 * 8, CONSOLE_WIDTH * 8, 8);
    } else {
        row++;
    }

    col = 0;
}

void console_print(const char *string) {
    char c;

    gfx_SetTextXY(CONSOLE_X + col * 8, CONSOLE_Y + row * 8);

    while ((c = *string++) && col < 26) {
        gfx_PrintChar(c);
        col++;
    }
}

void console_print_int(unsigned int num) {
    char buf[9];

    sprintf(buf, "%d", num);
    console_print(buf);
}
