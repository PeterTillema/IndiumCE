#include "main.h"
#include "console.h"
#include "parse.h"
#include "variables.h"

#include <fileioc.h>
#include <graphx.h>
#include <keypadc.h>
#include <string.h>

static char *input_programs[MAX_NR_PROGRAMS];

void force_exit(void) {
    while (os_GetCSC() != sk_Enter);
    gfx_End();
    exit(-1);
}

static int string_compare(const void *a, const void *b) {
    return strcmp(*(const char **) a, *(const char **) b);
}

static uint8_t get_all_basic_programs(void) {
    void *search_pos = NULL;
    uint8_t var_type;
    uint8_t cur_prog = 0;
    char *var_name;

    while ((var_name = ti_DetectAny(&search_pos, "", &var_type)) != NULL) {
        if (var_type == TI_PRGM_TYPE || var_type == TI_PPRGM_TYPE) {
            // Hidden programs
            if ((uint8_t) (*var_name) < 64) {
                *var_name += 64;
            }

            // Open the program
            ti_var_t slot = ti_OpenVar(var_name, "r", var_type);
            if (!slot)
                continue;

            uint8_t *data_ptr = ti_GetDataPtr(slot);

            ti_Close(slot);

            // If it's an assembly program, don't add
            if (*data_ptr == 0xEF && *(data_ptr + 1) == 0x7B)
                continue;

            input_programs[cur_prog] = malloc(9);
            if (input_programs[cur_prog] == NULL)
                break;

            strcpy(input_programs[cur_prog], var_name);

            if (cur_prog++ == 255)
                break;
        }
    }

    return cur_prog;
}

static void display_program_names(uint8_t offset, uint8_t amount) {
    for (uint8_t i = 0; i < amount; i++) {
        gfx_PrintStringXY(input_programs[offset + i], 132, 21 + 8 * i);
    }
}

static void clear_program_list(void) {
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(123, 20, 74, 210);
}

int main(void) {
    // Display main screen
    gfx_Begin();
    gfx_FillScreen(223);
    gfx_PrintStringXY("IndiumCE v0.0.1 - By Peter \"PT_\" Tillema", 30, 1);
    gfx_SetColor(0);
    gfx_HorizLine_NoClip(0, 10, gfx_lcdWidth);

    clear_program_list();

    uint8_t cur_prog = get_all_basic_programs();

    // No programs found
    if (!cur_prog) {
        gfx_PrintStringXY("Found no", 124, 21);
        gfx_PrintStringXY("programs!", 124, 29);

        force_exit();
    }

    // Sort the programs by name
    qsort(input_programs, cur_prog, sizeof(char *), string_compare);

    uint8_t selected = 0;
    uint8_t offset = 0;
    uint8_t max_to_display = (cur_prog <= 26 ? cur_prog : 26);
    sk_key_t key;

    display_program_names(offset, max_to_display);

    // Get the input
    while ((key = os_GetCSC()) != sk_Enter && key != sk_Clear) {
        gfx_PrintStringXY(">", 124, 21 + 8 * selected);

        // Move cursor around
        if (key == sk_Up) {
            if (selected) {
                gfx_FillRectangle_NoClip(124, 21 + 8 * selected, 8, 8);
                selected--;
            } else if (offset) {
                offset--;
                clear_program_list();
                display_program_names(offset, max_to_display);
            }
        } else if (key == sk_Down) {
            if (selected != max_to_display - 1) {
                gfx_FillRectangle_NoClip(124, 21 + 8 * selected, 8, 8);
                selected++;
            } else if (offset + max_to_display < cur_prog) {
                offset++;
                clear_program_list();
                display_program_names(offset, max_to_display);
            }
        }
    }

    if (key == sk_Clear) {
        gfx_End();
        return 0;
    }

    // Copy to new char array, free the rest
    char var_name[9] = {0};
    strcpy(var_name, input_programs[offset + selected]);

    for (uint8_t i = 0; i < cur_prog; i++) {
        free(input_programs[i]);
    }

    // Setup other graphics
    gfx_SetMonospaceFont(8);
    gfx_SetColor(223);
    gfx_FillRectangle_NoClip(123, 20, 74, 210);
    gfx_SetColor(255);
    console_clear();

    // Setup keypad
    kb_SetMode(MODE_3_CONTINUOUS);

    // Open the file for reading, either a program or a protected program
    ti_var_t input_slot;
    input_slot = ti_OpenVar(var_name, "r", TI_PRGM_TYPE);
    if (!input_slot) {
        input_slot = ti_OpenVar(var_name, "r", TI_PPRGM_TYPE);
    }
    if (!input_slot) {
        console_print("Can't open program");
        force_exit();
    }

    // First parse the program
    parse_full_program(input_slot, false, false);

    // Get all the variables
    get_all_os_variables();

    gfx_End();

    return 0;
}
