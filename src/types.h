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

    void opFromRad();

    void opFromDeg();

    void opRecip();

    void opSqr();

    void opCube();
};


class ComplexList {
public:
    vector<Complex> elements;

    explicit ComplexList(const tinystl::vector<Complex> &elements);

    ~ComplexList();

    void opRecip();

    void opSqr();

    void opCube();
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
};


#endif
