#include "operators.h"
#include "ast.h"
#include "errors.h"
#include "evaluate.h"
#include "globals.h"
#include "utils.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <tice.h>

// https://education.ti.com/html/webhelp/EG_TI84PlusCE/EN/content/eg_gsguide/m_expressions/exp_order_of_operations.HTML
// Commas are treated as a special operator, which means that if the token is a comma, all other operators will be
// moved properly to the output. It is also "right-associative", which means that if the top stack entry is also a
// comma, it won't be moved (which is clearly not what we want)
static const uint8_t operators[] = {
        tFromRad, tFromDeg, tRecip, tSqr, tTrnspos, tCube,
        tPower,
        tFact,
        tChs,
        tnPr, tnCr,
        tMul, tDiv,
        tAdd, tSub,
        tEQ, tLT, tGT, tLE, tGE, tNE,
        tAnd,
        tOr, tXor,
        tStore,
        tComma
};
static const uint8_t precedence[] = {
        1, 1, 1, 1, 1, 1,
        2,
        3,
        4,
        5, 5,
        6, 6,
        7, 7,
        8, 8, 8, 8, 8, 8,
        9,
        10, 10,
        11,
        255
};

uint8_t getOpPrecedence(uint8_t op) {
    void *index = memchr(operators, op, sizeof(operators));

    return precedence[(const uint8_t *) index - operators];
}

bool isUnaryOp(uint8_t prec) {
    return prec <= 4 && prec != 2;
}

BaseType *evalOperator(struct NODE *node) {
    uint8_t op = node->data.operand.op;
    BaseType *leftNode = evalNode(node->child);
    BaseType *result;

    if (isUnaryOp(getOpPrecedence(op))) {
        UnaryOperator *opNew;

        switch (op) {
            case tFromRad:
                opNew = new OpFromRad();
                break;
            case tFromDeg:
                opNew = new OpFromDeg();
                break;
            case tRecip:
                opNew = new OpRecip();
                break;
            case tSqr:
                opNew = new OpSqr();
                break;
            case tTrnspos:
                opNew = new OpTrnspos();
                break;
            case tCube:
                opNew = new OpCube();
                break;
            case tFact:
                opNew = new OpFact();
                break;
            case tChs:
                opNew = new OpChs();
                break;
            default:
                typeError();
        }

        result = leftNode->eval(*opNew);

        delete opNew;
    } else {
        BaseType *rightNode;
        BinaryOperator *opNew;

        if (op == tStore) {
            typeError();
        } else {
            rightNode = evalNode(node->child->next);

            switch (op) {
                case tPower:
                    opNew = new OpPower();
                    break;
                case tMul:
                    opNew = new OpMul();
                    break;
                case tDiv:
                    opNew = new OpDiv();
                    break;
                case tAdd:
                    opNew = new OpAdd();
                    break;
                case tSub:
                    opNew = new OpSub();
                    break;
                case tEQ:
                    opNew = new OpEQ();
                    break;
                case tLT:
                    opNew = new OpLT();
                    break;
                case tGT:
                    opNew = new OpGT();
                    break;
                case tLE:
                    opNew = new OpLE();
                    break;
                case tGE:
                    opNew = new OpGE();
                    break;
                case tNE:
                    opNew = new OpNE();
                    break;
                case tAnd:
                    opNew = new OpAnd();
                    break;
                case tOr:
                    opNew = new OpOr();
                    break;
                case tXor:
                    opNew = new OpXor();
                    break;
                default:
                    typeError();
            }

            result = leftNode->eval(*opNew, rightNode);

            delete opNew;
            delete rightNode;
        }
    }

    delete leftNode;

    free(node);

    return result;
}

BaseType *OpFromRad::eval(Number &rhs) {
    if (globals.inRadianMode) {
        return new Number(rhs.num);
    }

    return new Number(rhs.num * 180 / M_PI);
}

BaseType *OpFromDeg::eval(Number &rhs) {
    if (globals.inRadianMode) {
        return new Number(rhs.num * M_PI / 180);
    }

    return new Number(rhs.num);
}

BaseType *OpRecip::eval(Number &rhs) {
    if (rhs.num == 0) divideBy0Error();

    return new Number(1 / rhs.num);
}

BaseType *OpRecip::eval(Complex &rhs) {
    // 1 / (a + bi) = (a - bi) / (a² + b²)
    float denom = rhs.real * rhs.real + rhs.imag * rhs.imag;

    if (denom == 0) divideBy0Error();

    return new Complex(rhs.real / denom, rhs.imag / -denom);
}

BaseType *OpRecip::eval(__attribute__((unused)) Matrix &rhs) {
    // todo: matrix inversion
    typeError();
}

BaseType *OpSqr::eval(Number &rhs) {
    return new Number(rhs.num * rhs.num);
}

BaseType *OpSqr::eval(Complex &rhs) {
    // (a + bi)² = a² - b² + 2abi
    return new Complex(rhs.real * rhs.real - rhs.imag * rhs.imag, rhs.real * rhs.imag * 2);
}

BaseType *OpSqr::eval(Matrix &rhs) {
    return multiplyMatrices(rhs, rhs);
}

BaseType *OpTrnspos::eval(Matrix &rhs) {
    if (rhs.elements.empty()) dimensionError();
    if (rhs.elements[0].empty()) dimensionError();

    vector<vector<Number>> newElements(rhs.elements[0].size(), vector<Number>(rhs.elements.size()));

    for (unsigned int row = 0; row < rhs.elements.size(); row++) {
        for (unsigned int col = 0; col < rhs.elements[0].size(); col++) {
            newElements[col][row] = rhs.elements[row][col];
        }
    }

    return new Matrix(newElements);
}

BaseType *OpCube::eval(Number &rhs) {
    return new Number(rhs.num * rhs.num * rhs.num);
}

BaseType *OpCube::eval(Complex &rhs) {
    // (a + bi)³ = a³ - 3ab² + (3a²b - b³)i
    float realSqr = rhs.real * rhs.real;
    float imagSqr = rhs.imag * rhs.imag;

    return new Complex(rhs.real * (realSqr - 3 * imagSqr), rhs.imag * (3 * realSqr - imagSqr));
}

BaseType *OpCube::eval(Matrix &rhs) {
    auto matrixSquare = multiplyMatrices(rhs, rhs);
    auto matrixCube = multiplyMatrices(*matrixSquare, rhs);

    delete matrixSquare;

    return matrixCube;
}

BaseType *OpPower::eval(Number &lhs, Number &rhs) {
    return new Number(powf(lhs.num, rhs.num));
}

BaseType *OpPower::eval(Number &lhs, Complex &rhs) {
    // a^(b + ci) = a^b(cos(c*ln(a)) + isin(c*ln(a)))
    float clna = rhs.imag * logf(lhs.num);
    float apowb = powf(lhs.num, rhs.real);

    return new Complex(apowb * cosfMode(clna), apowb * sinfMode(clna));
}

BaseType *OpPower::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpPower::eval(Complex &lhs, Number &rhs) {
    // a + bi = r * (cos(theta) + isin(theta)), r=sqrt(a² + b²), tan(theta) = b / a
    // (a + bi) ^ N = r ^ N * (cos(Ntheta) + isin(Ntheta))
    float r = sqrtf(lhs.real * lhs.real + lhs.imag * lhs.imag);

    float theta;
    if (lhs.real == 0) {
        theta = M_PI_2;
    } else {
        theta = atanfMode(lhs.imag / lhs.real);
    }

    float Ntheta = rhs.num * theta;
    float rpowN = powf(r, rhs.num);

    return new Complex(rpowN * cosfMode(Ntheta), rpowN * sinfMode(Ntheta));
}

BaseType *OpPower::eval(Complex &lhs, Complex &rhs) {
    // (a + bi) ^ (c + di) =
    //      r = sqrt(a² + b²)
    //      tan(theta) = b / a
    // exp((cln(r)-dtheta)+i(dln(r)+ctheta))
    float lnr = logf(sqrtf(lhs.real * lhs.real + lhs.imag * lhs.imag));

    float theta;
    if (lhs.real == 0) {
        theta = M_PI_2;
    } else {
        theta = atanfMode(lhs.imag / lhs.real);
    }

    float inner = rhs.imag * lnr + rhs.real * theta;
    float multiply = expf(rhs.real * lnr - rhs.imag * theta);

    return new Complex(multiply * cosfMode(inner), multiply * sinfMode(inner));
}

BaseType *OpPower::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Number &rhs) {
    // todo: matrix ^ N
    typeError();
}

BaseType *OpFact::eval(Number &rhs) {
    float num = rhs.num;

    // 0! = 1
    if (num == 0) {
        return new Number(1);
    } else if (num == -0.5) {
        return new Number(1.772453850905516);
    }
    if (num < 0) domainError();

    // The ! operator goes from -0.5 to 69.5. Everything outside that is overflow error
    if (num > 69.5) overflowError();

    float rem = fmodf(num, 1);
    if (rem != 0 && rem != 0.5) domainError();

    float result = num;
    while (num > 0) {
        num--;

        if (num == 0) break;
        if (num == -0.5) {
            // Multiply by sqrt(pi)
            result *= 1.772453850905516;
            break;
        }

        result *= num;
    }

    return new Number(result);
}

BaseType *OpChs::eval(Number &rhs) {
    return new Number(-rhs.num);
}

BaseType *OpChs::eval(Complex &rhs) {
    return new Complex(-rhs.real, -rhs.imag);
}

BaseType *OpChs::eval(Matrix &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = rhs.elements;

    for (auto &row : newElements) {
        for (auto &col : row) {
            col = dynamic_cast<Number &>(*this->eval(col));
        }
    }

    return new Matrix(newElements);
}

BaseType *OpMul::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num * rhs.num);
}

BaseType *OpMul::eval(Number &lhs, Complex &rhs) {
    return new Complex(lhs.num * rhs.real, lhs.num * rhs.imag);
}

BaseType *OpMul::eval(Complex &lhs, Number &rhs) {
    return new Complex(lhs.real * rhs.num, lhs.imag * rhs.num);
}

BaseType *OpMul::eval(Complex &lhs, Complex &rhs) {
    // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
    return new Complex(lhs.real * rhs.real - lhs.imag * rhs.imag, lhs.real * rhs.imag + lhs.imag * rhs.real);
}

BaseType *OpMul::eval(Matrix &lhs, Matrix &rhs) {
    return multiplyMatrices(lhs, rhs);
}

BaseType *OpDiv::eval(Number &lhs, Number &rhs) {
    if (rhs.num == 0) divideBy0Error();

    return new Number(lhs.num / rhs.num);
}

BaseType *OpDiv::eval(Number &lhs, Complex &rhs) {
    // a / (b + ci) = (ab - aci) / (b² + c²)
    float denom = rhs.real * rhs.real + rhs.imag * rhs.imag;

    if (denom == 0) divideBy0Error();

    return new Complex(lhs.num * rhs.real / denom, -lhs.num * rhs.imag / denom);
}

BaseType *OpDiv::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpDiv::eval(Complex &lhs, Number &rhs) {
    if (rhs.num == 0) divideBy0Error();

    return new Complex(lhs.real / rhs.num, lhs.imag / rhs.num);
}

BaseType *OpDiv::eval(Complex &lhs, Complex &rhs) {
    // (a + bi) / (c + di) = ((ac + bd) + (bc - ad)i) / (c² + d²)
    float denom = rhs.real * rhs.real + rhs.imag * rhs.imag;

    if (denom == 0) divideBy0Error();

    return new Complex(
            (lhs.real * rhs.real + lhs.imag * rhs.imag) / denom,
            (lhs.imag * rhs.real - lhs.real * rhs.imag) / denom
            );
}

BaseType *OpDiv::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *OpAddSub::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpAddSub::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *OpAddSub::eval(Matrix &lhs, Matrix &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (lhs.elements.size() != rhs.elements.size()) dimensionMismatch();
    if (lhs.elements[0].size() != rhs.elements[0].size()) dimensionMismatch();

    auto newElements = lhs.elements;

    unsigned int rowIndex = 0;
    for (auto &row : newElements) {
        unsigned int colIndex = 0;

        for (auto &col : row) {
            col = dynamic_cast<Number &>(*this->eval(col, rhs.elements[rowIndex][colIndex]));
            colIndex++;
        }

        rowIndex++;
    }

    return new Matrix(newElements);
}

BaseType *OpAdd::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num + rhs.num);
}

BaseType *OpAdd::eval(Number &lhs, Complex &rhs) {
    return new Complex(lhs.num + rhs.real, rhs.imag);
}

BaseType *OpAdd::eval(Complex &lhs, Number &rhs) {
    return new Complex(lhs.real + rhs.num, lhs.imag);
}

BaseType *OpAdd::eval(Complex &lhs, Complex &rhs) {
    return new Complex(lhs.real + rhs.real, lhs.imag + rhs.imag);
}

BaseType *OpAdd::eval(String &lhs, String &rhs) {
    if (!lhs.length || !rhs.length) dimensionError();

    unsigned int newLength = lhs.length + rhs.length;
    char *newString = new char[newLength];

    memcpy(newString, lhs.string, lhs.length);
    memcpy(newString + lhs.length, rhs.string, rhs.length);

    return new String(newLength, newString);
}

BaseType *OpSub::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num - rhs.num);
}

BaseType *OpSub::eval(Number &lhs, Complex &rhs) {
    return new Complex(lhs.num - rhs.real, -rhs.imag);
}

BaseType *OpSub::eval(Complex &lhs, Number &rhs) {
    return new Complex(lhs.real - rhs.num, lhs.imag);
}

BaseType *OpSub::eval(Complex &lhs, Complex &rhs) {
    return new Complex(lhs.real - rhs.real, lhs.imag - rhs.imag);
}

BaseType *OpSub::eval(__attribute__((unused)) String &lhs, __attribute__((unused)) String &rhs) {
    typeError();
}

BaseType *OpEquality::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *OpEquality::eval(__attribute__((unused)) Number &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpEquality::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *OpEquality::eval(Complex &lhs, ComplexList &rhs) {
    if (rhs.elements.empty()) dimensionError();

    auto newElements = vector<Number>(rhs.elements.size());

    unsigned int index = 0;
    for (auto &cplx : rhs.elements) {
        newElements[index++] = dynamic_cast<Number &>(*this->eval(lhs, cplx));
    }

    return new List(newElements);
}

BaseType *OpEquality::eval(ComplexList &lhs, Complex &rhs) {
    if (lhs.elements.empty()) dimensionError();

    auto newElements = vector<Number>(lhs.elements.size());

    unsigned int index = 0;
    for (auto &cplx : lhs.elements) {
        newElements[index++] = dynamic_cast<Number &>(*this->eval(cplx, rhs));
    }

    return new List(newElements);
}

BaseType *OpEquality::eval(ComplexList &lhs, ComplexList &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (rhs.elements.size() != rhs.elements.size()) dimensionMismatch();

    auto newElements = vector<Number>(lhs.elements.size());

    unsigned int index = 0;
    for (auto &cplx : lhs.elements) {
        newElements[index] = dynamic_cast<Number &>(*this->eval(cplx, rhs.elements[index]));
        index++;
    }

    return new List(newElements);
}

BaseType *OpEquality::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Number &rhs) {
    typeError();
}

BaseType *OpEquality::eval(Matrix &lhs, Matrix &rhs) {
    if (lhs.elements.empty()) dimensionError();
    if (lhs.elements.size() != rhs.elements.size()) dimensionMismatch();
    if (lhs.elements[0].size() != rhs.elements[0].size()) dimensionMismatch();

    unsigned int rowIndex = 0;
    for (auto &row : lhs.elements) {
        unsigned int colIndex = 0;
        for (auto &col : row) {
            auto result = dynamic_cast<Number &>(*this->eval(col, rhs.elements[rowIndex][colIndex]));

            if (result.num == 0) return new Number(0);

            colIndex++;
        }

        rowIndex++;
    }

    return new Number(1);
}

BaseType *OpEQ::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num == rhs.num);
}

BaseType *OpEQ::eval(Complex &lhs, Complex &rhs) {
    return new Number(lhs.real == rhs.real && lhs.imag == rhs.imag);
}

BaseType *OpLT::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num < rhs.num);
}

BaseType *OpLT::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *OpLT::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpGT::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num > rhs.num);
}

BaseType *OpGT::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *OpGT::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpLE::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num < rhs.num);
}

BaseType *OpLE::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *OpLE::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpGE::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num > rhs.num);
}

BaseType *OpGE::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *OpGE::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpNE::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num != rhs.num);
}

BaseType *OpNE::eval(Complex &lhs, Complex &rhs) {
    return new Number(lhs.real != rhs.real || lhs.imag != rhs.imag);
}

BaseType *OpLogically::eval(__attribute__((unused)) Complex &lhs, __attribute__((unused)) Complex &rhs) {
    typeError();
}

BaseType *OpLogically::eval(__attribute__((unused)) Matrix &lhs, __attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpAnd::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num != 0 && rhs.num != 0);
}

BaseType *OpOr::eval(Number &lhs, Number &rhs) {
    return new Number(lhs.num != 0 || rhs.num != 0);
}

BaseType *OpXor::eval(Number &lhs, Number &rhs) {
    return new Number((lhs.num != 0) != (rhs.num != 0));
}
