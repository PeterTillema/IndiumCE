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

void Number::opPower(Number *rhs) const {
    rhs->num = powf(this->num, rhs->num);
}

void Number::opPower(Complex *rhs) const {
    // a^(b + ci) = a^b(cos(c*ln(a)) + isin(c*ln(a)))
    float clna = rhs->imag * logf(this->num);
    float apowb = powf(this->num, rhs->real);

    rhs->real = apowb * cosf(clna);
    rhs->imag = apowb * sinf(clna);
}

void Number::opPower(List *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    for (auto number : rhs->elements) {
        number.num = powf(this->num, number.num);
    }
}

void Number::opPower(ComplexList *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    for (auto number : rhs->elements) {
        float clna = number.imag * logf(this->num);
        float apowb = powf(this->num, number.imag);

        number.real = apowb * cosf(clna);
        number.imag = apowb * sinf(clna);
    }
}


Complex::Complex(float real, float imag) {
    this->real = real;
    this->imag = imag;
}

char *Complex::toString() const {
    static char buf[25];
    char *numBuf;

    if (this->real != 0) {
        numBuf = formatNum(this->real);
        strcpy(buf, numBuf);
    }

    if (this->imag != 0) {
        numBuf = formatNum(this->imag);

        // Eventually append a "+"
        if (this->real != 0) {
            if (*numBuf == 0x0B) {
                numBuf++;
                strcat(buf, "-");
            } else {
                strcat(buf, "+");
            }
        }

        // Only append if it's not a single "1"
        if (strlen(numBuf) != 1 || *numBuf != '1') strcat(buf, numBuf);

        strcat(buf, "\x80");
    }

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

void Complex::opPower(Number *rhs) {
    // a + bi = r * (cos(theta) + isin(theta)), r=sqrt(a² + b²), tan(theta) = b / a
    // (a + bi) ^ N = r ^ N * (cos(Ntheta) + isin(Ntheta))
    float r = sqrtf(this->real * this->real + this->imag * this->imag);

    float theta;
    if (this->real == 0) {
        theta = M_PI_2;
    } else {
        theta = atanf(this->imag / this->real);
    }

    float Ntheta = rhs->num * theta;
    float rpowN = powf(r, rhs->num);

    this->real = rpowN * cosf(Ntheta);
    this->imag = rpowN * sinf(Ntheta);
}

void Complex::opPower(Complex *rhs) {
    // (a + bi) ^ (c + di) =
    //      r = a² + b²
    //      tan(theta) = b / a
    //      inner = c * theta + 0.5 * d * ln(r)
    // r ^ (c / 2) * exp(-d * theta) * (cos(inner) + isin(inner)
    float r = this->real * this->real + this->imag * this->imag;

    float theta;
    if (this->real == 0) {
        theta = M_PI_2;
    } else {
        theta = atanf(this->imag / this->real);
    }

    float inner = rhs->real * theta + rhs->imag * logf(r) / 2;
    float multiply = powf(r, rhs->real / 2) * expf(-rhs->imag * theta);

    this->real = multiply * cosf(inner);
    this->imag = multiply * sinf(inner);
}

ComplexList *Complex::opPower(List *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    auto complexList = tinystl::vector<Complex>();

    float r = sqrtf(this->real * this->real + this->imag * this->imag);

    float theta;
    if (this->real == 0) {
        theta = M_PI_2;
    } else {
        theta = atanf(this->imag / this->real);
    }

    for (auto number : rhs->elements) {
        float Ntheta = number.num * theta;
        float rpowN = powf(r, number.num);

        complexList.push_back(Complex(rpowN * cosf(Ntheta), rpowN * sinf(Ntheta)));
    }

    return new ComplexList(complexList);
}

void Complex::opPower(ComplexList *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    float r = this->real * this->real + this->imag * this->imag;

    float theta;
    if (this->real == 0) {
        theta = M_PI_2;
    } else {
        theta = atanf(this->imag / this->real);
    }

    for (auto cplx : rhs->elements) {
        float inner = cplx.real * theta + cplx.imag * logf(r) / 2;
        float multiply = powf(r, cplx.real / 2) * expf(-cplx.imag * theta);

        cplx.real = multiply * cosf(inner);
        cplx.imag = multiply * sinf(inner);
    }
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

char *List::toString() const {
    if (elements.empty()) dimensionError();

    static char buf[40] = "{";

    for (auto number : elements) {
        strcat(buf, number.toString());
        strcat(buf, " ");

        if (strlen(buf) > 26) break;
    }

    if (strlen(buf) > 25) {
        buf[24] = ' ';
        buf[25] = 0x0F;
    } else {
        // Overwrite space with closing bracket
        buf[strlen(buf) - 1] = '}';
    }

    return buf;
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

void List::opPower(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opPower(rhs);
    }
}

void List::opPower(Complex *rhs) {
    if (elements.empty()) dimensionError();

    for (auto num : elements) {
        num.opPower(rhs);
    }
}

void List::opPower(List *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto num : elements) {
        num.opPower(&rhs->elements[index]);
        index++;
    }
}

void List::opPower(ComplexList *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    // todo
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

char *ComplexList::toString() const {
    if (elements.empty()) dimensionError();

    static char buf[50] = "{";

    for (auto number : elements) {
        strcat(buf, number.toString());
        strcat(buf, " ");

        if (strlen(buf) > 26) break;
    }

    if (strlen(buf) > 25) {
        buf[24] = ' ';
        buf[25] = 0x0F;
    } else {
        // Overwrite space with closing bracket
        buf[strlen(buf) - 1] = '}';
    }

    return buf;
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
