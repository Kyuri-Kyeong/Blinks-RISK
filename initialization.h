#ifndef INITIALIZATION_H
#define INITIALIZATION_H
#include "common.h"
#define PULSE_LENGTH 2000
#define TIMER_LENGTH 6000

namespace Initialization{
    enum SignalState { INERT, GO, RESOLVE };
    enum Operation { NONE, ADD, COLOR, DELETE };

    
    inline byte activatedColor[4] = {0, 0, 0, 0};
    
    void setup();
    void loop();
    void inertLoop();
    void goLoop();
    void resolveLoop();
    void displayColor();
    byte constructData(Stage stage, SignalState signalState, Operation operation, int color);
    Stage getStage(byte data);
    SignalState getSignalState(byte data);
    Operation getOperation(byte data);
    int getColor(byte data);
}
#endif
