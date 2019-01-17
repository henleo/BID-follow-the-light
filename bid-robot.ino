//we know this is shit code. we are also a little sorry.

const int LEFTLED = 3;
const int RIGHTLED = 4;
const int LEFTSENSOR = A0;
const int RIGHTSENSOR = A1;
const int BOARDLED = 13;
int DELTA = 50;
int lastValuesLeft[5];
int lastValuesRight[5];
int lastColorChanges[5];
int leftMotorState = 1;
int rightMotorState = 1;
int forward = 1;
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
    digitalWrite(LEFTLED, HIGH);
    digitalWrite(RIGHTLED, HIGH);
    while(findMode){
      findCircle();
    }
    delay(3000);
    Serial.println("found circle");
    Serial.println(String(leftMotorState));
    Serial.println(String(rightMotorState));
    while(!centerFound()){
      followSecant();
    }
    Serial.println("located center");
    goToCenter();
    putFlag();
    Serial.println("victory");
  }
  
}
/*
void accelerateForward(int pin){
  digitalWrite(pin, HIGH);
  delay(20);
  digitalWrite(pin, LOW);
}

void accelerateBackward(int pin){
  digitalWrite(pin, HIGH);
  delay(10);
  digitalWrite(pin, LOW);
  delay(10);
}

void stopMotor(int pin){
  digitalWrite(pin, LOW);
}
*/
void putFlag(){
  digitalWrite(LEFTLED, LOW);
  digitalWrite(RIGHTLED, LOW);
}

void goToCenter(){
  while(changesLeftUntilCenter){
    followSecant();
  }
}

bool centerFound(){
  return reversals > 1;
}

void followSecant(){
  pushToArray(lastValuesLeft, analogRead(LEFTSENSOR));
  pushToArray(lastValuesRight, analogRead(RIGHTSENSOR));
  
  adjustCourse();
  
  displayMotorState();
}

void adjustDirection(){
  if(lastColorChanges[0] == 1 && lastColorChanges[1] == 1){
    reversals++;
    Serial.println("U turn");
    turnNinetyDegrees();
  }
}

bool colorChange(int valuesArray[5]){
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

void logColorChange(int anArray[5]){
  int difference = anArray[0] - anArray[1];
  if(difference < 0){
    pushToArray(lastColorChanges, 1); //light to dark
  }
  else{
    pushToArray(lastColorChanges, 2); //dark to light
  }
  if(centerFound()){
    changesLeftUntilCenter--;
  }
  adjustDirection();
}

void adjustCourse(){
  /*
  if(colorChange(lastValuesRight) && !colorChange(lastValuesLeft)){
    if(leftMotorState == 0){
      rightMotorState = forward;
      leftMotorState = forward;
      logColorChange(lastValuesRight);
      Serial.println("right LOGGED");
    }
    else{
      rightMotorState = 0;
      leftMotorState = forward;
    }
  }
  else if(colorChange(lastValuesLeft) && !colorChange(lastValuesRight)){
    if(rightMotorState == 0){
      rightMotorState = forward;
      leftMotorState = forward;
      logColorChange(lastValuesLeft);
      Serial.println("right LOGGED");
    }
    else{
      rightMotorState = forward;
      leftMotorState = 0;
    }
  }
  else if(!colorChange(lastValuesRight) && !colorChange(lastValuesLeft)){
    rightMotorState = forward;
    leftMotorState = forward;
  }
  else if(colorChange(lastValuesRight) && colorChange(lastValuesLeft)){
    rightMotorState = forward;
    leftMotorState = forward;
    logColorChange(lastValuesLeft);
    Serial.println("BOTH LOGGED");
  }
  */
  if(colorChange(lastValuesRight) && rightMotorState==forward) {
    if(leftMotorState == forward) {
      rightMotorState = 0;
      leftMotorState = forward;
    }
    else {
      rightMotorState = forward;
      leftMotorState = forward;
      logColorChange(lastValuesRight);
      Serial.println("right LOGGED");
      return;
    }
  }
  if(colorChange(lastValuesLeft) && leftMotorState==forward) {
    if(rightMotorState == forward) {
      rightMotorState = forward;
      leftMotorState = 0;
    }
    else {
      rightMotorState = forward;
      leftMotorState = forward;
      logColorChange(lastValuesLeft);
      Serial.println("left LOGGED");
      return;
    }
  }
}

void pushToArray(int anArray[5], int value){
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
  
  delay(500);
}

void findCircle() {
  int leftValue = analogRead(LEFTSENSOR);
  int rightValue = analogRead(RIGHTSENSOR);
  /*
  Serial.println("\nRight:");
  Serial.println(String(rightValue));
  Serial.println("\nLeft:");
  Serial.println(String(leftValue));
  */
  pushToArray(lastValuesLeft, leftValue);
  pushToArray(lastValuesRight, rightValue);
  if(colorChange(lastValuesRight) && rightMotorState==forward) {
    if(leftMotorState == forward) {
      digitalWrite(RIGHTLED, LOW);
      rightMotorState = 0;
      digitalWrite(LEFTLED, HIGH);
      leftMotorState = forward;
    }
    else {
      digitalWrite(RIGHTLED, HIGH);
      rightMotorState = forward;
      digitalWrite(LEFTLED, HIGH);
      leftMotorState = forward;
      findMode = false;
      return;
    }
  }
  if(colorChange(lastValuesLeft) && leftMotorState==forward) {
    if(rightMotorState == forward) {
      digitalWrite(RIGHTLED, HIGH);
      rightMotorState = forward;
      digitalWrite(LEFTLED, LOW);
      leftMotorState = 0;
    }
    else {
      digitalWrite(RIGHTLED, HIGH);
      rightMotorState = forward;
      digitalWrite(LEFTLED, HIGH);
      leftMotorState = forward;
      findMode = false;
      return;
    }
  }
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

void turnNinetyDegrees() {
  for(int i=0; i < 5; i++) {
    digitalWrite(BOARDLED, HIGH);
    delay(50);
    digitalWrite(BOARDLED, LOW);
    delay(50);
  }
  delay(200);
}
