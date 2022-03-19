#ifndef TYPES_H
#define TYPES_H

#include "TINYSTL/vector.h"

#include <cstdint>

using tinystl::vector;

class Number;

class Complex;

class String;

class List;

class ComplexList;

class Matrix;

class Number {
public:
    float num{};

    Number() = default;

    explicit Number(float num);

    char *toString() const;

    void opFromRad();

    void opFromDeg();

    void opRecip();

    void opSqr();

    void opCube();

    void opPower(Number *rhs) const;

    void opPower(Complex *rhs) const;

    void opPower(List *rhs) const;

    void opPower(ComplexList *rhs) const;

    void opFact();

    void opChs();

    // nPr, nCr

    void opMul(Number *rhs) const;

    void opMul(Complex *rhs) const;

    void opMul(List *rhs) const;

    void opMul(ComplexList *rhs) const;

    void opMul(Matrix *rhs) const;
};


class Complex {
public:
    float real{};
    float imag{};

    Complex() = default;

    explicit Complex(float real, float imag);

    char *toString() const;

    void opRecip();

    void opSqr();

    void opCube();

    void opPower(Number *rhs);

    void opPower(Complex *rhs);

    ComplexList *opPower(List *rhs) const;

    void opPower(ComplexList *rhs) const;

    void opChs();

    // nPr, nCr

    void opMul(Number *rhs);

    void opMul(Complex *rhs);

    ComplexList *opMul(List *rhs) const;

    void opMul(ComplexList *rhs) const;
};


class String {
public:
    unsigned int length;
    char *string;

    explicit String(unsigned int length, char *string);

    ~String();
};


class List {
public:
    vector<Number> elements;

    explicit List(const tinystl::vector<Number> &elements);

    ~List();

    char *toString() const;

    void opFromRad();

    void opFromDeg();

    void opRecip();

    void opSqr();

    void opCube();

    void opPower(Number *rhs);

    ComplexList *opPower(Complex *rhs);

    void opPower(List *rhs);

    void opPower(ComplexList *rhs);

    void opFact();

    void opChs();

    // nPr, nCr

    void opMul(Number *rhs);

    ComplexList *opMul(Complex *rhs) const;

    void opMul(List *rhs);

    void opMul(ComplexList *rhs) const;
};


class ComplexList {
public:
    vector<Complex> elements;

    explicit ComplexList(const tinystl::vector<Complex> &elements);

    ~ComplexList();

    char *toString() const;

    void opRecip();

    void opSqr();

    void opCube();

    void opPower(Number *rhs);

    void opPower(Complex *rhs);

    void opPower(List *rhs);

    void opPower(ComplexList *rhs);

    void opChs();

    // nPr, nCr

    void opMul(Number *rhs);

    void opMul(Complex *rhs);

    void opMul(List *rhs);

    void opMul(ComplexList *rhs);
};


class Matrix {
public:
    vector<vector<Number>> elements;

    explicit Matrix(const vector<vector<Number>> &elements);

    ~Matrix();

    void opRecip();

    void opSqr();

    void opTrnspos();

    void opCube();

    void opPower(Number *rhs);

    void opChs();

    // nPr, nCr

    void opMul(Number *rhs);

    void opMul(Matrix *rhs);
};


#endif
