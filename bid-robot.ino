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

class Category {
  public:
    Category(char, int minimum = 1024, int maximum = 0, int mean = 0);
    int minimum;
    int maximum;
    int mean;
    char color;
    bool includes(int);
    bool operator==(const Category*);
    bool operator<(const Category*);
    bool operator>(const Category*);
};

Category::Category(char color, int minimum, int maximum, int mean) 
  : color(color), minimum(minimum), maximum(maximum), mean(mean) { }

bool Category::includes(int value) {
  return (value > this->minimum && value < this->maximum);
}

bool Category::operator==(const Category* other) {
  return this->color == other->color;
}

bool Category::operator<(const Category* other) {
  return this->mean < other->mean;
}

bool Category::operator>(const Category* other) {
  return this->mean > other->mean;
}


class Motor {
private:
  const int _in1;
  const int _in2;
  const int _en;
  int _power;
public:
  Motor(const int, const int, const int, int);
  int sensorBuffer[BUFFER_SIZE];
  int state;
  int trend;
  int bufferMean;
  Category* category;
  Category* prevCategory;
  void evalutateSensorData(int);
  int calculateBufferMean();
  int getChange();
  bool colorHasChanged();
  void start();
  void stop();
};

Motor::Motor(const int in1, const int in2, const int en, int power) 
  : _in1(in1), _in2(in2), _en(en), _power(power)
{
  this->state = driveDirection;
}

void Motor::evalutateSensorData(int sensorValue) {
  for(int i = BUFFER_SIZE -1; i > 0; i--){
    this->sensorBuffer[i] = this->sensorBuffer[i - 1];
  }
  this->sensorBuffer[0] = sensorValue;
  this->bufferMean = this->calculateBufferMean();
  this->trend = this->getChange();
}

int Motor::calculateBufferMean() {
  int min = 1024, max = 0, mean = 0;
  for(int i = 0; i < BUFFER_SIZE; i++){
    int value = this->sensorBuffer[i];
    if(value < min) min = value;
    if(value > max) max = value;
    mean += value;
  }
  return (int)((double) mean / BUFFER_SIZE);
}
int Motor::getChange(){
  // Can be either -1, 0 or 1 
  
  if(this->bufferMean < this->category->minimum) {
    return -1;
  }
  else if(this->bufferMean > this->category->maximum) {
    return 1;
  }
  else {
    return 0;
  }
}

bool Motor::colorHasChanged(){
  return this->trend;
}

void Motor::start(){
  this->state = driveDirection;
  if (driveDirection == 1) {
    digitalWrite(this->_in1, HIGH);
    digitalWrite(this->_in2, LOW);  
  }
  if (driveDirection == -1) {
    digitalWrite(this->_in1, LOW);
    digitalWrite(this->_in2, HIGH);  
  }
  analogWrite(this->_en, this->_power);
  delay(20);
  analogWrite(this->_en, this->_power - 20);
  delay(20);
  analogWrite(this->_en, this->_power  - 40);
  delay(20);
  analogWrite(this->_en, this->_power - 60);
  delay(20);
  analogWrite(this->_en, this->_power - 80);
  delay(20);
}

void Motor::stop() {
  this->state = 0;
  digitalWrite(this->_in1, LOW);
  digitalWrite(this->_in2, LOW);
  analogWrite(this->_en, 0);
}

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
  leftMotor = new Motor(IN1, IN2, ENL, 140);
  rightMotor = new Motor(IN3, IN4, ENR, 120);
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
