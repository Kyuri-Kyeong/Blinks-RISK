#include "initialization.h"

namespace Initialization{
    int playerColor, numPlayer;
    SignalState signalState;
    Operation operation;
    Stage stage;
    bool playerActivated;
    byte receivedData, sendData;
    
    void setup() {
        numPlayer = 0;
        playerColor = 0;
        stage = INIT;
        operation = NONE;
        playerActivated = false;
        signalState = INERT;
        receivedData = 0;
        sendData = 0;
    }

    void loop() {
        switch (signalState) {
            case INERT:
                inertLoop();
                break;
            case GO:
                goLoop();
                break;
            case RESOLVE:
                resolveLoop();
                break;
        }
        
        displayColor();
        sendData = constructData(stage, signalState, operation, numPlayer);
        setValueSentOnAllFaces(sendData);
    }

    void inertLoop() {
        //set myself to GO
        if (buttonLongPressed()){
            signalState = GO;
            playerActivated = true;
            playerColor = numPlayer % 4;
            numPlayer++;
            operation = ADD;
        }

        if (buttonDoubleClicked()) {
          _main_::currentStage = RECRUIT;
        }

        //listen for neighbors in GO
        FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) { //a neighbor!
                receivedData = getLastValueReceivedOnFace(f);
                if (getSignalState(receivedData) == GO) { //a neighbor saying GO!
                    signalState = GO;
                    operation = getOperation(receivedData);
                    numPlayer = getColor(receivedData);
                }
            }
        }
    }

    void goLoop() {
        signalState = RESOLVE; //I default to this at the start of the loop. Only if I see a problem does this not happen

        //look for neighbors who have not heard the GO news
        FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) { //a neighbor!
                receivedData = getLastValueReceivedOnFace(f);
                if (getSignalState(receivedData) == INERT) {//This neighbor doesn't know it's GO time. Stay in GO
                    signalState = GO;
                }
            }
        }
    }
    void resolveLoop() {
        signalState = INERT; //I default to this at the start of the loop. Only if I see a problem does this not happen

        //look for neighbors who have not moved to RESOLVE
        FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) { //a neighbor!
                receivedData = getLastValueReceivedOnFace(f);
                if (getSignalState(receivedData) == GO) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
                    signalState = RESOLVE;
                }
            }
        }
    }

    void displayColor() {
        if (playerActivated){
            setColor(colorPallete[playerColor % 4]);
        }
        else {
            int pulseProgress = millis() % PULSE_LENGTH;
            byte pulseMapped = map(pulseProgress, 0, PULSE_LENGTH, 0, 255);
            byte dimness = sin8_C(pulseMapped);

            setColor(makeColorHSB(0, 0, dimness));
        }
    }

    byte constructData(Stage stage, SignalState signalState, Operation operation, int color){
        return (stage << 6) | (signalState << 4) | (operation << 2) | color;
    }

    Stage getStage(byte data) {
        return (data >> 6) & 3;
    }

    SignalState getSignalState(byte data) {
        return (data >> 4) & 3;
    }

    Operation getOperation(byte data) {
        return (data >> 2) & 3;
    }

    int getColor(byte data) {
        return data & 3;
    }
}
