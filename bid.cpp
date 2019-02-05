#include "Arduino.h"
#include "bid.h" 

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

Motor::Motor(const int in1, const int in2, const int en, int power, int driveDirection, const int bufferSize)
  : _in1(in1), _in2(in2), _en(en), _power(power), driveDirection(driveDirection), _bufferSize(bufferSize)
{
  this->state = this->driveDirection;
}

void Motor::evalutateSensorData(int sensorValue) {
  for(int i = this->_bufferSize -1; i > 0; i--){
    this->sensorBuffer[i] = this->sensorBuffer[i - 1];
  }
  this->sensorBuffer[0] = sensorValue;
  this->bufferMean = this->calculateBufferMean();
  this->trend = this->getChange();
}

int Motor::calculateBufferMean() {
  int min = 1024, max = 0, mean = 0;
  for(int i = 0; i < this->_bufferSize; i++){
    int value = this->sensorBuffer[i];
    if(value < min) min = value;
    if(value > max) max = value;
    mean += value;
  }
  return (int)((double) mean / this->_bufferSize);
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
  this->state = this->driveDirection;
  if (this->driveDirection == 1) {
    digitalWrite(this->_in1, HIGH);
    digitalWrite(this->_in2, LOW);  
  }
  if (this->driveDirection == -1) {
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
