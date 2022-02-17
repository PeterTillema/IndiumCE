#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tice.h>

#define HOMESCREEN_X ((gfx_lcdWidth - 260) / 2)
#define HOMESCREEN_Y ((gfx_lcdHeight - 200) / 2)

void force_exit(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
