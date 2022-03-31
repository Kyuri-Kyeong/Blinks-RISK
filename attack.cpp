#include "attack.h"

namespace Attack{
    bool test;
    byte attackState;
    byte currentData[3];
    byte enemyData[3];
    
    void setup(){
        FOREACH_FACE(f){
          markDatagramReadOnFace(f);
        }
        randomize();
        currentData[0] = attackState;
        currentData[1] = random(3);
        currentData[2] = random(4)+1;
        test = false;
        attackState = NOTHING;
    }
    
    void loop() {
      FOREACH_FACE(f){
        if (isDatagramReadyOnFace(f)){
            *enemyData = getDatagramOnFace(f);
            if (enemyData[0] == NOTHING){
              attackState = NOTHING;
            }
            if (enemyData[0] == ATTACKING){
              attackState = DEFENDING;
            }
            if (enemyData[0] == ATTACK_SUCCESS){
                // If we were the attacker, we have to update the tile to reflect the current soldier count.          
                if (attackState == ATTACKING){
                  currentData[2] = enemyData[2];
                }
              attackState = NOTHING;   
                FOREACH_FACE(f){
                  currentData[0] = NOTHING;
                  sendDatagramOnFace(currentData, 3, f);
                }
            }
          markDatagramReadOnFace(f);
          }
      }
      
      if (buttonSingleClicked()){
        if (attackState == NOTHING){
          attackState = ATTACKING;
          FOREACH_FACE(f){
            currentData[0] = ATTACKING;
            sendDatagramOnFace(currentData, 3, f);
          }
        }
        if (attackState == DEFENDING){
          whoWins();
          enemyData[0] = ATTACK_SUCCESS;
          
          if (currentData[2] == 0){
            currentData[1] = enemyData[1];
            currentData[2] = enemyData[2]-1;
            enemyData[2] = 1;
            attackState = NOTHING;
          }
          // We broadcast how many soldiers are remaining and the attacker should pick up on it.
          FOREACH_FACE(f){
            sendDatagramOnFace(enemyData, 3, f);
          }
        }
      }
      
      displaySoldiers();
    }
    
    void whoWins(){
        byte enemySoldiers = enemyData[2]-1;
        byte currentSoldiers = currentData[2];
        
        byte attackDice = (enemySoldiers > 3)?3:enemySoldiers;
        byte defendDice = (currentSoldiers > 2)?2:currentSoldiers;
        
        byte attackResults[3] = {0,0,0};
        byte defendResults[2] = {0,0};
        
        for (byte i = 0; i < attackDice; i++){
          attackResults[i] = random(5)+1;
        }
        for (byte i = 0; i < defendDice; i++){
          defendResults[i] = random(5)+1;
        }
        
        byte smaller = (attackDice > defendDice)?defendDice:attackDice;
        
        for (byte i = 0; i < smaller; i++){
          byte maxAttack = 0;
          byte maxAIndex = -1;
          for (byte j = 0; j < 3; j++){
            if (attackResults[j] > maxAttack){
              maxAttack = attackResults[j];
              maxAIndex = j;
            }
          }
          attackResults[maxAIndex] = 0;
          
          byte maxDefence = 0;
          byte maxDIndex = -1;
          for (byte j = 0; j < 2; j++){
            if (defendResults[j] > maxDefence){
              maxDefence = defendResults[j];
              maxDIndex = j;
            }
          }
          defendResults[maxDIndex] = 0;
          if (maxAttack > maxDefence){
            currentSoldiers--;
          }
          else{
            enemySoldiers--;
          }
        }
        
        // We update the data structures to reflect the results.
        enemyData[2] = enemySoldiers+1;
        currentData[2] = currentSoldiers;
    }
    
    void displaySoldiers(){
        setColor(OFF);
        FOREACH_FACE(f) {
            if (f < currentData[2]) { //this face should be lit
                setColorOnFace(colorPallete[currentData[1]], f);
            }
        }
        if (test){
          setColor(WHITE);
        }
    }
}
