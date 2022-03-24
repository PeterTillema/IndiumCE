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

class OpPower : public BinaryOperator {
    BaseType *eval(Number &lhs, Number &rhs) override;

    BaseType *eval(Number &lhs, Complex &rhs) override;

    BaseType *eval(Number &lhs, Matrix &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override;

    BaseType *eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Number &rhs) override;
};

class OpFact : public UnaryOperator {
    BaseType *eval(Number &rhs) override;
};

class OpChs : public UnaryOperator {
    BaseType *eval(Number &rhs) override;

    BaseType *eval(Complex &rhs) override;

    BaseType *eval(Matrix &rhs) override;
};

class OpMul : public BinaryOperator {
    BaseType *eval(Number &lhs, Number &rhs) override;

    BaseType *eval(Number &lhs, Complex &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override;

    BaseType *eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpDiv : public BinaryOperator {
    BaseType *eval(Number &lhs, Number &rhs) override;

    BaseType *eval(Number &lhs, Complex &rhs) override;

    BaseType *eval(Number &lhs, Matrix &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override;

    BaseType *eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Number &rhs) override;
};

class OpAddSub : public BinaryOperator {
    BaseType *eval(Number &lhs, Number &rhs) override = 0;

    BaseType *eval(Number &lhs, Complex &rhs) override = 0;

    BaseType *eval(Number &lhs, Matrix &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override = 0;

    BaseType *eval(Complex &lhs, Complex &rhs) override = 0;

    BaseType *eval(String &lhs, String &rhs) override = 0;

    BaseType *eval(Matrix &lhs, Number &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpAdd : public OpAddSub {
    BaseType *eval(Number &lhs, Number &rhs) override;

    BaseType *eval(Number &lhs, Complex &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override;

    BaseType *eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(String &lhs, String &rhs) override;
};

class OpSub : public OpAddSub {
    BaseType *eval(Number &lhs, Number &rhs) override;

    BaseType *eval(Number &lhs, Complex &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override;

    BaseType *eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(String &lhs, String &rhs) override;
};

class OpEquality : public BinaryOperator {
    BaseType *eval(Number &lhs, Number &rhs) override = 0;

    BaseType *eval(Number &lhs, Complex &rhs) override;

    BaseType *eval(Number &lhs, Matrix &rhs) override;

    BaseType *eval(Complex &lhs, Number &rhs) override;

    BaseType *eval(Complex &lhs, Complex &rhs) override = 0;

    BaseType *eval(Complex &lhs, ComplexList &rhs) override;

    BaseType *eval(ComplexList &lhs, Complex &rhs) override;

    BaseType *eval(ComplexList &lhs, ComplexList &rhs) override;

    BaseType *eval(Matrix &lhs, Number &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpEQ : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override;

    BaseType * eval(Complex &lhs, Complex &rhs) override;
};

class OpLT : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override;

    BaseType * eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpGT : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override;

    BaseType * eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpLE : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override;

    BaseType * eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpGE : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override;

    BaseType * eval(Complex &lhs, Complex &rhs) override;

    BaseType *eval(Matrix &lhs, Matrix &rhs) override;
};

class OpNE : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override;

    BaseType * eval(Complex &lhs, Complex &rhs) override;
};

class OpLogically : public OpEquality {
    BaseType * eval(Number &lhs, Number &rhs) override = 0;

    BaseType * eval(Complex &lhs, Complex &rhs) override;

    BaseType * eval(Matrix &lhs, Matrix &rhs) override;
};

class OpAnd : public OpLogically {
    BaseType * eval(Number &lhs, Number &rhs) override;
};

class OpOr : public OpLogically {
    BaseType * eval(Number &lhs, Number &rhs) override;
};

class OpXor : public OpLogically {
    BaseType * eval(Number &lhs, Number &rhs) override;
};

uint8_t getOpPrecedence(uint8_t op);

bool isUnaryOp(uint8_t prec);

BaseType *evalOperator(struct NODE *op_node);

#endif
