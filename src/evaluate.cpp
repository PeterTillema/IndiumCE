#include "evaluate.h"
#include "ast.h"
#include "commands.h"
#include "functions.h"
#include "operators.h"
#include "types.h"
#include "variables.h"

#include <cstring>

struct NODE *evalNode(struct NODE *node) {
    enum etype type = node->data.type;
    struct NODE *result = node;

    switch (type) {
        case ET_NUMBER: {
            result = new NODE();
            result->data.type = ET_NUMBER;
            result->data.operand.num = node->data.operand.num;

            break;
        }

        case ET_COMPLEX: {
            result = new NODE();
            result->data.type = ET_COMPLEX;
            result->data.operand.cplx = new Complex(node->data.operand.cplx->real, node->data.operand.cplx->imag);

            break;
        }

        case ET_VARIABLE: {
            struct var_real *varNode = variables[node->data.operand.variableNr];

            result = new NODE();

            if (varNode->complex) {
                result->data.type = ET_COMPLEX;
                result->data.operand.cplx = new Complex(varNode->value.cplx->real, varNode->value.cplx->imag);
            } else {
                result->data.type = ET_NUMBER;
                result->data.operand.num = new Number(varNode->value.num->num);
            }

            break;
        }

        case ET_STRING: {
            String *stringNode = strings[node->data.operand.stringNr];

            auto string_data = new char[stringNode->length];
            memcpy(string_data, stringNode->string, stringNode->length);

            result = new NODE();
            result->data.type = ET_TEMP_STRING;
            result->data.operand.string = new String(stringNode->length, string_data);

            break;
        }

        case ET_EQU: {
            String *equNode = equations[node->data.operand.equationNr];

            auto string_data = new char[equNode->length];
            memcpy(string_data, equNode->string, equNode->length);

            result = new NODE();
            result->data.type = ET_TEMP_STRING;
            result->data.operand.string = new String(equNode->length, string_data);

            break;
        }

        case ET_LIST:
        case ET_CUSTOM_LIST: {
            struct var_list *listNode;

            if (type == ET_LIST) {
                listNode = lists[node->data.operand.listNr];
            } else {
                listNode = &customLists[node->data.operand.customListNr]->list;
            }

            result = new NODE();

            if (listNode->complex) {
                auto listData = listNode->list.complexList->elements;

                result->data.type = ET_TEMP_LIST_COMPLEX;
                result->data.operand.complexList = new ComplexList(listData);
            } else {
                auto listData = listNode->list.list->elements;

                result->data.type = ET_TEMP_LIST;
                result->data.operand.list = new List(listData);
            }

            break;
        }

        case ET_MATRIX: {
            Matrix *matrix = matrices[node->data.operand.matrixNr];
            auto matrixData = matrix->elements;

            result = new NODE();
            result->data.type = ET_TEMP_MATRIX;
            result->data.operand.matrix = new Matrix(matrixData);

            break;
        }

        case ET_OPERATOR:
            result = evalOperator(node);
            break;

        case ET_FUNCTION_CALL:
            result = evalFunction(node);
            break;

        case ET_COMMAND:
            evalCommand(node);
            break;

        default:
            break;
    }

    return result;
}

void evalNodes(struct NODE *node) {
    while (node != nullptr) {
        struct NODE *result = evalNode(node);

        if (result != nullptr) {
            // todo: store to Ans
            nodeFree(result);
        }

        node = node->next;
    }
}
