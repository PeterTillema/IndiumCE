#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdbool.h>
#include <stdint.h>

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

struct var_real {
    bool complex;
    float real;
    float imag;
};

struct var_list {
    bool complex;
    unsigned int size;
    float *data;
};

struct var_custom_list {
    char name[5];
    struct var_list var_list;
};

struct var_matrix {
    uint8_t rows;
    uint8_t cols;
    float *data;
};

struct var_string {
    unsigned int length;
    char *data;
};

extern struct var_real variables[26];
extern struct var_string strings[10];
extern struct var_list lists[6];
extern struct var_matrix matrices[10];
extern struct var_custom_list custom_lists[50];

void get_all_os_variables(void);

#ifdef __cplusplus
}
#endif

#endif
