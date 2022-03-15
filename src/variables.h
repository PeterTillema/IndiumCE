#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct var_cplx {
    float real;
    float imag;
};

struct var_real {
    bool complex;
    struct var_cplx cplx;
};

struct var_list {
    bool complex;
    unsigned int size;
    float data[1];
};

struct var_custom_list {
    char name[5];
    struct var_list list;
};

struct var_matrix {
    uint8_t rows;
    uint8_t cols;
    float data[1];
};

struct var_string {
    unsigned int length;
    char data[1];
};

extern struct var_real *variables[26];
extern struct var_string *strings[10];
extern struct var_string *equations[31];
extern struct var_list *lists[6];
extern struct var_custom_list *custom_lists[50];
extern struct var_matrix *matrices[10];

void get_all_os_variables(void);

#ifdef __cplusplus
}
#endif

#endif
