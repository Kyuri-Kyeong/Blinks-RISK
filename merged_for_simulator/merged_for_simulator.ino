enum Stage {INIT, RECRUIT, ATTACK};
enum SignalState { INERT, GO, RESOLVE };
enum attackStates {NOTHING, ATTACKING, DEFENDING, ATTACK_SUCCESS};
Color colorPallete[4] ={RED, GREEN, BLUE, YELLOW};
enum Operation { NONE, ADD, COLOR, DELETE };
byte activatedColor[4] = {0, 0, 0, 0};

//namespace Attack {
bool test;
byte attackState;
byte currentData[3];
byte enemyData[3];
    
void Attacksetup() {
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

void Attackloop() {
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
      AttackwhoWins();
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

  AttackdisplaySoldiers();
}

void AttackwhoWins(){
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

void AttackdisplaySoldiers(){
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
//} namespace Attack ends

//namespace Initialization{
int playerColor, numPlayer;
SignalState signalState;
Operation operation;
Stage stage;
bool playerActivated;
byte receivedData, sendData;

void Initializationsetup() {
  numPlayer = 0;
  playerColor = 0;
  stage = INIT;
  operation = NONE;
  playerActivated = false;
  signalState = INERT;
  receivedData = 0;
  sendData = 0;
}

void Initializationloop() {
  switch (signalState) {
    case INERT:
      InitializationinertLoop();
      break;
    case GO:
      InitializationgoLoop();
      break;
    case RESOLVE:
      InitializationresolveLoop();
      break;
  }

  InitializationdisplayColor();
  sendData = InitializationconstructData(stage, signalState, operation, numPlayer);
  setValueSentOnAllFaces(sendData);
}

void InitializationinertLoop() {
  //set myself to GO
  if (buttonLongPressed()){
    signalState = GO;
    playerActivated = true;
    playerColor = numPlayer % 4;
    numPlayer++;
    operation = ADD;
  }

  if (buttonDoubleClicked()) {
    currentStage = RECRUIT;
  }

  //listen for neighbors in GO
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //a neighbor!
      receivedData = getLastValueReceivedOnFace(f);
      if (InitializationgetSignalState(receivedData) == GO) { //a neighbor saying GO!
        signalState = GO;
        operation = InitializationgetOperation(receivedData);
        numPlayer = InitializationgetColor(receivedData);
      }
    }
  }
}

void InitializationgoLoop() {
  signalState = RESOLVE; //I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not heard the GO news
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //a neighbor!
      receivedData = getLastValueReceivedOnFace(f);
      if (InitializationgetSignalState(receivedData) == INERT) {//This neighbor doesn't know it's GO time. Stay in GO
        signalState = GO;
      }
    }
  }
}
void InitializationresolveLoop() {
  signalState = INERT; //I default to this at the start of the loop. Only if I see a problem does this not happen

  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) { //a neighbor!
      receivedData = getLastValueReceivedOnFace(f);
      if (InitializationgetSignalState(receivedData) == GO) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        signalState = RESOLVE;
      }
    }
  }
}

void InitializationdisplayColor() {
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

byte InitializationconstructData(Stage stage, SignalState signalState, Operation operation, int color){
  return (stage << 6) | (signalState << 4) | (operation << 2) | color;
}

Stage InitializationgetStage(byte data) {
  return (data >> 6) & 3;
}

SignalState InitializationgetSignalState(byte data) {
  return (data >> 4) & 3;
}

Operation InitializationgetOperation(byte data) {
  return (data >> 2) & 3;
}

int InitializationgetColor(byte data) {
  return data & 3;
}
//} namespace Initialization ends


//namespace Recruit{
byte recruitmentInfo[3] = {GO, random(3), 2};
byte newRecruitmentInfo[3];
byte currentPlayer = 0;
byte currentSoldiers = 2;

void Recruitloop() {
  switch (signalState) {
    case INERT:
      RecruitinertLoop();
      break;
    case GO:
      RecruitgoLoop();
      break;
    case RESOLVE:
      RecruitresolveLoop();
      break;
  }
  RecruitdisplaySignalState();
}
void RecruitinertLoop() {
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
void RecruitgoLoop() {
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
void RecruitresolveLoop() {
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
void RecruitdisplaySignalState() {
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
//} namespace Recruit ends
//namespace _main_ {
Stage currentStage = INIT;

void Mainsetup() {
  Initializationsetup();
  Attacksetup();
}

void Mainloop(){
  switch (currentStage){
    case INIT: 
      Initializationloop(); 
      break;
    case RECRUIT: 
      Recruitloop(); 
      break;
    case ATTACK: 
      Attackloop(); 
      break;
  }
}
//}

void setup() {
  Mainsetup();
}

void loop() {
  Mainloop();
}
