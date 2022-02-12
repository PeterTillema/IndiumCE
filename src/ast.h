#ifndef AST_H
#define AST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum etype {
    ET_NUM = 0,
    ET_COMPLEX,
    ET_VARIABLE,
    ET_STRING,
    ET_TEMP_STRING,
    ET_LIST,
    ET_TEMP_LIST,
    ET_CUSTOM_LIST,
    ET_MATRIX,
    ET_OPERATOR,
    ET_FUNCTION_CALL,   // The difference between a function call and a command is that a function call can be used in
    ET_COMMAND          // an expression, whereas a command should be the first token on a line.
};

struct cplx_t {
    float real;
    float imag;
};

union operand_t {
    float num;
    struct cplx_t *cplx_ptr;
    uint8_t variable_nr;
    uint8_t string_nr;
    uint8_t temp_string_nr;
    uint8_t equation_nr;
    uint8_t list_nr;
    uint8_t temp_list_nr;
    uint8_t custom_list_nr;
    uint8_t matrix_nr;
    uint8_t temp_matrix_nr;
    uint8_t op;
    unsigned int func;
    unsigned int command;
};

struct element_t {
    enum etype type;
    union operand_t operand;
};

struct NODE {
    struct NODE *next;
    struct NODE *child;
    struct element_t data;
};

struct NODE *node_alloc(enum etype type);

#ifdef __cplusplus
}
#endif

#endif
