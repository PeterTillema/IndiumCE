#ifndef TYPES_H
#define TYPES_H

#include "errors.h"
#include "TINYSTL/vector.h"

#include <cstdint>

using tinystl::vector;

class Operator;

class BaseType {
public:
    virtual ~BaseType() = default;

    virtual char *toString() const;

    virtual BaseType *eval(Operator &op) = 0;
};

class Number : public BaseType {
public:
    float num = 0;

    Number() = default;

    ~Number() override = default;

    explicit Number(float num);

    char *toString() const override;

    BaseType *eval(Operator &op) override;
};

class Complex : public BaseType {
public:
    float real;
    float imag;

    Complex(float real, float imag);

    ~Complex() override = default;

    BaseType *eval(Operator &op) override;
};

class List : public BaseType {
public:
    vector<Number *> elements;

    explicit List(vector<Number *> &elements);

    ~List() override;

    BaseType * eval(Operator &op) override;
};

class ComplexList : public BaseType {
public:
    vector<Complex *> elements;

    explicit ComplexList(const vector<Complex *> &elements);

    ~ComplexList() override;

    BaseType *eval(Operator &op) override;
};

class String : public BaseType {
public:
    unsigned int length;
    char *string;

    explicit String(unsigned int length, char *string);

    ~String() override;

    BaseType *eval(Operator &op) override;
};

class Matrix : public BaseType {
public:
    vector<vector<Number *>> elements;

    explicit Matrix(vector<vector<Number *>> &elements);

    ~Matrix() override;

    BaseType *eval(Operator &op) override;
};

class Operator {
public:
    virtual ~Operator() = default;

    virtual BaseType *eval(Number &rhs);

    virtual BaseType *eval(Complex &rhs);

    virtual BaseType *eval(List &rhs);

    virtual BaseType *eval(ComplexList &rhs);

    virtual BaseType *eval(String &rhs);

    virtual BaseType *eval(Matrix &rhs);
};

class UnaryOperator : public Operator {
};


#endif
