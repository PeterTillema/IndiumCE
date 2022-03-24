#include "types.h"
#include "errors.h"
#include "utils.h"

#include <cstring>
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

TypeType Number::type() {
    return TypeType::NUMBER;
}

BaseType *Number::eval(UnaryOperator &op) {
    return op.eval(*this);
}

BaseType *Number::eval(BinaryOperator &op, BaseType *rhs) {
    return op.eval(*this, *rhs);
}

Complex::Complex(float real, float imag) {
    this->real = real;
    this->imag = imag;
}

TypeType Complex::type() {
    return TypeType::COMPLEX;
}

char *Complex::toString() const {
    static char buf[25];
    char *numBuf;

    if (real != 0) {
        numBuf = formatNum(real);
        strcpy(buf, numBuf);
    }

    if (imag != 0) {
        numBuf = formatNum(imag);

        // Eventually append a "+"
        if (real != 0) {
            if (*numBuf == 0x1A) {
                numBuf++;
                strcat(buf, "-");
            } else {
                strcat(buf, "+");
            }
        }

        // Only append if it's not a single "1"
        if (strlen(numBuf) != 1 || *numBuf != '1') strcat(buf, numBuf);

        strcat(buf, "\xD7");
    }

    return buf;
}

BaseType *Complex::eval(UnaryOperator &op) {
    return op.eval(*this);
}

BaseType *Complex::eval(BinaryOperator &op, BaseType *rhs) {
    return op.eval(*this, *rhs);
}

List::List(vector<Number> &elements) {
    this->elements = elements;
}

List::~List() {
    elements.clear();
}

TypeType List::type() {
    return TypeType::LIST;
}

char *List::toString() const {
    if (elements.empty()) dimensionError();

    static char buf[40] = "{";

    for (const auto &number: elements) {
        strcat(buf, number.toString());
        strcat(buf, " ");

        if (strlen(buf) > 26) break;
    }

    if (strlen(buf) > 25) {
        buf[25] = 0xCE;
    } else {
        // Overwrite space with closing bracket
        buf[strlen(buf) - 1] = '}';
    }

    return buf;
}

BaseType *List::eval(UnaryOperator &op) {
    return op.eval(*this);
}

BaseType *List::eval(BinaryOperator &op, BaseType *rhs) {
    return op.eval(*this, *rhs);
}

ComplexList::ComplexList(const vector<Complex> &elements) {
    this->elements = elements;
}

ComplexList::~ComplexList() {
    elements.clear();
}

TypeType ComplexList::type() {
    return TypeType::COMPLEX_LIST;
}

char *ComplexList::toString() const {
    if (elements.empty()) dimensionError();

    static char buf[50] = "{";

    for (const auto &number: elements) {
        strcat(buf, number.toString());
        strcat(buf, " ");

        if (strlen(buf) > 26) break;
    }

    if (strlen(buf) > 25) {
        buf[25] = 0xCE;
    } else {
        // Overwrite space with closing bracket
        buf[strlen(buf) - 1] = '}';
    }

    return buf;
}

BaseType *ComplexList::eval(UnaryOperator &op) {
    return op.eval(*this);
}

BaseType *ComplexList::eval(BinaryOperator &op, BaseType *rhs) {
    return op.eval(*this, *rhs);
}

String::String(unsigned int length, char *string) {
    this->length = length;
    this->string = string;
}

String::~String() {
    delete string;
}

char *String::toString() const {
    // todo
    typeError();
}

TypeType String::type() {
    return TypeType::STRING;
}

BaseType *String::eval(UnaryOperator &op) {
    return op.eval(*this);
}

BaseType *String::eval(BinaryOperator &op, BaseType *rhs) {
    return op.eval(*this, *rhs);
}

Matrix::Matrix(vector<vector<Number>> &elements) {
    this->elements = elements;
}

Matrix::~Matrix() {
    elements.clear();
}

char *Matrix::toString() const {
    // Matrix to string depends on the context: when using Disp, it is displayed as a real 2D matrix. When using
    // Output(, it is converted to a string without spaces. Don't convert it here, but at the place where it's
    // necessary.
    typeError();
}

TypeType Matrix::type() {
    return TypeType::MATRIX;
}

BaseType *Matrix::eval(UnaryOperator &op) {
    return op.eval(*this);
}

BaseType *Matrix::eval(BinaryOperator &op, BaseType *rhs) {
    return op.eval(*this, *rhs);
}

BaseType *UnaryOperator::eval(__attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *UnaryOperator::eval(__attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *UnaryOperator::eval(List &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &number : newElements) {
        number = dynamic_cast<Number &>(*this->eval(number));
    }

    return new List(newElements);
}

BaseType *UnaryOperator::eval(ComplexList &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(cplx));
    }

    return new ComplexList(newElements);
}

BaseType *UnaryOperator::eval(__attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *UnaryOperator::eval(__attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(Number &lhs, BaseType &rhs) {
    switch (rhs.type()) {
        case TypeType::NUMBER:
            return this->eval(lhs, dynamic_cast<Number &>(rhs));
        case TypeType::COMPLEX:
            return this->eval(lhs, dynamic_cast<Complex &>(rhs));
        case TypeType::LIST:
            return this->eval(lhs, dynamic_cast<List &>(rhs));
        case TypeType::COMPLEX_LIST:
            return this->eval(lhs, dynamic_cast<ComplexList &>(rhs));
        case TypeType::STRING:
            return this->eval(lhs, dynamic_cast<String &>(rhs));
        case TypeType::MATRIX:
            return this->eval(lhs, dynamic_cast<Matrix &>(rhs));
    }
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(Number &lhs, List &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &number : newElements) {
        number = dynamic_cast<Number &>(*this->eval(lhs, number));
    }

    return new List(newElements);
}

BaseType *BinaryOperator::eval(Number &lhs, ComplexList &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(lhs, cplx));
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(Number &lhs, Matrix &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &row : newElements) {
        for (auto &col : row) {
            col = dynamic_cast<Number &>(*this->eval(lhs, col));
        }
    }

    return new Matrix(newElements);
}

BaseType *BinaryOperator::eval(Complex &lhs, BaseType &rhs) {
    switch (rhs.type()) {
        case TypeType::NUMBER:
            return this->eval(lhs, dynamic_cast<Number &>(rhs));
        case TypeType::COMPLEX:
            return this->eval(lhs, dynamic_cast<Complex &>(rhs));
        case TypeType::LIST:
            return this->eval(lhs, dynamic_cast<List &>(rhs));
        case TypeType::COMPLEX_LIST:
            return this->eval(lhs, dynamic_cast<ComplexList &>(rhs));
        case TypeType::STRING:
            return this->eval(lhs, dynamic_cast<String &>(rhs));
        case TypeType::MATRIX:
            return this->eval(lhs, dynamic_cast<Matrix &>(rhs));
    }
}

BaseType *BinaryOperator::eval(__attribute__((unused))Complex &lhs, __attribute__((unused))Number &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(Complex &lhs, List &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = vector<Complex>(rhs.elements.size());

    unsigned int index = 0;
    for (auto &number : rhs.elements) {
        newElements[index++] = dynamic_cast<Complex &>(*this->eval(lhs, number));
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(Complex &lhs, ComplexList &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(lhs, cplx));
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(List &lhs, BaseType &rhs) {
    switch (rhs.type()) {
        case TypeType::NUMBER:
            return this->eval(lhs, dynamic_cast<Number &>(rhs));
        case TypeType::COMPLEX:
            return this->eval(lhs, dynamic_cast<Complex &>(rhs));
        case TypeType::LIST:
            return this->eval(lhs, dynamic_cast<List &>(rhs));
        case TypeType::COMPLEX_LIST:
            return this->eval(lhs, dynamic_cast<ComplexList &>(rhs));
        case TypeType::STRING:
            return this->eval(lhs, dynamic_cast<String &>(rhs));
        case TypeType::MATRIX:
            return this->eval(lhs, dynamic_cast<Matrix &>(rhs));
    }
}

BaseType *BinaryOperator::eval(List &lhs, Number &rhs) {
    if (lhs.elements.empty()) dimensionError();

    auto newElements = lhs.elements;

    for (auto &number : newElements) {
        number = dynamic_cast<Number &>(*this->eval(number, rhs));
    }

    return new List(newElements);
}

BaseType *BinaryOperator::eval(List &lhs, Complex &rhs) {
    if (lhs.elements.empty()) dimensionError();

    auto newElements = vector<Complex>(lhs.elements.size());

    unsigned int index = 0;
    for (auto &number : lhs.elements) {
        newElements[index++] = dynamic_cast<Complex &>(*this->eval(number, rhs));
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(List &lhs, List &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (lhs.elements.size() != rhs.elements.size()) dimensionMismatch();

    auto newElements = lhs.elements;

    unsigned int index = 0;
    for (auto &number : newElements) {
        number = dynamic_cast<Number &>(*this->eval(number, rhs.elements[index]));
        index++;
    }

    return new List(newElements);
}

BaseType *BinaryOperator::eval(List &lhs, ComplexList &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (lhs.elements.size() != rhs.elements.size()) dimensionMismatch();

    auto newElements = rhs.elements;

    unsigned int index = 0;
    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(lhs.elements[index], cplx));
        index++;
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(__attribute__((unused)) List &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) List &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(ComplexList &lhs, BaseType &rhs) {
    switch (rhs.type()) {
        case TypeType::NUMBER:
            return this->eval(lhs, dynamic_cast<Number &>(rhs));
        case TypeType::COMPLEX:
            return this->eval(lhs, dynamic_cast<Complex &>(rhs));
        case TypeType::LIST:
            return this->eval(lhs, dynamic_cast<List &>(rhs));
        case TypeType::COMPLEX_LIST:
            return this->eval(lhs, dynamic_cast<ComplexList &>(rhs));
        case TypeType::STRING:
            return this->eval(lhs, dynamic_cast<String &>(rhs));
        case TypeType::MATRIX:
            return this->eval(lhs, dynamic_cast<Matrix &>(rhs));
    }
}

BaseType *BinaryOperator::eval(ComplexList &lhs, Number &rhs) {
    if (lhs.elements.empty()) dimensionError();

    auto newElements = lhs.elements;

    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(cplx, rhs));
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(ComplexList &lhs, Complex &rhs) {
    if (lhs.elements.empty()) dimensionError();

    auto newElements = lhs.elements;

    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(cplx, rhs));
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(ComplexList &lhs, List &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (lhs.elements.size() != rhs.elements.size()) dimensionMismatch();

    auto newElements = lhs.elements;

    unsigned int index = 0;
    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(cplx, rhs.elements[index]));
        index++;
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(ComplexList &lhs, ComplexList &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (lhs.elements.size() != rhs.elements.size()) dimensionMismatch();

    auto newElements = lhs.elements;

    unsigned int index = 0;
    for (auto &cplx : newElements) {
        cplx = dynamic_cast<Complex &>(*this->eval(cplx, rhs.elements[index]));
        index++;
    }

    return new ComplexList(newElements);
}

BaseType *BinaryOperator::eval(__attribute__((unused)) ComplexList &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) ComplexList &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(String &lhs, BaseType &rhs) {
    switch (rhs.type()) {
        case TypeType::NUMBER:
            return this->eval(lhs, dynamic_cast<Number &>(rhs));
        case TypeType::COMPLEX:
            return this->eval(lhs, dynamic_cast<Complex &>(rhs));
        case TypeType::LIST:
            return this->eval(lhs, dynamic_cast<List &>(rhs));
        case TypeType::COMPLEX_LIST:
            return this->eval(lhs, dynamic_cast<ComplexList &>(rhs));
        case TypeType::STRING:
            return this->eval(lhs, dynamic_cast<String &>(rhs));
        case TypeType::MATRIX:
            return this->eval(lhs, dynamic_cast<Matrix &>(rhs));
    }
}

BaseType *BinaryOperator::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) List &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) ComplexList &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(Matrix &lhs, BaseType &rhs) {
    switch (rhs.type()) {
        case TypeType::NUMBER:
            return this->eval(lhs, dynamic_cast<Number &>(rhs));
        case TypeType::COMPLEX:
            return this->eval(lhs, dynamic_cast<Complex &>(rhs));
        case TypeType::LIST:
            return this->eval(lhs, dynamic_cast<List &>(rhs));
        case TypeType::COMPLEX_LIST:
            return this->eval(lhs, dynamic_cast<ComplexList &>(rhs));
        case TypeType::STRING:
            return this->eval(lhs, dynamic_cast<String &>(rhs));
        case TypeType::MATRIX:
            return this->eval(lhs, dynamic_cast<Matrix &>(rhs));
    }
}
BaseType *BinaryOperator::eval(Matrix &lhs, Number &rhs) {
    if (lhs.elements.empty()) dimensionError();

    auto newElements = lhs.elements;

    for (auto &row : newElements) {
        for (auto &col : row) {
            col = dynamic_cast<Number &>(*this->eval(col, rhs));
        }
    }

    return new Matrix(newElements);
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) List &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) ComplexList &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *BinaryOperator::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Matrix &rhs) {
    // Binary operator with 2 matrices is only the same for + and -, all the other operators are either type error
    // or a different routine (like * and /).
    typeError();
}
