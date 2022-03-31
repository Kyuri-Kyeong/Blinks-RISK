#include "main.h"
#include "initialization.h"
#include "recruit.h"
#include "attack.h"

namespace _main_ {
    Stage currentStage = INIT;
    
    void setup() {
        Initialization::setup();
        Attack::setup();
    }
    
    void loop(){
        switch (currentStage){
            case INIT: 
                Initialization::loop(); 
                break;
            case RECRUIT: 
                Recruit::loop(); 
                break;
            case ATTACK: 
                Attack::loop(); 
                break;
        }
    }
}
