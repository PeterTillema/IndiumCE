#include "evaluate.h"
#include "errors.h"
#include "globals.h"
#include "parse.h"
#include "variables.h"
#include "main.h"

#include <fileioc.h>
#include <fontlibc.h>
#include <graphx.h>
#include <intce.h>
#include <keypadc.h>
#include <new>
#include <ti/getcsc.h>
#include <ti/screen.h>
#include <ti/tokens.h>

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
    gfx_HorizLine_NoClip(0, HOMESCREEN_Y - 1, GFX_LCD_WIDTH);
    gfx_SetColor(223);
    gfx_FillRectangle_NoClip(0, 0, GFX_LCD_WIDTH, HOMESCREEN_Y - 1);
    gfx_PrintStringXY("IndiumCE v0.0.1 - By Peter \"PT_\" Tillema", 30, 1);
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(0, HOMESCREEN_Y, GFX_LCD_WIDTH, GFX_LCD_HEIGHT - HOMESCREEN_Y);

    // Setup font
    fontlib_font_t *font = fontlib_GetFontByIndex("OSLFONT", 0);
    if (font == nullptr) {
        gfx_PrintStringXY("Large font not found. Please transfer the", 1, HOMESCREEN_Y + 1);
        gfx_PrintStringXY("right appvars to this calculator!", 1, HOMESCREEN_Y + 10);

        forceExit();
    }

    fontlib_SetFont(font, static_cast<fontlib_load_options_t>(0));
    fontlib_SetWindow(HOMESCREEN_X, HOMESCREEN_Y, GFX_LCD_WIDTH - 2 * HOMESCREEN_X, GFX_LCD_HEIGHT - HOMESCREEN_Y);
    fontlib_SetLineSpacing(3, 3);
    fontlib_SetNewlineCode(0);
    fontlib_SetNewlineOptions(FONTLIB_AUTO_SCROLL | FONTLIB_PRECLEAR_NEWLINE);
    fontlib_SetFirstPrintableCodePoint(1);
    fontlib_HomeUp();

    // Setup keypad
    kb_DisableOnLatch();
    kb_SetMode(MODE_0_IDLE);

    // Setup other things
    get_all_os_variables();
    globals = Globals();
    std::set_new_handler(memoryError);

    auto root = parseProgram(input_slot, false, false);
    evalNodes(root);

    fontlib_DrawString("                      Done");

    while (!os_GetCSC());

    gfx_End();

    return 0;
}
