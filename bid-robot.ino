//we know this is shit code. we are also a little sorry.

const int IN1 = 10;
const int IN2 = 9;
const int IN3 = 8;
const int IN4 = 7;
const int ENL = 6;
const int ENR = 5;
const int UTBLACK = 20;
const int UTGRAY = 70;

const int LEFTSENSOR = A1;
const int RIGHTSENSOR = A0;
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
int bordersUntilCenter = 2;

class Category {
  public:
    Category(int, int, int);
    int min;
    int max;
    int mean;
    char color[1];
} blackCategory, grayCategory, whiteCategory;

Category::Category(int min = 1024, int max = 0, int mean = 0) {
  this->min = min;
  this->max = max;
  this->mean = mean;
}
class Motor {
public:
  Motor();
  int direction;
  int state;
  Category category;
}

Motor::Motor() {
  this->direction = 1;
  this->state = direction;
  this->category = grayCategory;
}
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
  
}

void loop() {
  if(!centerFound()){
    while(!centerFound()){
      alignWithSecant();
    }
    
    goToCenter();
    putFlag();
  }
}

void alignWithSecant(){
  getSensorData();
  adjustCourse();
}

void getSensorData(){
  if(leftMotorState){
    pushToBuffer(leftSensorBuffer, analogRead(LEFTSENSOR));
  }
  if(rightMotorState){
    pushToBuffer(rightSensorBuffer, analogRead(RIGHTSENSOR));
  }
}

void adjustCourse(){
  if(colorHasChanged(leftSensorBuffer) && colorHasChanged(rightSensorBuffer)){
    drive();
    logBorder(leftSensorBuffer);
  }
  else if(!colorHasChanged(leftSensorBuffer) && !colorHasChanged(rightSensorBuffer)) {
    drive();
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
  analogWrite(ENR, LOW);
  analogWrite(ENL, LOW);
  delay(200);
}

bool colorHasChanged(int valuesArray[5]){
  return getCategory() != currentCategory;
}

Category getCategory(int valuesArray[5]){
  /*
  int difference = valuesArray[0] - valuesArray[1];
  if(abs(difference) > DELTA){
    return true;
  }
  */
  int min = 1024, max = 0, mean = 0;
  for(int i = 0; i < 0; i++){
    value = valuesArray[i];
    if(value < min) min = value;
    if(value > max) max = value;
    mean += value;
  }
  mean = (int)((double) mean / 5.0)
  newData = getCategory(mean);

  if(mean < blackCategory.max) {
    return blackCategory;
  }
  else if(mean < grayCategory.max) {
    return grayCategory;
  }
  else {
    return whiteCategory;
  }
}

int getCategory(int value){
  if(value < ) return 1; //black
  else if(value < UTGRAY) return 2; //grey
  else return 3; //white
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
    bordersUntilCenter--;
  }
  adjustDirection();
  //Serial.println("crossed border");
}

void adjustDirection(){
  if(borderLog[0] == 1 && borderLog[1] == 1){
    reversals++;
    //Serial.println("U turn");
    turnAround();
  }
}

void pushToBuffer(int anArray[5], int value){
  for(int i = 4; i > 0; i--){
    anArray[i] = anArray[i - 1];
  }
  anArray[0] = value;
}

void turnAround() {
  forward = -1;
  if(leftMotorState) {
    leftMotorState = forward;
    startLeftMotor();
  }
  if(rightMotorState){
    rightMotorState = forward;
    startRightMotor();
  }
}


void putFlag(){
  stopLeftMotor();
  stopRightMotor();
  digitalWrite(BOARDLED, HIGH);
}

void goToCenter(){
  while(bordersUntilCenter){
    alignWithSecant();
  }
}

bool centerFound(){
  return reversals > 1;
}

void turnLeft(){
  rightMotorState = forward;
  startRightMotor();
  leftMotorState = 0;
  stopLeftMotor();
}

void turnRight(){
  rightMotorState = 0;
  stopRightMotor();
  leftMotorState = forward;
  startLeftMotor();
  Serial.println("turn right");
}

void drive(){
  rightMotorState = forward;
  leftMotorState = forward;
  if (forward == 1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW); 
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);  
  }
  if (forward == -1) {
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
  
  Serial.println("drive");
  
}

void startLeftMotor(){
  if (forward == 1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);  
  }
  if (forward == -1) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);  
  }
  analogWrite(ENL, 140);
  delay(20);
  analogWrite(ENL, 120);
  delay(20);
  analogWrite(ENL, 100);
  delay(20);
  analogWrite(ENL, 80);
  delay(20);
  analogWrite(ENL, 60);
  delay(20);
}

void stopLeftMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENL, 0);
}

void startRightMotor(){
  if (forward == 1) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);  
  }
  if (forward == -1) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);  
  }
  analogWrite(ENR, 120);
  delay(20);
  analogWrite(ENR, 100);
  delay(20);
  analogWrite(ENR, 80);
  delay(20);
  analogWrite(ENR, 60);
  delay(20);
  analogWrite(ENR, 40);
  delay(20);
}

void stopRightMotor() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENR, 0);
}


/* NOT USED

void turnNinetyDegrees() {
  startLeftMotor();
  stopRightMotor();
  delay(500);
  startLeftMotor();
  startRightMotor();
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
*/
