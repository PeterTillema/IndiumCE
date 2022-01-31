#ifndef ROUTINES_H
#define ROUTINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fileioc.h>

void seek_prev(ti_var_t slot);

bool is_2_byte_token(int token);

#ifdef __cplusplus
}
#endif

#endif
