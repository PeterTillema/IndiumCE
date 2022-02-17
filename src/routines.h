#ifndef ROUTINES_H
#define ROUTINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fileioc.h>

void seek_prev(ti_var_t slot);

bool is_2_byte_token(int token);

int next_token(ti_var_t slot);

int current_token(void);

int peek_token(ti_var_t slot);

bool is_end_of_line(int token);

extern unsigned int parse_col;

#ifdef __cplusplus
}
#endif

#endif
