#include "functions.h"
#include "ast.h"
#include "evaluate.h"
#include "main.h"
#include "types.h"
#include "utils.h"

#include <ti/tokens.h>

BaseType *unaryFunction(NODE *firstChild, unsigned int childNo, UnaryFunction *function) {
    if (childNo != 1) argumentsError();

    auto arg = evalNode(firstChild);
    BaseType *result = arg->eval(*function);

    delete arg;

    return result;
}

BaseType *evalFunction(struct NODE *funcNode) {
    unsigned int childNo = 0;
    BaseType *result = nullptr;

    struct NODE *tmp = funcNode->child;
    while (tmp != nullptr) {
        childNo++;
        tmp = tmp->next;
    }

    unsigned int func = funcNode->data.operand.func;

    if (childNo == 1) {
        UnaryFunction *funcHandle;
        switch (func) {
            case tRound:
                funcHandle = new FuncRound();
                break;
            case tSin:
                funcHandle = new FuncSin();
                break;
            case tCos:
                funcHandle = new FuncCos();
                break;
            case tTan:
                funcHandle = new FuncTan();
                break;
            default:
                argumentsError();
        }

        result = unaryFunction(funcNode->child, childNo, funcHandle);
    }

    return result;
}

BaseType *UnaryFunction::eval(__attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *UnaryFunction::eval(__attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *UnaryFunction::eval(List &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &number : newElements) {
        number = dynamic_cast<Number &>(*this->eval(number));
    }

    return new List(newElements);
}

BaseType *UnaryFunction::eval(ComplexList &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(cplx));
    }

    return new ComplexList(newElements);
}

BaseType *UnaryFunction::eval(__attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *UnaryFunction::eval(__attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *FuncSin::eval(Number &rhs) {
    return new Number(sinfMode(rhs.num));
}

BaseType *FuncCos::eval(Number &rhs) {
    return new Number(cosfMode(rhs.num));
}

BaseType *FuncTan::eval(Number &rhs) {
    return new Number(tanfMode(rhs.num));
}

BaseType *FuncRound::eval(Number &rhs) {
    // todo: use a custom routine, as this one sucks!
    // return new Number(roundf_custom(rhs.num * 1e9) / 1e9);
    return new Number(roundf_custom(rhs.num));
}

BaseType *FuncRound::eval(Complex &rhs) {
    return new Complex(roundf_custom(rhs.real * 1e9) / 1e9, roundf_custom(rhs.imag * 1e9) / 1e9);
}

BaseType *FuncRound::eval(Matrix &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &row : newElements) {
        for (auto &col : row) {
            col = dynamic_cast<Number &>(*this->eval(col));
        }
    }

    return new Matrix(newElements);
}
