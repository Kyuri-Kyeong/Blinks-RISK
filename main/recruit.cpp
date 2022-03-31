#include "recruit.h"

namespace Recruit{
    enum signalStates { INERT, GO, RESOLVE };
    byte signalState = INERT;
    
    byte recruitmentInfo[3] = {GO, random(3), 2};
    byte newRecruitmentInfo[3];
    byte currentPlayer = 0;
    byte currentSoldiers = 2;
    bool test = false;
    
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
        displaySignalState();
    }
    void inertLoop() {
        //set myself to GO
        if (buttonSingleClicked()) {
          	signalState = GO;
          	recruitmentInfo[2]--;
          	currentSoldiers++;
            FOREACH_FACE(f) {
              sendDatagramOnFace(recruitmentInfo, 3, f);
            }
        }
        //listen for neighbors in GO
        FOREACH_FACE(f) {
            if (isDatagramReadyOnFace(f)) { //a neighbor!
              	*newRecruitmentInfo = getDatagramOnFace(f);
                if (newRecruitmentInfo[0] == GO) { //a neighbor saying GO!
                    recruitmentInfo[0] = newRecruitmentInfo[0];
                    if (recruitmentInfo[1] == newRecruitmentInfo[1]){
    	                recruitmentInfo[2] = newRecruitmentInfo[2];
                    }
                    FOREACH_FACE(f) {
                        sendDatagramOnFace(newRecruitmentInfo, 3, f);
                    }
    		      	signalState = GO;
                }
    	        markDatagramReadOnFace(f);
            }
        }
    }
    void goLoop() {
        signalState = RESOLVE; //I default to this at the start of the loop. Only if I see a problem does this not happen
    
        //look for neighbors who have not heard the GO news
        FOREACH_FACE(f) {
            if (isDatagramReadyOnFace(f)) { //a neighbor!
                *newRecruitmentInfo = getDatagramOnFace(f);
                if (newRecruitmentInfo[0] == INERT) {//This neighbor doesn't know it's GO time. Stay in GO
                    signalState = GO;
                }
    	        markDatagramReadOnFace(f);
            }
        }
    }
    void resolveLoop() {
        signalState = INERT; //I default to this at the start of the loop. Only if I see a problem does this not happen
    
        //look for neighbors who have not moved to RESOLVE
        FOREACH_FACE(f) {
            if (isDatagramReadyOnFace(f)) { //a neighbor!
                *newRecruitmentInfo = getDatagramOnFace(f);
                if (newRecruitmentInfo[0] == GO) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
                    signalState  = RESOLVE;
                }
    	        markDatagramReadOnFace(f);
            }
        }
    }
    void displaySignalState() {
        setColor(OFF);
        FOREACH_FACE(f) {
            if (f < currentSoldiers) { //this face should be lit
                setColorOnFace(colorPallete[recruitmentInfo[1]], f);
            }
          	else if (f < recruitmentInfo[2]+currentSoldiers && currentPlayer == recruitmentInfo[1]){
              setColorOnFace(WHITE, f);
            }
        }
      	if (test){
          setColor(WHITE);
        }
    } 
}
