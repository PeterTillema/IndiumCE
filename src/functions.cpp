#include "functions.h"
#include "ast.h"
#include "types.h"

#include <cstdlib>

BaseType *evalFunction(struct NODE *funcNode) {
    unsigned int childNo = 0;
    BaseType *result = nullptr;

    struct NODE *tmp = funcNode->child;
    while (tmp != nullptr) {
        childNo++;
        tmp = tmp->next;
    }

    unsigned int func = funcNode->data.operand.func;

    switch (func) {
        default:
            break;
    }

    free(funcNode);

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
