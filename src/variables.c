#include "variables.h"
#include "expression.h"

#include <debug.h>
#include <fileioc.h>
#include <stdint.h>
#include <tice.h>
#include <string.h>

struct var_real variables[26] = {0};
struct var_string strings[10];
struct var_list lists[6];
struct var_matrix matrices[10];
struct var_custom_list custom_lists[50];

static uint8_t custom_list_index = 0;

static void (*handlers[256])(const char *, void *);

static void handle_real(const char *var_name, void *data) {
    if (var_name[0] >= tA && var_name[0] <= tTheta) {
        float real = os_RealToFloat((const real_t *) data);

        unsigned int index = var_name[0] - 'A';
        variables[index].complex = false;
        variables[index].real = real;
    }
}

static void handle_cplx(const char *var_name, void *data) {
    if (var_name[0] >= tA && var_name[0] <= tTheta) {
        float real = os_RealToFloat(&((const cplx_t *) data)->real);
        float imag = os_RealToFloat(&((const cplx_t *) data)->imag);

        unsigned int index = var_name[0] - 'A';
        variables[index].complex = true;
        variables[index].real = real;
        variables[index].imag = imag;
    }
}

static void handle_list(const char *var_name, void *data) {
    list_t *list = (list_t *)data;

    if (!list->dim) return;

    float *list_data = (float *)malloc(list->dim * sizeof(float));
    if (!list_data) return;

    for (unsigned int i = 0; i < list->dim; i++) {
        list_data[i] = os_RealToFloat(&list->items[i]);
    }

    if (var_name[1] >= 'A') {
        // Custom list
        if (custom_list_index == 50) parse_error("Too much lists");
        memcpy(custom_lists[custom_list_index].name, var_name + 1, 5);
        custom_lists[custom_list_index].var_list.complex = false;
        custom_lists[custom_list_index].var_list.size = list->dim;
        custom_lists[custom_list_index].var_list.data = list_data;
        custom_list_index++;
    } else {
        // OS list
        unsigned int index = (unsigned int)var_name[1];
        lists[index].complex = false;
        lists[index].size = list->dim;
        lists[index].data = list_data;
    }
}

static void handle_list_cplx(const char *var_name, void *data) {
    cplx_list_t *list = (cplx_list_t *)data;

    if (!list->dim) return;

    float *list_data = (float *)malloc(list->dim * sizeof(float) * 2);
    if (!list_data) return;

    for (unsigned int i = 0; i < list->dim; i++) {
        list_data[i * 2] = os_RealToFloat(&list->items[i].real);
        list_data[i * 2 + 1] = os_RealToFloat(&list->items[i].imag);
    }

    if (var_name[1] >= 'A') {
        // Custom list
        if (custom_list_index == 50) parse_error("Too much lists");
        memcpy(custom_lists[custom_list_index].name, var_name + 1, 5);
        custom_lists[custom_list_index].var_list.complex = false;
        custom_lists[custom_list_index].var_list.size = list->dim;
        custom_lists[custom_list_index].var_list.data = list_data;
        custom_list_index++;
    } else {
        // OS list
        unsigned int index = (unsigned int)var_name[1];
        lists[index].complex = false;
        lists[index].size = list->dim;
        lists[index].data = list_data;
    }
}

static void handle_matrix(const char *var_name, void *data) {
    matrix_t *matrix = (matrix_t *)data;

    if (!matrix->rows || !matrix->cols) return;

    float *matrix_data = (float *)malloc(matrix->rows * matrix->cols * sizeof(float));
    if (!matrix_data) return;

    unsigned index = 0;
    for (uint8_t row = 0; row < matrix->rows; row++) {
        for (uint8_t col = 0; col < matrix->cols; col++) {
            matrix_data[index] = os_RealToFloat(&matrix->items[index]);
            index++;
        }
    }

    index = (unsigned int)var_name[1];
    matrices[index].rows = matrix->rows;
    matrices[index].cols = matrix->cols;
    matrices[index].data = matrix_data;
}

static void handle_string(const char *var_name, void *data) {
    string_t *string = (string_t *)data;

    if (!string->len) return;

    char *string_data = (char *)malloc(string->len);
    if (!string_data) return;

    memcpy(string_data, string->data, string->len);

    unsigned int index = (unsigned int)var_name[1];
    strings[index].length = string->len;
    strings[index].data = string_data;
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
        if (var_type >13) continue;

        handlers[var_type](var_name, data);
    }
}

static void (*handlers[256])(const char *, void *) = {
        handle_real,            // TI_REAL_TYPE,
        handle_list,            // TI_REAL_LIST_TYPE
        handle_matrix,          // TI_MATRIX_TYPE
        handle_unimplemented,   // TI_EQU_TYPE
        handle_string,          // TI_STRING_TYPE
        handle_unimplemented,   // TI_PRGM_TYPE
        handle_unimplemented,   // TI_PPRGM_TYPE
        handle_unimplemented,   // TI_REAL_LIST_TYPE
        handle_unimplemented,   // TI_REAL_LIST_TYPE
        handle_unimplemented,   // TI_REAL_LIST_TYPE
        handle_unimplemented,   // TI_REAL_LIST_TYPE
        handle_unimplemented,   // TI_REAL_LIST_TYPE
        handle_cplx,            // TI_CPLX_TYPE
        handle_list_cplx,       // TI_CPLX_LIST_TYPE
};
