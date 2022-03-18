#include "types.h"
#include "errors.h"
#include "globals.h"
#include "utils.h"

#include <cmath>
#include <cstring>

Number::Number(float num) {
    this->num = num;
}

char *Number::toString() const {
    return formatNum(this->num);
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

void Number::opFact() {
    // 0! = 1
    if (this->num == 0) {
        this->num = 1;
        return;
    }

    bool isNeg = this->num < 0;
    float absNum = fabsf(this->num);

    // The ! operator goes from -69.5 to 69.5. Everything outside that is overflow error
    if (absNum > 69.5) overflowError();

    float rem = fmodf(absNum, 1);
    if (rem != 0 && rem != 0.5) domainError();

    float result = absNum;
    while (absNum > 0) {
        absNum--;

        if (absNum == 0) break;
        if (absNum == -0.5) {
            result *= 1.772453850905516027298167483;
            break;
        }

        result *= absNum;
    }

    if (isNeg) result = -result;

    this->num = result;
}

void Number::opChs() {
    this->num = -this->num;
}


Complex::Complex(float real, float imag) {
    this->real = real;
    this->imag = imag;
}

char *Complex::toString() const {
    static char buf[25];

    char *numBuf = formatNum(this->real);
    strcpy(buf, numBuf);

    numBuf = formatNum(this->imag);

    // Eventually append a "+"
    char *p = numBuf;
    if (*numBuf != '-') {
        strcat(buf, "+");
    } else {
        p++;
    }

    // Only append if it's not a single "1"
    if (strlen(p) == 1 && *p == '1')
        *p = '\0';

    strcat(buf, numBuf);
    strcat(buf, "i");

    return buf;
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

    this->real *= realSqr - 3 * imagSqr;
    this->imag *= 3 * realSqr - imagSqr;
}

void Complex::opChs() {
    this->real = -this->real;
    this->imag = -this->imag;
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

void List::opFact() {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opFact();
    }
}

void List::opChs() {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opChs();
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

void ComplexList::opChs() {
    if (elements.empty()) dimensionError();

    for (auto cplx : elements) {
        cplx.opChs();
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

void Matrix::opChs() {
    if (elements.empty()) dimensionError();

    for (const auto& row : elements) {
        for (auto num : row) {
            num.opChs();
        }
    }
}
