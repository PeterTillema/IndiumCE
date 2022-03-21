#ifndef OPERATORS_H
#define OPERATORS_H

#include <cstdint>

#include "types.h"

#define MAX_PRECEDENCE 11

class OpFromRad : public UnaryOperator {
    BaseType *eval(Number &rhs) override;
};

class OpFromDeg : public UnaryOperator {
    BaseType *eval(Number &rhs) override;
};

class OpRecip : public UnaryOperator {
    BaseType *eval(Number &rhs) override;

    BaseType *eval(Complex &rhs) override;

    BaseType *eval(Matrix &rhs) override;
};

class OpSqr : public UnaryOperator {
    BaseType *eval(Number &rhs) override;

    BaseType *eval(Complex &rhs) override;

    BaseType *eval(Matrix &rhs) override;
};

class OpTrnspos : public UnaryOperator {
    BaseType *eval(Matrix &rhs) override;
};

class OpCube : public UnaryOperator {
    BaseType *eval(Number &rhs) override;

    BaseType *eval(Complex &rhs) override;

    BaseType *eval(Matrix &rhs) override;
};

class OpFact : public UnaryOperator {
    BaseType *eval(Number &rhs) override;

    BaseType *eval(Matrix &rhs) override;;
};

class OpChs : public UnaryOperator {
    BaseType *eval(Number &rhs) override;

    BaseType *eval(Complex &rhs) override;
};

uint8_t getOpPrecedence(uint8_t op);

bool isUnaryOp(uint8_t prec);

BaseType *evalOperator(struct NODE *op_node);

#endif
