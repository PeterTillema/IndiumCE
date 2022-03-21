#include "variables.h"

#include "errors.h"

#include <cstring>
#include <tice.h>
#include <TINYSTL/vector.h>

struct var_real *variables[26];
String *strings[10];
String *equations[31];
struct var_list *lists[6];
struct var_custom_list *customLists[50];
Matrix *matrices[10];

static uint8_t custom_list_index = 0;

using tinystl::vector;

static void handle_real(const char *varname, void *data) {
    if (varname[0] >= tA && varname[0] <= tTheta) {
        auto real = new var_real();

        real->complex = false;
        real->value.num = new Number(os_RealToFloat((real_t *) data));

        unsigned int index = varname[0] - 'A';
        variables[index] = real;
    }
}

static void handle_cplx(const char *varname, void *data) {
    if (varname[0] >= tA && varname[0] <= tTheta) {
        auto oldCplx = (cplx_t *) data;
        auto cplx = new var_real();

        cplx->complex = true;
        cplx->value.cplx = new Complex(os_RealToFloat(&oldCplx->real), os_RealToFloat(&oldCplx->imag));

        auto index = varname[0] - 'A';
        variables[index] = cplx;
    }
}

static void handle_list(const char *varname, void *data) {
    auto oldList = (list_t *) data;

    auto list_data = vector<Number *>(oldList->dim);

    for (unsigned int i = 0; i < oldList->dim; i++) {
        list_data[i] = new Number(os_RealToFloat(&oldList->items[i]));
    }

    if (varname[1] >= 'A') {
        // Custom list
        if (custom_list_index == 50) parseError("Too much custom lists");

        auto custom_list = new var_custom_list();
        memcpy(custom_list->name, varname + 1, 5);
        custom_list->list.complex = false;
        custom_list->list.list.list = new List(list_data);

        customLists[custom_list_index++] = custom_list;
    } else {
        // OS list
        auto list = new var_list();
        list->complex = false;
        list->list.list = new List(list_data);

        auto index = (unsigned char) varname[1];
        lists[index] = list;
    }
}

static void handle_list_cplx(const char *varname, void *data) {
    auto oldList = (cplx_list_t *) data;

    auto list_data = vector<Complex *>(oldList->dim);

    for (unsigned int i = 0; i < oldList->dim; i++) {
        list_data[i] = new Complex(os_RealToFloat(&oldList->items[i].real), os_RealToFloat(&oldList->items[i].imag));
    }

    if (varname[1] >= 'A') {
        // Custom list
        if (custom_list_index == 50) parseError("Too much custom lists");

        auto new_list = new var_custom_list();
        memcpy(new_list->name, varname + 1, 5);
        new_list->list.complex = true;
        new_list->list.list.complexList = new ComplexList(list_data);

        customLists[custom_list_index++] = new_list;
    } else {
        // OS list
        auto list = new var_list();
        list->complex = true;
        list->list.complexList = new ComplexList(list_data);

        auto index = (unsigned char) varname[1];
        lists[index] = list;
    }
}

static void handle_matrix(const char *varname, void *data) {
    auto matrix = (matrix_t *) data;

    vector<vector<Number *>> matrix_data(matrix->rows, vector<Number *>(matrix->cols));

    unsigned int index = 0;
    for (uint8_t row = 0; row < matrix->rows; row++) {
        for (uint8_t col = 0; col < matrix->cols; col++) {
            matrix_data[row][col] = new Number(os_RealToFloat(&matrix->items[index]));
            index++;
        }
    }

    index = (unsigned char) varname[1];
    matrices[index] = new Matrix(matrix_data);
}

static void handle_string(const char *varname, void *data) {
    auto string = (string_t *) data;

    auto string_data = new char[string->len];
    memcpy(string_data, string->data, string->len);

    unsigned int index = (unsigned char) varname[1];
    strings[index] = new String(string->len, string_data);
}

static void handle_equation(const char *varname, void *data) {
    auto equation = (equ_t *) data;

    auto equation_data = new char[equation->len];

    memcpy(equation_data, equation->data, equation->len);

    unsigned int index = (unsigned char) varname[1];
    equations[index] = new String(equation->len, equation_data);
}

static void handle_unimplemented(__attribute__((unused)) const char *varname, __attribute__((unused)) void *data) {}

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

void get_all_os_variables() {
    char varname[9];
    uint24_t var_type;
    uint24_t name_length;
    void *data = nullptr;

    /**
     * After lots of debugging, I found out that ti_DetectAny( starts at the variable-length VAT, which doesn't
     * include OS vars, lists, matrices etc. So that's why we have to use a horrible custom routine to make it work!
     */
    void *entry = os_GetSymTablePtr();
    while ((entry = os_NextSymEntry(entry, &var_type, &name_length, varname, &data)) != nullptr) {
        varname[name_length] = '\0';

        // Skip Ans, we will treat it special afterwards
        if (varname[0] == tAns) continue;

        // Get a valid handler
        if (var_type >= sizeof(handlers) / sizeof(handlers[0])) continue;

        handlers[var_type](varname, data);
    }
}


