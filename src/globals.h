#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstdint>

#define NORMAL_MODE 0
#define SCI_MODE 1
#define ENG_MODE 3

bool in_degree_mode();
uint8_t get_fix_nr();
uint8_t get_key();
uint8_t get_norm_sci_end_mode();

class Globals {
public:
    uint8_t getKeyValue;
    bool inRadianMode;
    uint8_t fixNr;
    uint8_t normalSciEngMode;

    Globals();

    uint8_t getKey();

    void updateKey();
};

extern Globals globals;

#endif
