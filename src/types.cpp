#include "types.h"
#include "errors.h"
#include "utils.h"

#include <TINYSTL/vector.h>

using tinystl::vector;

char *BaseType::toString() const {
    typeError();
}

char *Number::toString() const {
    return formatNum(num);
}

Number::Number(float num) {
    this->num = num;
}

BaseType *Number::eval(Operator &op) {
    return op.eval(*this);
}

Complex::Complex(float real, float imag) {
    this->real = real;
    this->imag = imag;
}

BaseType *Complex::eval(Operator &op) {
    return op.eval(*this);
}

List::List(vector<Number *> &elements) {
    this->elements = elements;
}

BaseType *List::eval(Operator &op) {
    return op.eval(*this);
}

List::~List() {
    elements.clear();
}

ComplexList::ComplexList(const vector<Complex *> &elements) {
    this->elements = elements;
}

BaseType *ComplexList::eval(Operator &op) {
    return op.eval(*this);
}

ComplexList::~ComplexList() {
    elements.clear();
}

String::String(unsigned int length, char *string) {
    this->length = length;
    this->string = string;
}

BaseType *String::eval(Operator &op) {
    return op.eval(*this);
}

String::~String() {
    delete string;
}

Matrix::Matrix(vector<vector<Number *>> &elements) {
    this->elements = elements;
}

BaseType *Matrix::eval(Operator &op) {
    return op.eval(*this);
}

Matrix::~Matrix() {
    elements.clear();
}

BaseType *Operator::eval(__attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *Operator::eval(__attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *Operator::eval(List &rhs) {
    auto newElements = vector<Number *>(rhs.elements.size());

    unsigned int index = 0;
    for (auto &number : rhs.elements) {
        newElements[index++] = dynamic_cast<Number *>(this->eval(*number));
    }

    return new List(newElements);
}

BaseType *Operator::eval(ComplexList &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = vector<Complex *>(rhs.elements.size());

    unsigned int index = 0;
    for (auto &cplx : rhs.elements) {
        newElements[index++] = dynamic_cast<Complex *>(this->eval(*cplx));
    }

    return new ComplexList(newElements);
}

BaseType *Operator::eval(__attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *Operator::eval(__attribute__((unused)) Matrix &rhs) {
    auto newElements = rhs.elements;

    unsigned int rowIndex = 0;
    for (auto &row : rhs.elements) {
        unsigned int colIndex = 0;

        for (auto &col : row) {
            newElements[rowIndex][colIndex] = dynamic_cast<Number *>(this->eval(*col));
            colIndex++;
        }
        rowIndex++;
    }

    return new Matrix(newElements);
}
