#include "operators.h"
#include "ast.h"
#include "errors.h"
#include "evaluate.h"
#include "globals.h"

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
    float denom = rhs.real * rhs.real + rhs.imag + rhs.imag;

    if (denom == 0) divideBy0Error();

    return new Complex(rhs.real / denom, rhs.imag / -denom);
}

BaseType *OpRecip::eval(Matrix &rhs) {
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
    // todo: matrix multiplication
    typeError();
}

BaseType *OpTrnspos::eval(Matrix &rhs) {
    // todo: matrix transpose
    typeError();
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
    // todo: matrix multiplication
    typeError();
}

BaseType *OpFact::eval(Number &rhs) {
    // 0! = 1
    if (rhs.num == 0) {
        return new Number(1);
    }

    bool isNeg = rhs.num < 0;
    float absNum = fabsf(rhs.num);

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

    return new Number(result);
}

BaseType *OpFact::eval(__attribute__((unused)) Matrix &rhs) {
    typeError();
}

BaseType *OpChs::eval(Number &rhs) {
    return new Number(-rhs.num);
}

BaseType *OpChs::eval(Complex &rhs) {
    return new Complex(-rhs.real, -rhs.imag);
}

BaseType *evalOperator(struct NODE *node) {
    uint8_t op = node->data.operand.op;
    BaseType *leftNode = evalNode(node->child);
    // BaseType *rightNode = nullptr;
    BaseType *result = nullptr;

    if (isUnaryOp(getOpPrecedence(op))) {
        Operator *opNew;

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
        BaseType *rightNode = nullptr;

        if (op == tStore) {
        } else {
            rightNode = evalNode(node->child->next);

            switch (op) {
                case tMul:
                    break;
                case tAdd:
                    break;
                default:
                    result = nullptr;
            }
        }
    }

    delete leftNode;
    free(node);

    return result;
}
