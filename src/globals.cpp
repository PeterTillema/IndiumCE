#include "globals.h"

#include <keypadc.h>

Globals::Globals() {
    getKeyValue = 0;
    inRadianMode = !in_degree_mode();
    fixNr = get_fix_nr();
    normalSciEngMode = get_norm_sci_end_mode();
}

uint8_t Globals::getKey() {
    uint8_t keyValue = getKeyValue;

    getKeyValue = 0;

    return keyValue;
}

void Globals::updateKey() {
    // Do a single key scan
    kb_EnableInt = (1 << 0);
    kb_SetMode(MODE_2_SINGLE);
    while (!(kb_IntAcknowledge & (1 << 0)));

    uint8_t key = get_key();

    // The arrow keys and [DEL] can be repeated
    if (key == getKeyValue && key != 23 && key != 24 && key != 25 && key != 26 && key != 34) {
        key = 0;
    }
    getKeyValue = key;
}

Globals globals;
