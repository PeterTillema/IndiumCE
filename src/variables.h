#ifndef VARIABLES_H
#define VARIABLES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct custom_list {
    char name[5];
    float* elements;
};

extern float variables[26];
extern uint8_t* strings[10];
extern float* lists[6];
extern float* matrices[10];
extern struct custom_list custom_lists[50];

void get_all_os_variables(void);

#define varA (variables[0])
#define varB (variables[1])
#define varC (variables[2])
#define varD (variables[3])
#define varE (variables[4])
#define varF (variables[5])
#define varG (variables[6])
#define varH (variables[7])
#define varI (variables[8])
#define varJ (variables[9])
#define varK (variables[10])
#define varL (variables[11])
#define varM (variables[12])
#define varN (variables[13])
#define varO (variables[14])
#define varP (variables[15])
#define varQ (variables[16])
#define varR (variables[17])
#define varS (variables[18])
#define varT (variables[19])
#define varU (variables[20])
#define varV (variables[21])
#define varW (variables[22])
#define varX (variables[23])
#define varY (variables[24])
#define varZ (variables[25])
#define Str0 (strings[0])
#define Str1 (strings[1])
#define Str2 (strings[2])
#define Str3 (strings[3])
#define Str4 (strings[4])
#define Str5 (strings[5])
#define Str6 (strings[6])
#define Str7 (strings[7])
#define Str8 (strings[8])
#define Str9 (strings[9])
#define L1 (lists[0])
#define L2 (lists[1])
#define L3 (lists[2])
#define L4 (lists[3])
#define L5 (lists[4])
#define L6 (lists[5])
#define matA (matrices[0])
#define matB (matrices[1])
#define matC (matrices[2])
#define matD (matrices[3])
#define matE (matrices[4])
#define matF (matrices[5])
#define matG (matrices[6])
#define matH (matrices[7])
#define matI (matrices[8])
#define matJ (matrices[9])

#ifdef __cplusplus
}
#endif

#endif
