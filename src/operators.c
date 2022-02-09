#include "operators.h"

#include <stdint.h>
#include <tice.h>
#include <string.h>

// https://education.ti.com/html/webhelp/EG_TI84PlusCE/EN/content/eg_gsguide/m_expressions/exp_order_of_operations.HTML
// Commas are treated as a special operator, which means that if a comma if the token is a comma, all other operators
// will be moved properly to the output. It is also "right-associative", which means that if the top stack entry is also
// a comma, it won't be moved (which is clearly not what we want)
static const char operators[] = {
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
const char *operator_chars[] = {
        "^^r", "^^o", "^^-1", "^^2", "^^T", "^^3",
        "^",
        "!",
        "~",
        "nPr", "nCr",
        "*", "/",
        "+", "-",
        "==", "<", ">", "<=", "GE", "!=",
        "and",
        "or", "xor",
        "->",
        ","
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

uint8_t get_operator_precedence(uint8_t op) {
    char *index = memchr(operators, op, sizeof(operators));

    return precedence[index - operators];
}

bool is_unary_op(uint8_t prec) {
    return prec <= 4 && prec != 2;
}

const char *get_op_string(uint8_t op) {
    char *index = memchr(operators, op, sizeof(operators));

    return operator_chars[index - operators];
}
