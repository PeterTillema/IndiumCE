#include "utils.h"
#include "globals.h"

#include <cstring>
#include <fileioc.h>
#include <tice.h>

static int pt = 0;
static int ct = 0;
static int nt = -2;

extern unsigned int parseLine;
extern unsigned int parseCol;

char *formatNum(float num) {
    static char buf[20];

    if (globals.fixNr == 255) {
        sprintf(buf, "%f", num);
    } else {
        sprintf(buf, "%10.*f", globals.fixNr, num);
    }

    // Strip trailing zeroes
    char *p = buf + strlen(buf) - 1;
    while (*p == '0' && --p > buf);

    // Eventually strip decimal point too
    if (*p == '.') p--;
    *(p + 1) = '\0';

    return buf;
}

bool is2ByteTok(int token) {
    uint8_t All2ByteTokens[9] = {tExtTok, tVarMat, tVarLst, tVarPict, tVarGDB, tVarOut, tVarSys, tVarStrng, t2ByteTok};

    return memchr(All2ByteTokens, token, sizeof(All2ByteTokens)) != nullptr;
}

void seekPrev(ti_var_t slot) {
    if (nt != EOF) {
        ti_Seek(-1, SEEK_CUR, slot);
    }

    nt = ct;
    ct = pt;
    pt = 0;
    parseCol--;
}

int tokenNext(ti_var_t slot) {
    if (nt == -2) {
        nt = ti_GetC(slot);
    }

    pt = ct;
    ct = nt;
    nt = ti_GetC(slot);
    parseCol++;

    return ct;
}

int tokenCurrent() {
    return ct;
}

int tokenPeek() {
    return nt;
}

bool endOfLine(int token) {
    return token == EOF || (uint8_t) token == tEnter || (uint8_t) token == tColon;
}
