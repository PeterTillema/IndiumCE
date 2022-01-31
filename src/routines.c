#include "routines.h"

#include <fileioc.h>
#include <string.h>

void seek_prev(ti_var_t slot) {
    ti_Seek(-1, SEEK_CUR, slot);
}

bool is_2_byte_token(int token) {
    uint8_t All2ByteTokens[9] = {tExtTok, tVarMat, tVarLst, tVarPict, tVarGDB, tVarOut, tVarSys, tVarStrng, t2ByteTok};

    return memchr(All2ByteTokens, token, sizeof(All2ByteTokens)) != NULL;
}
