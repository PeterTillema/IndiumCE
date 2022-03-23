#ifndef TYPES_H
#define TYPES_H

#include "errors.h"
#include "TINYSTL/vector.h"

#include <cstdint>

using tinystl::vector;

class UnaryOperator;

class BinaryOperator;

class BaseType {
public:
    virtual ~BaseType() = default;

    virtual char *toString() const;

    virtual BaseType *eval(UnaryOperator &op) = 0;

    virtual BaseType *eval(BinaryOperator &op, BaseType *rhs) = 0;
};

class Number : public BaseType {
public:
    float num = 0;

    Number() = default;

    explicit Number(float num);

    ~Number() override = default;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;
};

class Complex : public BaseType {
public:
    float real = 0;
    float imag = 0;

    Complex() = default;

    Complex(float real, float imag);

    ~Complex() override = default;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;
};

class List : public BaseType {
public:
    vector<Number> elements;

    explicit List(vector<Number> &elements);

    ~List() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;
};

class ComplexList : public BaseType {
public:
    vector<Complex> elements;

    explicit ComplexList(const vector<Complex> &elements);

    ~ComplexList() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;
};

class String : public BaseType {
public:
    unsigned int length;
    char *string;

    explicit String(unsigned int length, char *string);

    ~String() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;
};

class Matrix : public BaseType {
public:
    vector<vector<Number>> elements;

    explicit Matrix(vector<vector<Number>> &elements);

    ~Matrix() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;
};

class UnaryOperator {
public:
    virtual ~UnaryOperator() = default;

    virtual BaseType *eval(Number &rhs);

    virtual BaseType *eval(Complex &rhs);

    virtual BaseType *eval(List &rhs);

    virtual BaseType *eval(ComplexList &rhs);

    virtual BaseType *eval(String &rhs);

    virtual BaseType *eval(Matrix &rhs);
};

class BinaryOperator {
public:
    virtual ~BinaryOperator() = default;

    BaseType *eval(Number &lhs, BaseType &rhs);

    BaseType *eval(Number &lhs, Number &rhs);

    BaseType *eval(Number &lhs, Complex &rhs);

    BaseType *eval(Number &lhs, List &rhs);

    BaseType *eval(Number &lhs, ComplexList &rhs);

    BaseType *eval(Number &lhs, String &rhs);

    BaseType *eval(Number &lhs, Matrix &rhs);

    BaseType *eval(Complex &lhs, BaseType &rhs);

    BaseType *eval(Complex &lhs, Number &rhs);

    BaseType *eval(Complex &lhs, Complex &rhs);

    BaseType *eval(Complex &lhs, List &rhs);

    BaseType *eval(Complex &lhs, ComplexList &rhs);

    BaseType *eval(Complex &lhs, String &rhs);

    BaseType *eval(Complex &lhs, Matrix &rhs);

    BaseType *eval(List &lhs, BaseType &rhs);

    BaseType *eval(List &lhs, Number &rhs);

    BaseType *eval(List &lhs, Complex &rhs);

    BaseType *eval(List &lhs, List &rhs);

    BaseType *eval(List &lhs, ComplexList &rhs);

    BaseType *eval(List &lhs, String &rhs);

    BaseType *eval(List &lhs, Matrix &rhs);

    BaseType *eval(ComplexList &lhs, BaseType &rhs);

    BaseType *eval(ComplexList &lhs, Number &rhs);

    BaseType *eval(ComplexList &lhs, Complex &rhs);

    BaseType *eval(ComplexList &lhs, List &rhs);

    BaseType *eval(ComplexList &lhs, ComplexList &rhs);

    BaseType *eval(ComplexList &lhs, String &rhs);

    BaseType *eval(ComplexList &lhs, Matrix &rhs);

    BaseType *eval(String &lhs, BaseType &rhs);

    BaseType *eval(String &lhs, Number &rhs);

    BaseType *eval(String &lhs, Complex &rhs);

    BaseType *eval(String &lhs, List &rhs);

    BaseType *eval(String &lhs, ComplexList &rhs);

    BaseType *eval(String &lhs, String &rhs);

    BaseType *eval(String &lhs, Matrix &rhs);

    BaseType *eval(Matrix &lhs, BaseType &rhs);

    BaseType *eval(Matrix &lhs, Number &rhs);

    BaseType *eval(Matrix &lhs, Complex &rhs);

    BaseType *eval(Matrix &lhs, List &rhs);

    BaseType *eval(Matrix &lhs, ComplexList &rhs);

    BaseType *eval(Matrix &lhs, String &rhs);

    BaseType *eval(Matrix &lhs, Matrix &rhs);
};


#endif
