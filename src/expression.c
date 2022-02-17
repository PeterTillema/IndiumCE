#include "expression.h"
#include "ast.h"
#include "main.h"
#include "operators.h"
#include "routines.h"
#include "variables.h"

#include <debug.h>
#include <fileioc.h>
#include <fontlibc.h>
#include <math.h>
#include <string.h>
#include <tice.h>

static struct NODE *output_stack[500];
static struct NODE *op_stack[100];
static unsigned int output_stack_nr;
static unsigned int op_stack_nr;
static uint8_t nested_funcs = 0;
static bool need_mul_op;

unsigned int parse_line = 1;
unsigned int parse_col = 0;

static void (*functions[256])(ti_var_t slot, int token);

void parse_error(const char *string) {
    char buf[26];

    fontlib_DrawStringL(string, 26);
    fontlib_Newline();

    sprintf(buf, "Line %d column %d", parse_line, parse_col);
    fontlib_DrawString(buf);

    // And exit the program
    force_exit();
}

static void print_node(struct NODE *tree, uint8_t depth) { // NOLINT(misc-no-recursion)
    while (tree != NULL) {
        for (uint8_t i = 0; i < depth; i++) dbg_sprintf(dbgout, "  ");

        switch (tree->data.type) {
            case ET_NUM: // NOLINT(bugprone-branch-clone)
                dbg_sprintf(dbgout, "Number: %f\n", tree->data.operand.num);
                break;
            case ET_COMPLEX:
                dbg_sprintf(dbgout, "Complex: %f + %f\n", tree->data.operand.cplx_ptr->real,
                            tree->data.operand.cplx_ptr->imag);
                break;
            case ET_VARIABLE:
                dbg_sprintf(dbgout, "Variable: %c\n", tree->data.operand.variable_nr + 'A');
                break;
            case ET_STRING:
                dbg_sprintf(dbgout, "String Str%d\n", tree->data.operand.string_nr);
                break;
            case ET_EQU:
                dbg_sprintf(dbgout, "Equation %d\n", tree->data.operand.equation_nr);
                break;
            case ET_LIST:
                dbg_sprintf(dbgout, "List L%d\n", tree->data.operand.list_nr);
                break;
            case ET_MATRIX:
                dbg_sprintf(dbgout, "Matrix [%c]\n", tree->data.operand.matrix_nr + 'A');
                break;
            case ET_OPERATOR:
                dbg_sprintf(dbgout, "Operator: %s\n", get_op_string(tree->data.operand.op));
                break;
            case ET_FUNCTION_CALL:
                dbg_sprintf(dbgout, "Function: %04X\n", tree->data.operand.func);
                break;
            default:
                break;
        }

        if (tree->data.type != ET_FUNCTION_CALL || tree->data.operand.func != tString)
            print_node(tree->child, depth + 1);

        tree = tree->next;
    }
}

static void add_to_output(struct NODE *tmp) {
    if (output_stack_nr == 500) parse_error("Memory error");

    output_stack[output_stack_nr++] = tmp;
}

static void add_to_stack(struct NODE *tmp) {
    if (op_stack_nr == 100) parse_error("Memory error");

    op_stack[op_stack_nr++] = tmp;
}

static void push_op(uint8_t precedence, int token) {
    while (op_stack_nr) {
        // Previous element on the stack should be an operator
        struct NODE *prev = op_stack[op_stack_nr - 1];
        if (prev->data.type != ET_OPERATOR) break;

        // Check if we need to move the previous operator to the output stack
        uint8_t prev_precedence = get_operator_precedence(prev->data.operand.op);
        if (prev_precedence > precedence ||
            (prev_precedence == precedence && (token == tPower || token == tComma)))
            break;

        // Check for unary operator and set the args of the operator
        if (is_unary_op(prev_precedence)) {
            if (!output_stack_nr) parse_error("Syntax error");

            prev->child = output_stack[output_stack_nr - 1];

            output_stack[output_stack_nr - 1] = prev;
        } else {
            if (output_stack_nr < 2) parse_error("Syntax error");

            prev->child = output_stack[output_stack_nr - 2];
            prev->child->next = output_stack[output_stack_nr - 1];

            output_stack[output_stack_nr - 2] = prev;
            output_stack_nr--;
        }

        op_stack_nr--;
    }
}

static void push_rparen(uint8_t tok) {
    uint8_t arg_count = 1;

    nested_funcs--;

    // Search for the matching left parenthesis/bracket. Everything we encounter can only be a comma, which is used in a
    // function. If any comma is found, it must be function, as (1, 2) is invalid.
    for (unsigned int i = op_stack_nr; i-- > 0;) {
        struct NODE *tmp = op_stack[i];

        if (tmp->data.type == ET_FUNCTION_CALL && tmp->data.operand.func == tok) {
            // This is the closing } or ) which is a single function without an extra parenthesis
            if (arg_count <= output_stack_nr && (tok == tLBrace || tok == tLBrack)) {
                struct NODE *tree = tmp->child = output_stack[output_stack_nr - arg_count];

                // Set the arguments of the function
                for (uint8_t j = 1; j < arg_count; j++) {
                    tree->next = output_stack[output_stack_nr - arg_count + j];
                    tree = tree->next;
                }

                output_stack_nr -= arg_count;
                op_stack_nr--;

                // Insert the function in the output queue
                add_to_output(tmp);

                return;
            } else if (i && arg_count <= output_stack_nr && op_stack[i - 1]->data.type == ET_FUNCTION_CALL && op_stack[i - 1]->data.operand.func != tLParen) {
                // This is a real function, like sin or cos. Free the parenthesis, and set all arguments from the
                // output queue as the children of this function.
                struct NODE *func_node = op_stack[i - 1];
                struct NODE *tree = func_node->child = output_stack[output_stack_nr - arg_count];

                free(tmp);

                // Set the arguments of the function
                for (uint8_t j = 1; j < arg_count; j++) {
                    tree->next = output_stack[output_stack_nr - arg_count + j];
                    tree = tree->next;
                }

                output_stack_nr -= arg_count;
                op_stack_nr -= 2;

                // Insert the function in the output queue
                add_to_output(func_node);

                return;
            } else if (arg_count == 1) {
                // It is a standalone parenthesis, it should have only 1 argument. Only free the stack entry, as the
                // last output queue item is already the correct one.
                free(tmp);
                op_stack_nr--;

                return;
            } else {
                break;
            }
        } else if (tmp->data.type == ET_OPERATOR && tmp->data.operand.op == tComma) {
            arg_count++;
            op_stack_nr--;

            free(tmp);
        } else {
            break;
        }
    }

    parse_error("Unexpected \")\"");
}

static void empty_op_stack(void) {
    for (unsigned int i = op_stack_nr; i-- > 0;) {
        if (i >= op_stack_nr) continue;

        struct NODE *tmp = op_stack[i];

        if (tmp->data.type == ET_OPERATOR) push_op(MAX_PRECEDENCE + 1, MAX_PRECEDENCE + 1);
        else if (tmp->data.type == ET_FUNCTION_CALL) push_rparen(tLParen);
    }

    if (output_stack_nr != 1) parse_error("Invalid expression");
}

struct NODE *token_expression(ti_var_t slot, int token) {
    return parse_expression_line(slot, token, false, false);
}

struct NODE *parse_expression_line(ti_var_t slot, int token, bool stop_at_comma, bool stop_at_paren) {
    // Reset expression things
    // memset(&output_stack, 0, sizeof(output_stack));
    // memset(&op_stack, 0, sizeof(op_stack));
    output_stack_nr = 0;
    op_stack_nr = 0;
    need_mul_op = false;

    while (token != EOF && token != tEnter && token != tColon) {
        if (token == tComma && stop_at_comma && !nested_funcs) break;
        if (token == tRParen && stop_at_paren && !nested_funcs) break;

        if (boot_CheckOnPressed()) parse_error("[ON]-key pressed");

        // Execute the token function
        (*functions[token])(slot, token);

        // And get the new token
        token = next_token(slot);
    }

    empty_op_stack();

    print_node(output_stack[0], 0);

    return output_stack[0];
}

static void token_operator(__attribute__((unused)) ti_var_t slot, int token) {
    need_mul_op = false;

    if (token == tStore) {
        // Multiple stores are not implemented
        if (op_stack_nr && op_stack[0]->data.type == ET_OPERATOR && op_stack[0]->data.operand.op == tStore)
            parse_error("Syntax error");

        empty_op_stack();
    }

    // Push the operator to the stack
    uint8_t op_precedence = get_operator_precedence(token);
    push_op(op_precedence, token);

    // And allocate memory for the new operator
    struct NODE *op_node = node_alloc(ET_OPERATOR);
    op_node->data.operand.op = token;

    add_to_stack(op_node);
}

static void token_rbrack(ti_var_t slot, int token) {
    need_mul_op = true;

    push_op(MAX_PRECEDENCE + 1, token);
    push_rparen(tLBrack);

    // Check if a "[" is coming, in which case we need an extra comma
    token = peek_token(slot);
    if ((uint8_t) token == tLBrack) {
        token_operator(slot, tComma);
    }
}

static void token_rbrace(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    need_mul_op = true;

    push_op(MAX_PRECEDENCE + 1, token);
    push_rparen(tLBrace);
}

static void token_rparen(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    need_mul_op = true;

    // This forces all operators to be moved to the output stack
    // After that, push the right parenthesis
    push_op(MAX_PRECEDENCE + 1, token);
    push_rparen(tLParen);
}

static void token_function(ti_var_t slot, int token) {
    if (need_mul_op) token_operator(slot, tMul);

    // Allocate space for the function
    struct NODE *func_node = node_alloc(ET_FUNCTION_CALL);
    func_node->data.operand.func = token;

    add_to_stack(func_node);
    nested_funcs++;

    if (token != tLParen && token != tLBrace && token != tLBrack) {
        // Eventually push an extra (
        struct NODE *paren_node = node_alloc(ET_FUNCTION_CALL);
        paren_node->data.operand.func = tLParen;

        add_to_stack(paren_node);
    }
}

static void token_number(ti_var_t slot, int token) {
    float num = 0;
    bool in_exp = false;
    bool negative_num = false;
    bool negative_exp = false;
    bool is_complex = false;
    int exp = 0;
    int frac_num = 0;
    uint8_t exp_num = 0;
    uint8_t tok = token;

    if (need_mul_op) token_operator(slot, tMul);
    need_mul_op = true;

    // Set some booleans
    if (tok == tee) {
        num = 1;
        exp_num = 1;
        in_exp = true;
    } else if (tok == tDecPt) {
        frac_num++;
    } else if (tok != tChs) {
        num = (float) tok - t0;
    }

    // If it's a negation character, check if it's from a number or from another expression
    if (tok == tChs) {
        token = peek_token(slot);
        tok = token;

        if (!(tok == tee || tok == tDecPt || (tok >= t0 && tok <= t9))) {
            // It's used as a negation character, push the unary function and continue
            token_operator(slot, tChs);

            return;
        }

        negative_num = true;
    }

    while ((token = next_token(slot)) != EOF) {
        tok = token;

        // Should be a valid num char
        if (!(tok == tee || tok == tDecPt || tok == tii || tok == tChs || (tok >= t0 && tok <= t9))) break;

        if (tok == tee) {
            if (in_exp) parse_error("Syntax error");

            in_exp = true;
        } else if (tok == tDecPt) {
            if (frac_num || in_exp) parse_error("Syntax error");

            frac_num = -1;
        } else if (tok == tii) {
            is_complex = true;

            break;
        } else if (tok == tChs) {
            if (!in_exp || exp_num != 1) parse_error("Syntax error");

            negative_exp = true;
        } else if (in_exp) {
            exp = exp * 10 + tok - t0;
            exp_num++;
        } else if (frac_num) {
            num += powf(10, (float) frac_num) * (float) (tok - t0);
            frac_num--;
        } else {
            num = num * 10 + (float) (tok - t0);
        }
    }

    if (token != tii) seek_prev(slot);

    // Get the right number, based on the exponent and negative flag
    if (negative_exp) exp = -exp;
    if (in_exp) num = num * powf(10, (float) exp);
    if (negative_num) num = -num;

    // And add it to the output stack
    if (is_complex) {
        struct cplx_t *cplx = malloc(sizeof(struct cplx_t));
        if (cplx == NULL) parse_error("Memory error");

        cplx->real = 0;
        cplx->imag = num;

        struct NODE *cplx_node = node_alloc(ET_COMPLEX);
        cplx_node->data.operand.cplx_ptr = cplx;

        add_to_output(cplx_node);
    } else {
        struct NODE *num_node = node_alloc(ET_NUM);
        num_node->data.operand.num = num;

        add_to_output(num_node);
    }
}

static void token_variable(__attribute__((unused)) ti_var_t slot, int token) {
    if (need_mul_op) token_operator(slot, tMul);
    need_mul_op = true;

    struct NODE *var_node = node_alloc(ET_VARIABLE);
    var_node->data.operand.variable_nr = token - tA;

    add_to_output(var_node);
}

static void token_os_list(ti_var_t slot, __attribute__((unused)) int token) {
    if (need_mul_op) token_operator(slot, tMul);

    uint8_t list_nr = next_token(slot);

    // Check if it's a list element
    if (peek_token(slot) == tLParen) {
        next_token(slot);
        token_function(slot, 0x5D + (list_nr << 8));
    } else {
        struct NODE *list_node = node_alloc(ET_LIST);
        list_node->data.operand.list_nr = list_nr;

        add_to_output(list_node);
        need_mul_op = true;
    }
}

static void token_os_matrix(ti_var_t slot, __attribute__((unused)) int token) {
    if (need_mul_op) token_operator(slot, tMul);

    uint8_t matrix_nr = next_token(slot);

    // Check if it's a matrix element
    if (peek_token(slot) == tLParen) {
        next_token(slot);
        token_function(slot, 0x5C + (matrix_nr << 8));
    } else {
        struct NODE *matrix_node = node_alloc(ET_MATRIX);
        matrix_node->data.operand.matrix_nr = matrix_nr;

        add_to_output(matrix_node);
        need_mul_op = true;
    }
}

static void token_os_string(ti_var_t slot, __attribute__((unused)) int token) {
    if (need_mul_op) token_operator(slot, tMul);
    need_mul_op = true;

    uint8_t str_nr = next_token(slot);

    struct NODE *str_node = node_alloc(ET_STRING);
    str_node->data.operand.string_nr = str_nr;

    add_to_output(str_node);
}

static void token_os_equ(ti_var_t slot, __attribute__((unused)) int token) {
    if (need_mul_op) token_operator(slot, tMul);
    need_mul_op = true;

    uint8_t equ_nr = next_token(slot);

    if (equ_nr >= 0x80) equ_nr -= 0x80 - 28;
    else if (equ_nr >= 0x40) equ_nr -= 0x40 - 22;
    else if (equ_nr >= 0x20) equ_nr -= 0x20 - 10;

    struct NODE *equ_node = node_alloc(ET_EQU);
    equ_node->data.operand.equation_nr = equ_nr;

    add_to_output(equ_node);
}

static void token_string(ti_var_t slot, int token) {
    if (need_mul_op) token_operator(slot, tMul);

    uint8_t *start_ptr = ti_GetDataPtr(slot) + 1;
    unsigned int length = 0;

    do {
        token = next_token(slot);
        length++;

        if (is_2_byte_token(token)) {
            next_token(slot);
            length++;
        }
    } while (token != EOF && (uint8_t) token != tEnter && (uint8_t) token != tStore && (uint8_t) token != tString);

    if (length == 1) parse_error("Invalid string");

    if ((uint8_t) token == tEnter || (uint8_t) token == tStore) {
        seek_prev(slot);
    } else {
        need_mul_op = true;
    }

    // This is a fake struct NODE, in fact it's just a pointer to the raw string
    struct var_string *string_memory = malloc(length - 1 + 3);
    if (string_memory == NULL) parse_error("Memory error");

    string_memory->length = length - 1;
    memcpy(string_memory->data, start_ptr, length - 1);

    struct NODE *string_node = node_alloc(ET_FUNCTION_CALL);
    string_node->data.operand.func = tString;
    string_node->child = (struct NODE *) string_memory;

    add_to_output(string_node);
}

static void token_empty_func(__attribute__((unused)) ti_var_t slot, int token) {
    if (need_mul_op) token_operator(slot, tMul);
    need_mul_op = true;

    struct NODE *func_node = node_alloc(ET_FUNCTION_CALL);
    func_node->data.operand.func = token;

    add_to_output(func_node);
}

static void token_pi(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    if (need_mul_op) token_operator(slot, tMul);
    need_mul_op = true;

    struct NODE *pi_node = node_alloc(ET_NUM);
    pi_node->data.operand.num = M_PI;

    add_to_output(pi_node);
}

static void token_rand(ti_var_t slot, __attribute__((unused)) int token) {
    if (need_mul_op) token_operator(slot, tMul);

    // Check if it's a matrix element
    if (peek_token(slot) == tLParen) {
        next_token(slot);
        token_function(slot, tRand);
    } else {
        struct NODE *rand_node = node_alloc(ET_FUNCTION_CALL);
        rand_node->data.operand.func = tRand;

        add_to_output(rand_node);
        need_mul_op = true;
    }
}

static void token_unimplemented(__attribute__((unused)) ti_var_t slot, __attribute__((unused)) int token) {
    parse_error("Token not implemented");
}

static void (*functions[256])(ti_var_t, int) = {
        token_unimplemented, // **unused**
        token_unimplemented, // ►DMS
        token_unimplemented, // ►Dec
        token_unimplemented, // ►Frac
        token_operator,      // →
        token_unimplemented, // Boxplot
        token_function,      // [
        token_rbrack,        // ]
        token_function,      // {
        token_rbrace,        // }
        token_operator,      // r
        token_operator,      // °
        token_operator,      // ֿ¹
        token_operator,      // ²
        token_operator,      // T
        token_operator,      // ³
        token_function,      // (
        token_rparen,        // )
        token_function,      // round(
        token_function,      // pxl-Test(
        token_function,      // augment(
        token_function,      // rowSwap(
        token_function,      // row+(
        token_function,      // *row(
        token_function,      // *row+(
        token_function,      // max(
        token_function,      // min(
        token_function,      // R►Pr(
        token_function,      // R►Pθ(
        token_function,      // R►Px(
        token_function,      // R►Py(
        token_function,      // median(
        token_function,      // randM(
        token_function,      // mean(
        token_function,      // solve(
        token_function,      // seq(
        token_function,      // fnInt(
        token_function,      // nDeriv(
        token_unimplemented, // **unused**
        token_function,      // fMin(
        token_function,      // fMax(
        token_unimplemented, //
        token_string,        // "
        token_operator,      // ,
        token_empty_func,    // i
        token_unimplemented, // !
        token_unimplemented, // CubicReg
        token_unimplemented, // QuartReg
        token_number,        // 0
        token_number,        // 1
        token_number,        // 2
        token_number,        // 3
        token_number,        // 4
        token_number,        // 5
        token_number,        // 6
        token_number,        // 7
        token_number,        // 8
        token_number,        // 9
        token_number,        // .
        token_number,        // |E
        token_operator,      // or
        token_operator,      // xor
        token_unimplemented, // :
        token_unimplemented, // \n
        token_operator,      // and
        token_variable,      // A
        token_variable,      // B
        token_variable,      // C
        token_variable,      // D
        token_variable,      // E
        token_variable,      // F
        token_variable,      // G
        token_variable,      // H
        token_variable,      // I
        token_variable,      // J
        token_variable,      // K
        token_variable,      // L
        token_variable,      // M
        token_variable,      // N
        token_variable,      // O
        token_variable,      // P
        token_variable,      // Q
        token_variable,      // R
        token_variable,      // S
        token_variable,      // T
        token_variable,      // U
        token_variable,      // V
        token_variable,      // W
        token_variable,      // X
        token_variable,      // Y
        token_variable,      // Z
        token_variable,      // theta
        token_os_matrix,     // 2-byte token
        token_os_list,       // 2-byte token
        token_os_equ,        // 2-byte token
        token_unimplemented, // prgm
        token_unimplemented, // 2-byte token
        token_unimplemented, // 2-byte token
        token_unimplemented, // 2-byte token
        token_unimplemented, // 2-byte token
        token_unimplemented, // Radian
        token_unimplemented, // Degree
        token_unimplemented, // Normal
        token_unimplemented, // Sci
        token_unimplemented, // Eng
        token_unimplemented, // Float
        token_operator,      // =
        token_operator,      // <
        token_operator,      // >
        token_operator,      // <=
        token_operator,      // >=
        token_operator,      // !=
        token_operator,      // +
        token_operator,      // - (sub)
        token_empty_func,    // Ans
        token_unimplemented, // Fix
        token_unimplemented, // Horiz
        token_unimplemented, // Full
        token_unimplemented, // Func
        token_unimplemented, // Param
        token_unimplemented, // Polar
        token_unimplemented, // Seq
        token_unimplemented, // IndpntAuto
        token_unimplemented, // IndpntAsk
        token_unimplemented, // DependAuto
        token_unimplemented, // DependAsk
        token_unimplemented, // 2-byte token
        token_unimplemented, // square mark
        token_unimplemented, // plus mark
        token_unimplemented, // dot mark
        token_operator,      // *
        token_operator,      // /
        token_unimplemented, // Trace
        token_unimplemented, // ClrDraw
        token_unimplemented, // ZStandard
        token_unimplemented, // ZTrig
        token_unimplemented, // ZBox
        token_unimplemented, // Zoom In
        token_unimplemented, // Zoom Out
        token_unimplemented, // ZSquare
        token_unimplemented, // ZInteger
        token_unimplemented, // ZPrevious
        token_unimplemented, // ZDecimal
        token_unimplemented, // ZoomStat
        token_unimplemented, // ZoomRcl
        token_unimplemented, // PrintScreen
        token_unimplemented, // ZoomSto
        token_unimplemented, // Text(
        token_operator,      // nPr
        token_operator,      // nCr
        token_unimplemented, // FnOn
        token_unimplemented, // FnOff
        token_unimplemented, // StorePic
        token_unimplemented, // RecallPic
        token_unimplemented, // StoreGDB
        token_unimplemented, // RecallGDB
        token_unimplemented, // Line(
        token_unimplemented, // Vertical
        token_unimplemented, // Pt-On(
        token_unimplemented, // Pt-Off(
        token_unimplemented, // Pt-Change(
        token_unimplemented, // Pxl-On(
        token_unimplemented, // Pxl-Off(
        token_unimplemented, // Pxl-Change(
        token_unimplemented, // Shade(
        token_unimplemented, // Circle(
        token_unimplemented, // Horizontal
        token_unimplemented, // Tangent(
        token_unimplemented, // DrawInv
        token_unimplemented, // DrawF
        token_os_string,     // 2-byte token
        token_rand,          // rand
        token_pi,            // pi
        token_empty_func,    // getKey
        token_unimplemented, // '
        token_unimplemented, // ?
        token_number,        // - (neg)
        token_function,      // int(
        token_function,      // abs(
        token_function,      // det(
        token_function,      // identity(
        token_function,      // dim(
        token_function,      // sum(
        token_function,      // prod(
        token_function,      // not(
        token_function,      // iPart(
        token_function,      // fPart(
        token_unimplemented, // 187
        token_function,      // √(
        token_function,      // ³√(
        token_function,      // ln(
        token_function,      // e^(
        token_function,      // log(
        token_function,      // 10^(
        token_function,      // sin(
        token_function,      // sinֿ¹(
        token_function,      // cos(
        token_function,      // cosֿ¹(
        token_function,      // tan(
        token_function,      // tanֿ¹(
        token_function,      // sinh(
        token_function,      // sihֿ¹(
        token_function,      // cosh(
        token_function,      // coshֿ¹(
        token_function,      // tanh(
        token_function,      // tanhֿ¹(
        token_unimplemented, // If
        token_unimplemented, // Then
        token_unimplemented, // Else
        token_unimplemented, // While
        token_unimplemented, // Repeat
        token_unimplemented, // For(
        token_unimplemented, // End
        token_unimplemented, // Return
        token_unimplemented, // Lbl
        token_unimplemented, // Goto
        token_unimplemented, // Pause
        token_unimplemented, // Stop
        token_unimplemented, // IS>(
        token_unimplemented, // DS<(
        token_unimplemented, // Input
        token_unimplemented, // Prompt
        token_unimplemented, // Disp
        token_unimplemented, // DispGraph
        token_unimplemented, // Output(
        token_unimplemented, // ClrHome
        token_unimplemented, // Fill(
        token_unimplemented, // SortA(
        token_unimplemented, // SortD(
        token_unimplemented, // DispTable
        token_unimplemented, // Menu(
        token_unimplemented, // Send(
        token_unimplemented, // Get(
        token_unimplemented, // PlotsOn
        token_unimplemented, // PlotsOff
        token_unimplemented, // ∟
        token_unimplemented, // Plot1(
        token_unimplemented, // Plot2(
        token_unimplemented, // Plot3(
        token_unimplemented, // 2-byte token
        token_operator,      // ^
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
