#include "evaluate.h"
#include "ast.h"
#include "commands.h"
#include "functions.h"
#include "operators.h"
#include "types.h"
#include "variables.h"

#include <cstring>

BaseType *evalNode(struct NODE *node) {
    enum etype type = node->data.type;

    switch (type) {
        case ET_NUMBER:
            return new Number(node->data.operand.num->num);
        case ET_COMPLEX:
            return new Complex(node->data.operand.cplx->real, node->data.operand.cplx->imag);
        case ET_VARIABLE: {
            struct var_real *varNode = variables[node->data.operand.variableNr];

            if (varNode->complex) {
                return new Complex(varNode->value.cplx->real, varNode->value.cplx->imag);
            } else {
                return new Number(varNode->value.num->num);
            }
        }

        case ET_STRING: {
            String *stringNode = strings[node->data.operand.stringNr];

            auto string_data = new char[stringNode->length];
            memcpy(string_data, stringNode->string, stringNode->length);

            return new String(stringNode->length, string_data);
        }

        case ET_EQU: {
            String *equNode = equations[node->data.operand.equationNr];

            auto string_data = new char[equNode->length];
            memcpy(string_data, equNode->string, equNode->length);

            return new String(equNode->length, string_data);
        }

        case ET_LIST:
        case ET_CUSTOM_LIST: {
            struct var_list *listNode;

            if (type == ET_LIST) {
                listNode = lists[node->data.operand.listNr];
            } else {
                listNode = &customLists[node->data.operand.customListNr]->list;
            }

            if (listNode->complex) {
                auto listData = listNode->list.complexList->elements;

                return new ComplexList(listData);
            } else {
                auto listData = listNode->list.list->elements;

                return new List(listData);
            }
        }

        case ET_MATRIX: {
            Matrix *matrix = matrices[node->data.operand.matrixNr];
            auto matrixData = matrix->elements;

            return new Matrix(matrixData);
        }

        case ET_OPERATOR:
            return evalOperator(node);

        case ET_FUNCTION_CALL:
            return evalFunction(node);

        case ET_COMMAND:
            evalCommand(node);
            break;

        default:
            break;
    }

    return nullptr;
}

void evalNodes(struct NODE *node) {
    while (node != nullptr) {
        BaseType *result = evalNode(node);

        // todo: store to Ans
        delete result;

        node = node->next;
    }
}
