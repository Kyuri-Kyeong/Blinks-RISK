#ifndef ATTACK_H
#define ATTACK_H
#include "common.h"


namespace Attack {
  
    enum attackStates {NOTHING, ATTACKING, DEFENDING, ATTACK_SUCCESS};
    void setup();
    void loop();
    void whoWins();
    void displaySoldiers();
}

#endif
