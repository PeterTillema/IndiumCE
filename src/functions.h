#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "ast.h"
#include "types.h"

class UnaryFunction {
public:
    virtual ~UnaryFunction() = default;

    virtual BaseType *eval(Number &rhs);

    virtual BaseType *eval(Complex &rhs);

    virtual BaseType *eval(List &rhs);

    virtual BaseType *eval(ComplexList &rhs);

    virtual BaseType *eval(String &rhs);

    virtual BaseType *eval(Matrix &rhs);
};

BaseType *evalFunction(struct NODE *evalNode);

#endif
