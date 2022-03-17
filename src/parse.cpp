#include "parse.h"
#include "ast.h"
#include "errors.h"
#include "expression.h"
#include "utils.h"

#include <fileioc.h>
#include <keypadc.h>

extern struct NODE *(*parseFunctions[256])(ti_var_t, int);

unsigned int parseLine = 1;
unsigned int parseCol = 0;

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

        // Call the function
        auto node = (*parseFunctions[token])(slot, token);

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

static struct NODE *tokenUnimplemented(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    parseError("Token not implemented");
}

struct NODE *(*parseFunctions[256])(ti_var_t, int) = {
        tokenUnimplemented,      // **unused**
        tokenUnimplemented,      // ►DMS
        tokenUnimplemented,      // ►Dec
        tokenUnimplemented,      // ►Frac
        tokenUnimplemented,      // →
        tokenUnimplemented,      // Boxplot
        tokenExpression,         // [
        tokenExpression,         // ]
        tokenExpression,         // {
        tokenExpression,         // }
        tokenExpression,         // r
        tokenExpression,         // °
        tokenExpression,         // ֿ¹
        tokenExpression,         // ²
        tokenExpression,         // T
        tokenExpression,         // ³
        tokenExpression,         // (
        tokenUnimplemented,      // )
        tokenExpression,         // round(
        tokenExpression,         // pxl-Test(
        tokenExpression,         // augment(
        tokenExpression,         // rowSwap(
        tokenExpression,         // row+(
        tokenExpression,         // *row(
        tokenExpression,         // *row+(
        tokenExpression,         // max(
        tokenExpression,         // min(
        tokenExpression,         // R►Pr(
        tokenExpression,         // R►Pθ(
        tokenExpression,         // R►Px(
        tokenExpression,         // R►Py(
        tokenExpression,         // median(
        tokenExpression,         // randM(
        tokenExpression,         // mean(
        tokenExpression,         // solve(
        tokenExpression,         // seq(
        tokenExpression,         // fnInt(
        tokenExpression,         // nDeriv(
        tokenUnimplemented,      // **unused**
        tokenExpression,         // fMin(
        tokenExpression,         // fMax(
        tokenUnimplemented,      //
        tokenExpression,         // "
        tokenUnimplemented,      // ,
        tokenExpression,         // i
        tokenExpression,         // !
        tokenUnimplemented,      // CubicReg
        tokenUnimplemented,      // QuartReg
        tokenExpression,         // 0
        tokenExpression,         // 1
        tokenExpression,         // 2
        tokenExpression,         // 3
        tokenExpression,         // 4
        tokenExpression,         // 5
        tokenExpression,         // 6
        tokenExpression,         // 7
        tokenExpression,         // 8
        tokenExpression,         // 9
        tokenExpression,         // .
        tokenExpression,         // |E
        tokenExpression,         // or
        tokenExpression,         // xor
        tokenExpression,         // :
        tokenNewline,            // \n
        tokenExpression,         // and
        tokenExpression,         // A
        tokenExpression,         // B
        tokenExpression,         // C
        tokenExpression,         // D
        tokenExpression,         // E
        tokenExpression,         // F
        tokenExpression,         // G
        tokenExpression,         // H
        tokenExpression,         // I
        tokenExpression,         // J
        tokenExpression,         // K
        tokenExpression,         // L
        tokenExpression,         // M
        tokenExpression,         // N
        tokenExpression,         // O
        tokenExpression,         // P
        tokenExpression,         // Q
        tokenExpression,         // R
        tokenExpression,         // S
        tokenExpression,         // T
        tokenExpression,         // U
        tokenExpression,         // V
        tokenExpression,         // W
        tokenExpression,         // X
        tokenExpression,         // Y
        tokenExpression,         // Z
        tokenExpression,         // theta
        tokenExpression,         // 2-byte token
        tokenExpression,         // 2-byte token
        tokenExpression,         // 2-byte token
        tokenUnimplemented,      // prgm
        tokenUnimplemented,      // 2-byte token
        tokenUnimplemented,      // 2-byte token
        tokenUnimplemented,      // 2-byte token
        tokenUnimplemented,      // 2-byte token
        tokenCommandStandalone,  // Radian
        tokenCommandStandalone,  // Degree
        tokenCommandStandalone,  // Normal
        tokenCommandStandalone,  // Sci
        tokenCommandStandalone,  // Eng
        tokenCommandStandalone,  // Float
        tokenExpression,         // =
        tokenExpression,         // <
        tokenExpression,         // >
        tokenExpression,         // <=
        tokenExpression,         // >=
        tokenExpression,         // !=
        tokenExpression,         // +
        tokenExpression,         // - (sub)
        tokenExpression,         // Ans
        tokenUnimplemented,      // Fix
        tokenCommandStandalone,  // Horiz
        tokenCommandStandalone,  // Full
        tokenCommandStandalone,  // Func
        tokenCommandStandalone,  // Param
        tokenCommandStandalone,  // Polar
        tokenCommandStandalone,  // Seq
        tokenCommandStandalone,  // IndpntAuto
        tokenCommandStandalone,  // IndpntAsk
        tokenCommandStandalone,  // DependAuto
        tokenCommandStandalone,  // DependAsk
        tokenUnimplemented,      // 2-byte token
        tokenUnimplemented,      // square mark
        tokenUnimplemented,      // plus mark
        tokenUnimplemented,      // dot mark
        tokenExpression,         // *
        tokenExpression,         // /
        tokenCommandStandalone,  // Trace
        tokenCommandStandalone,  // ClrDraw
        tokenCommandStandalone,  // ZStandard
        tokenCommandStandalone,  // ZTrig
        tokenCommandStandalone,  // ZBox
        tokenCommandStandalone,  // Zoom In
        tokenCommandStandalone,  // Zoom Out
        tokenCommandStandalone,  // ZSquare
        tokenCommandStandalone,  // ZInteger
        tokenCommandStandalone,  // ZPrevious
        tokenCommandStandalone,  // ZDecimal
        tokenCommandStandalone,  // ZoomStat
        tokenCommandStandalone,  // ZoomRcl
        tokenUnimplemented,      // PrintScreen
        tokenCommandStandalone,  // ZoomSto
        tokenCommandParen,       // Text(
        tokenExpression,         // nPr
        tokenExpression,         // nCr
        tokenUnimplemented,      // FnOn
        tokenUnimplemented,      // FnOff
        tokenUnimplemented,      // StorePic
        tokenUnimplemented,      // RecallPic
        tokenUnimplemented,      // StoreGDB
        tokenUnimplemented,      // RecallGDB
        tokenCommandParen,       // Line(
        tokenUnimplemented,      // Vertical
        tokenCommandParen,       // Pt-On(
        tokenCommandParen,       // Pt-Off(
        tokenCommandParen,       // Pt-Change(
        tokenCommandParen,       // Pxl-On(
        tokenCommandParen,       // Pxl-Off(
        tokenCommandParen,       // Pxl-Change(
        tokenCommandParen,       // Shade(
        tokenCommandParen,       // Circle(
        tokenUnimplemented,      // Horizontal
        tokenCommandParen,       // Tangent(
        tokenUnimplemented,      // DrawInv
        tokenUnimplemented,      // DrawF
        tokenExpression,         // 2-byte token
        tokenExpression,         // rand
        tokenExpression,         // pi
        tokenExpression,         // getKey
        tokenUnimplemented,      // '
        tokenUnimplemented,      // ?
        tokenExpression,         // - (neg)
        tokenExpression,         // int(
        tokenExpression,         // abs(
        tokenExpression,         // det(
        tokenExpression,         // identity(
        tokenExpression,         // dim(
        tokenExpression,         // sum(
        tokenExpression,         // prod(
        tokenExpression,         // not(
        tokenExpression,         // iPart(
        tokenExpression,         // fPart(
        tokenUnimplemented,      // 2-byte token
        tokenExpression,         // √(
        tokenExpression,         // ³√(
        tokenExpression,         // ln(
        tokenExpression,         // e^(
        tokenExpression,         // log(
        tokenExpression,         // 10^(
        tokenExpression,         // sin(
        tokenExpression,         // sinֿ¹(
        tokenExpression,         // cos(
        tokenExpression,         // cosֿ¹(
        tokenExpression,         // tan(
        tokenExpression,         // tanֿ¹(
        tokenExpression,         // sinh(
        tokenExpression,         // sihֿ¹(
        tokenExpression,         // cosh(
        tokenExpression,         // coshֿ¹(
        tokenExpression,         // tanh(
        tokenExpression,         // tanhֿ¹(
        tokenUnimplemented,      // If
        tokenUnimplemented,      // Then
        tokenUnimplemented,      // Else
        tokenUnimplemented,      // While
        tokenUnimplemented,      // Repeat
        tokenUnimplemented,      // For(
        tokenUnimplemented,      // End
        tokenCommandStandalone,  // Return
        tokenUnimplemented,      // Lbl
        tokenUnimplemented,      // Goto
        tokenUnimplemented,      // Pause
        tokenCommandStandalone,  // Stop
        tokenUnimplemented,      // IS>(
        tokenUnimplemented,      // DS<(
        tokenUnimplemented,      // Input
        tokenUnimplemented,      // Prompt
        tokenCommandArgs,        // Disp
        tokenCommandStandalone,  // DispGraph
        tokenCommandParen,       // Output(
        tokenCommandStandalone,  // ClrHome
        tokenCommandParen,       // Fill(
        tokenCommandParen,       // SortA(
        tokenCommandParen,       // SortD(
        tokenCommandStandalone,  // DispTable
        tokenCommandParen,       // Menu(
        tokenCommandParen,       // Send(
        tokenCommandParen,       // Get(
        tokenUnimplemented,      // PlotsOn
        tokenUnimplemented,      // PlotsOff
        tokenUnimplemented,      // ∟
        tokenCommandParen,       // Plot1(
        tokenCommandParen,       // Plot2(
        tokenCommandParen,       // Plot3(
        tokenUnimplemented,      // 2-byte token
        tokenExpression,         // ^
        tokenUnimplemented,      // ×√
        tokenUnimplemented,      // 1-Var Stats
        tokenUnimplemented,      // 2-Var Stats
        tokenUnimplemented,      // LinReg(a+bx)
        tokenUnimplemented,      // ExpReg
        tokenUnimplemented,      // LnReg
        tokenUnimplemented,      // PwrReg
        tokenUnimplemented,      // Med-Med
        tokenUnimplemented,      // QuadReg
        tokenUnimplemented,      // ClrList
        tokenUnimplemented,      // ClrTable
        tokenUnimplemented,      // Histogram
        tokenUnimplemented,      // xyLine
        tokenUnimplemented,      // Scatter
        tokenUnimplemented       // LinReg(ax+b)
};
