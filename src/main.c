#include "main.h"
#include "evaluate.h"
#include "parse.h"
#include "variables.h"
#include "font/font.h"

#include <fileioc.h>
#include <fontlibc.h>
#include <graphx.h>
#include <intce.h>
#include <keypadc.h>
#include <string.h>
#include <tice.h>

void force_exit(void) {
    while (os_GetCSC() != sk_Enter);
    gfx_End();
    exit(-1);
}

int main(int argc, char *argv[]) {
    ti_var_t input_slot = 0;

    // Get the program argument, if it exists
    if (argc >= 2) {
        char *p = argv[1];

        // Skip eventually the program byte
        if (*p == tProg) p++;

        // Get either the normal or protected program
        input_slot = ti_OpenVar(p, "r", TI_PRGM_TYPE);
        if (!input_slot) input_slot = ti_OpenVar(p, "r", TI_PPRGM_TYPE);
    }

    // If no valid entry is found in the arguments, ask one time for user input
    if (!input_slot) {
        char buf[9] = {0};

        os_ClrHome();
        os_GetStringInput("Program name: ", buf, 8);

        // Get either the normal or protected program
        input_slot = ti_OpenVar(buf, "r", TI_PRGM_TYPE);
        if (!input_slot) input_slot = ti_OpenVar(buf, "r", TI_PPRGM_TYPE);
    }

    // Not found, so error!
    if (!input_slot) {
        os_ClrHome();
        os_PutStrFull("Input program not found");
        while (!os_GetCSC());

        return -1;
    }

    // Setup graphics
    gfx_Begin();
    gfx_FillScreen(223);
    gfx_PrintStringXY("IndiumCE v0.0.1 - By Peter \"PT_\" Tillema", 30, 1);
    gfx_HorizLine_NoClip(0, 10, gfx_lcdWidth);
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(HOMESCREEN_X - 2, HOMESCREEN_Y, 260 + 2, 200 + 2);

    // Setup font
    fontlib_SetFont(font, 0);
    fontlib_SetWindow(HOMESCREEN_X, HOMESCREEN_Y, 260, 200);
    fontlib_SetNewlineOptions(FONTLIB_AUTO_SCROLL | FONTLIB_PRECLEAR_NEWLINE);
    fontlib_HomeUp();

    // Setup keypad
    int_Enable();
    kb_SetMode(MODE_3_CONTINUOUS);

    // First parse the program
    struct NODE *root = parse_full_program(input_slot, false, false);

    // Get all the variables
    get_all_os_variables();

    // And evaluate the program
    evaluate_consecutive_nodes(root);

    gfx_End();

    return 0;
}
