#include "Position.hpp"

int M1 = 12; int ENA = 3;
int M2 = 13; int ENB = 11;

int pwmPin = ENA;
int dirPin = M1;

int sensorPin = A2;

Position position(sensorPin);

void setup() {
  Serial.begin(57600);

  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  pinMode(sensorPin, INPUT);

  delay(1000);
}

int incData = 0;


const int MIN_VALUE = 48;
const int MAX_VALUE = 971;

int maxi = 1777;

void loop() {
  // if (incData > 10) {
  //   return;
  // }

  incData++;

  // if (Serial.available()) {
  //   incData = Serial.read();
  //
  // }

  // Serial.println(incData);


  position.update();

  // Serial.print("Flips: ");
  // Serial.println(position.flips);

  double rh = 0.065659;
  double ts = -.0107 * position.updatedPosition + 4.9513;

  Serial.print("Handle: ");
  Serial.println(position.updatedPosition);
  // Serial.println(rh * (ts * 3.14159 / 180));

  return;

  /*if (sensorValue > 380) {
    digitalWrite(dirPin, HIGH);
    analogWrite(pwmPin, 100);


    delay(10);

    analogWrite(pwmPin, 0);
  }*/

  digitalWrite(dirPin, HIGH);
  analogWrite(pwmPin, 100);

  delay(50);

  analogWrite(pwmPin, 0);
  delay(1000);


  delay(1000);


  digitalWrite(dirPin, LOW);
  analogWrite(pwmPin, 100);

  delay(50);

  analogWrite(pwmPin, 0);

  delay(1000);



  delay(1000);
}
