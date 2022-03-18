#include "operators.h"
#include "ast.h"
#include "errors.h"
#include "evaluate.h"

#include <cstdlib>
#include <cstring>
#include <tice.h>

// https://education.ti.com/html/webhelp/EG_TI84PlusCE/EN/content/eg_gsguide/m_expressions/exp_order_of_operations.HTML
// Commas are treated as a special operator, which means that if the token is a comma, all other operators will be
// moved properly to the output. It is also "right-associative", which means that if the top stack entry is also a
// comma, it won't be moved (which is clearly not what we want)
static const uint8_t operators[] = {
        tFromRad, tFromDeg, tRecip, tSqr, tTrnspos, tCube,
        tPower,
        tFact,
        tChs,
        tnPr, tnCr,
        tMul, tDiv,
        tAdd, tSub,
        tEQ, tLT, tGT, tLE, tGE, tNE,
        tAnd,
        tOr, tXor,
        tStore,
        tComma
};
static const uint8_t precedence[] = {
        1, 1, 1, 1, 1, 1,
        2,
        3,
        4,
        5, 5,
        6, 6,
        7, 7,
        8, 8, 8, 8, 8, 8,
        9,
        10, 10,
        11,
        255
};

uint8_t getOpPrecedence(uint8_t op) {
    void *index = memchr(operators, op, sizeof(operators));

    return precedence[(const uint8_t *) index - operators];
}

bool isUnaryOp(uint8_t prec) {
    return prec <= 4 && prec != 2;
}

static struct NODE *opFromRad(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opFromRad();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opFromRad();
    else typeError();

    return node;
}

static struct NODE *opFromDeg(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opFromDeg();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opFromDeg();
    else typeError();

    return node;
}

static struct NODE *opRecip(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opRecip();
    else if (type == ET_COMPLEX) node->data.operand.cplx->opRecip();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opRecip();
    else if (type == ET_TEMP_LIST_COMPLEX) node->data.operand.complexList->opRecip();
    else if (type == ET_TEMP_MATRIX) node->data.operand.matrix->opRecip();
    else typeError();

    return node;
}

static struct NODE *opSqr(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opSqr();
    else if (type == ET_COMPLEX) node->data.operand.cplx->opSqr();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opSqr();
    else if (type == ET_TEMP_LIST_COMPLEX) node->data.operand.complexList->opSqr();
    else if (type == ET_TEMP_MATRIX) node->data.operand.matrix->opSqr();
    else typeError();

    return node;
}

static struct NODE *opCube(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opCube();
    else if (type == ET_COMPLEX) node->data.operand.cplx->opCube();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opCube();
    else if (type == ET_TEMP_LIST_COMPLEX) node->data.operand.complexList->opCube();
    else if (type == ET_TEMP_MATRIX) node->data.operand.matrix->opCube();
    else typeError();

    return node;
}

static struct NODE *opPower(struct NODE *node) {
    struct NODE *right = evalNode(node->next);

    enum etype type = node->data.type;
    enum etype type2 = right->data.type;

    if (type == ET_NUMBER) {
        if (type2 == ET_NUMBER) node->data.operand.num->opPower(right->data.operand.num);
        else if (type2 == ET_COMPLEX) node->data.operand.num->opPower(right->data.operand.cplx);
        else if (type2 == ET_TEMP_LIST) node->data.operand.num->opPower(right->data.operand.list);
        else if (type2 == ET_TEMP_LIST_COMPLEX) node->data.operand.num->opPower(right->data.operand.complexList);
        else typeError();

        nodeFree(node);

        return right;
    } else if (type == ET_COMPLEX) {
        if (type2 == ET_NUMBER) {
            node->data.operand.cplx->opPower(right->data.operand.num);

            nodeFree(right);

            return node;
        } else if (type2 == ET_COMPLEX) {
            node->data.operand.cplx->opPower(right->data.operand.cplx);

            nodeFree(right);

            return node;
        } else if (type2 == ET_TEMP_LIST) {
            auto result = new NODE();
            result->data.type = ET_TEMP_LIST_COMPLEX;
            result->data.operand.complexList = node->data.operand.cplx->opPower(right->data.operand.list);

            nodeFree(node);
            nodeFree(right);

            return result;
        } else if (type2 == ET_TEMP_LIST_COMPLEX) {
            node->data.operand.cplx->opPower(right->data.operand.complexList);

            nodeFree(node);

            return right;
        } else typeError();
    } else if (type == ET_TEMP_LIST) {
        if (type2 == ET_NUMBER) {
            node->data.operand.list->opPower(right->data.operand.num);

            nodeFree(right);

            return node;
        } else if (type2 == ET_COMPLEX) {
            node->data.operand.list->opPower(right->data.operand.cplx);

            nodeFree(right);

            return node;
        }
    } else {
        typeError();
    }
}

static struct NODE *opFact(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opFact();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opFact();
    else typeError();

    return node;
}

static struct NODE *opChs(struct NODE *node) {
    enum etype type = node->data.type;

    if (type == ET_NUMBER) node->data.operand.num->opChs();
    else if (type == ET_COMPLEX) node->data.operand.cplx->opChs();
    else if (type == ET_TEMP_LIST) node->data.operand.list->opChs();
    else if (type == ET_TEMP_LIST_COMPLEX) node->data.operand.complexList->opChs();
    else if (type == ET_TEMP_MATRIX) node->data.operand.matrix->opChs();
    else typeError();

    return node;
}

struct NODE *evalOperator(struct NODE *node) {
    uint8_t op = node->data.operand.op;
    struct NODE *leftNode = evalNode(node->child);
    struct NODE *result;

    leftNode->next = node->child->next;

    switch (op) {
        case tFromRad:
            result = opFromRad(leftNode);
            break;
        case tFromDeg:
            result = opFromDeg(leftNode);
            break;
        case tRecip:
            result = opRecip(leftNode);
            break;
        case tSqr:
            result = opSqr(leftNode);
            break;
        case tCube:
            result = opCube(leftNode);
            break;
        case tPower:
            result = opPower(leftNode);
            break;
        case tFact:
            result = opFact(leftNode);
            break;
        case tChs:
            result = opChs(leftNode);
            break;
        default:
            result = nullptr;
    }

    free(node);

    return result;
}
