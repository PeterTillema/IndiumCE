#include "evaluate.h"
#include "ast.h"
#include "commands.h"
#include "functions.h"
#include "operators.h"
#include "variables.h"
#include "expression.h"

#include <stdlib.h>
#include <string.h>

struct NODE *evaluate_node(struct NODE *node) {
    enum etype type = node->data.type;
    struct NODE *result = NULL;

    /**
     * Do nothing when it's a temp list, string or matrix. These are probably the results
     * of earlier operations/functions, and will be taken care of at the operator/function
     * itself.
     */

    if (type == ET_NUM) {
        // Number: allocate new node, store num into it
        result = node_alloc(ET_NUM);
        result->data.operand.num = node->data.operand.num;
    }

    else if (type == ET_COMPLEX) {
        // Complex: allocate new node, allocate cplx node, copy
        result = node_alloc(ET_COMPLEX);

        struct var_cplx *cplx = (struct var_cplx *)malloc(sizeof(struct var_cplx));
        if (cplx == NULL) parse_error("Memory error");

        cplx->real = node->data.operand.cplx->real;
        cplx->imag = node->data.operand.cplx->imag;
        result->data.operand.cplx = cplx;
    }

    else if (type == ET_VARIABLE) {
        // Variable: allocate new node, either store num into it or allocate cplx node, copy
        struct var_real *var_node = variables[node->data.operand.variable_nr];

        if (var_node->complex) {
            result = node_alloc(ET_COMPLEX);

            struct var_cplx *cplx = (struct var_cplx *)malloc(sizeof(struct var_cplx));
            if (cplx == NULL) parse_error("Memory error");

            cplx->real = var_node->cplx.real;
            cplx->imag = var_node->cplx.imag;
            result->data.operand.cplx = cplx;
        } else {
            result = node_alloc(ET_NUM);
            result->data.operand.num = var_node->cplx.real;
        }
    }

    else if (type == ET_STRING) {
        // String: allocate space, copy data
        struct var_string *string_node = strings[node->data.operand.string_nr];

        result = node_alloc(ET_TEMP_STRING);

        struct var_string *string = (struct var_string *)malloc(string_node->length + 3);
        if (string == NULL) parse_error("Memory error");

        string->length = string_node->length;
        memcpy(string->data, string_node->data, string_node->length);

        result->data.operand.temp_string = string;
    }

    else if (type == ET_EQU) {
        // Equation: allocate space, copy data
        struct var_string *equation_node = equations[node->data.operand.equation_nr];

        result = node_alloc(ET_TEMP_STRING);

        struct var_string *equation = (struct var_string *)malloc(equation_node->length + 3);
        if (equation == NULL) parse_error("Memory error");

        equation->length = equation_node->length;
        memcpy(equation->data, equation_node->data, equation_node->length);

        result->data.operand.temp_string = equation;
    }

    else if (type == ET_LIST) {
        // List: allocate space, copy data
        struct var_list *list_node = lists[node->data.operand.list_nr];

        result = node_alloc(ET_TEMP_LIST);

        struct var_list *list = (struct var_list *)malloc(4 + list_node->size * sizeof(float) * (list_node->complex ? 2 : 1));
        if (list == NULL) parse_error("Memory error");

        list->complex = list_node->complex;
        list->size = list_node->size;
        memcpy(list->data, list_node->data, list_node->size * sizeof(float) * (list_node->complex ? 2 : 1));

        result->data.operand.temp_list = list;
    }

    else if (type == ET_CUSTOM_LIST) {
        // Custom list: allocate space, copy data
        struct var_custom_list *list_node = custom_lists[node->data.operand.custom_list_nr];

        result = node_alloc(ET_TEMP_LIST);

        struct var_list *list = (struct var_list *)malloc(4 + list_node->list.size * sizeof(float) * (list_node->list.complex ? 2 : 1));
        if (list == NULL) parse_error("Memory error");

        list->complex = list_node->list.complex;
        list->size = list_node->list.size;
        memcpy(list->data, list_node->list.data, list_node->list.size * sizeof(float) * (list_node->list.complex ? 2 : 1));

        result->data.operand.temp_list = list;
    }

    else if (type == ET_MATRIX) {
        // Matrix: allocate space, copy data
        struct var_matrix *matrix_node = matrices[node->data.operand.matrix_nr];

        result = node_alloc(ET_TEMP_MATRIX);

        struct var_matrix *matrix = (struct var_matrix *)malloc(2 + matrix_node->rows * matrix_node->cols * sizeof(float));
        if (matrix == NULL) parse_error("Memory error");

        matrix->rows = matrix_node->rows;
        matrix->cols = matrix_node->cols;
        memcpy(matrix->data, matrix_node->data, matrix_node->rows * matrix_node->cols * sizeof(float));

        result->data.operand.temp_matrix = matrix;
    }

    else if (type == ET_OPERATOR) {
        // Operator: evaluate the operator (separate function) and store in the result
        result = evaluate_operator(node);
    }

    else if (type == ET_FUNCTION_CALL) {
        // Function call: evaluate the function (separate function) and store in the result
        result = evaluate_function(node);
    }

    else if (type == ET_COMMAND) {
        // Command: evaluate the command
        evaluate_command(node);
    }

    return result;
}

void evaluate_consecutive_nodes(struct NODE *node) {
    while (node != NULL) {
        struct NODE *result = evaluate_node(node);

        if (result != NULL) {
            // todo: store to Ans
            // todo: free the node + contents
        }

        node = node->next;
    }
}
