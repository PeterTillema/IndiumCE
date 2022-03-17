#include "evaluate.h"
#include "font/font.h"
#include "globals.h"
#include "parse.h"
#include "variables.h"

#include <intce.h>
#include <fileioc.h>
#include <fontlibc.h>
#include <graphx.h>
#include <keypadc.h>

#define HOMESCREEN_X ((gfx_lcdWidth - 260) / 2)
#define HOMESCREEN_Y ((gfx_lcdHeight - 200) / 2)

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
        os_GetStringInput((char *) "Program name: ", buf, 8);

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
    fontlib_SetFont(font, static_cast<fontlib_load_options_t>(0));
    fontlib_SetWindow(HOMESCREEN_X, HOMESCREEN_Y, 260, 200);
    fontlib_SetNewlineOptions(FONTLIB_AUTO_SCROLL | FONTLIB_PRECLEAR_NEWLINE);
    fontlib_HomeUp();

    // Setup keypad
    kb_DisableOnLatch();
    kb_SetMode(MODE_0_IDLE);

    // Setup other things
    get_all_os_variables();
    globals = Globals();

    auto root = parseProgram(input_slot, false, false);
    evalNodes(root);

    while (!os_GetCSC());

    gfx_End();

    return 0;
}
