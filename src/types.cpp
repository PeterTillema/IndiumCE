#include "types.h"
#include "errors.h"
#include "globals.h"

#include <cmath>

Number::Number(float num) {
    this->num = num;
}

void Number::opFromRad() {
    if (!globals.inRadianMode) {
        this->num *= 180 / M_PI;
    }
}

void Number::opFromDeg() {
    if (globals.inRadianMode) {
        this->num *= M_PI / 180;
    }
}

void Number::opRecip() {
    if (this->num == 0) divideBy0Error();

    this->num = 1 / this->num;
}

void Number::opSqr() {
    this->num *= this->num;
}

void Number::opCube() {
    this->num *= this->num * this->num;
}


Complex::Complex(float real, float imag) {
    this->real = real;
    this->imag = imag;
}

void Complex::opRecip() {
    // 1 / (a + bi) = (a - bi) / (a² + b²)
    float denom = this->real * this->real + this->imag + this->imag;

    if (denom == 0) divideBy0Error();
    this->real /= denom;
    this->imag /= -denom;
}

void Complex::opSqr() {
    // (a + bi)² = a² - b² + 2abi
    float tmp = this->real;

    this->real = this->real * this->real - this->imag * this->imag;
    this->imag *= 2 * tmp;
}

void Complex::opCube() {
    // (a + bi)³ = a³ - 3ab² + (3a²b - b³)i
    float realSqr = this->real * this->real;
    float imagSqr = this->imag * this->imag;

    this->real = realSqr * this->real - 3 * this->real * imagSqr;
    this->imag *= 3 * realSqr - imagSqr;
}


String::String(unsigned int length, char *string) {
    this->length = length;
    this->string = string;
}

String::~String() {
    delete string;
}


List::List(const tinystl::vector<Number> &elements) {
    this->elements = elements;
}

List::~List() {
    elements.clear();
}

void List::opFromRad() {
    if (elements.empty()) dimensionError();

    if (!globals.inRadianMode) {
        for (auto num : elements) {
            num.num *= 180 / M_PI;
        }
    }
}

void List::opFromDeg() {
    if (elements.empty()) dimensionError();

    if (globals.inRadianMode) {
        for (auto num : elements) {
            num.num *= M_PI / 180;
        }
    }
}

void List::opRecip() {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opRecip();
    }
}

void List::opSqr() {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opSqr();
    }
}

void List::opCube() {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opCube();
    }
}


ComplexList::ComplexList(const tinystl::vector<Complex> &elements) {
    this->elements = elements;
}

ComplexList::~ComplexList() {
    elements.clear();
}

void ComplexList::opRecip() {
    if (elements.empty()) dimensionError();

    for (auto cplx : elements) {
        cplx.opRecip();
    }
}

void ComplexList::opSqr() {
    if (elements.empty()) dimensionError();

    for (auto cplx : elements) {
        cplx.opSqr();
    }
}

void ComplexList::opCube() {
    if (elements.empty()) dimensionError();

    for (auto cplx : elements) {
        cplx.opCube();
    }
}


Matrix::Matrix(const tinystl::vector<tinystl::vector<Number>> &elements) {
    this->elements = elements;
}

Matrix::~Matrix() {
    elements.clear();
}

void Matrix::opRecip() {
    // todo: matrix inversion
}

void Matrix::opSqr() {
    // todo: matrix square
}

void Matrix::opTrnspos() {
    // todo: matrix transpose
}

void Matrix::opCube() {
    // todo: matrix cube
}
