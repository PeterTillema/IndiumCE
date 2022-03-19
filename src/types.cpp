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
            result *= 1.772453850905516;
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

    for (auto number: rhs->elements) {
        number.num = powf(this->num, number.num);
    }
}

void Number::opPower(ComplexList *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    for (auto &number: rhs->elements) {
        float clna = number.imag * logf(this->num);
        float apowb = powf(this->num, number.imag);

        number.real = apowb * cosf(clna);
        number.imag = apowb * sinf(clna);
    }
}

void Number::opMul(Number *rhs) const {
    rhs->num *= this->num;
}

void Number::opMul(Complex *rhs) const {
    rhs->real *= this->num;
    rhs->imag *= this->num;
}

void Number::opMul(List *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    for (auto &number: rhs->elements) {
        number.num *= this->num;
    }
}

void Number::opMul(ComplexList *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    for (auto &number: rhs->elements) {
        number.real *= this->num;
        number.imag *= this->num;
    }
}

void Number::opMul(Matrix *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    for (const auto &row: rhs->elements) {
        for (auto number: row) {
            number.num *= this->num;
        }
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

    for (auto number: rhs->elements) {
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

    for (auto cplx: rhs->elements) {
        float inner = cplx.real * theta + cplx.imag * logf(r) / 2;
        float multiply = powf(r, cplx.real / 2) * expf(-cplx.imag * theta);

        cplx.real = multiply * cosf(inner);
        cplx.imag = multiply * sinf(inner);
    }
}

void Complex::opMul(Number *rhs) {
    this->real *= rhs->num;
    this->imag *= rhs->num;
}

void Complex::opMul(Complex *rhs) {
    // (a + bi) * (c + di) = (ac - bd) + (ad + bd)i
    float tmp = this->real;
    this->real = this->real * rhs->real - this->imag * rhs->imag;
    this->imag = tmp * rhs->imag + this->imag * rhs->real;
}

ComplexList *Complex::opMul(List *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    auto complexList = tinystl::vector<Complex>();

    for (auto &number: rhs->elements) {
        complexList.push_back(Complex(this->real * number.num, this->imag * number.num));
    }

    return new ComplexList(complexList);
}

void Complex::opMul(ComplexList *rhs) const {
    if (rhs->elements.empty()) dimensionError();

    float tmp = this->real;

    for (auto &cplx: rhs->elements) {
        cplx.real = tmp * cplx.real - this->imag * cplx.imag;
        cplx.imag = tmp * cplx.imag + this->imag * cplx.real;
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

    for (auto number: elements) {
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
        for (auto &num: elements) {
            num.num *= 180 / M_PI;
        }
    }
}

void List::opFromDeg() {
    if (elements.empty()) dimensionError();

    if (globals.inRadianMode) {
        for (auto &num: elements) {
            num.num *= M_PI / 180;
        }
    }
}

void List::opRecip() {
    if (elements.empty()) dimensionError();

    for (auto &num: elements) {
        num.opRecip();
    }
}

void List::opSqr() {
    if (elements.empty()) dimensionError();

    for (auto &num: elements) {
        num.opSqr();
    }
}

void List::opCube() {
    if (elements.empty()) dimensionError();

    for (auto &num: elements) {
        num.opCube();
    }
}

void List::opFact() {
    if (elements.empty()) dimensionError();

    for (auto &num: elements) {
        num.opFact();
    }
}

void List::opChs() {
    if (elements.empty()) dimensionError();

    for (auto &num: elements) {
        num.opChs();
    }
}

void List::opPower(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (auto &num: elements) {
        num.num = powf(num.num, rhs->num);
    }
}

ComplexList *List::opPower(Complex *rhs) {
    if (elements.empty()) dimensionError();

    auto complexList = tinystl::vector<Complex>();

    for (auto &number: elements) {
        Complex rhs2 = Complex(rhs->real, rhs->imag);

        number.opPower(&rhs2);
        complexList.push_back(rhs2);
    }

    return new ComplexList(complexList);
}

void List::opPower(List *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &num: elements) {
        num.opPower(&rhs->elements[index]);
        index++;
    }
}

void List::opPower(ComplexList *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &num: elements) {
        num.opPower(&rhs->elements[index]);
        index++;
    }
}

void List::opMul(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (auto &num : elements) {
        num.num *= rhs->num;
    }
}

ComplexList *List::opMul(Complex *rhs) const {
    if (elements.empty()) dimensionError();

    auto complexList = tinystl::vector<Complex>();

    for (auto &num: elements) {
        complexList.push_back(Complex(num.num * rhs->real, num.num * rhs->imag));
    }

    return new ComplexList(complexList);
}

void List::opMul(List *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &num: elements) {
        num.num *= rhs->elements[index].num;
        index++;
    }
}

void List::opMul(ComplexList *rhs) const {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &cplx: rhs->elements) {
        cplx.real *= elements[index].num;
        cplx.imag *= elements[index].num;
        index++;
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

    for (auto &cplx: elements) {
        cplx.opRecip();
    }
}

void ComplexList::opSqr() {
    if (elements.empty()) dimensionError();

    for (auto &cplx: elements) {
        cplx.opSqr();
    }
}

void ComplexList::opCube() {
    if (elements.empty()) dimensionError();

    for (auto &cplx: elements) {
        cplx.opCube();
    }
}

void ComplexList::opChs() {
    if (elements.empty()) dimensionError();

    for (auto &cplx: elements) {
        cplx.opChs();
    }
}

char *ComplexList::toString() const {
    if (elements.empty()) dimensionError();

    static char buf[50] = "{";

    for (auto number: elements) {
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

void ComplexList::opPower(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (auto &cplx: elements) {
        cplx.opPower(rhs);
    }
}

void ComplexList::opPower(Complex *rhs) {
    if (elements.empty()) dimensionError();

    for (auto &cplx: elements) {
        cplx.opPower(rhs);
    }
}

void ComplexList::opPower(List *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &num: elements) {
        num.opPower(&rhs->elements[index]);
        index++;
    }
}

void ComplexList::opPower(ComplexList *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &num: elements) {
        num.opPower(&rhs->elements[index]);
        index++;
    }
}

void ComplexList::opMul(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (auto &cplx : elements) {
        cplx.opMul(rhs);
    }
}

void ComplexList::opMul(Complex *rhs) {
    if (elements.empty()) dimensionError();

    for (auto &cplx : elements) {
        cplx.opMul(rhs);
    }
}

void ComplexList::opMul(List *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &cplx: elements) {
        cplx.real *= rhs->elements[index].num;
        cplx.imag *= rhs->elements[index].num;
        index++;
    }
}

void ComplexList::opMul(ComplexList *rhs) {
    if (elements.empty()) dimensionError();
    if (elements.size() != rhs->elements.size()) dimensionMismatch();

    unsigned int index = 0;
    for (auto &cplx : elements) {
        cplx.opMul(&rhs->elements[index]);
        index++;
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

    for (const auto &row: elements) {
        for (auto num: row) {
            num.opChs();
        }
    }
}

void Matrix::opPower(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (const auto &row: elements) {
        for (auto num: row) {
            num.num = powf(num.num, rhs->num);
        }
    }
}

void Matrix::opMul(Number *rhs) {
    if (elements.empty()) dimensionError();

    for (const auto &row: elements) {
        for (auto num: row) {
            num.num *= rhs->num;
        }
    }
}

void Matrix::opMul(Matrix *rhs) {
    // todo: matrix multiplication
}
