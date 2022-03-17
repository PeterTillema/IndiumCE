#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstdint>

bool in_degree_mode();
uint8_t get_fix_nr();
uint8_t get_key();

class Globals {
public:
    uint8_t getKeyValue;
    bool inRadianMode;
    uint8_t fixNr;

    Globals();

    uint8_t getKey();

    void updateKey();
};

extern Globals globals;

#endif
