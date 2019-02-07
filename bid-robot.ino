#include "bid.h"

const int IN1 = 10;
const int IN2 = 9;
const int IN3 = 8;
const int IN4 = 7;
const int ENL = 6;
const int ENR = 5;
const int LEFTSENSOR = A1;
const int RIGHTSENSOR = A0;
const int BOARDLED = 13;
const int COLOR_MEASURES = 20;
const int BUFFER_SIZE = 3;
bool CENTER_FOUND = false;
int driveDirection = 1;
int getCategoryCall = 1;
int wiggleValue = 1; //Used to collect diverse data

Category * blackCategory;
Category * grayCategory;
Category * whiteCategory;

Motor * leftMotor;
Motor * rightMotor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENL, OUTPUT);
  pinMode(ENR, OUTPUT);
  pinMode(BOARDLED, OUTPUT);
  pinMode(LEFTSENSOR, INPUT);
  pinMode(RIGHTSENSOR, INPUT);
  leftMotor = new Motor(IN1, IN2, ENL, 120, driveDirection, BUFFER_SIZE);
  rightMotor = new Motor(IN3, IN4, ENR, 120, driveDirection, BUFFER_SIZE);
}

void loop() {
  if(!CENTER_FOUND){
    getCategories();
    drive();
    while(!CENTER_FOUND){
      evaluateSensorData();
      updateCategories();
      if(wentPastTarget()) {
        CENTER_FOUND = true;
        break;
      }
      adjustCourse();
    }
    
    goToCenter();
    putFlag();
  }
}

void getCategories() {
  Category * firstCategory = getCategoryFromSensor();
  leftMotor->changeCategory(firstCategory);
  rightMotor->changeCategory(firstCategory);
  driveToNextCategory();
  Category * secondCategory = getCategoryFromSensor();
  leftMotor->category = secondCategory;
  rightMotor->category = secondCategory;
  driveToNextCategory();
  Category * thirdCategory = getCategoryFromSensor();
  findLightPattern(firstCategory, secondCategory, thirdCategory);
  leftMotor->prevCategory = secondCategory;
  rightMotor->prevCategory = secondCategory;
  leftMotor->category = thirdCategory;
  rightMotor->category = thirdCategory;
}

void findLightPattern(Category* firstCategory, Category* secondCategory,Category* thirdCategory) {
  if(*firstCategory < *secondCategory) {
    if(*secondCategory < *thirdCategory) {
      setCategories(firstCategory, secondCategory, thirdCategory);
    }
    else if(*thirdCategory > *firstCategory) {
      turnAround();
      setCategories(firstCategory, thirdCategory, secondCategory);
    }
    else {
      setCategories(thirdCategory, firstCategory, secondCategory);
    }
  }
  else {
    if(*thirdCategory < *secondCategory) {
      turnAround();
      setCategories(thirdCategory, secondCategory, firstCategory);
    }
    else if(*thirdCategory > *firstCategory) {
      turnAround();
      setCategories(secondCategory, firstCategory, thirdCategory);
    }
    else if(*firstCategory > *thirdCategory) {
      setCategories(secondCategory, thirdCategory, firstCategory);
    }
    else {
      // SPECIAL CASE: gray - white - gray
      CENTER_FOUND = true;
    }
  }
}

void setCategories(Category* black, Category* gray, Category* white) {
  blackCategory = black;
  blackCategory->color = 'b';
  grayCategory = gray;
  grayCategory->color = 'g';
  double deltaBG = ((double)gray->mean/(black->maximum + black->mean + gray->mean + gray->minimum));
  blackCategory->minimum = (int)max((black->mean - (black->mean - black->minimum)*deltaBG), 0);
  blackCategory->maximum = (int)(black->mean + (black->maximum - black->mean)*deltaBG);
  grayCategory->minimum = blackCategory->maximum;
  whiteCategory = white;
  whiteCategory->color = 'w';
  double deltaGW = ((double)black->mean/(gray->maximum + gray->mean + white->mean + white->minimum));
  grayCategory->maximum = (int)(gray->mean + (gray->maximum - gray->mean)*deltaGW);
  whiteCategory->minimum = grayCategory->maximum;
  whiteCategory->maximum = (int)(white->mean + (white->maximum - white->mean)*deltaGW);
}

Category* getCategoryFromSensor() {
  int min = 1024, max = 0, mean = 0;
  for(int i = 0; i < COLOR_MEASURES; i++){
    int leftValue = analogRead(LEFTSENSOR);
    if(leftValue < min) min = leftValue;
    if(leftValue > max) max = leftValue;
    mean += leftValue;
    int rightValue = analogRead(RIGHTSENSOR);
    if(rightValue < min) min = rightValue;
    if(rightValue > max) max = rightValue;
    mean += rightValue;
    delay(50);
    //wiggleRobot();
  }
  wiggleValue = 1;
  blinkDebug(3);
  mean = (int)((double) mean / (COLOR_MEASURES * 2));
  return new Category('#', min, max, mean);
}

void wiggleRobot() {
  if(wiggleValue == 2) {
    turnAround(); 
    wiggleValue=0;
  }
  drive();
  stopMotors();
  delay(50);
  wiggleValue++;
}


void driveToNextCategory() {
  drive();
  while(true) {
    evaluateSensorData();
    if(!leftMotor->colorHasChanged() && !rightMotor->colorHasChanged()) {
      rightMotor->category->minimum = min(rightMotor->bufferMean, rightMotor->category->minimum);
      rightMotor->category->maximum = max(rightMotor->bufferMean, rightMotor->category->maximum);
      leftMotor->category->minimum = min(leftMotor->bufferMean, leftMotor->category->minimum);
      leftMotor->category->maximum = max(leftMotor->bufferMean, leftMotor->category->maximum);
    }
    if(leftMotor->colorHasChanged() && rightMotor->colorHasChanged()){
      stopMotors();
      break;
    }
    adjustCourse();
  }
}

void evaluateSensorData(){
  if(leftMotor->state){
    leftMotor->evalutateSensorData(analogRead(LEFTSENSOR));
  }
  if(rightMotor->state){
   rightMotor->evalutateSensorData(analogRead(RIGHTSENSOR));
  }
}

void updateCategories(){
  leftMotor->prevCategory = leftMotor->category;
  leftMotor->category = getCategory(leftMotor->bufferMean);
  rightMotor->prevCategory = rightMotor->category;
  rightMotor->category = getCategory(rightMotor->bufferMean); 
}

bool wentPastTarget() {
  return (
    leftMotor->colorHasChanged() &&
    rightMotor->colorHasChanged() &&
    *(leftMotor->category) == *grayCategory &&
    *(rightMotor->category) == *grayCategory &&
    *(leftMotor->prevCategory) == *whiteCategory &&
    *(rightMotor->prevCategory) == *whiteCategory
  );
}

Category* getCategory(int value){
  if(blackCategory->includes(value)) return blackCategory;
  if(grayCategory->includes(value)) return grayCategory;
  else return whiteCategory;
}

void adjustCourse(){
  if(leftMotor->colorHasChanged() && rightMotor->colorHasChanged()){
    drive();
  }
  else if(!leftMotor->colorHasChanged() && !rightMotor->colorHasChanged()) {
    drive();
  }
  else if(leftMotor->colorHasChanged()) {
    if(rightMotor->state == 0) {
      drive();
    }
    else {
      turnLeft();
    }
  }
  else if(rightMotor->colorHasChanged()) {
    if(leftMotor->state == 0) {
      drive();
    }
    else {
      turnRight();
    }
  }
  analogWrite(ENR, LOW);
  analogWrite(ENL, LOW);
  delay(120);
}

void turnAround() {
  driveDirection = -driveDirection;
  leftMotor->driveDirection = driveDirection;
  rightMotor->driveDirection = driveDirection;

  if(leftMotor->state && rightMotor->state){
    drive();
  }
  else if(leftMotor->state){
    leftMotor->start();
  }
  else if(rightMotor->state){
    rightMotor->start();
  }
}


void putFlag(){
  stopMotors();
  digitalWrite(BOARDLED, HIGH);
}

void goToCenter(){
  turnAround();
  driveToNextCategory();
}

void turnLeft(){
  leftMotor->stop();
  rightMotor->start();
}

void turnRight(){
  rightMotor->stop();
  leftMotor->start();
}

void drive(){
  rightMotor->state = driveDirection;
  leftMotor->state = driveDirection;
  if (driveDirection == 1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW); 
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);  
  }
  if (driveDirection == -1) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);  
  }
  analogWrite(ENR, 100);
  analogWrite(ENL, 120);
  delay(20);
  analogWrite(ENR, 80);
  analogWrite(ENL, 100);
  delay(20);
  analogWrite(ENR, 60);
  analogWrite(ENL, 80);
  delay(20);
  analogWrite(ENR, 40);
  analogWrite(ENL, 60);
  delay(20);
  analogWrite(ENR, 20);
  analogWrite(ENL, 40);
  delay(20);
}

void stopMotors() {
  leftMotor->stop();
  rightMotor->stop();
}

void blinkDebug(int times) {
  for(int i=0; i < times; i++) {
    digitalWrite(BOARDLED, HIGH);
    delay(200);
    digitalWrite(BOARDLED, LOW);
    delay(200);
  }
}

