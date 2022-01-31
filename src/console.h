#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#define CONSOLE_X 56
#define CONSOLE_Y 80
#define CONSOLE_WIDTH 26
#define CONSOLE_HEIGHT 10

void console_clear(void);

void console_newline(void);

void console_print(char *string);

void console_print_int(unsigned int num);

#ifdef __cplusplus
}
#endif

#endif
