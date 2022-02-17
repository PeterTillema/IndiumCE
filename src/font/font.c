#include <fontlibc.h>
#include <stdint.h>

static const uint8_t font_data[] = {
    #include "font.inc"
};

const fontlib_font_t *font = (fontlib_font_t *)font_data;