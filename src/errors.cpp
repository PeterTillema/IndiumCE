#include "errors.h"

#include <cstdio>
#include <fontlibc.h>
#include <graphx.h>
#include <tice.h>

extern unsigned int parseLine;
extern unsigned int parseCol;

void forceExit() {
    while (os_GetCSC() != sk_Enter);
    gfx_End();
    exit(-1);
}

void parseError(const char *string) {
    char buf[20];

    fontlib_DrawStringL(string, 26);
    fontlib_Newline();

    sprintf(buf, "Line %d column %d", parseLine, parseCol);
    fontlib_DrawString(buf);

    // And exit the program
    forceExit();
}

void memoryError() {
    parseError("Insufficient memory");
}

void typeError() {
    parseError("Incompatible datatypes");
}

void divideBy0Error() {
    parseError("Divide by 0");
}

void dimensionError() {
    parseError("Invalid dimension");
}

void overflowError() {
    parseError("Overflow error");
}

void domainError() {
    parseError("Domain error");
}
