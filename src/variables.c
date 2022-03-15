#include "variables.h"
#include "expression.h"

#include <fileioc.h>
#include <stdint.h>
#include <tice.h>
#include <string.h>

struct var_real *variables[26] = {0};
struct var_string *strings[10] = {0};
struct var_string *equations[31] = {0};
struct var_list *lists[6] = {0};
struct var_custom_list *custom_lists[50] = {0};
struct var_matrix *matrices[10] = {0};

static uint8_t custom_list_index = 0;

static void (*handlers[14])(const char *, void *);

static void handle_real(const char *var_name, void *data) {
    if (var_name[0] >= tA && var_name[0] <= tTheta) {
        struct var_real *real_data = (struct var_real *)malloc(sizeof(struct var_real));
        if (!real_data) return;

        real_data->complex = false;
        real_data->cplx.real = os_RealToFloat((real_t *) data);

        unsigned int index = var_name[0] - 'A';
        variables[index] = real_data;
    }
}

static void handle_cplx(const char *var_name, void *data) {
    if (var_name[0] >= tA && var_name[0] <= tTheta) {
        cplx_t *cplx = (cplx_t *)data;

        struct var_real *cplx_data = (struct var_real *)malloc(sizeof(struct var_real));
        if (!cplx_data) return;

        cplx_data->complex = true;
        cplx_data->cplx.real = os_RealToFloat(&cplx->real);
        cplx_data->cplx.imag = os_RealToFloat(&cplx->imag);

        unsigned int index = var_name[0] - 'A';
        variables[index] = cplx_data;
    }
}

static void handle_list(const char *var_name, void *data) {
    list_t *list = (list_t *)data;

    if (var_name[1] >= 'A') {
        // Custom list
        if (custom_list_index == 50) parse_error("Too much custom lists");

        struct var_custom_list *custom_list = (struct var_custom_list *)malloc(9 + list->dim * sizeof(float));
        if (!custom_list) return;

        for (unsigned int i = 0; i < list->dim; i++) {
            custom_list->list.data[i] = os_RealToFloat(&list->items[i]);
        }

        custom_list->list.complex = false;
        custom_list->list.size = list->dim;
        memcpy(custom_list->name, var_name + 1, 5);

        custom_lists[custom_list_index++] = custom_list;
    } else {
        // OS list
        struct var_list *list_data = (struct var_list *)malloc(4 + list->dim * sizeof(float));
        if (!list_data) return;

        for (unsigned int i = 0; i < list->dim; i++) {
            list_data->data[i] = os_RealToFloat(&list->items[i]);
        }

        list_data->complex = false;
        list_data->size = list->dim;

        unsigned int index = (unsigned int)var_name[1];
        lists[index] = list_data;
    }
}

static void handle_list_cplx(const char *var_name, void *data) {
    cplx_list_t *list = (cplx_list_t *)data;

    if (var_name[1] >= 'A') {
        // Custom list
        if (custom_list_index == 50) parse_error("Too much custom lists");

        struct var_custom_list *custom_list = (struct var_custom_list *)malloc(9 + list->dim * sizeof(float) * 2);
        if (!custom_list) return;

        for (unsigned int i = 0; i < list->dim; i++) {
            custom_list->list.data[i * 2] = os_RealToFloat(&list->items[i].real);
            custom_list->list.data[i * 2 + 1] = os_RealToFloat(&list->items[i].imag);
        }

        custom_list->list.complex = true;
        custom_list->list.size = list->dim;
        memcpy(custom_list->name, var_name + 1, 5);

        custom_lists[custom_list_index++] = custom_list;
    } else {
        // OS list
        struct var_list *list_data = (struct var_list *)malloc(4 + list->dim * sizeof(float) * 2);
        if (!list_data) return;

        for (unsigned int i = 0; i < list->dim; i++) {
            list_data->data[i * 2] = os_RealToFloat(&list->items[i].real);
            list_data->data[i * 2 + 1] = os_RealToFloat(&list->items[i].imag);
        }

        list_data->complex = true;
        list_data->size = list->dim;

        unsigned int index = (unsigned int)var_name[1];
        lists[index] = list_data;
    }
}

static void handle_matrix(const char *var_name, void *data) {
    matrix_t *matrix = (matrix_t *)data;

    struct var_matrix *matrix_data = (struct var_matrix *)malloc(matrix->rows * matrix->cols * sizeof(float) + 2);
    if (!matrix_data) return;

    unsigned index = 0;
    for (uint8_t row = 0; row < matrix->rows; row++) {
        for (uint8_t col = 0; col < matrix->cols; col++) {
            matrix_data->data[index] = os_RealToFloat(&matrix->items[index]);
            index++;
        }
    }

    matrix_data->rows = matrix->rows;
    matrix_data->cols = matrix->cols;

    index = (unsigned int)var_name[1];
    matrices[index] = matrix_data;
}

static void handle_string(const char *var_name, void *data) {
    string_t *string = (string_t *)data;

    struct var_string *string_data = (struct var_string *)malloc(string->len + 3);
    if (!string_data) return;

    string_data->length = string->len;
    memcpy(string_data->data, string->data, string->len);

    unsigned int index = (unsigned int)var_name[1];
    strings[index] = string_data;
}

static void handle_equation(const char *var_name, void *data) {
    equ_t *equation = (equ_t *)data;

    struct var_string *equation_data = (struct var_string *)malloc(equation->len + sizeof(unsigned int));
    if (!equation_data) return;

    equation_data->length = equation->len;
    memcpy(equation_data->data, equation->data, equation->len);

    unsigned int index = (unsigned int)var_name[1];
    equations[index] = equation_data;
}

static void handle_unimplemented(__attribute__((unused)) const char *var_name, __attribute__((unused)) void *data) {}

void get_all_os_variables(void) {
    char var_name[9];
    uint24_t var_type;
    uint24_t name_length;
    void *data = 0;

    /**
     * After lots of debugging, I found out that ti_DetectAny( starts at the variable-length VAT, which doesn't
     * include OS vars, lists, matrices etc. So that's why we have to use a horrible custom routine to make it work!
     */
    void *entry = os_GetSymTablePtr();
    while ((entry = os_NextSymEntry(entry, &var_type, &name_length, var_name, &data)) != NULL) {
        var_name[name_length] = '\0';

        // Skip Ans, we will treat it special afterwards
        if (var_name[0] == tAns) continue;

        // Get a valid handler
        if (var_type >= sizeof(handlers) / sizeof(handlers[0])) continue;

        handlers[var_type](var_name, data);
    }
}

static void (*handlers[14])(const char *, void *) = {
        handle_real,            // Real
        handle_list,            // Real List
        handle_matrix,          // Matrix
        handle_equation,        // Equation
        handle_string,          // String
        handle_unimplemented,   // Program
        handle_unimplemented,   // Protected Program
        handle_unimplemented,   // Picture
        handle_unimplemented,   // GDB
        handle_unimplemented,   // Unknown
        handle_unimplemented,   // Unknown Equation
        handle_unimplemented,   // New Equation
        handle_cplx,            // Complex
        handle_list_cplx,       // Complex List
};
