#include "variables.h"
#include "ast.h"

#include <debug.h>
#include <fileioc.h>
#include <stdint.h>
#include <tice.h>

struct var_real variables[26];
__attribute__((unused)) struct var_string strings[10];
__attribute__((unused)) struct var_list lists[6];
__attribute__((unused)) struct var_matrix matrices[10];
__attribute__((unused)) struct var_custom_list custom_lists[50];

static void (*handlers[256])(const char *, void *);

static void handle_real(const char *var_name, void *data) {
    if (var_name[0] >= tA && var_name[0] <= tTheta) {
        float real = os_RealToFloat((const real_t *) data);

        unsigned int index = var_name[0] - 'A';
        variables[index].complex = false;
        variables[index].real = real;

        dbg_sprintf(dbgout, "Found real %s: %f\n", var_name, real);
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

        dbg_sprintf(dbgout, "Found complex %s: %f + %fi\n", var_name, real, imag);
    }
}

static void handle_list(const char *var_name, void *data) {
    list_t *list = (list_t *)data;

    dbg_sprintf(dbgout, "Found list %d with %d elements\n", var_name[1], list->dim);

    if (!list->dim) return;

    float *list_data = (float *)malloc(list->dim * sizeof(float));
    if (!list_data) return;

    for (unsigned int i = 0; i < list->dim; i++) {
        list_data[i] = os_RealToFloat(&list->items[i]);
        dbg_sprintf(dbgout, " -- %f\n", list_data[i]);
    }

    unsigned int index = (unsigned int)var_name[1];
    lists[index].complex = false;
    lists[index].size = list->dim;
    lists[index].data = list_data;
}

static void handle_list_cplx(const char *var_name, void *data) {
    cplx_list_t *list = (cplx_list_t *)data;

    dbg_sprintf(dbgout, "Found list %d with %d elements\n", var_name[1], list->dim);

    if (!list->dim) return;

    float *list_data = (float *)malloc(list->dim * sizeof(float) * 2);
    if (!list_data) return;

    for (unsigned int i = 0; i < list->dim; i++) {
        list_data[i * 2] = os_RealToFloat(&list->items[i].real);
        list_data[i * 2 + 1] = os_RealToFloat(&list->items[i].imag);
        dbg_sprintf(dbgout, " -- %f + %fi\n", list_data[i * 2], list_data[i * 2 + 1]);
    }

    unsigned int index = (unsigned int)var_name[1];
    lists[index].complex = true;
    lists[index].size = list->dim;
    lists[index].data = list_data;
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
        handle_unimplemented,   // TI_MATRIX_TYPE
        handle_unimplemented,   // TI_EQU_TYPE
        handle_unimplemented,   // TI_STRING_TYPE
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
