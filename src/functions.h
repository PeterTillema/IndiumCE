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

class FuncRound : public UnaryFunction {
    BaseType * eval(Number &rhs) override;

    BaseType * eval(Complex &rhs) override;

    BaseType * eval(Matrix &rhs) override;
};

class FuncSin : public UnaryFunction {
    BaseType * eval(Number &rhs) override;
};

class FuncCos : public UnaryFunction {
    BaseType * eval(Number &rhs) override;
};

class FuncTan : public UnaryFunction {
    BaseType * eval(Number &rhs) override;
};

BaseType *evalFunction(struct NODE *evalNode);

#endif
