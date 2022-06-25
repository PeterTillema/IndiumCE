#include "parse.h"
#include "ast.h"
#include "errors.h"
#include "utils.h"
#include "operators.h"
#include "variables.h"

#include <fileioc.h>
#include <keypadc.h>
#include <cmath>
#include <cstring>

extern struct NODE *(*parseFunctions[256])(ti_var_t, int);

unsigned int parseLine = 1;
unsigned int parseCol = 0;

// tinystl::vector<struct NODE *> outputStack doesn't work, linking keeps freezing and not generating code
// in the allowed number of passes. Design flaw somewhere?
static struct NODE *outputStack[500];
static struct NODE *opStack[100];
static unsigned int outputStackNr;
static unsigned int opStackNr;
static uint8_t nestedFuncs = 0;
static bool needMulOp;

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

static struct NODE *tokenUnimplemented(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    parseError("Token not implemented");
}

#define UNEXPRESSION(func) (reinterpret_cast<void (*)(ti_var_t, int)>(((unsigned int *)(func) + 0x800000)))

struct NODE *expressionLine(ti_var_t slot, int token, bool stopAtComma, bool stopAtParen) {
    // Reset expression things
    outputStackNr = 0;
    opStackNr = 0;
    needMulOp = false;

    while (token != EOF && token != tEnter && token != tColon) {
        if (token == tComma && stopAtComma && !nestedFuncs) break;
        if (token == tRParen && stopAtParen && !nestedFuncs) break;

        if (kb_On) parseError("[ON]-key pressed");

        auto func = parseFunctions[token];
        if ((unsigned int)((uint64_t)(func)) >= 0x800000) {
            tokenUnimplemented(slot, token);
        }

        // Execute the token function
        (*UNEXPRESSION(func))(slot, token);

        // And get the new token
        token = tokenNext(slot);
    }

    emptyOpStack();

    return outputStack[0];
}

#undef UNEXPRESSION

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

/**
 * This function parses the entire program, reading it line by line
 * @param slot fileioc slot to read the data from
 * @param expectEnd Boolean to allow stopping the program at "End", which is after a loop/statement
 * @param expectElse Boolean to allow stopping the program at "Else", which is inside an If-statement
 * @return Node to start parsing at (i.e. the root)
 */
struct NODE *parseProgram(ti_var_t slot, bool expectEnd, bool expectElse) {
    int token;
    struct NODE *root = nullptr;
    struct NODE *tail = nullptr;

    while ((token = tokenNext(slot)) != EOF) {
        if (kb_On) parseError("[ON]-key pressed");

        // Skip if's a colon
        if (token == tColon) {
            continue;
        }

        // Return if we hit an Else/End, but only if it's valid!
        if ((token == tEnd && expectEnd) || (token == tElse && expectElse))
            return root;

        auto func = parseFunctions[token];
        struct NODE *node;
        if ((unsigned int)(uint64_t)(func) < 0x800000) {
            node = expressionLine(slot, token, false, false);
        } else {
            node = (*func)(slot, token);
        }

        // Advance line and column
        parseLine++;
        parseCol = 0;

        // Need to insert it?
        if (node == nullptr)
            continue;

        // Insert it to the chain
        if (root == nullptr) {
            root = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    return root;
}

static struct NODE *tokenCommandStandalone(__attribute__((unused)) ti_var_t slot, int token) {
    if (!endOfLine(tokenPeek())) parseError("Syntax error");

    auto func_node = new NODE();
    func_node->data.type = ET_FUNCTION_CALL;
    func_node->data.operand.func = token;

    return func_node;
}

static struct NODE *tokenCommand(ti_var_t slot, int token, bool endParen) {
    auto commandNode = new NODE();
    commandNode->data.type = ET_COMMAND;
    commandNode->data.operand.command = token;

    struct NODE *tree = nullptr;

    for (;;) {
        token = tokenNext(slot);

        // Get the next expression
        auto expr = expressionLine(slot, token, true, endParen);

        if (tree == nullptr) {
            tree = commandNode->child = expr;
        } else {
            tree->next = expr;
            tree = tree->next;
        }

        token = tokenCurrent();

        // There are a few stop cases possible:
        //  - EOF, enter or a colon. In that case, get back to the previous token to handle the colon/newline properly
        //  - Comma, in which case we just continue with parsing the next expression
        //  - Right parenthesis, which should be the last token on a line
        if (endOfLine(token)) {
            seekPrev(slot);

            break;
        }

        if (endParen && token == tRParen) {
            if (!endOfLine(tokenPeek())) parseError("Syntax error");

            break;
        }
    }

    return commandNode;
}

static struct NODE *tokenCommandArgs(ti_var_t slot, int token) {
    return tokenCommand(slot, token, false);
}

static struct NODE *tokenCommandParen(ti_var_t slot, int token) {
    return tokenCommand(slot, token, true);
}

static struct NODE *tokenNewline(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    return nullptr;
}

#define EXPRESSION(func) (reinterpret_cast<NODE *(*)(ti_var_t, int)>(((unsigned int*)(&(func)) - 0x800000)))

struct NODE *(*parseFunctions[256])(ti_var_t, int) = {
        tokenUnimplemented,                // **unused**
        tokenUnimplemented,                // ►DMS
        tokenUnimplemented,                // ►Dec
        tokenUnimplemented,                // ►Frac
        EXPRESSION(tokenOperator),         // →
        tokenUnimplemented,                // Boxplot
        EXPRESSION(tokenFunction),         // [
        EXPRESSION(tokenRBrack),           // ]
        EXPRESSION(tokenFunction),         // {
        EXPRESSION(tokenRBrace),           // }
        EXPRESSION(tokenOperator),         // r
        EXPRESSION(tokenOperator),         // °
        EXPRESSION(tokenOperator),         // ֿ¹
        EXPRESSION(tokenOperator),         // ²
        EXPRESSION(tokenOperator),         // T
        EXPRESSION(tokenOperator),         // ³
        EXPRESSION(tokenFunction),         // (
        EXPRESSION(tokenRParen),           // )
        EXPRESSION(tokenFunction),         // round(
        EXPRESSION(tokenFunction),         // pxl-Test(
        EXPRESSION(tokenFunction),         // augment(
        EXPRESSION(tokenFunction),         // rowSwap(
        EXPRESSION(tokenFunction),         // row+(
        EXPRESSION(tokenFunction),         // *row(
        EXPRESSION(tokenFunction),         // *row+(
        EXPRESSION(tokenFunction),         // max(
        EXPRESSION(tokenFunction),         // min(
        EXPRESSION(tokenFunction),         // R►Pr(
        EXPRESSION(tokenFunction),         // R►Pθ(
        EXPRESSION(tokenFunction),         // R►Px(
        EXPRESSION(tokenFunction),         // R►Py(
        EXPRESSION(tokenFunction),         // median(
        EXPRESSION(tokenFunction),         // randM(
        EXPRESSION(tokenFunction),         // mean(
        EXPRESSION(tokenFunction),         // solve(
        EXPRESSION(tokenFunction),         // seq(
        EXPRESSION(tokenFunction),         // fnInt(
        EXPRESSION(tokenFunction),         // nDeriv(
        tokenUnimplemented,                // **unused**
        EXPRESSION(tokenFunction),         // fMin(
        EXPRESSION(tokenFunction),         // fMax(
        tokenUnimplemented,                //
        EXPRESSION(tokenString),           // "
        EXPRESSION(tokenOperator),         // ,
        EXPRESSION(tokenEmptyFunc),        // i
        EXPRESSION(tokenOperator),         // !
        tokenUnimplemented,                // CubicReg
        tokenUnimplemented,                // QuartReg
        EXPRESSION(tokenNumber),           // 0
        EXPRESSION(tokenNumber),           // 1
        EXPRESSION(tokenNumber),           // 2
        EXPRESSION(tokenNumber),           // 3
        EXPRESSION(tokenNumber),           // 4
        EXPRESSION(tokenNumber),           // 5
        EXPRESSION(tokenNumber),           // 6
        EXPRESSION(tokenNumber),           // 7
        EXPRESSION(tokenNumber),           // 8
        EXPRESSION(tokenNumber),           // 9
        EXPRESSION(tokenNumber),           // .
        EXPRESSION(tokenNumber),           // |E
        EXPRESSION(tokenOperator),         // or
        EXPRESSION(tokenOperator),         // xor
        tokenUnimplemented,                // :
        tokenNewline,                      // \n
        EXPRESSION(tokenOperator),         // and
        EXPRESSION(tokenVariable),         // A
        EXPRESSION(tokenVariable),         // B
        EXPRESSION(tokenVariable),         // C
        EXPRESSION(tokenVariable),         // D
        EXPRESSION(tokenVariable),         // E
        EXPRESSION(tokenVariable),         // F
        EXPRESSION(tokenVariable),         // G
        EXPRESSION(tokenVariable),         // H
        EXPRESSION(tokenVariable),         // I
        EXPRESSION(tokenVariable),         // J
        EXPRESSION(tokenVariable),         // K
        EXPRESSION(tokenVariable),         // L
        EXPRESSION(tokenVariable),         // M
        EXPRESSION(tokenVariable),         // N
        EXPRESSION(tokenVariable),         // O
        EXPRESSION(tokenVariable),         // P
        EXPRESSION(tokenVariable),         // Q
        EXPRESSION(tokenVariable),         // R
        EXPRESSION(tokenVariable),         // S
        EXPRESSION(tokenVariable),         // T
        EXPRESSION(tokenVariable),         // U
        EXPRESSION(tokenVariable),         // V
        EXPRESSION(tokenVariable),         // W
        EXPRESSION(tokenVariable),         // X
        EXPRESSION(tokenVariable),         // Y
        EXPRESSION(tokenVariable),         // Z
        EXPRESSION(tokenVariable),         // theta
        EXPRESSION(tokenOSMatrix),         // 2-byte token (Matrices)
        EXPRESSION(tokenOSList),           // 2-byte token (Lists)
        EXPRESSION(tokenOsEqu),            // 2-byte token
        tokenUnimplemented,                // prgm
        tokenUnimplemented,                // 2-byte token (Pictures)
        tokenUnimplemented,                // 2-byte token (GDBs)
        tokenUnimplemented,                // 2-byte token (Statistics)
        tokenUnimplemented,                // 2-byte token (Window)
        tokenCommandStandalone,           // Radian
        tokenCommandStandalone,           // Degree
        tokenCommandStandalone,           // Normal
        tokenCommandStandalone,           // Sci
        tokenCommandStandalone,           // Eng
        tokenCommandStandalone,           // Float
        EXPRESSION(tokenOperator),        // =
        EXPRESSION(tokenOperator),        // <
        EXPRESSION(tokenOperator),        // >
        EXPRESSION(tokenOperator),        // <=
        EXPRESSION(tokenOperator),        // >=
        EXPRESSION(tokenOperator),        // !=
        EXPRESSION(tokenOperator),        // +
        EXPRESSION(tokenOperator),        // - (sub)
        EXPRESSION(tokenEmptyFunc),       // Ans
        tokenUnimplemented,               // Fix
        tokenCommandStandalone,           // Horiz
        tokenCommandStandalone,           // Full
        tokenCommandStandalone,           // Func
        tokenCommandStandalone,           // Param
        tokenCommandStandalone,           // Polar
        tokenCommandStandalone,           // Seq
        tokenCommandStandalone,           // IndpntAuto
        tokenCommandStandalone,           // IndpntAsk
        tokenCommandStandalone,           // DependAuto
        tokenCommandStandalone,           // DependAsk
        tokenUnimplemented,               // 2-byte token
        tokenUnimplemented,               // square mark
        tokenUnimplemented,               // plus mark
        tokenUnimplemented,               // dot mark
        EXPRESSION(tokenOperator),        // *
        EXPRESSION(tokenOperator),        // /
        tokenCommandStandalone,           // Trace
        tokenCommandStandalone,           // ClrDraw
        tokenCommandStandalone,           // ZStandard
        tokenCommandStandalone,           // ZTrig
        tokenCommandStandalone,           // ZBox
        tokenCommandStandalone,           // Zoom In
        tokenCommandStandalone,           // Zoom Out
        tokenCommandStandalone,           // ZSquare
        tokenCommandStandalone,           // ZInteger
        tokenCommandStandalone,           // ZPrevious
        tokenCommandStandalone,           // ZDecimal
        tokenCommandStandalone,           // ZoomStat
        tokenCommandStandalone,           // ZoomRcl
        tokenUnimplemented,               // PrintScreen
        tokenCommandStandalone,           // ZoomSto
        tokenCommandParen,                // Text(
        EXPRESSION(tokenOperator),        // nPr
        EXPRESSION(tokenOperator),        // nCr
        tokenUnimplemented,               // FnOn
        tokenUnimplemented,               // FnOff
        tokenUnimplemented,               // StorePic
        tokenUnimplemented,               // RecallPic
        tokenUnimplemented,               // StoreGDB
        tokenUnimplemented,               // RecallGDB
        tokenCommandParen,                // Line(
        tokenUnimplemented,               // Vertical
        tokenCommandParen,                // Pt-On(
        tokenCommandParen,                // Pt-Off(
        tokenCommandParen,                // Pt-Change(
        tokenCommandParen,                // Pxl-On(
        tokenCommandParen,                // Pxl-Off(
        tokenCommandParen,                // Pxl-Change(
        tokenCommandParen,                // Shade(
        tokenCommandParen,                // Circle(
        tokenUnimplemented,               // Horizontal
        tokenCommandParen,                // Tangent(
        tokenUnimplemented,               // DrawInv
        tokenUnimplemented,               // DrawF
        EXPRESSION(tokenOsString),        // 2-byte token (String)
        EXPRESSION(tokenRand),            // rand
        EXPRESSION(tokenPi),              // pi
        EXPRESSION(tokenEmptyFunc),       // getKey
        tokenUnimplemented,               // '
        tokenUnimplemented,               // ?
        EXPRESSION(tokenOperator),        // - (neg)
        EXPRESSION(tokenFunction),        // int(
        EXPRESSION(tokenFunction),        // abs(
        EXPRESSION(tokenFunction),        // det(
        EXPRESSION(tokenFunction),        // identity(
        EXPRESSION(tokenFunction),        // dim(
        EXPRESSION(tokenFunction),        // sum(
        EXPRESSION(tokenFunction),        // prod(
        EXPRESSION(tokenFunction),        // not(
        EXPRESSION(tokenFunction),        // iPart(
        EXPRESSION(tokenFunction),        // fPart(
        tokenUnimplemented,               // 2-byte token
        EXPRESSION(tokenFunction),        // √(
        EXPRESSION(tokenFunction),        // ³√(
        EXPRESSION(tokenFunction),        // ln(
        EXPRESSION(tokenFunction),        // e^(
        EXPRESSION(tokenFunction),        // log(
        EXPRESSION(tokenFunction),        // 10^(
        EXPRESSION(tokenFunction),        // sin(
        EXPRESSION(tokenFunction),        // sinֿ¹(
        EXPRESSION(tokenFunction),        // cos(
        EXPRESSION(tokenFunction),        // cosֿ¹(
        EXPRESSION(tokenFunction),        // tan(
        EXPRESSION(tokenFunction),        // tanֿ¹(
        EXPRESSION(tokenFunction),        // sinh(
        EXPRESSION(tokenFunction),        // sihֿ¹(
        EXPRESSION(tokenFunction),        // cosh(
        EXPRESSION(tokenFunction),        // coshֿ¹(
        EXPRESSION(tokenFunction),        // tanh(
        EXPRESSION(tokenFunction),        // tanhֿ¹(
        tokenUnimplemented,               // If
        tokenUnimplemented,               // Then
        tokenUnimplemented,               // Else
        tokenUnimplemented,               // While
        tokenUnimplemented,               // Repeat
        tokenUnimplemented,               // For(
        tokenUnimplemented,               // End
        tokenCommandStandalone,           // Return
        tokenUnimplemented,               // Lbl
        tokenUnimplemented,               // Goto
        tokenUnimplemented,               // Pause
        tokenCommandStandalone,           // Stop
        tokenUnimplemented,               // IS>(
        tokenUnimplemented,               // DS<(
        tokenUnimplemented,               // Input
        tokenUnimplemented,               // Prompt
        tokenCommandArgs,                 // Disp
        tokenCommandStandalone,           // DispGraph
        tokenCommandParen,                // Output(
        tokenCommandStandalone,           // ClrHome
        tokenCommandParen,                // Fill(
        tokenCommandParen,                // SortA(
        tokenCommandParen,                // SortD(
        tokenCommandStandalone,           // DispTable
        tokenCommandParen,                // Menu(
        tokenCommandParen,                // Send(
        tokenCommandParen,                // Get(
        tokenUnimplemented,               // PlotsOn
        tokenUnimplemented,               // PlotsOff
        tokenUnimplemented,               // ∟
        tokenCommandParen,                // Plot1(
        tokenCommandParen,                // Plot2(
        tokenCommandParen,                // Plot3(
        tokenUnimplemented,               // 2-byte token
        EXPRESSION(tokenOperator),        // ^
        tokenUnimplemented,               // ×√
        tokenUnimplemented,               // 1-Var Stats
        tokenUnimplemented,               // 2-Var Stats
        tokenUnimplemented,               // LinReg(a+bx)
        tokenUnimplemented,               // ExpReg
        tokenUnimplemented,               // LnReg
        tokenUnimplemented,               // PwrReg
        tokenUnimplemented,               // Med-Med
        tokenUnimplemented,               // QuadReg
        tokenUnimplemented,               // ClrList
        tokenUnimplemented,               // ClrTable
        tokenUnimplemented,               // Histogram
        tokenUnimplemented,               // xyLine
        tokenUnimplemented,               // Scatter
        tokenUnimplemented                // LinReg(ax+b)
};

#undef EXPRESSION
