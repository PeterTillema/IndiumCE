#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum etype {
    ET_NUM = 0,
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

union operand_t {
    float num;
    uint8_t variable_nr;
    uint8_t string_nr;
    uint8_t *temp_string_ptr;
    uint8_t list_nr;
    uint8_t *temp_list_ptr;
    char custom_list_name[5];
    uint8_t matrix_nr;
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

void node_free(struct NODE *node);

#ifdef __cplusplus
}
#endif

#endif
