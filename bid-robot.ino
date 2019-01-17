//we know this is shit code. we are also a little sorry.

const int LEFTLED = 3;
const int RIGHTLED = 4;
const int LEFTSENSOR = A0;
const int RIGHTSENSOR = A1;
const int BOARDLED = 13;
int DELTA = 50;
int leftSensorBuffer[5];
int rightSensorBuffer[5];
int borderLog[5] = {0};
int forward = 1;
int leftMotorState = forward;
int rightMotorState = forward;
int reversals = 0;
bool findMode = true;
int currentFindCounter = 0;
int targetFindCounter = 0;
int changesLeftUntilCenter = 2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LEFTLED, OUTPUT);
  pinMode(RIGHTLED, OUTPUT);
  pinMode(BOARDLED, OUTPUT);
  pinMode(LEFTSENSOR, INPUT);
  pinMode(RIGHTSENSOR, INPUT);
  
}

void loop() {
  /*
  digitalWrite(LEFTLED, LOW);
  digitalWrite(RIGHTLED, HIGH);
  Serial.println("\nRight:");
  Serial.println(String(analogRead(RIGHTSENSOR)));
  Serial.println("\nLeft:");
  Serial.println(String(analogRead(LEFTSENSOR)));
  delay(500);
  */
  
  if(!centerFound()){ 
    while(!borderLog[0]){
      findCircle();
    }
    delay(500);
    
    Serial.println("found circle");
    while(!centerFound()){
      alignWithSecant();
    }
    Serial.println("located center");
    
    goToCenter();
    putFlag();
    Serial.println("put flag");
  }
  
   // alignWithSecant();
}

void findCircle() {
  alignWithSecant();
  if(currentFindCounter == ((targetFindCounter / 2 + 1)*100)) {
    turnNinetyDegrees();
    currentFindCounter = 0;
    targetFindCounter++;
  }
  else {
    currentFindCounter++;
  }  
  delay(10);
}

void alignWithSecant(){
  getSensorData();
  
  adjustCourse();
  
  displayMotorState();
}

void adjustCourse(){
  if(colorHasChanged(leftSensorBuffer) && colorHasChanged(rightSensorBuffer)){
    drive();
    logBorder(leftSensorBuffer);
  }
  else if(colorHasChanged(leftSensorBuffer)) {
    if(rightMotorState == 0) {
      drive();
      logBorder(leftSensorBuffer);
    }
    else {
      turnLeft();
    }
  }
  else if(colorHasChanged(rightSensorBuffer)) {
    if(leftMotorState == 0) {
      drive();
      logBorder(rightSensorBuffer);
    }
    else {
      turnRight();
    }
  }
}

bool colorHasChanged(int valuesArray[5]){
  /*
  int difference = valuesArray[0] - valuesArray[1];
  if(abs(difference) > DELTA){
    return true;
  }
  */
  return getCategory(valuesArray[0]) != getCategory(valuesArray[1]);
}

int getCategory(int value){
  if(value < 60) return 1; //black
  else if(value < 160) return 2; //grey
  else if(value < 400) return 3; //white
}

void logBorder(int anArray[5]){
  int difference = anArray[0] - anArray[1];
  if(difference < 0){
    pushToBuffer(borderLog, 1); //light to dark
  }
  else{
    pushToBuffer(borderLog, 2); //dark to light
  }
  if(centerFound()){
    changesLeftUntilCenter--;
  }
  adjustDirection();
  Serial.println("crossed border");
}

void adjustDirection(){
  if(borderLog[0] == 1 && borderLog[1] == 1){
    reversals++;
    Serial.println("U turn");
    turnAround();
  }
}

void pushToBuffer(int anArray[5], int value){
  for(int i = 4; i > 0; i--){
    anArray[i] = anArray[i - 1];
  }
  anArray[0] = value;
}

void displayMotorState(){
  switch(leftMotorState) {
    case 0 :  digitalWrite(LEFTLED, LOW);
              break;
    case 1 :  digitalWrite(LEFTLED, HIGH);
              break;
  }
  switch(rightMotorState) {
    case 0 :  digitalWrite(RIGHTLED, LOW);
              break;
    case 1 :  digitalWrite(RIGHTLED, HIGH);
              break;
  }
  
  delay(1000);
}

void turnNinetyDegrees() {
  for(int i=0; i < 5; i++) {
    digitalWrite(BOARDLED, HIGH);
    delay(50);
    digitalWrite(BOARDLED, LOW);
    delay(50);
  }
  delay(200);
}

void turnAround() {
  turnNinetyDegrees();
}


void putFlag(){
  digitalWrite(LEFTLED, LOW);
  digitalWrite(RIGHTLED, LOW);
  digitalWrite(BOARDLED, HIGH);
}

void goToCenter(){
  while(changesLeftUntilCenter){
    alignWithSecant();
  }
}

bool centerFound(){
  return reversals > 1;
}

void turnLeft(){
  rightMotorState = forward;
  leftMotorState = 0;
}

void turnRight(){
  rightMotorState = 0;
  leftMotorState = forward;
  Serial.println("turn right");
}

void drive(){
  rightMotorState = forward;
  leftMotorState = forward;
  Serial.println("turn right");
}

void getSensorData(){
  if(leftMotorState){
    pushToBuffer(leftSensorBuffer, analogRead(LEFTSENSOR));
    //Serial.println("\nLEFT SENSOR: ");
    //Serial.println(String(leftSensorBuffer[0]));
  }
  if(rightMotorState){
    pushToBuffer(rightSensorBuffer, analogRead(RIGHTSENSOR));
    //Serial.println("\nRIGHT SENSOR: ");
    //Serial.println(String(rightSensorBuffer[0]));
  }
  //Serial.println("\n");
}
