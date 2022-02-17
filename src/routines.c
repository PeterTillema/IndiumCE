#include "routines.h"

#include <fileioc.h>
#include <string.h>

static int cur_token;

void seek_prev(ti_var_t slot) {
    // If the current offset is the same as the size of the program, the last ti_GetC returned EOF, in which case we
    // do not want to get to the previous token!
    if (ti_Tell(slot) != ti_GetSize(slot)) {
        ti_Seek(-1, SEEK_CUR, slot);
        parse_col--;
    }
}

bool is_2_byte_token(int token) {
    uint8_t All2ByteTokens[9] = {tExtTok, tVarMat, tVarLst, tVarPict, tVarGDB, tVarOut, tVarSys, tVarStrng, t2ByteTok};

    return memchr(All2ByteTokens, token, sizeof(All2ByteTokens)) != NULL;
}

int next_token(ti_var_t slot) {
    int token = ti_GetC(slot);

    cur_token = token;
    parse_col++;

    return token;
}

int current_token(void) {
    return cur_token;
}

int peek_token(ti_var_t slot) {
    int token = ti_GetC(slot);

    parse_col++;
    seek_prev(slot);

    return token;
}

bool is_end_of_line(int token) {
    return token == EOF || (uint8_t) token == tEnter || (uint8_t) token == tColon;
}
