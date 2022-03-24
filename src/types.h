#ifndef TYPES_H
#define TYPES_H

#include "errors.h"
#include "TINYSTL/vector.h"

#include <cstdint>

using tinystl::vector;

class UnaryOperator;

class BinaryOperator;

class UnaryFunction;

enum class TypeType {
    NUMBER, COMPLEX, LIST, COMPLEX_LIST, STRING, MATRIX
};

class BaseType {
public:
    virtual ~BaseType() = default;

    virtual char *toString() const;

    virtual BaseType *eval(UnaryOperator &op) = 0;

    virtual BaseType *eval(BinaryOperator &op, BaseType *rhs) = 0;

    virtual BaseType *eval(UnaryFunction &func) = 0;

    virtual TypeType type() = 0;
};

class Number : public BaseType {
public:
    float num = 0;

    Number() = default;

    explicit Number(float num);

    ~Number() override = default;

    TypeType type() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;

    BaseType *eval(UnaryFunction &func) override;
};

class Complex : public BaseType {
public:
    float real = 0;
    float imag = 0;

    Complex() = default;

    Complex(float real, float imag);

    ~Complex() override = default;

    TypeType type() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;

    BaseType *eval(UnaryFunction &func) override;
};

class List : public BaseType {
public:
    vector<Number> elements;

    explicit List(vector<Number> &elements);

    ~List() override;

    TypeType type() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;

    BaseType *eval(UnaryFunction &func) override;
};

class ComplexList : public BaseType {
public:
    vector<Complex> elements;

    explicit ComplexList(const vector<Complex> &elements);

    ~ComplexList() override;

    TypeType type() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;

    BaseType *eval(UnaryFunction &func) override;
};

class String : public BaseType {
public:
    unsigned int length;
    char *string;

    explicit String(unsigned int length, char *string);

    ~String() override;

    TypeType type() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;

    BaseType *eval(UnaryFunction &func) override;
};

class Matrix : public BaseType {
public:
    vector<vector<Number>> elements;

    explicit Matrix(vector<vector<Number>> &elements);

    ~Matrix() override;

    TypeType type() override;

    char *toString() const override;

    BaseType *eval(UnaryOperator &op) override;

    BaseType *eval(BinaryOperator &op, BaseType *rhs) override;

    BaseType *eval(UnaryFunction &func) override;
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

    virtual BaseType *eval(Number &lhs, BaseType &rhs);

    virtual BaseType *eval(Number &lhs, Number &rhs);

    virtual BaseType *eval(Number &lhs, Complex &rhs);

    virtual BaseType *eval(Number &lhs, List &rhs);

    virtual BaseType *eval(Number &lhs, ComplexList &rhs);

    virtual BaseType *eval(Number &lhs, String &rhs);

    virtual BaseType *eval(Number &lhs, Matrix &rhs);

    virtual BaseType *eval(Complex &lhs, BaseType &rhs);

    virtual BaseType *eval(Complex &lhs, Number &rhs);

    virtual BaseType *eval(Complex &lhs, Complex &rhs);

    virtual BaseType *eval(Complex &lhs, List &rhs);

    virtual BaseType *eval(Complex &lhs, ComplexList &rhs);

    virtual BaseType *eval(Complex &lhs, String &rhs);

    virtual BaseType *eval(Complex &lhs, Matrix &rhs);

    virtual BaseType *eval(List &lhs, BaseType &rhs);

    virtual BaseType *eval(List &lhs, Number &rhs);

    virtual BaseType *eval(List &lhs, Complex &rhs);

    virtual BaseType *eval(List &lhs, List &rhs);

    virtual BaseType *eval(List &lhs, ComplexList &rhs);

    virtual BaseType *eval(List &lhs, String &rhs);

    virtual BaseType *eval(List &lhs, Matrix &rhs);

    virtual BaseType *eval(ComplexList &lhs, BaseType &rhs);

    virtual BaseType *eval(ComplexList &lhs, Number &rhs);

    virtual BaseType *eval(ComplexList &lhs, Complex &rhs);

    virtual BaseType *eval(ComplexList &lhs, List &rhs);

    virtual BaseType *eval(ComplexList &lhs, ComplexList &rhs);

    virtual BaseType *eval(ComplexList &lhs, String &rhs);

    virtual BaseType *eval(ComplexList &lhs, Matrix &rhs);

    virtual BaseType *eval(String &lhs, BaseType &rhs);

    virtual BaseType *eval(String &lhs, Number &rhs);

    virtual BaseType *eval(String &lhs, Complex &rhs);

    virtual BaseType *eval(String &lhs, List &rhs);

    virtual BaseType *eval(String &lhs, ComplexList &rhs);

    virtual BaseType *eval(String &lhs, String &rhs);

    virtual BaseType *eval(String &lhs, Matrix &rhs);

    virtual BaseType *eval(Matrix &lhs, BaseType &rhs);

    virtual BaseType *eval(Matrix &lhs, Number &rhs);

    virtual BaseType *eval(Matrix &lhs, Complex &rhs);

    virtual BaseType *eval(Matrix &lhs, List &rhs);

    virtual BaseType *eval(Matrix &lhs, ComplexList &rhs);

    virtual BaseType *eval(Matrix &lhs, String &rhs);

    virtual BaseType *eval(Matrix &lhs, Matrix &rhs);
};


#endif
