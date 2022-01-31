#ifndef PARSE_H
#define PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fileioc.h>

struct NODE *parse_full_program(ti_var_t slot, bool end_valid, bool else_valid);

#ifdef __cplusplus
}
#endif

#endif
