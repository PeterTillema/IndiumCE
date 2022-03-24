#include "expression.h"

#include "ast.h"
#include "errors.h"
#include "operators.h"
#include "utils.h"
#include "variables.h"

#include <cmath>
#include <cstring>
#include <fileioc.h>
#include <keypadc.h>

// tinystl::vector<struct NODE *> outputStack doesn't work, linking keeps freezing and not generating code
// in the allowed number of passes. Design flaw somewhere?
static struct NODE *outputStack[500];
static struct NODE *opStack[100];
static unsigned int outputStackNr;
static unsigned int opStackNr;
static uint8_t nestedFuncs = 0;
static bool needMulOp;

extern void (*expressionFunctions[256])(ti_var_t slot, int token);

static void addToOutput(struct NODE *tmp) {
    if (outputStackNr == 500) memoryError();

    outputStack[outputStackNr++] = tmp;
}

static void addToStack(struct NODE *tmp) {
    if (opStackNr == 100) memoryError();

    opStack[opStackNr++] = tmp;
}

static void pushOp(uint8_t precedence, int token) {
    while (opStackNr) {
        // Previous element on the stack should be an operator
        struct NODE *prev = opStack[opStackNr - 1];
        if (prev->data.type != ET_OPERATOR) break;

        // Check if we need to move the previous operator to the output stack
        uint8_t prevPrec = getOpPrecedence(prev->data.operand.op);
        if (prevPrec > precedence ||
            (prevPrec == precedence && (token == tPower || token == tChs || token == tComma)))
            break;

        // Check for unary operator and set the args of the operator
        if (isUnaryOp(prevPrec)) {
            if (!outputStackNr) parseError("Syntax error");

            prev->child = outputStack[outputStackNr - 1];

            outputStack[outputStackNr - 1] = prev;
        } else {
            if (outputStackNr < 2) parseError("Syntax error");

            prev->child = outputStack[outputStackNr - 2];
            prev->child->next = outputStack[outputStackNr - 1];

            outputStack[outputStackNr - 2] = prev;
            outputStackNr--;
        }

        opStackNr--;
    }
}

static void pushRParen(uint8_t tok) {
    uint8_t argCount = 1;

    nestedFuncs--;

    // Search for the matching left parenthesis/bracket. Everything we encounter can only be a comma, which is used in a
    // function. If any comma is found, it must be function, as (1, 2) is invalid.
    for (unsigned int i = opStackNr; i-- > 0;) {
        struct NODE *tmp = opStack[i];

        if (tmp->data.type == ET_FUNCTION_CALL && tmp->data.operand.func == tok) {
            // This is the closing } or ) which is a single function without an extra parenthesis
            if (argCount <= outputStackNr && (tok == tLBrace || tok == tLBrack)) {
                struct NODE *tree = tmp->child = outputStack[outputStackNr - argCount];

                // Set the arguments of the function
                for (uint8_t j = 1; j < argCount; j++) {
                    tree->next = outputStack[outputStackNr - argCount + j];
                    tree = tree->next;
                }

                outputStackNr -= argCount;
                opStackNr--;

                // Insert the function in the output queue
                addToOutput(tmp);

                return;
            } else if (i && argCount <= outputStackNr && opStack[i - 1]->data.type == ET_FUNCTION_CALL &&
                       opStack[i - 1]->data.operand.func != tLParen) {
                // This is a real function, like sin or cos. Free the parenthesis, and set all arguments from the
                // output queue as the children of this function.
                struct NODE *funcNode = opStack[i - 1];
                struct NODE *tree = funcNode->child = outputStack[outputStackNr - argCount];

                free(tmp);

                // Set the arguments of the function
                for (uint8_t j = 1; j < argCount; j++) {
                    tree->next = outputStack[outputStackNr - argCount + j];
                    tree = tree->next;
                }

                outputStackNr -= argCount;
                opStackNr -= 2;

                // Insert the function in the output queue
                addToOutput(funcNode);

                return;
            } else if (argCount == 1) {
                // It is a standalone parenthesis, it should have only 1 argument. Only free the stack entry, as the
                // last output queue item is already the correct one.
                free(tmp);
                opStackNr--;

                return;
            } else {
                break;
            }
        } else if (tmp->data.type == ET_OPERATOR && tmp->data.operand.op == tComma) {
            argCount++;
            opStackNr--;

            free(tmp);
        } else {
            break;
        }
    }

    parseError("Unexpected \")\"");
}

static void emptyOpStack() {
    for (unsigned int i = opStackNr; i-- > 0;) {
        if (i >= opStackNr) continue;

        struct NODE *tmp = opStack[i];

        if (tmp->data.type == ET_OPERATOR) pushOp(MAX_PRECEDENCE + 1, MAX_PRECEDENCE + 1);
        else if (tmp->data.type == ET_FUNCTION_CALL) pushRParen(tmp->data.operand.func);
    }

    if (outputStackNr != 1) parseError("Invalid expression");
}

struct NODE *expressionLine(ti_var_t slot, int token, bool stopAtComma, bool stopAtParen) {
    // Reset expression things
    outputStackNr = 0;
    opStackNr = 0;
    needMulOp = false;

    while (token != EOF && token != tEnter && token != tColon) {
        if (token == tComma && stopAtComma && !nestedFuncs) break;
        if (token == tRParen && stopAtParen && !nestedFuncs) break;

        if (kb_On) parseError("[ON]-key pressed");

        // Execute the token function
        (*expressionFunctions[token])(slot, token);

        // And get the new token
        token = tokenNext(slot);
    }

    emptyOpStack();

    return outputStack[0];
}

struct NODE *tokenExpression(ti_var_t slot, int token) {
    return expressionLine(slot, token, false, false);
}

static void tokenOperator(__attribute__((unused)) ti_var_t slot, int token) {
    needMulOp = false;

    if (token == tStore) {
        // Multiple stores are not implemented
        if (opStackNr && opStack[0]->data.type == ET_OPERATOR && opStack[0]->data.operand.op == tStore)
            parseError("Syntax error");

        emptyOpStack();
    }

    // Push the operator to the stack
    uint8_t op_precedence = getOpPrecedence(token);
    pushOp(op_precedence, token);

    // And allocate memory for the new operator
    auto op_node = new NODE();
    op_node->data.type = ET_OPERATOR;
    op_node->data.operand.op = token;

    addToStack(op_node);
}

static void tokenRBrack(ti_var_t slot, int token) {
    needMulOp = true;

    pushOp(MAX_PRECEDENCE + 1, token);
    pushRParen(tLBrack);

    // Check if a "[" is coming, in which case we need an extra comma
    token = tokenPeek();
    if ((uint8_t) token == tLBrack) {
        tokenOperator(slot, tComma);
    }
}

static void tokenRBrace(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    needMulOp = true;

    pushOp(MAX_PRECEDENCE + 1, token);
    pushRParen(tLBrace);
}

static void tokenRParen(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    needMulOp = true;

    // This forces all operators to be moved to the output stack
    // After that, push the right parenthesis
    pushOp(MAX_PRECEDENCE + 1, token);
    pushRParen(tLParen);
}

static void tokenFunction(ti_var_t slot, int token) {
    if (needMulOp) tokenOperator(slot, tMul);

    // Allocate space for the function
    auto node = new NODE();
    node->data.type = ET_FUNCTION_CALL;
    node->data.operand.func = token;

    addToStack(node);
    nestedFuncs++;

    if (token != tLParen && token != tLBrace && token != tLBrack) {
        // Eventually push an extra (
        auto parenNode = new NODE();
        parenNode->data.type = ET_FUNCTION_CALL;
        parenNode->data.operand.func = tLParen;

        addToStack(parenNode);
    }
}

static void tokenNumber(ti_var_t slot, int token) {
    float num = 0;
    bool inExp = false;
    bool negativeExp = false;
    bool isComplex = false;
    int exp = 0;
    int fracNum = 0;
    uint8_t expNum = 0;
    uint8_t tok = token;

    if (needMulOp) tokenOperator(slot, tMul);
    needMulOp = true;

    // Set some booleans
    if (tok == tee) {
        num = 1;
        expNum = 1;
        inExp = true;
    } else if (tok == tDecPt) {
        fracNum++;
    } else {
        num = (float) tok - t0;
    }

    while ((token = tokenNext(slot)) != EOF) {
        tok = token;

        // Should be a valid num char
        if (!(tok == tee || tok == tDecPt || tok == tii || tok == tChs || (tok >= t0 && tok <= t9))) break;

        if (tok == tee) {
            if (inExp) parseError("Syntax error");

            inExp = true;
        } else if (tok == tDecPt) {
            if (fracNum || inExp) parseError("Syntax error");

            fracNum = -1;
        } else if (tok == tii) {
            isComplex = true;

            break;
        } else if (tok == tChs) {
            if (!inExp || expNum != 1) parseError("Syntax error");

            negativeExp = true;
        } else if (inExp) {
            exp = exp * 10 + tok - t0;
            expNum++;
        } else if (fracNum) {
            num += powf(10, (float) fracNum) * (float) (tok - t0);
            fracNum--;
        } else {
            num = num * 10 + (float) (tok - t0);
        }
    }

    if (token != tii) seekPrev(slot);

    // Get the right number, based on the exponent and negative flag
    if (negativeExp) exp = -exp;
    if (inExp) num = num * powf(10, (float) exp);

    // And add it to the output stack
    auto node = new NODE();
    if (isComplex) {
        node->data.type = ET_COMPLEX;
        node->data.operand.cplx = new Complex(0, num);

        addToOutput(node);
    } else {
        node->data.type = ET_NUMBER;
        node->data.operand.num = new Number(num);

        addToOutput(node);
    }
}

static void tokenVariable(__attribute__((unused)) ti_var_t slot, int token) {
    if (needMulOp) tokenOperator(slot, tMul);
    needMulOp = true;

    auto node = new NODE();
    node->data.type = ET_VARIABLE;
    node->data.operand.variableNr = token - tA;

    addToOutput(node);
}

static void tokenOSList(ti_var_t slot, __attribute__((unused)) int token) {
    if (needMulOp) tokenOperator(slot, tMul);

    uint8_t listNr = tokenNext(slot);

    // Check if it's a list element
    if (tokenPeek() == tLParen) {
        tokenNext(slot);
        tokenFunction(slot, 0x5D + (listNr << 8));
    } else {
        auto node = new NODE();
        node->data.type = ET_LIST;
        node->data.operand.listNr = listNr;

        addToOutput(node);
        needMulOp = true;
    }
}

static void tokenOSMatrix(ti_var_t slot, __attribute__((unused)) int token) {
    if (needMulOp) tokenOperator(slot, tMul);

    uint8_t matrixNr = tokenNext(slot);

    // Check if it's a matrix element
    if (tokenPeek() == tLParen) {
        tokenNext(slot);
        tokenFunction(slot, 0x5C + (matrixNr << 8));
    } else {
        auto node = new NODE();
        node->data.type = ET_MATRIX;
        node->data.operand.matrixNr = matrixNr;

        addToOutput(node);
        needMulOp = true;
    }
}

static void tokenOsString(ti_var_t slot, __attribute__((unused)) int token) {
    if (needMulOp) tokenOperator(slot, tMul);
    needMulOp = true;

    uint8_t strNr = tokenNext(slot);

    auto node = new NODE();
    node->data.type = ET_STRING;
    node->data.operand.stringNr = strNr;

    addToOutput(node);
}

static void tokenOsEqu(ti_var_t slot, __attribute__((unused)) int token) {
    if (needMulOp) tokenOperator(slot, tMul);
    needMulOp = true;

    uint8_t equNr = tokenNext(slot);

    if (equNr >= 0x80) equNr -= 0x80 - 28;
    else if (equNr >= 0x40) equNr -= 0x40 - 22;
    else if (equNr >= 0x20) equNr -= 0x20 - 10;

    auto node = new NODE();
    node->data.type = ET_EQU;
    node->data.operand.equationNr = equNr;

    addToOutput(node);
}

static void tokenString(ti_var_t slot, int token) {
    if (needMulOp) tokenOperator(slot, tMul);

    uint8_t *startPtr = (uint8_t *) ti_GetDataPtr(slot) + 1;
    unsigned int length = 0;

    do {
        token = tokenNext(slot);
        length++;

        if (is2ByteTok(token)) {
            tokenNext(slot);
            length++;
        }
    } while (token != EOF && (uint8_t) token != tEnter && (uint8_t) token != tStore && (uint8_t) token != tString);

    if ((uint8_t) token == tEnter || (uint8_t) token == tStore) {
        seekPrev(slot);
    } else {
        needMulOp = true;
    }

    // This is a fake struct NODE, in fact it's just a pointer to the raw string
    auto stringMemory = (struct var_string *) new char[length - 1 + 3];

    stringMemory->length = length - 1;
    memcpy(stringMemory->data, startPtr, length - 1);

    auto node = new NODE();
    node->data.type = ET_FUNCTION_CALL;
    node->data.operand.func = tString;
    node->child = (struct NODE *) stringMemory;

    addToOutput(node);
}

static void tokenEmptyFunc(__attribute__((unused)) ti_var_t slot, int token) {
    if (needMulOp) tokenOperator(slot, tMul);
    needMulOp = true;

    auto node = new NODE();
    node->data.type = ET_FUNCTION_CALL;
    node->data.operand.func = token;

    addToOutput(node);
}

static void tokenPi(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    if (needMulOp) tokenOperator(slot, tMul);
    needMulOp = true;

    auto node = new NODE();
    node->data.type = ET_NUMBER;
    node->data.operand.num = new Number(M_PI);

    addToOutput(node);
}

static void tokenRand(ti_var_t slot, __attribute__((unused)) int token) {
    if (needMulOp) tokenOperator(slot, tMul);

    // Check if it's a matrix element
    if (tokenPeek() == tLParen) {
        tokenNext(slot);
        tokenFunction(slot, tRand);
    } else {
        auto node = new NODE();
        node->data.type = ET_FUNCTION_CALL;
        node->data.operand.func = tRand;

        addToOutput(node);
        needMulOp = true;
    }
}

static void tokenUnimplemented(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    parseError("Token not implemented");
}

void (*expressionFunctions[256])(ti_var_t, int) = {
        tokenUnimplemented,  // **unused**
        tokenUnimplemented,  // ►DMS
        tokenUnimplemented,  // ►Dec
        tokenUnimplemented,  // ►Frac
        tokenOperator,       // →
        tokenUnimplemented,  // Boxplot
        tokenFunction,       // [
        tokenRBrack,         // ]
        tokenFunction,       // {
        tokenRBrace,         // }
        tokenOperator,       // r
        tokenOperator,       // °
        tokenOperator,       // ֿ¹
        tokenOperator,       // ²
        tokenOperator,       // T
        tokenOperator,       // ³
        tokenFunction,       // (
        tokenRParen,         // )
        tokenFunction,       // round(
        tokenFunction,       // pxl-Test(
        tokenFunction,       // augment(
        tokenFunction,       // rowSwap(
        tokenFunction,       // row+(
        tokenFunction,       // *row(
        tokenFunction,       // *row+(
        tokenFunction,       // max(
        tokenFunction,       // min(
        tokenFunction,       // R►Pr(
        tokenFunction,       // R►Pθ(
        tokenFunction,       // R►Px(
        tokenFunction,       // R►Py(
        tokenFunction,       // median(
        tokenFunction,       // randM(
        tokenFunction,       // mean(
        tokenFunction,       // solve(
        tokenFunction,       // seq(
        tokenFunction,       // fnInt(
        tokenFunction,       // nDeriv(
        tokenUnimplemented,  // **unused**
        tokenFunction,       // fMin(
        tokenFunction,       // fMax(
        tokenUnimplemented,  //
        tokenString,         // "
        tokenOperator,       // ,
        tokenEmptyFunc,      // i
        tokenOperator,       // !
        tokenUnimplemented,  // CubicReg
        tokenUnimplemented,  // QuartReg
        tokenNumber,         // 0
        tokenNumber,         // 1
        tokenNumber,         // 2
        tokenNumber,         // 3
        tokenNumber,         // 4
        tokenNumber,         // 5
        tokenNumber,         // 6
        tokenNumber,         // 7
        tokenNumber,         // 8
        tokenNumber,         // 9
        tokenNumber,         // .
        tokenNumber,         // |E
        tokenOperator,       // or
        tokenOperator,       // xor
        tokenUnimplemented,  // :
        tokenUnimplemented,  // \n
        tokenOperator,       // and
        tokenVariable,       // A
        tokenVariable,       // B
        tokenVariable,       // C
        tokenVariable,       // D
        tokenVariable,       // E
        tokenVariable,       // F
        tokenVariable,       // G
        tokenVariable,       // H
        tokenVariable,       // I
        tokenVariable,       // J
        tokenVariable,       // K
        tokenVariable,       // L
        tokenVariable,       // M
        tokenVariable,       // N
        tokenVariable,       // O
        tokenVariable,       // P
        tokenVariable,       // Q
        tokenVariable,       // R
        tokenVariable,       // S
        tokenVariable,       // T
        tokenVariable,       // U
        tokenVariable,       // V
        tokenVariable,       // W
        tokenVariable,       // X
        tokenVariable,       // Y
        tokenVariable,       // Z
        tokenVariable,       // theta
        tokenOSMatrix,       // 2-byte token (Matrices)
        tokenOSList,         // 2-byte token (Lists)
        tokenOsEqu,          // 2-byte token (Equations)
        tokenUnimplemented,  // prgm
        tokenUnimplemented,  // 2-byte token (Pictures)
        tokenUnimplemented,  // 2-byte token (GDBs)
        tokenUnimplemented,  // 2-byte token (Statistics)
        tokenUnimplemented,  // 2-byte token (Window)
        tokenUnimplemented,  // Radian
        tokenUnimplemented,  // Degree
        tokenUnimplemented,  // Normal
        tokenUnimplemented,  // Sci
        tokenUnimplemented,  // Eng
        tokenUnimplemented,  // Float
        tokenOperator,       // =
        tokenOperator,       // <
        tokenOperator,       // >
        tokenOperator,       // <=
        tokenOperator,       // >=
        tokenOperator,       // !=
        tokenOperator,       // +
        tokenOperator,       // - (sub)
        tokenEmptyFunc,      // Ans
        tokenUnimplemented,  // Fix
        tokenUnimplemented,  // Horiz
        tokenUnimplemented,  // Full
        tokenUnimplemented,  // Func
        tokenUnimplemented,  // Param
        tokenUnimplemented,  // Polar
        tokenUnimplemented,  // Seq
        tokenUnimplemented,  // IndpntAuto
        tokenUnimplemented,  // IndpntAsk
        tokenUnimplemented,  // DependAuto
        tokenUnimplemented,  // DependAsk
        tokenUnimplemented,  // 2-byte token (Format)
        tokenUnimplemented,  // square mark
        tokenUnimplemented,  // plus mark
        tokenUnimplemented,  // dot mark
        tokenOperator,       // *
        tokenOperator,       // /
        tokenUnimplemented,  // Trace
        tokenUnimplemented,  // ClrDraw
        tokenUnimplemented,  // ZStandard
        tokenUnimplemented,  // ZTrig
        tokenUnimplemented,  // ZBox
        tokenUnimplemented,  // Zoom In
        tokenUnimplemented,  // Zoom Out
        tokenUnimplemented,  // ZSquare
        tokenUnimplemented,  // ZInteger
        tokenUnimplemented,  // ZPrevious
        tokenUnimplemented,  // ZDecimal
        tokenUnimplemented,  // ZoomStat
        tokenUnimplemented,  // ZoomRcl
        tokenUnimplemented,  // PrintScreen
        tokenUnimplemented,  // ZoomSto
        tokenUnimplemented,  // Text(
        tokenOperator,       // nPr
        tokenOperator,       // nCr
        tokenUnimplemented,  // FnOn
        tokenUnimplemented,  // FnOff
        tokenUnimplemented,  // StorePic
        tokenUnimplemented,  // RecallPic
        tokenUnimplemented,  // StoreGDB
        tokenUnimplemented,  // RecallGDB
        tokenUnimplemented,  // Line(
        tokenUnimplemented,  // Vertical
        tokenUnimplemented,  // Pt-On(
        tokenUnimplemented,  // Pt-Off(
        tokenUnimplemented,  // Pt-Change(
        tokenUnimplemented,  // Pxl-On(
        tokenUnimplemented,  // Pxl-Off(
        tokenUnimplemented,  // Pxl-Change(
        tokenUnimplemented,  // Shade(
        tokenUnimplemented,  // Circle(
        tokenUnimplemented,  // Horizontal
        tokenUnimplemented,  // Tangent(
        tokenUnimplemented,  // DrawInv
        tokenUnimplemented,  // DrawF
        tokenOsString,       // 2-byte token (Strings)
        tokenRand,           // rand
        tokenPi,             // pi
        tokenEmptyFunc,      // getKey
        tokenUnimplemented,  // '
        tokenUnimplemented,  // ?
        tokenOperator,       // - (neg)
        tokenFunction,       // int(
        tokenFunction,       // abs(
        tokenFunction,       // det(
        tokenFunction,       // identity(
        tokenFunction,       // dim(
        tokenFunction,       // sum(
        tokenFunction,       // prod(
        tokenFunction,       // not(
        tokenFunction,       // iPart(
        tokenFunction,       // fPart(
        tokenUnimplemented,  // 2-byte token (BB)
        tokenFunction,       // √(
        tokenFunction,       // ³√(
        tokenFunction,       // ln(
        tokenFunction,       // e^(
        tokenFunction,       // log(
        tokenFunction,       // 10^(
        tokenFunction,       // sin(
        tokenFunction,       // sinֿ¹(
        tokenFunction,       // cos(
        tokenFunction,       // cosֿ¹(
        tokenFunction,       // tan(
        tokenFunction,       // tanֿ¹(
        tokenFunction,       // sinh(
        tokenFunction,       // sihֿ¹(
        tokenFunction,       // cosh(
        tokenFunction,       // coshֿ¹(
        tokenFunction,       // tanh(
        tokenFunction,       // tanhֿ¹(
        tokenUnimplemented,  // If
        tokenUnimplemented,  // Then
        tokenUnimplemented,  // Else
        tokenUnimplemented,  // While
        tokenUnimplemented,  // Repeat
        tokenUnimplemented,  // For(
        tokenUnimplemented,  // End
        tokenUnimplemented,  // Return
        tokenUnimplemented,  // Lbl
        tokenUnimplemented,  // Goto
        tokenUnimplemented,  // Pause
        tokenUnimplemented,  // Stop
        tokenUnimplemented,  // IS>(
        tokenUnimplemented,  // DS<(
        tokenUnimplemented,  // Input
        tokenUnimplemented,  // Prompt
        tokenUnimplemented,  // Disp
        tokenUnimplemented,  // DispGraph
        tokenUnimplemented,  // Output(
        tokenUnimplemented,  // ClrHome
        tokenUnimplemented,  // Fill(
        tokenUnimplemented,  // SortA(
        tokenUnimplemented,  // SortD(
        tokenUnimplemented,  // DispTable
        tokenUnimplemented,  // Menu(
        tokenUnimplemented,  // Send(
        tokenUnimplemented,  // Get(
        tokenUnimplemented,  // PlotsOn
        tokenUnimplemented,  // PlotsOff
        tokenUnimplemented,  // ∟
        tokenUnimplemented,  // Plot1(
        tokenUnimplemented,  // Plot2(
        tokenUnimplemented,  // Plot3(
        tokenUnimplemented,  // 2-byte token
        tokenOperator,       // ^
        tokenUnimplemented,  // ×√
        tokenUnimplemented,  // 1-Var Stats
        tokenUnimplemented,  // 2-Var Stats
        tokenUnimplemented,  // LinReg(a+bx)
        tokenUnimplemented,  // ExpReg
        tokenUnimplemented,  // LnReg
        tokenUnimplemented,  // PwrReg
        tokenUnimplemented,  // Med-Med
        tokenUnimplemented,  // QuadReg
        tokenUnimplemented,  // ClrList
        tokenUnimplemented,  // ClrTable
        tokenUnimplemented,  // Histogram
        tokenUnimplemented,  // xyLine
        tokenUnimplemented,  // Scatter
        tokenUnimplemented   // LinReg(ax+b)
};
