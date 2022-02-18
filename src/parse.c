#include "parse.h"
#include "ast.h"
#include "expression.h"
#include "routines.h"

#include <fileioc.h>
#include <string.h>

static struct NODE *(*functions[256])(ti_var_t slot, int token);

/**
 * This function parses the entire program, reading it line by line
 * @param slot fileioc slot to read the data from
 * @param expect_end Boolean to allow stopping the program at "End", which is after a loop/statement
 * @param expect_else Boolean to allow stopping the program at "Else", which is inside an If-statement
 * @return Node to start parsing at (i.e. the root)
 */
struct NODE *parse_full_program(ti_var_t slot, bool expect_end, bool expect_else) {
    int token;
    struct NODE *root = NULL;
    struct NODE *tail = NULL;

    while ((token = next_token(slot)) != EOF) {
        if (boot_CheckOnPressed()) parse_error("[ON]-key pressed");

        // Skip if's a colon
        if (token == tColon) {
            continue;
        }

        // Return if we hit an Else/End, but only if it's valid!
        if ((token == tEnd && expect_end) || (token == tElse && expect_else))
            return root;

        // Call the function
        struct NODE *new_node = (*functions[token])(slot, token);

        // Advance line and column
        parse_line++;
        parse_col = 0;

        // Need to insert it?
        if (new_node == NULL)
            continue;

        // Insert it to the chain
        if (root == NULL) {
            root = tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    return root;
}

static struct NODE *standalone_func(ti_var_t slot, int token) {
    if (!is_end_of_line(peek_token(slot))) parse_error("Syntax error");

    struct NODE *func_node = node_alloc(ET_FUNCTION_CALL);
    func_node->data.operand.func = token;

    return func_node;
}

static struct NODE *parse_command(ti_var_t slot, int token, bool end_paren) {
    struct NODE *command_node = node_alloc(ET_COMMAND);
    command_node->data.operand.command = token;

    struct NODE *tree = NULL;

    for (;;) {
        token = next_token(slot);

        // Get the next expression
        struct NODE *expr = parse_expression_line(slot, token, true, end_paren);

        if (tree == NULL) {
            tree = command_node->child = expr;
        } else {
            tree->next = expr;
            tree = tree->next;
        }

        token = current_token();

        // There are a few stop cases possible:
        //  - EOF, enter or a colon. In that case, get back to the previous token to handle the colon/newline properly
        //  - Comma, in which case we just continue with parsing the next expression
        //  - Right parenthesis, which should be the last token on a line
        if (is_end_of_line(token)) {
            seek_prev(slot);

            break;
        }

        if (end_paren && token == tRParen) {
            if (!is_end_of_line(peek_token(slot))) parse_error("Syntax error");

            break;
        }
    }

    return command_node;
}

static struct NODE *command_args(ti_var_t slot, int token) {
    return parse_command(slot, token, false);
}

static struct NODE *command_paren(ti_var_t slot, int token) {
    return parse_command(slot, token, true);
}

static struct NODE *token_newline(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    return NULL;
}

static struct NODE *token_unimplemented(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    parse_error("Token not implemented");
}

static struct NODE *(*functions[256])(ti_var_t, int) = {
        token_unimplemented, // **unused**
        token_unimplemented, // ►DMS
        token_unimplemented, // ►Dec
        token_unimplemented, // ►Frac
        token_unimplemented, // →
        token_unimplemented, // Boxplot
        token_expression,    // [
        token_expression,    // ]
        token_expression,    // {
        token_expression,    // }
        token_expression,    // r
        token_expression,    // °
        token_expression,    // ֿ¹
        token_expression,    // ²
        token_expression,    // T
        token_expression,    // ³
        token_expression,    // (
        token_unimplemented, // )
        token_expression,    // round(
        token_expression,    // pxl-Test(
        token_expression,    // augment(
        token_expression,    // rowSwap(
        token_expression,    // row+(
        token_expression,    // *row(
        token_expression,    // *row+(
        token_expression,    // max(
        token_expression,    // min(
        token_expression,    // R►Pr(
        token_expression,    // R►Pθ(
        token_expression,    // R►Px(
        token_expression,    // R►Py(
        token_expression,    // median(
        token_expression,    // randM(
        token_expression,    // mean(
        token_expression,    // solve(
        token_expression,    // seq(
        token_expression,    // fnInt(
        token_expression,    // nDeriv(
        token_unimplemented, // **unused**
        token_expression,    // fMin(
        token_expression,    // fMax(
        token_unimplemented, //
        token_expression,    // "
        token_unimplemented, // ,
        token_expression,    // i
        token_expression,    // !
        token_unimplemented, // CubicReg
        token_unimplemented, // QuartReg
        token_expression,    // 0
        token_expression,    // 1
        token_expression,    // 2
        token_expression,    // 3
        token_expression,    // 4
        token_expression,    // 5
        token_expression,    // 6
        token_expression,    // 7
        token_expression,    // 8
        token_expression,    // 9
        token_expression,    // .
        token_expression,    // |E
        token_expression,    // or
        token_expression,    // xor
        token_expression,    // :
        token_newline,       // \n
        token_expression,    // and
        token_expression,    // A
        token_expression,    // B
        token_expression,    // C
        token_expression,    // D
        token_expression,    // E
        token_expression,    // F
        token_expression,    // G
        token_expression,    // H
        token_expression,    // I
        token_expression,    // J
        token_expression,    // K
        token_expression,    // L
        token_expression,    // M
        token_expression,    // N
        token_expression,    // O
        token_expression,    // P
        token_expression,    // Q
        token_expression,    // R
        token_expression,    // S
        token_expression,    // T
        token_expression,    // U
        token_expression,    // V
        token_expression,    // W
        token_expression,    // X
        token_expression,    // Y
        token_expression,    // Z
        token_expression,    // theta
        token_expression,    // 2-byte token
        token_expression,    // 2-byte token
        token_expression,    // 2-byte token
        token_unimplemented, // prgm
        token_unimplemented, // 2-byte token
        token_unimplemented, // 2-byte token
        token_unimplemented, // 2-byte token
        token_unimplemented, // 2-byte token
        standalone_func,     // Radian
        standalone_func,     // Degree
        standalone_func,     // Normal
        standalone_func,     // Sci
        standalone_func,     // Eng
        standalone_func,     // Float
        token_expression,    // =
        token_expression,    // <
        token_expression,    // >
        token_expression,    // <=
        token_expression,    // >=
        token_expression,    // !=
        token_expression,    // +
        token_expression,    // - (sub)
        token_expression,    // Ans
        token_unimplemented, // Fix
        standalone_func,     // Horiz
        standalone_func,     // Full
        standalone_func,     // Func
        standalone_func,     // Param
        standalone_func,     // Polar
        standalone_func,     // Seq
        standalone_func,     // IndpntAuto
        standalone_func,     // IndpntAsk
        standalone_func,     // DependAuto
        standalone_func,     // DependAsk
        token_unimplemented, // 2-byte token
        token_unimplemented, // square mark
        token_unimplemented, // plus mark
        token_unimplemented, // dot mark
        token_expression,    // *
        token_expression,    // /
        standalone_func,     // Trace
        standalone_func,     // ClrDraw
        standalone_func,     // ZStandard
        standalone_func,     // ZTrig
        standalone_func,     // ZBox
        standalone_func,     // Zoom In
        standalone_func,     // Zoom Out
        standalone_func,     // ZSquare
        standalone_func,     // ZInteger
        standalone_func,     // ZPrevious
        standalone_func,     // ZDecimal
        standalone_func,     // ZoomStat
        standalone_func,     // ZoomRcl
        token_unimplemented, // PrintScreen
        standalone_func,     // ZoomSto
        command_paren,       // Text(
        token_expression,    // nPr
        token_expression,    // nCr
        token_unimplemented, // FnOn
        token_unimplemented, // FnOff
        token_unimplemented, // StorePic
        token_unimplemented, // RecallPic
        token_unimplemented, // StoreGDB
        token_unimplemented, // RecallGDB
        command_paren,       // Line(
        token_unimplemented, // Vertical
        command_paren,       // Pt-On(
        command_paren,       // Pt-Off(
        command_paren,       // Pt-Change(
        command_paren,       // Pxl-On(
        command_paren,       // Pxl-Off(
        command_paren,       // Pxl-Change(
        command_paren,       // Shade(
        command_paren,       // Circle(
        token_unimplemented, // Horizontal
        command_paren,       // Tangent(
        token_unimplemented, // DrawInv
        token_unimplemented, // DrawF
        token_expression,    // 2-byte token
        token_expression,    // rand
        token_expression,    // pi
        token_expression,    // getKey
        token_unimplemented, // '
        token_unimplemented, // ?
        token_expression,    // - (neg)
        token_expression,    // int(
        token_expression,    // abs(
        token_expression,    // det(
        token_expression,    // identity(
        token_expression,    // dim(
        token_expression,    // sum(
        token_expression,    // prod(
        token_expression,    // not(
        token_expression,    // iPart(
        token_expression,    // fPart(
        token_unimplemented, // 2-byte token
        token_expression,    // √(
        token_expression,    // ³√(
        token_expression,    // ln(
        token_expression,    // e^(
        token_expression,    // log(
        token_expression,    // 10^(
        token_expression,    // sin(
        token_expression,    // sinֿ¹(
        token_expression,    // cos(
        token_expression,    // cosֿ¹(
        token_expression,    // tan(
        token_expression,    // tanֿ¹(
        token_expression,    // sinh(
        token_expression,    // sihֿ¹(
        token_expression,    // cosh(
        token_expression,    // coshֿ¹(
        token_expression,    // tanh(
        token_expression,    // tanhֿ¹(
        token_unimplemented, // If
        token_unimplemented, // Then
        token_unimplemented, // Else
        token_unimplemented, // While
        token_unimplemented, // Repeat
        token_unimplemented, // For(
        token_unimplemented, // End
        standalone_func,     // Return
        token_unimplemented, // Lbl
        token_unimplemented, // Goto
        token_unimplemented, // Pause
        standalone_func,     // Stop
        token_unimplemented, // IS>(
        token_unimplemented, // DS<(
        token_unimplemented, // Input
        token_unimplemented, // Prompt
        command_args,        // Disp
        standalone_func,     // DispGraph
        command_paren,       // Output(
        standalone_func,     // ClrHome
        command_paren,       // Fill(
        command_paren,       // SortA(
        command_paren,       // SortD(
        standalone_func,     // DispTable
        command_paren,       // Menu(
        command_paren,       // Send(
        command_paren,       // Get(
        token_unimplemented, // PlotsOn
        token_unimplemented, // PlotsOff
        token_unimplemented, // ∟
        command_paren,       // Plot1(
        command_paren,       // Plot2(
        command_paren,       // Plot3(
        token_unimplemented, // 2-byte token
        token_expression,    // ^
        token_unimplemented, // ×√
        token_unimplemented, // 1-Var Stats
        token_unimplemented, // 2-Var Stats
        token_unimplemented, // LinReg(a+bx)
        token_unimplemented, // ExpReg
        token_unimplemented, // LnReg
        token_unimplemented, // PwrReg
        token_unimplemented, // Med-Med
        token_unimplemented, // QuadReg
        token_unimplemented, // ClrList
        token_unimplemented, // ClrTable
        token_unimplemented, // Histogram
        token_unimplemented, // xyLine
        token_unimplemented, // Scatter
        token_unimplemented  // LinReg(ax+b)
};
