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
const int COLOR_MEASURES = 50;
const int BUFFER_SIZE = 4;
bool CENTER_FOUND = false;
int driveDirection = 1;

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
  leftMotor = new Motor(IN1, IN2, ENL, 140, driveDirection, BUFFER_SIZE);
  rightMotor = new Motor(IN3, IN4, ENR, 120, driveDirection, BUFFER_SIZE);
}

void loop() {
  if(!CENTER_FOUND){
    getCategories();
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
  driveToNextCategory();
  Category * secondCategory = getCategoryFromSensor();
  driveToNextCategory();
  Category * thirdCategory = getCategoryFromSensor();
  findLightPattern(firstCategory, secondCategory, thirdCategory);
}

void findLightPattern(Category* firstCategory, Category* secondCategory,Category* thirdCategory) {
  if(firstCategory < secondCategory) {
    if(secondCategory < thirdCategory) {
      // black - gray - white
      setCategories(firstCategory, secondCategory, thirdCategory);
    }
    else if(thirdCategory > firstCategory) {
      // black - white - gray
      turnAround();
      setCategories(firstCategory, thirdCategory, secondCategory);
    }
    else {
      // gray - white - black
      setCategories(thirdCategory, firstCategory, secondCategory);
    }
  }
  else {
    if(thirdCategory < secondCategory) {
      // white - gray - black
      turnAround();
      setCategories(thirdCategory, secondCategory, firstCategory);
    }
    else if(thirdCategory > firstCategory) {
      // gray - black - white
      turnAround();
      setCategories(secondCategory, firstCategory, thirdCategory);
    }
    else if(firstCategory > thirdCategory) {
      // white - black - gray
      setCategories(secondCategory, thirdCategory, firstCategory);
    }
    else {
      // SPECIAL CASE: gray - white - gray
      CENTER_FOUND = true;
    }
  }
  leftMotor->category = thirdCategory;
  rightMotor->category = thirdCategory;
}

void setCategories(Category* black, Category* gray, Category* white) {
  blackCategory = black;
  blackCategory->color = 'b';
  grayCategory = gray;
  grayCategory->color = 'g';
  whiteCategory = white;
  whiteCategory->color = 'w';
}

Category* getCategoryFromSensor() {
  int min = 1024, max = 0, mean = 0;
  for(int i = 0; i < COLOR_MEASURES; i++){
    int value = analogRead(A0);
    if(value < min) min = value;
    if(value > max) max = value;
    mean += value;
  }
  mean = (int)((double) mean / COLOR_MEASURES);
  return new Category('#', min, max, mean);
}

void driveToNextCategory() {
  while(true) {
    evaluateSensorData();
    if(leftMotor->colorHasChanged() && rightMotor->colorHasChanged()){
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
    leftMotor->category == grayCategory &&
    rightMotor->category == grayCategory &&
    leftMotor->prevCategory == whiteCategory &&
    rightMotor->prevCategory == whiteCategory
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
  delay(200);
}

void turnAround() {
  driveDirection = -1;
  leftMotor->driveDirection = driveDirection;
  rightMotor->driveDirection = driveDirection;
  
  if(leftMotor->state) {
    leftMotor->start();
  }
  if(rightMotor->state){
    rightMotor->start();
  }
}


void putFlag(){
  leftMotor->stop();
  rightMotor->stop();
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
