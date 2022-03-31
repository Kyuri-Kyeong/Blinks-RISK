#ifndef HEADER_MAIN
#define HEADER_MAIN
#include "common.h"

enum Stage {INIT, RECRUIT, ATTACK};

namespace _main_ {
    extern Stage currentStage;
    void setup();
    void loop();
}

#endif
