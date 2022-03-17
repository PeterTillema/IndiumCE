#ifndef VARIABLES_H
#define VARIABLES_H

#include "types.h"

struct var_real {
    bool complex;
    union value_t {
        Number *num;
        Complex *cplx;
    } value;
};

struct var_list {
    bool complex;
    union list_t {
        List *list;
        ComplexList *complexList;
    } list;
};

struct var_custom_list {
    char name[5];
    struct var_list list;
};

struct var_string {
    unsigned int length;
    char data[1];
};

extern struct var_real *variables[26];
extern String *strings[10];
extern String *equations[31];
extern struct var_list *lists[6];
extern struct var_custom_list *customLists[50];
extern Matrix *matrices[10];

void get_all_os_variables();

#endif
