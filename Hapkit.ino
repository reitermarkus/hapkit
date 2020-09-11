#include "position.hpp"

int M1 = 12; int ENA = 3;
int M2 = 13; int ENB = 11;

int pwmPin = ENA;
int dirPin = M1;

int sensorPin = A2;

/// These were found be reading the sensor repeatedly
// and calculating the min/max values.
const int MIN_VALUE = 48;
const int MAX_VALUE = 971;
const float POSITION_10_DEGREE_DIFFERENCE = 726.6;

float positionToDegrees(int position) {
  return position / POSITION_10_DEGREE_DIFFERENCE * 10.0;
}


Position position(sensorPin, MIN_VALUE, MAX_VALUE);

void setup() {
  Serial.begin(57600);

  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  pinMode(sensorPin, INPUT);

  delay(1000);

  position.init();
}

int i = 0;

int maxi = 1777;

void loop() {
  i++;

  position.update();

  double rh = 0.065659;
  double ts = -.0107 * position.updatedPosition + 4.9513;


  if (i % 10000 == 0) {
    Serial.print("Handle Position: ");
    Serial.println(position.updatedPosition);

    Serial.print("Handle Degrees: ");
    Serial.println(positionToDegrees(position.updatedPosition));
  }


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
